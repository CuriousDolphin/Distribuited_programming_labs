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
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "../errlib.h"
#include "../sockwrap.h"
#define LISTENQ 15
#define MAXBUFL 50
#define MAXBUFF 100
#define MAX_UINT16T 0xffff
#define CHUNK_SIZE 1500
#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif
void signal_handler(int);

int send_n(int, char *, size_t);
char *prog_name;

int main(int argc, char *argv[])
{

	signal(SIGCHLD, signal_handler); /* SIGNAL HANDLER PER KILLARE GLI ZOMBIE*/
	int id_socket;
	int port;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	prog_name = argv[0];
	char buf[MAXBUFL];
	char file_name[MAXBUFF];
	char response[MAXBUFF]; /* command for reply ..ok err */
	char size[4];
	char timestamp[4];
	char err[7]; /*stringa errore*/
	strcpy(err, "-ERR\r\n");

	if (argc != 2 || argv[1] < 0)
	{
		err_quit("usage: %s need <port>\n ", prog_name);
	}

	port = atoi(argv[1]);
	id_socket = Socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(id_socket, (SA *)&servaddr, sizeof(servaddr));

	trace(err_msg("(%s) socket created", prog_name));
	trace(err_msg("(%s) listening on %s:%u", prog_name, inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port)));
	Listen(id_socket, LISTENQ);
	int new_connection;

	while (1)
	{
		trace(err_msg("(%s) waiting for connections ...", prog_name));
		fflush(stdout);
		new_connection = accept(id_socket, (SA *)&cliaddr, &cliaddrlen);
		trace(err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));
		int child_pid;
		child_pid = fork();
		if (child_pid < 0)
		{
			printf("fork failed =(");
			exit(0);
		}
		else if (child_pid > 0)
		{ /* PROCESSO PADRE */
			close(new_connection);
		}
		else
		{ /* PROCESSO FIGLIO */
			int pid = getpid();
			close(id_socket);
			int exit_condition = 0;
			int res_sel;		//RISULTATO SELECT
			int times = 15; //14 secondi
			struct timeval tval;
			fd_set cset;									 //insieme di socket su cui agisce la SELECT
			FD_ZERO(&cset);								 //azzero il set
			FD_SET(new_connection, &cset); //ASSOCIO IL SOCKET ALL'INSIEME
			tval.tv_sec = times;
			tval.tv_usec = 0; //imposto il tempo nell astruttura
			res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
			if (res_sel == -1)
			{
				Send(new_connection, err, strlen(err), 0); // error message
				printf("select() failed");
				close(new_connection);
				return 0;
			}
			else if (res_sel <= 0) //TIMEOUT
			{
				Send(new_connection, err, strlen(err), 0); // error message
				printf("\n\t(%d)---timeout exceded %d seconds\n", pid, times);
				close(new_connection);
				return 0;
			}
			else
			{
				while (exit_condition == 0) /* serve per rispondere ad ulteriori richieste del client (più parametri) 
																			per non chiudere la connessione prima di aver servito tutti i file necessari */
				{

					char get_buf[4] = "";
					int n_read = recv(new_connection, buf, MAXBUFL, 0);
					int n_arg = 0;
					if (n_read <= 0)
					{
						exit_condition = 1;
						break;
					}
					n_arg = sscanf(buf, "%s %s", get_buf, file_name);
					printf("\n\t(%d)--file name: %s,\n\t(%d)--command: %s \n", pid, file_name, pid, get_buf);
					if (n_arg == 2 && strcmp(get_buf, "GET") == 0)
					{
						struct stat st;
						stat(file_name, &st);
						uint32_t len = htonl(st.st_size);
						uint32_t tim = htonl(st.st_mtime); // TIMESTAMP
						FILE *F;
						F = fopen(file_name, "r"); /*apertura FILE */
						if (F == NULL)
						{
							printf("\n\tERROR FILE NOT FOUND %s\n", file_name);
							fflush(stdout);
							Send(new_connection, err, strlen(err), 0);
							close(new_connection);
							exit_condition = 1;
							exit(0);
							break;
						}
						else
						{
							char *file_buf;
							file_buf = malloc(CHUNK_SIZE * sizeof(char));
							printf("\n\t(%d)--File opened %s", pid, file_name); /* LETTURA FILE */
							fflush(stdout);
							rewind(F);
							strcpy(response, "");
							sprintf(timestamp, "%u", ntohl(st.st_mtime));
							strcat(response, "+OK\r\n");
							Send(new_connection, response, strlen(response), 0); /* +ok */
							Send(new_connection, &len, sizeof(len), 0);					 /* LENGTH*/
							int i = 0;																					 //CHUNK COUNTER
							size_t nsended = 0;																	 //Byte inviati
							int stop = 0;
							int read = 0;			//byte letti dalla fread
							while (stop == 0) /* INVIO CHUNK di 1500 byte alla volta */
							{
								read = fread(file_buf, 1, CHUNK_SIZE, F);
								nsended += send_n(new_connection, file_buf, read);
								i++;
								if (read < CHUNK_SIZE) // condizione di terminazione
									stop = 1;
							}
							free(file_buf);
							fclose(F);
							Send(new_connection, &tim, 4, 0); //INVIO TIMESTAMP
							printf("\n\t(%d)--sended %s  %ld bytes (%d chunks )\n\n", pid, file_name, nsended, i);
						}
					}
					else
					{
						Send(new_connection, err, strlen(err), 0);
						printf("\n\t(%d)--error number argument (sscanf)", pid);
						exit_condition = 1;
					}
				}
			}
			close(new_connection); /* CHIUSURA CONNESSIONE FIGLIO */
			exit(0);							 /* USCITA FIGLIO */
		}

		sprintf(size, "any");
		sprintf(timestamp, "any");
	}

	return 0;
}

void signal_handler(int signal)
{
	int pid = wait(NULL);
	printf("\n(%s) - connection closed  zombie killato: (%d)-\n(%s) still waiting for connections ...\n", prog_name, pid, prog_name);
}

int send_n(int s, char *ptr, size_t nbytes)
{
	size_t nleft;
	ssize_t nwritten;
	for (nleft = nbytes; nleft > 0;)
	{
		nwritten = send(s, ptr, nleft, 0);
		if (nwritten <= 0) /* error */
		{
			printf("\n send failed");
			return (nwritten);
		}

		else
		{
			nleft -= nwritten;
			ptr += nwritten;
		}
	}

	return (nbytes - nleft);
}