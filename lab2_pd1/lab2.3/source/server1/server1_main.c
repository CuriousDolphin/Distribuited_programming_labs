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
#define MAXBUFF 100
#define MAXRES 500000 /* 100mb */
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
	char err[7];		
	
	strcpy(err,"-ERR\r\n");
	buf=malloc(MAXBUFL*sizeof(char));
	response=malloc(MAXBUFL * sizeof(char));
	
	file_name=malloc(MAXBUFF*sizeof(char));
	file_buf=malloc(100000000*sizeof(char));
	FILE *F;
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
		trace ( err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)) );

		int exit_condition=0;
			while(exit_condition==0){
					char get_buf[4]="";	
					int n_read =Recv(connection,buf,MAXBUFL,0);
					int n_arg=0;
					if (n_read==0){
						exit_condition=1;
						break;
					}
					//printf("\n\t--ricevuti: (%d) byte :%s \n",n_read,buf);
					n_arg=sscanf(buf,"%s %s",get_buf,file_name) ;
					printf("\n\t--file name: %s,command: %s \n",file_name,get_buf);
					if(n_arg == 2 && strcmp(get_buf,"GET")==0){
						
						F=fopen(file_name,"r");  	/*apertura FILE */

						if(F==NULL){
							printf("\n\tERROR FILE NOT FOUND %s\n",file_name);
							Send(connection,err,strlen(err),0);
						
							close(connection);
							exit_condition=1;
							break;
							
						}else{ 	
							printf("\n\t FILE APERTO");					/* LETTURA FILE */
							uint32_t file_len=0;
							size_t cont=0;
							while(fscanf(F,"%c",&file_buf[file_len]) != EOF){
								
								file_len++;
								cont++;
							}
							printf("\n\t FILE LETTO: %u",file_len);	
							strcpy(response,"");

							//file_buf[file_len]='\0';

							struct stat st;
							stat(file_name,&st);
							uint32_t len=htonl(file_len);
							uint32_t tim=htonl(st.st_mtime);

				
							sprintf(timestamp,"%u",ntohl(st.st_mtime));
							strcat(response,"+OK\r\n");
							Send(connection,response,strlen(response),0);
							Send(connection,&len,sizeof(len),0);

							size_t nleft; ssize_t nwritten;
							for(nleft=cont;nleft >0;){
								nwritten=send(connection,file_buf,nleft,0); 
								if(nwritten <=0 ){/*ERRORE*/
									//	return (nwritten);
								}else{
									nleft -=nwritten;
									file_buf +=nwritten;
								}

							}

							Send(connection,&tim,4,0);
							
							printf("\n\t--sended %s (%ld) \n\n",file_name,cont);
							fclose(F);
						}			
						
					}else{
						
						Send(connection,err,strlen(err),0);
						printf("\n\t--error number argument (sscanf)");
						exit_condition=1;
						close(connection);
					}
				
			}		
		
		sprintf(size,"any");
		sprintf(timestamp,"any");
		if(exit_condition==1)
			close(connection);
	//free(response);
	//free(file_buf);
	}
	
	return 0;
}

