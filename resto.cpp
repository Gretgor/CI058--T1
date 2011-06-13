#include "resto.h"


Conexao Cria_Conexao() {
  Conexao c = (Conexao) malloc(sizeof(_Conexao));
  c->socket = Rawsocket("eth0");
  c->seq_envio = c->seq_recebimento = 0;

  return c;
}


void Destroi_Conexao(Conexao c) {
  free (c);
}



void Msg2String(Mensagem msg, uchar* &mensagem, uint &tamanho_msg) {   // Transforma uma msg em string.
  uint i;
  tamanho_msg = (sizeof(uchar)) * (4+msg->Tamanho);
  mensagem = (uchar*)malloc (tamanho_msg);
  uchar paridade = 0;

  mensagem[0] = MARCA;
  mensagem[1] = msg->Tamanho;
  mensagem[2] = ((msg->Sequencia & 0xF) << 4) | (msg->Tipo & 0xF);

  for (i=0;i<msg->Tamanho; i++) {
    mensagem[i+3] = msg->dados[i];
    paridade ^= msg->dados[i];                // (XOR).
  }    
  mensagem[msg->Tamanho+3] = paridade;        // Coloca a 'paridade' no final.
}


int String2Msg (uchar *mensagem, Mensagem msg) {           // Transforma uma string em msg.
  uint i;
  uchar paridade = 0;

  if (mensagem[0] != MARCA)
    throw(ERR_MARCA);

  msg->Tamanho = mensagem[1];
  msg->Sequencia = (mensagem[2] >> 4) & 0xF;
  msg->Tipo = mensagem[2] & 0xF;

  if (msg->Tamanho > 0) {
    msg->dados = (uchar*) malloc(msg->Tamanho);
    for(i=0;i<msg->Tamanho;i++) {
      (msg->dados)[i] = mensagem[i+3];
      paridade ^= mensagem[i+3];                     // (XOR).
    }
    if (mensagem[msg->Tamanho+3] != paridade) {      // Verifica paridade.
      throw(ERR_SEQ);
      return 0;
    }
  }
  else
    msg->dados = NULL;
  return 1;
}
