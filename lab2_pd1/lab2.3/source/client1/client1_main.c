#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../errlib.h"
#include "../sockwrap.h"
#define LISTENQ 15
#define MAXBUFL 255


#define MAX_UINT16T 0xffff

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif
char *prog_name;

int main (int argc, char *argv[])
{
	int id_socket;
	int port;
	int res_addr;
	char *addr;
	struct sockaddr_in  cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	
	prog_name = argv[0];
	if (argc!=3 ){
		err_quit ("usage: %s need <address> <port>\n ", prog_name);
	}
	
	port=atoi(argv[2]);
    addr=argv[1];
	id_socket = Socket(AF_INET, SOCK_STREAM, 0);
	/* CONFIGURAZIONE SOCKET */
    cliaddr.sin_family = AF_INET; 
    cliaddr.sin_port = htons(port); /* CONVERSIONE NETWORK ORDER SHORT*/
	res_addr = inet_aton(addr,&cliaddr.sin_addr); /* inserimento indirizzo da stringa dotted */
    if(res_addr==0){
        printf("indirizzo non valido inet_aton() failed");
    }
	Connect(id_socket,(struct sockaddr*)&cliaddr,cliaddrlen);
	
	return 0;
}
