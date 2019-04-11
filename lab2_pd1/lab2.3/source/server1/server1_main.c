/*
 * TEMPLATE 
 */
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
#define MAXBUFL 50
#define MAXBUFF 20

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
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	prog_name = argv[0];
	char* buf;
	char* file_buf;
	char* file_name;
	buf=malloc(MAXBUFL*sizeof(char));
	file_buf=malloc(2000*sizeof(char));
	file_name=malloc(MAXBUFF*sizeof(char));
	if (argc!=2 || argv[1]<0){
		err_quit ("usage: %s need <port>\n ", prog_name);
	}
	
	port=atoi(argv[1]);
	id_socket = Socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port); 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(id_socket, (SA*) &servaddr, sizeof(servaddr));

	trace ( err_msg("(%s) socket created",prog_name) );
	trace ( err_msg("(%s) listening on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)) );
	Listen(id_socket, LISTENQ);
	int connection;
	while(1){
		trace( err_msg ("(%s) waiting for connections ...", prog_name) );
		connection=Accept(id_socket, (SA*) &cliaddr, &cliaddrlen);
		if(connection>0){
			trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );
		}
		char get_buf[4];	
		int n_read =Recv(connection,buf,MAXBUFL,0);
		int n_arg;
		printf("\n--ricevuti: (%d) byte \n",n_read);
		n_arg=sscanf(buf,"%s %s",get_buf,file_name) ;
		if(n_arg == 2){
			printf("\n--comando:%s\n--filename:%s\n",get_buf,file_name);
			FILE* F;
			F=fopen(file_name,"r");  	/*apertura FILE */

			if(F==NULL){
				printf("\nERRORE apertura FILE\n");
				
			}else{ 						/* LETTURA FILE */
				int file_len=0;
				while(fscanf(F,"%c",&file_buf[file_len]) != EOF){
					
					file_len++;
					
				}

			printf("\n-----%s:(%d)\n",file_buf,file_len);
			}
			
			
		}else{
			printf("\n--error number argument (sscanf)");
		}
		
	
	}
	return 0;
}

