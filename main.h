#ifndef MAIN_H
#define MAIN_H

#include <sstream>
#include <sys/stat.h>

#include "resto.h"
#include "transmissor.h"
#include "rawsocket.h"


void Imprime_Ajuda(string prog_name);
void Imprime_Comandos();

void Modo_Cliente();
void Request_Command_Local(string cmd);
void Request_Command(string cmd, Conexao c);
void Request_Get(string cmd, Conexao c);
void Request_Put(string cmd, Conexao c);

void Modo_Servidor();
void Send_Command(string cmd, Conexao c);
void Send_Get(string cmd, Conexao c);
void Send_Put(Conexao c);

void Mostra_Tela(Mensagem msg);

#endif
