#include "rawsocket.h"


int Rawsocket(const char *device) {           // Cria um RawSocket
	int sock = socket(PF_PACKET,SOCK_RAW,0);  // Cria um socket do tipo RawSocket.
	if(sock==-1) {
		throw(strerror(errno));
	}
        
        // Descobre o Id da interface ("eth#").
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));       // Inicializa com '0'.

	memcpy(ifr.ifr_name, device, sizeof(device)); // Copia a parte da memoria de 'device' em 'ifr.ifr_name'.
	if(ioctl(sock, SIOCGIFINDEX, &ifr)==-1) {
		throw(strerror(errno));
	}
	int deviceid = ifr.ifr_ifindex;

	
  struct sockaddr_ll sll;               // "Binda", no socket, o endereco guardado em 'sll'.

  memset(&sll, 0, sizeof(sll));         // Inicializa com '0'.

  sll.sll_family = AF_PACKET;           // Sempre AF_PACKET.
  sll.sll_ifindex = deviceid;           // Id da interface. ("eth#")
  sll.sll_protocol = htons(ETH_P_ALL);  // Protocolo da Camada Fisica.

  if(bind(sock, (struct sockaddr *) &sll, sizeof(sll))==-1) {
    throw(strerror(errno));
  }

  // Coloca a interface em modo Promiscuo.
    struct packet_mreq mr;

    memset(&mr, 0, sizeof(mr));         // Inicializa com '0'.

    mr.mr_ifindex = deviceid;           // Id da interface que sera mudada. ("eth#")
    mr.mr_type = PACKET_MR_PROMISC;     // Modo Promiscuo.

    if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {  // Coloca o socket 'sock' em modo promiscuo.
      throw(strerror(errno));
    }
  return sock;
}
