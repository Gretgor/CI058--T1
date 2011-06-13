#include "main.h"

#define PROMPT "CMD$> "


void Imprime_Ajuda(string prog_name) {
  cout << "Modo: " << endl;
  cout << prog_name << " --cliente" << endl;
  cout << prog_name << " --servidor" << endl;
}


void Imprime_Comandos() {
  cout << "Comandos:\nRemotos-> ls, cd, get, put" << endl;
  cout << "Locais-> lls, lcd" << endl;
}



void Modo_Cliente() {
  string comando, cmd;
  Conexao c;
  try {
    c = Cria_Conexao ();                  // Inicializa conexao.
  }
  catch (string s_comando) { cout << s_comando; return ;}

  while (! cin.eof()) {
    cout << PROMPT;
    getline(cin,comando);
    istringstream s_comando(comando);
    s_comando >> cmd;
    if (cmd == "lls" || cmd == "lcd" || cmd == "lpwd" )
      Request_Command_Local(comando);
    else if (cmd == "ls" || cmd == "cd" || cmd == "pwd")
      Request_Command(comando, c);
    else if (cmd == "get")
      Request_Get(comando, c);
    else if (cmd == "put")
      Request_Put(comando, c);
    else if (cmd == "exit" || cmd == "quit")
      return;
    else
    {
      if (comando.size() > 0) {              // Ignora msg a seguir caso seja apenas um '/n'.
        cout << cmd << " nao eh um comando valido." << endl;;
        Imprime_Comandos();
      }
    }
  }
}


void Request_Command_Local(string s_comando) {
  string cmd;
  
  s_comando.erase(0,1);
  istringstream comando(s_comando);
  comando >> cmd;
  
  if (cmd == "cd" )
  {
    s_comando.erase(0,3);               // Separa o 'path' 'do comando'.
    if (chdir(s_comando.c_str()))
      cout << "Erro na mudanca local de diretorio. " << endl;
  }
  else
    system(s_comando.c_str());
}


void Request_Command(string s_comando, Conexao c) {
  _Mensagem _msg, *msg;
  msg = &_msg;

  try {
    Manda_String(c, TIPO_C, s_comando.c_str()); // Envia um Comando 's'.
    Manda_String(c, TIPO_F, "");                // Final de Transmissao.

    Recebe_Msg(c,msg);
    if (msg->Tipo == TIPO_E) {         // Erro.
      cout << "Erro: Comando invalido." << endl;
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);
    }
    while (msg->Tipo == TIPO_A) {      // Mostra na tela.
      Mostra_Tela(msg);
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);
    }
    Limpa_Dados(msg);
  }
  catch (string err) {
    cout << err;
  }
}


void Request_Get(string s_comando, Conexao c) {
  _Mensagem _msg, *msg;
  msg = &_msg;

  try {
    Manda_String(c, TIPO_C, s_comando.c_str()); // Envia um Comando 's'.
    Manda_String(c, TIPO_F, "");        // Final de Transmissao.

    Recebe_Msg(c,msg);
    if (msg->Tipo == TIPO_E) {        // Erro.
      cout << "Erro na execucao do comando: " << endl;
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);
		
      while (msg->Tipo == TIPO_A) {   // Mostra na tela.
        Mostra_Tela(msg);
        Limpa_Dados(msg);
        Recebe_Msg(c,msg);
      }
      Limpa_Dados(msg);
    }
    else {
      string descritor;
      string nome_arq;
      long tam_arq;
      char buffer[256];

      while (msg->Tipo == TIPO_9) {                         // Descritor do arquivo.
        strncpy(buffer, (char*) msg->dados, msg->Tamanho);  // Guarda em 'buffer'
        buffer[msg->Tamanho] = '\0';                        // os dados da msg.
        descritor = descritor + buffer;   // <Nome do arquivo>#<Tamanho>.
        Limpa_Dados(msg);
        Recebe_Msg(c,msg);
      }

      istringstream s_comando(descritor);
      getline(s_comando,nome_arq,'#');            // Copia o nome do arquivo para 'nome_arq'.
      s_comando >> tam_arq;
      
      
      FILE *arq;
      arq = fopen(nome_arq.c_str(), "w");
      if (arq == NULL) {
        Manda_String(c, TIPO_E, "");
        cout << "Erro: Nao foi possivel abrir o arquivo" << endl;;
      }
      else {
        Manda_String(c, TIPO_F, "");                            // Final de Transmissao.
        Recebe_Msg(c,msg);
        while (msg->Tipo == TIPO_2) {                           // Dados.
          fwrite(msg->dados,sizeof(char), msg->Tamanho, arq);   // Comeca a copiar o arquivo.
          Limpa_Dados(msg);
          Recebe_Msg(c,msg);
        }
        fclose(arq);
      }
    }
  }
  catch (string err) {
    cout << err;
  }
}


void Request_Put(string s_comando, Conexao c) {
  _Mensagem _msg, *msg;
  msg = &_msg;

  try {
    istringstream aux_string(s_comando);
    string cmd;
    string descritor, nome_arq;
    long tamanho_arq;
    char ch;

    aux_string >> cmd;
    aux_string.get(ch);

    char tam_arq [16];
    getline(aux_string, nome_arq);            // Copia o comando para 'nome_arq'.
		
    struct stat stat_buffer;                            // Pega informacoes do arquivo
    int result = stat(nome_arq.c_str(),&stat_buffer);   // e os guarda em 'buf'.

    if (result == -1 || !S_ISREG(stat_buffer.st_mode)) {       // Checka o tipo e as permissoes do arquivo.
      string msg_erro;
      if (result == -1)	
        cout << "Erro: '" + nome_arq + "' nao encontrado." << endl;
      else
        cout << "Erro: Voce nao tem permissao para alterar o arquivo '" + nome_arq + "'." << endl;
      return;
    }
    
    FILE *arq = fopen(nome_arq.c_str(),"r");

    if (arq == NULL) {
      cout << "Erro: Nao foi possivel abrir o arquivo '" << nome_arq << "'" << endl;
      return;
    }
    fseek(arq,0,SEEK_END);
    tamanho_arq = ftell(arq);
    fseek(arq,0,SEEK_SET);

    Manda_String(c, TIPO_C, cmd.c_str());     // Envia comando.
    Manda_String(c, TIPO_F, "");              // Final de Transmissao.

    snprintf(tam_arq, 15, "%ld",tamanho_arq);
    descritor = nome_arq + "#" + tam_arq;
    Manda_String(c,TIPO_9, descritor.c_str()); // Descritor do arquivo.
    Manda_String(c, TIPO_F, "");               // Final de Transmissao.

    Recebe_Msg(c,msg);

    if (msg->Tipo == TIPO_E) {                 // Erro.
      cout << "Erro na execucao do comando:" << endl;
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);

      while (msg->Tipo == TIPO_A) {            // Mostra na Tela.
        Mostra_Tela(msg);
        Limpa_Dados(msg);
        Recebe_Msg(c,msg);
      }
      Limpa_Dados(msg);
    }
    else {
      Manda_Arquivo(c, arq);
      Manda_String(c, TIPO_F,"");             // Final de Transmissao.
    }
  }
  catch (string err) {
    cout << err;
  }
}



void Modo_Servidor() {
  string cmd;
  Conexao c;
  _Mensagem _msg;
  Mensagem msg = &_msg;
  bool encerra = false;

  try {
    c = Cria_Conexao ();                  // Inicializa conexao.
  }
  catch (string s_comando) { cout << s_comando; return; }

  while (!encerra)  {
    char buffer[256];
    cmd = "";
    Recebe_Msg(c,msg);

    while (msg->Tipo == TIPO_C) {
      strncpy(buffer, (char*) msg->dados, msg->Tamanho); // Guarda em 'buffer'
      buffer[msg->Tamanho] = '\0';                       // os dados da msg. (no caso, o comando)
      cmd = cmd + buffer;
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);
    }
    Send_Command(cmd, c);
  }
}


void Send_Command(string str_comando, Conexao c) {
  string cmd, cmd_esconde;
  istringstream s_comando(str_comando);

  s_comando >> cmd;
  if (cmd == "cd") {
    string path;
    char * err;
    char ch;
    s_comando.get(ch);
    getline(s_comando,path);               // Copia o comando para 'path'.

    if (chdir(path.c_str()) != 0) {                 // Muda o diretorio para 'path'.
      err = strerror(errno);
      Manda_String(c, TIPO_E, "");                  // Erro.
      string msg_erro = err + (": " + path + "'\n");
      Manda_String(c, TIPO_A, msg_erro.c_str());    // Mostra na tela.
    }
    else {
      Manda_String(c,TIPO_A,"");
    }
  }
  else if (cmd == "put") {
   Send_Put(c);
   return;
  }
  else if (cmd == "get") {
    Send_Get(str_comando, c);
    return;
  }
  else {
    FILE *erro, *saida;
    char *dados = NULL;
    size_t n = 0;
    ssize_t tam;

    cmd_esconde = str_comando + " 2>&1 > /dev/null";               // Nao deixa o comando aparecer na tela do servidor.
    erro = popen(cmd_esconde.c_str(),"r");
    tam = getdelim(&dados, &n, EOF, erro);         // Guarda em 'dados' o conteudo de 'erro'.

    if (tam > 0) {
      Manda_String(c, TIPO_E, "");       // Erro.
      Manda_String(c, TIPO_A, dados);    // Imprime o erro na tela.

      if (dados)
        free (dados);
    }
    else {
      if(dados)
        free (dados);
      dados = NULL;
      n = 0;
      saida = popen(str_comando.c_str(),"r");                 // Executa o comando 'str' e guarda em 'saida'.
      tam = getdelim(&dados, &n, EOF, saida);         // Guarda em 'dados' o conteudo de 'saida'.
      if (tam > 0) 
        Manda_String(c, TIPO_A, dados);               // Mostra na tela do cliente o resultado do comando.
      else
        Manda_String(c, TIPO_A,"");
			
      if (dados)
        free (dados);
      pclose(saida);
    }
  pclose(erro);
  }
  Manda_String(c,TIPO_F,"");                          // Final de Transmissao.
}


void Send_Get(string s_comando, Conexao c) {
  _Mensagem _msg, *msg;
  msg = &_msg;

  try {
    istringstream aux_string(s_comando);
    string cmd;
    string descritor, nome_arq;
    long tamanho_arq;
    char ch;

    aux_string >> cmd;
    aux_string.get(ch);

    char tam_arq [16];
    getline(aux_string, nome_arq);                  // Copia o comando para 'nome_arq'.
                                                   // (Para saber o nome do arquivo pelos parametros)
	
    struct stat buf;                               // Pega as informacoes do arquivo
    int result = stat(nome_arq.c_str(),&buf);      // e os guarda em 'buf'.

    if (result == -1 || !S_ISREG(buf.st_mode)) {   // Checka o tipo e as permissoes do arquivo.
      string msg_erro;
      if (result == -1)	
        msg_erro = "Erro: '" + nome_arq + "' nao encontrado.\n";
      else
        msg_erro = "Erro: Voce nao tem permissao para alterar o arquivo '" + nome_arq + "' .\n";
      Manda_String(c, TIPO_E, "");                        // Erro.
      Manda_String(c, TIPO_A, msg_erro.c_str());          // Mostra na tela do cliente o erro apropriado.
      Manda_String(c, TIPO_F, "");                        // Final de Transmissao.
      return;
    }
    FILE *arq = fopen(nome_arq.c_str(),"r");
    if (arq == NULL) {
      string msg_erro = "Erro: nao foi possivel abrir o arquivo '" + nome_arq + "'.\n";
      Manda_String(c, TIPO_E, "");                        // Erro.
      Manda_String(c, TIPO_A, msg_erro.c_str());          // Mostra na tela do cliente o erro apropriado.
      Manda_String(c, TIPO_F, "");                        // Final de Transmissao.
      return;
    }
    fseek(arq,0,SEEK_END);
    tamanho_arq = ftell(arq);
    fseek(arq,0,SEEK_SET);

    snprintf(tam_arq, 15, "%ld",tamanho_arq);
    descritor = nome_arq + "#" + tam_arq;                 // <Nome_do_arquivo>#<Tamanho>
    Manda_String(c,TIPO_9, descritor.c_str());            // Descritor do Arquivo.
    Manda_String(c, TIPO_F, "");                          // Final de Transmissao.

    Recebe_Msg(c,msg);

    if (msg->Tipo == TIPO_E) {                 // Um Erro aqui nao interessa para
      Limpa_Dados(msg);                        // o servidor.
      return;
    }
    else {
      Manda_Arquivo(c, arq);
      Manda_String(c, TIPO_F,"");              // Final de Transmissao.
    }
  }
  catch (string err) {
    cout << err;
  }
}


void Send_Put(Conexao c) {
  _Mensagem _msg;
  Mensagem msg = &_msg;
	
  string descritor;
  string nome_arq;
  long tam_arq;
  char buffer[256];

  Recebe_Msg(c,msg);

  while (msg->Tipo == TIPO_9) {                  	// Descritor de Arquivo.
    strncpy(buffer, (char*) msg->dados, msg->Tamanho);  // Guarda em 'buffer'
    buffer[msg->Tamanho] = '\0';                        // os dados da msg.
    descritor = descritor + buffer;                     // <Nome_do_Arquivo>#<Tamanho>.
    Limpa_Dados(msg);
    Recebe_Msg(c,msg);
  }

  istringstream s_comando(descritor);
  getline(s_comando, nome_arq, '#');                    // Copia o nome do arquivo para 'nome_arq'. 
  s_comando >> tam_arq;


  FILE *arq;
  arq = fopen(nome_arq.c_str(), "w");
  if (arq == NULL) {
    Manda_String(c, TIPO_E, "");          // Erro.
    Manda_String(c, TIPO_A, "Erro: Nao foi possivel abrir o arquivo.\n");  // Mostra na tela do cliente.
    Manda_String(c, TIPO_F, "");          // Final de Transmissao.
  }
  else {
    Manda_String(c, TIPO_F, "");          // Final de Transmissao.
    Recebe_Msg(c,msg);

    while (msg->Tipo == TIPO_2) {                          // Dados.
      fwrite(msg->dados,sizeof(char), msg->Tamanho, arq);  // Comeca a copiar os arquivos.
      Limpa_Dados(msg);
      Recebe_Msg(c,msg);
    }
    fclose(arq);
    }
}



void Mostra_Tela(Mensagem msg) {
  int i;
  if (msg->Tipo != TIPO_A)
    return;
  for (i=0;i<msg->Tamanho;i++) {
    if (msg->dados[i] != 0)
      putchar(msg->dados[i]);
  }
}



int main(int argc, char **argv) {
  if (argc != 2) {
    Imprime_Ajuda(argv[0]);
    return 1;
  }

  if (strcmp(argv[1],"--cliente") == 0)
    Modo_Cliente();
  else if (strcmp(argv[1],"--servidor") == 0)
    Modo_Servidor();
    else {
      Imprime_Ajuda(argv[0]);
      return 2;
    } 
  return 0;
}

