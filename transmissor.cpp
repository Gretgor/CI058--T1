#include "transmissor.h"


void Manda_String (Conexao c, uchar Tipo, const char *str) {
  _Mensagem _msg;
  Mensagem msg = &_msg;
  uint tamanho, pos_atual = 0;
  bool envia = true;

  tamanho = strlen(str);
  msg->Tipo = Tipo;

  if (tamanho == 0) {
    msg->dados = (uchar*)"";
    msg->Sequencia = c->seq_envio;
    msg->Tamanho = 1;
    Manda_Msg(c, msg);
  }
  else {
    while (envia) {
      msg->Sequencia = c->seq_envio;
      if (pos_atual + TAM_DADOS >= tamanho) {                       // Caso seja o ultimo pacote da msg.
        msg->dados = (uchar*) malloc (sizeof(uchar) * (tamanho - pos_atual));
        msg->Tamanho = tamanho-pos_atual;
        strncpy((char*)msg->dados,str+pos_atual,tamanho - pos_atual);  // Copia para 'msg->dados' o restante dos dados.
        envia = false;
      }
      else {                                                  // Caso a msg tenha mais pacotes para enviar ainda.
        msg->dados = (uchar*) malloc (sizeof(uchar) * TAM_DADOS);
        msg->Tamanho = TAM_DADOS;
        strncpy((char*)msg->dados,str+pos_atual,TAM_DADOS);         // Copia para 'msg->dados' a parte certa dos dados.
      }
      Manda_Msg(c, msg);
      Limpa_Dados(msg);
      pos_atual += TAM_DADOS;
    }
  }
}


int Manda_Msg (Conexao c, Mensagem msg)  {
  uchar *mensagem;
  uint tamanho;

  Msg2String(msg, mensagem, tamanho);     // Transforma a 'msg' em uma string 'mensagem'.

  _Mensagem _rec;
  Mensagem rec = &_rec;
  
  uchar buffer[TAM_MSG];
  struct pollfd pfd;
  int pollRet;
  int num_tent;
  int aceitou;
  int result;
	
  pfd.fd = c->socket;                      // Indica qual arquivo ele se refere.
  pfd.events = POLLIN;                     // POLLIN = tem dados para ler.
	
  aceitou = 0;
  do {
    num_tent = 0;
    do {
      if ((result = send(c->socket, mensagem, tamanho, 0)) == -1) {    // Manda a msg para o outro PC.
        perror("send");
        return 0;
      }
      pollRet = poll(&pfd, 1, TEMPO_TIMEOUT);           // Verifica o Timeout com a funcao poll().
      if (pollRet < 0) {
        perror("poll");
        return 0;
      } 
      num_tent++;
    } while (pollRet == 0 && num_tent < N_TENT);
    if (pollRet == 0) {
      fprintf(stderr, "Conexao perdida.\n");
      return 0;
    }
    if ((result = recv(c->socket, buffer, TAM_MSG, 0)) == -1) {        // Coloca em 'buffer' a msg recebida.
      perror("recv");
      return 0;
    }

    try {
      String2Msg((uchar*) buffer, rec);          // Transforma a string 'buffer' em uma msg.
    }
    catch (int err) {
      cout << err;
      return 0;
    }

    if (rec->Tipo == TIPO_1) {              // Nack.
	// Manda denovo. //
    } else if (rec->Tipo != TIPO_0) {       // Ack.
      fprintf(stderr, "Erro: Tipo invalido./n");
      return 0;
      } else {
          aceitou = 1;
        }
  } while (!aceitou);
  c->seq_envio = ProxSequencia(c->seq_envio);
  return 1;
}


int Recebe_Msg (Conexao c, Mensagem msg) {
  uchar buffer[TAM_MSG];
  ssize_t recebidos;
  int recebeu;
  int erro_seq = 0;

  recebeu = 0;
  do {
    do {
      recebidos = recv(c->socket, buffer, TAM_MSG, 0);        // Coloca em 'buffer' a msg recebida.
        if (recebidos < 0) {
          perror("recv");
          return 0;
        }
    }  while ((buffer[0] != MARCA));
	
    try {
      String2Msg((uchar*)buffer, msg);              // Transforma a string 'buff' em uma msg.
    }
    catch (int err) { 
      if (err == ERR_MARCA)
        cout << err;
      else if (err == ERR_SEQ)
        erro_seq = 1;
    }
	
    if (erro_seq) {
      if ((msg->Sequencia == AntSequencia(c->seq_envio))){
        Manda_Ack(c, AntSequencia(c->seq_envio));
        continue;
      } else {
        fprintf(stderr, "Erro: Sequencia invalida.\n");
        return 0;
      }
    }
    Manda_Ack(c, c->seq_envio);
    recebeu = 1;
		
  } while (!recebeu);
  c->seq_recebimento = ProxSequencia(c->seq_recebimento);
  return 1;
}


int Manda_Arquivo (Conexao c, FILE *arq) {
  uchar buffer[TAM_DADOS];
  int nbytes;
  _Mensagem msg;

  msg.Sequencia = ProxSequencia(c->seq_envio);
  msg.Tipo = TIPO_2;
  msg.dados = buffer;
  msg.Tamanho = TAM_DADOS;

  while((nbytes=fread(buffer,1,TAM_DADOS,arq)) == TAM_DADOS){      // Envia o arquivo por partes.
    Manda_Msg(c, &msg);
    msg.Sequencia = ProxSequencia(msg.Sequencia);
  }
  if (nbytes>0) {                                      // Ateh chegar no final dele
    msg.Tamanho = nbytes;                              // (Quando o tamanho restante dele)
    Manda_Msg(c,&msg);                                 // (eh menor do que TAM_DADOS.          )
    c->seq_envio = msg.Sequencia;
  }
  else {                                               // (Ou quando o tamanho dele)
    c->seq_envio = AntSequencia(msg.Sequencia);        // (for multiplo de TAM_DADOS.    )
  }
  return 1;
}



int Manda_Ack(Conexao c, uchar sequencia){
  uchar *mensagem;
  uint tamanho;

  _Mensagem msg = {17,sequencia,TIPO_0,NULL};         // Ack.
  msg.dados = (uchar*) malloc(sizeof(uchar)*17);
  strcpy((char*)msg.dados,"");

  Msg2String(&msg,mensagem,tamanho);                 // Transforma 'msg' em uma string 'mensagem'.
  if (send(c->socket, mensagem, tamanho, 0) == -1) {  // Manda a msg para o outro PC.
    perror("send");
    return 0;
  }
  return 1;
}


int Manda_Nack(Conexao c, uchar sequencia){
  uchar *mensagem;
  uint tamanho;

  _Mensagem msg = {17,sequencia,TIPO_1,NULL};         // Nack.
  msg.dados = (uchar*) malloc(sizeof(uchar)*17);
  strcpy((char*)msg.dados,"");

  Msg2String(&msg,mensagem,tamanho);                 // Transforma 'msg' em uma string 'mensagem'.
  if (send(c->socket, mensagem, tamanho, 0) != -1) {  // Manda a msg para o outro PC.
    perror("send");
    return 0;
  }
  return 1;
}



void Limpa_Dados (Mensagem msg) {
  if (msg->dados && (msg->Tipo == TIPO_C ||  msg->Tipo == TIPO_9 || msg->Tipo == TIPO_2 )) {
    free(msg->dados);
    msg->dados = NULL;
  }
}
