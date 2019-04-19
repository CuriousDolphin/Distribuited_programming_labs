#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <rpc/xdr.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../errlib.h"
#include "../sockwrap.h"
#define LISTENQ 15
#define MAXBUFL 50


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
	char *request;
	char *command_buf;
	char *file_buf;
	//char file_name[20];
	
	//file_buf=malloc(4096*sizeof(char));
	command_buf=malloc(10*sizeof(char));
	request=malloc(MAXBUFL*sizeof(char));
	struct sockaddr_in  cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	FILE* F=NULL;
	//char* buf;
	//buf=malloc(5*sizeof(char));
	prog_name = argv[0];
	if ( argc<3 ){
		err_quit ("wrong params usage: %s need <address> <port>\n ", prog_name);
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
	int i;
	
	for(i=3;i<argc;i++){
		strcpy(request,"");
		strcat(request,"GET ");
		strcat(request,argv[i]);
		strcat(request,"\r\n");
		//printf("\t sending: %s\n",request);
		Send(id_socket,request,strlen(request),0);
		 struct timeval tval;
        fd_set cset;              //insieme di socket su cui agisce la SELECT
        FD_ZERO(&cset);          //azzero il set
        FD_SET(id_socket,&cset); //ASSOCIO IL SOCKET ALL'INSIEME
        int time=10;
        tval.tv_sec=time; tval.tv_usec=0; //imposto il tempo nell astruttura
        int res_sel;
        /* SELECT */
        res_sel=select(FD_SETSIZE, &cset,NULL,NULL,&tval);
        if(res_sel == -1){
            printf("select() failed");
            return -1;
        }
        if(res_sel>0){
			//strcpy(command_buf,"");
			uint32_t len;
			uint32_t timest;
			uint32_t timestamp;
			uint32_t file_len;
            Recv(id_socket,command_buf,5,0); /* COMMAND */
            if(strcmp(command_buf,"+OK\r\n")==0 ){
				//len=0;
				printf("\t--Received Response OK");
				Recv(id_socket,&len,4,0); /* LUNGHEZZA */
				file_len=ntohl(len);
				ssize_t file_size=file_len;
				printf("\t--Received file len: '%u' byte",file_len);
				fflush(stdout);
				file_buf=malloc(file_size*sizeof(char)); /* ALLOCAZIONE DINAMICA, FREE PIU GIU */
				char *start_memory=file_buf;
				 /* RICEZIONE FILE */
				ssize_t nread; size_t nleft;
				 for(nleft=file_len; nleft >0 ;){
					nread=Recv(id_socket,file_buf,nleft,0);
					if(nread > 0){
						nleft -=nread;
						file_buf += nread;
					}
				} 	  	
				file_buf=start_memory;								
				F=fopen(argv[i],"w");
				if(F==NULL){
					printf("errore salvataggio file");
					fflush(stdout);
					return -1;
				}  

				fwrite(file_buf,1,file_size,F);
				fclose(F); 
				
				struct stat st;
				stat(argv[i],&st) ;
				Recv(id_socket,&timest,4,0);
				timestamp=htonl(timest);	
				printf("\n\t---'%s' downloaded %ld byte",argv[i],st.st_size);
				printf("\n\t--Received  timestamp: %u  \n\n",timestamp);	
				free(file_buf); 
			}else{
				printf("\t--Server error\n");
				fflush(stdout);
				close(id_socket);
				break;
			}	
            
        }else{
            printf("---timeout exceded %d seconds\n",time);
            close(id_socket);
        }
	}
	close(id_socket);
	return 0;
}
