#ifndef RAWSOCKET_H
#define RAWSOCKET_H

#include <iostream>
#include <cstdio>
#include <cstdlib>

#include <iterator>

#include <cstring>
#include <cerrno>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>  // Cabecalho Ethernet.
#include <net/if.h>        // Estrutura 'ifr'.
#include <netinet/in.h>    // Definicao de protocolos.


using namespace std;

#define MARCA 0x7e                              //  0111 1110/127/~ em Hex.
#define ProxSequencia(seq) (((seq) + 1) % 16 )  // Realiza a sequencicao de
#define AntSequencia(seq) (((seq) + 15) % 16 )  // 4 bits (16 possibilidades).

#define TIPO_0 0 // Ack.
#define TIPO_1 1 // Nack.
#define TIPO_2 2 // Dados.
#define TIPO_9 3 // Descritor de Arquivo.
#define TIPO_A 4 // Mostra na Tela.
#define TIPO_C 5 // Comandos.
#define TIPO_E 6 // Erro.
#define TIPO_F 7 // Final de Transmissao.


#define TEMPO_TIMEOUT 1000              // (em ms).
#define N_TENT 5                        // Numero maximo de tentativas.

#define ERR_SEQ   0x1
#define ERR_MARCA 0x2

#define TAM_DADOS 251                   // (em bytes).
#define TAM_MSG   255

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct  {
  uchar Tamanho;
  uchar Sequencia;
  uchar Tipo;
  uchar *dados;
} _Mensagem, *Mensagem;

typedef struct {
  int socket;
  uchar seq_envio;
  uchar seq_recebimento;
} _Conexao, *Conexao;



int Rawsocket(const char *device);

#endif
