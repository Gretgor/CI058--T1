#ifndef RESTO_H
#define RESTO_H

#include "rawsocket.h"

Conexao Cria_Conexao();
void Destroi_Conexao(Conexao c);

void Msg2String(Mensagem msg, uchar* &mensagem, uint &tamanho_msg);
int String2Msg (uchar *mensagem, Mensagem msg);

#endif
