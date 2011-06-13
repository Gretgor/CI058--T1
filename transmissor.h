#ifndef TRANSMISSOR_H
#define TRANSMISSOR_H

#include <memory.h>
#include <sys/poll.h>
#include <arpa/inet.h>

#include "rawsocket.h"
#include "resto.h"


void Manda_String (Conexao c, uchar Tipo, const char *str);

int Manda_Msg (Conexao c, Mensagem msg);
int Recebe_Msg (Conexao c, Mensagem msg);
int Manda_Arquivo (Conexao c, FILE* arq);

int Manda_Ack(Conexao c, uchar sequencia);
int Manda_Nack(Conexao c, uchar sequencia);

void Limpa_Dados (Mensagem msg);

#endif
