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

    
    
    id_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("Socket creato -> %d \n",id_socket);
    
    /* CONFIGURAZIONE SOCKET */
    saddr.sin_family = AF_INET; 
    saddr.sin_port = htons(port); /* CONVERSIONE NETWORK ORDER SHORT*/
    //saddr.sin_addr.s_addr = htonl(addr);
    res_addr = inet_aton(addr,&saddr.sin_addr); /* inserimento indirizzo da stringa dotted */
    if(res_addr==0){
        printf("indirizzo non valido inet_aton() failed");
    }



    /* APERTURA CONNESSIONE */
    result = connect(id_socket,(struct sockaddr*)&saddr,sizeof(saddr));
    if (result == -1)
    {
        err_quit("connect() failed");
    }else{
        printf("\n-Socket connection estabilited\n");
    }
    
    char* str1;
    char* str2;
    str1=malloc(sizeof(char)*30);
    str2=malloc(sizeof(char)*30);

        
    scanf("%s %s",str1,str2);  
    strcat(str1," ");
    strcat(str1,str2);
    strcat(str1,"\r\n");
    printf("\n--byte to send: %ld",strlen(str1));                 
    int byte_sent=send(id_socket,str1,strlen(str1),0);      /* INVIO STRINGA */
    printf("\n--sended byte: %d \n",byte_sent);    
        
    int byte_receved = 0;
    char c = ' ';
    char* buffer = malloc(50*sizeof(char));
    while(c!='\n')
    {
        if(read(id_socket,&c,1) != 1) return -1;
        buffer[byte_receved++] = c;
    }
    if(buffer[0] < '1' || buffer[0] > '9')
        printf("overflow\n");
    else
    {
        printf("res: %s\n",buffer);
    }
    
}