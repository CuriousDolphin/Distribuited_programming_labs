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
#define MAX_UINT16T 0xffff
#define CHUNK_SIZE 1500

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif
char *prog_name;
int send_n(int, char *, size_t);
int set_select(int, int);
int main(int argc, char *argv[])
{
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
	int connection;
	int times = 15; //14 secondi

	while (1)
	{
		trace(err_msg("(%s) waiting for connections ...", prog_name));
		fflush(stdout);
		connection = accept(id_socket, (SA *)&cliaddr, &cliaddrlen);
		trace(err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));

		int exit_condition = 0;
		while (exit_condition == 0)
		{

			fflush(stdout);
			char get_buf[4] = "";
			int res_sel;							 //RISULTATO SELECT
			fd_set cset;							 //insieme di socket su cui agisce la SELECT
			FD_ZERO(&cset);						 //azzero il set
			FD_SET(connection, &cset); //ASSOCIO IL SOCKET ALL'INSIEME
			struct timeval tval;
			tval.tv_sec = times;
			tval.tv_usec = 0; //imposto il tempo nell astruttura
			res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
			if (res_sel == -1)
			{
				//Send(connection, err, strlen(err), 0); // error message
				printf("select() failed");
				fflush(stdout);
				close(connection);
				break;
			}
			else if (res_sel == 0) //TIMEOUT
			{
				//Send(connection, err, strlen(err), 0); // error message
				printf("---timeout exceded %d seconds\n", times);
				fflush(stdout);
				close(connection);
				break;
			}
			else
			{
				int n_read = recv(connection, buf, MAXBUFL, MSG_NOSIGNAL);

				fflush(stdout);
				if (n_read <= 0)
				{
					exit_condition = 1;
					close(connection);
					break;
				}
				int n_arg = 0;
				n_arg = sscanf(buf, "%s %s", get_buf, file_name);
				printf("\n\t--file name: %s,\n\t--command: %s \n", file_name, get_buf);
				fflush(stdout);
				if (n_arg == 2 && strcmp(get_buf, "GET") == 0)
				{
					struct stat st;
					stat(file_name, &st);
					uint32_t len = htonl(st.st_size);
					uint32_t tim = htonl(st.st_mtime);
					FILE *F;
					F = fopen(file_name, "r"); /*apertura FILE */
					if (F == NULL)
					{
						printf("\n\tERROR FILE NOT FOUND %s\n", file_name);
						fflush(stdout);
						send(connection, err, strlen(err), MSG_NOSIGNAL);

						close(connection);
						exit_condition = 1;
						break;
					}
					else
					{
						char *file_buf;
						file_buf = malloc(CHUNK_SIZE * sizeof(char)); /* INVIO MASSIMO 1500 byte alla volta */
						printf("\n\t--File opened %s", file_name);		/* LETTURA FILE */
						fflush(stdout);
						rewind(F);

						fflush(stdout);

						strcpy(response, "");
						sprintf(timestamp, "%u", ntohl(st.st_mtime));
						strcat(response, "+OK\r\n");
						size_t nsended = send(connection, response, strlen(response), MSG_NOSIGNAL); /* +ok */
						if (nsended <= 0)
						{
							exit_condition = 1;
							close(connection);
							break;
						}

						//sleep(15);

						nsended = send(connection, &len, sizeof(len), MSG_NOSIGNAL); /*dimensione*/
						if (nsended <= 0)
						{
							exit_condition = 1;
							close(connection);
							break;
						}
						int i = 0; //CHUNK COUNTER
						nsended = 0;
						int n_sended = 0;			 //Byte inviati
						size_t tot_sended = 0; // totale byte inviati
						int stop = 0;
						int read = 0;			//byte letti dalla fread
						while (stop == 0) /* INVIO CHUNK di 1500 byte alla volta */
						{
							read = fread(file_buf, 1, CHUNK_SIZE, F);
							n_sended = send_n(connection, file_buf, read);
							if (n_sended <= 0)
							{
								exit_condition = 1;
								close(connection);
								break;
							}
							tot_sended += n_sended;
							i++;
							if (read < CHUNK_SIZE) // condizione di terminazione
								stop = 1;
						}
						if (n_sended <= 0) // se si chiude la connessione mentre invio
						{
							exit_condition = 1;
							close(connection);
							break;
						}
						free(file_buf);
						fclose(F);
						Send(connection, &tim, 4, MSG_NOSIGNAL); //INVIO TIMESTAMP
						printf("\n\t--sended %s  %ld bytes (%d chunks )\n\n", file_name, tot_sended, i);
						fflush(stdout);
					}
				}
				else
				{
					Send(connection, err, strlen(err), MSG_NOSIGNAL);
					printf("\n\t--error number argument (sscanf)");
					exit_condition = 1;
					close(connection);
				}
			}
		}
		sprintf(size, "any");
		sprintf(timestamp, "any");
		close(connection);
	}
	return 0;
}
int send_n(int s, char *ptr, size_t nbytes)
{
	size_t nleft;
	ssize_t nwritten;
	for (nleft = nbytes; nleft > 0;)
	{
		nwritten = send(s, ptr, nleft, MSG_NOSIGNAL);

		if (nwritten <= 0) /* error */
		{
			printf("\n send failed %d %ld", s, nwritten);
			return -1;
		}

		else
		{
			nleft -= nwritten;
			ptr += nwritten;
		}
	}

	return (nbytes - nleft);
}
