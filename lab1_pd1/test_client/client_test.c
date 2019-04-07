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
    int s=0; /* SOCKET*/
    int result;
    int res_addr;

    /* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3 || strlen(argv[1]) > 16 || strlen(argv[1]) < 7)
    {
        err_quit ("errore parametri in ingresso,inserire indirizzo server e porta");
    }
		
	port=atoi(argv[2]);
    addr=argv[1];

    printf("\nClient in avvio address: %s:%d \n",addr,port);

    
    
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("Socket creato -> %d \n",s);
    
    /* CONFIGURAZIONE SOCKET */
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(port); /* CONVERSIONE NETWORK ORDER SHORT*/
    //saddr.sin_addr.s_addr = htonl(addr);
    res_addr = inet_aton(addr,&saddr.sin_addr); /* inserimento indirizzo da stringa dotted */
    if(res_addr==0){
        printf("indirizzo non valido inet_aton() failed");
    }



    /* APERTURA CONNESSIONE */
    result = connect(s,(struct sockaddr*)&saddr,sizeof(saddr));
    if (result == -1)
    {
        err_quit("connect() failed");
    }else{
        printf("\nSocket correttamente avviato,chiusura.\n");
    }
        
}