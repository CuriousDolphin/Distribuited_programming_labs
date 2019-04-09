/*
 *  warning: this is just a sample server to permit testing the clients; it is not as optimized or robust as it should be
 */
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

#include "errlib.h"
#include "sockwrap.h"

//#define SRVPORT 1500

#define LISTENQ 15
#define MAXBUFL 255

#define MSG_ERR "wrong operands\r\n"
#define MSG_OVF "overflow\r\n"

#define MAX_UINT16T 0xffff
//#define STATE_OK 0x00
//#define STATE_V  0x01

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;





int main (int argc, char *argv[]) {

	
	int id_socket;
	ssize_t n_rec;
	short port;
	struct sockaddr_in servaddr,clienaddr;
	char* buf;
	buf=malloc(32*sizeof(char));
	socklen_t clienaddrlen = sizeof(clienaddr);

	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=2){
		printf("ERROR insert port");
		return -1;
	}
		
	port=atoi(argv[1]);

	/* create socket */
	id_socket = Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	/* specify address to bind to */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(id_socket, (SA*) &servaddr, sizeof(servaddr));

	trace ( err_msg("(%s) socket created",prog_name) );
	trace ( err_msg("(%s) listening for UDP on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );

	while(1){
		n_rec=recvfrom(id_socket,buf,32,0,(struct sockaddr *) &clienaddr, &clienaddrlen);
		
		if(n_rec >0){
			buf[n_rec]='\0';
			printf("\n--receved %ld bytes : %s\n",n_rec,buf);
			fflush(stdout);
			Sendto(id_socket,buf,strlen(buf),0,(struct sockaddr*) &clienaddr ,clienaddrlen);
		}
		

	}
	
	
	return 0;
}

