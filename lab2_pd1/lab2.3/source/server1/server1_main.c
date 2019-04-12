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
#include <sys/stat.h>
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
#define MAXRES 4096 /* 4kb */
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
	char* response;
	char size[4];
	char timestamp[4];
	FILE *F;
	buf=malloc(MAXBUFL*sizeof(char));
	file_buf=malloc(2000*sizeof(char));
	file_name=malloc(MAXBUFF*sizeof(char));
	response=malloc(MAXRES * sizeof(char));
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
		connection=accept(id_socket, (SA*) &cliaddr, &cliaddrlen);
		if(connection>0){
			trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );
		}else{
			printf("NO CONNECTION");
		}
		char get_buf[4];	
		int n_read =Recv(connection,buf,MAXBUFL,0);
		int n_arg=0;
		
		printf("\n--ricevuti: (%d) byte \n",n_read);
		n_arg=sscanf(buf,"%s %s",get_buf,file_name) ;
		if(n_arg == 2 /*&& strcmp(get_buf,"GET")*/){
			printf("\n--comando:%s\n--filename:%s\n",get_buf,file_name);
			F=fopen(file_name,"r");  	/*apertura FILE */

			if(F==NULL){
				printf("\nERRORE apertura FILE\n");
				
			}else{ 						/* LETTURA FILE */
				uint32_t file_len=0;
				int cont=0;
				while(fscanf(F,"%c",&file_buf[file_len]) != EOF){
					
					file_len++;
					cont++;
				}
				strcpy(response,"");

				file_buf[file_len]='\0';
			   	//printf("\n-----%s:(%d)\n",file_buf,file_len);
				
				
				struct stat st;
				stat(file_name,&st);
				uint32_t len=htonl(file_len);
				uint32_t tim=htonl(st.st_mtime);

				sprintf(size,"%u",ntohl(file_len));
				sprintf(timestamp,"%u",ntohl(st.st_mtime));
				strcat(response,"+OK\r\n");
				//strcat(response,size);
				//strcat(response,file_buf);
				//strcat(response,timestamp);
				printf("\n-------- BUILD response: \n %s%u%s%u \n\n",response,len,file_buf,tim);

				
				Send(connection,response,strlen(response),0);
				Send(connection,&len,sizeof(len),0);
				Send(connection,file_buf,strlen(file_buf),0);

				Send(connection,&tim,4,0);
				
				
				
			}			
			
		}else{
			char err[7];
			strcpy(err,"-err\r\n");
			Send(connection,err,strlen(err),0);
			printf("\n--error number argument (sscanf)");
			close(connection);
		}
		fclose(F);
		
		
		sprintf(size,"any");
		sprintf(timestamp,"any");
		close(connection);
	}
	return 0;
}

