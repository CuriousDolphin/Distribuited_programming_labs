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
    ssize_t receved;
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
    printf("\nClient in avvio address: %s:%d : %s\n",addr,port,str);

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







    int exit_condition=0;
    while(exit_condition <= 4){

        printf("\n----------TRASMISSIONE #(%d)-------------\n",exit_condition+1);
        result=sendto(id_socket,str,strlen(str),0,(struct sockaddr*)&saddr,sizeof(saddr));
        if(result != -1){
            printf("\n-byte spediti %d \n",result);
        }else{
            printf("-error sending datagram\n");
        }
        
        
        
        struct timeval tval;
        fd_set cset;              //insieme di socket su cui agisce la SELECT
        FD_ZERO(&cset);          //azzero il set
        FD_SET(id_socket,&cset); //ASSOCIO IL SOCKET ALL'INSIEME
        int time=3;
        tval.tv_sec=time; tval.tv_usec=0; //imposto il tempo nell astruttura
        int res_sel;

        /* SELECT */
        res_sel=select(FD_SETSIZE, &cset,NULL,NULL,&tval);
        if(res_sel == -1){
            printf("select() failed");
            return -1;
        }

        if(res_sel>0){
            socklen_t s_len=sizeof(saddr);
            receved=recvfrom(id_socket,str_rec,32,0,(struct sockaddr*)&saddr,&s_len);
            if(receved != -1){
                printf("--byte ricevuti: %ld \n--datagram ricevuto: %s\n",receved,str_rec);
                exit_condition=5;
            }else{
                printf("--error in receiving response\n");
            }
            
        }else{
            printf("--timeout exceded %d seconds\n",time);
            exit_condition++;
        }

    }
    
    
    
         
}