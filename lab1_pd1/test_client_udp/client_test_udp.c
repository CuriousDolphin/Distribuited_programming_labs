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
char *prog_name;

int main (int argc, char *argv[]) {
    int port;
    char *addr;
    struct sockaddr_in saddr;
    int id_socket; /* SOCKET*/
    int result;
    int receved;
    int res_addr;
    char* str;
    char* str_rec;
    str=malloc(32*sizeof(char));
    str_rec=malloc(32*sizeof(char));
    /* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=4 || strlen(argv[1]) > 16 || strlen(argv[1]) < 7)
    {
        err_quit ("errore parametri in ingresso,inserire indirizzo server, porta e datagram da inviare");
    }
		
	port=atoi(argv[2]);
    addr=argv[1];
    str=argv[3];
    printf("\nClient in avvio address: %s:%d -> DGRAM %s\n",addr,port,str);

    
    
    id_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    printf("Socket creato -> %d \n",id_socket);
    
    /* CONFIGURAZIONE SOCKET */
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(port); /* CONVERSIONE NETWORK ORDER SHORT*/
    //saddr.sin_addr.s_addr = htonl(addr);
    res_addr = inet_aton(addr,&saddr.sin_addr); /* inserimento indirizzo da stringa dotted */
    if(res_addr==0){
        printf("indirizzo non valido inet_aton() failed");
    }



    
    result=sendto(id_socket,str,strlen(str),0,(struct sockaddr*)&saddr,sizeof(saddr));
    printf("--spediti %d caratteri\n",result);
    
    socklen_t s_len=sizeof(saddr);
    
    receved=recvfrom(id_socket,str_rec,strlen(str_rec),0,(struct sockaddr*)&saddr,&s_len);
    printf("---byte ricevuti: %d \n---datagram: %s\n",receved,str_rec);   
    

    
}