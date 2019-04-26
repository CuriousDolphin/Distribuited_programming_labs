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

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif
char *prog_name;

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
	char err[7];				/*stringa errore*/
	char *start_memory; // mi serve per ripristinare il puntatore
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

	while (1)
	{
		trace(err_msg("(%s) waiting for connections ...", prog_name));
		connection = accept(id_socket, (SA *)&cliaddr, &cliaddrlen);
		trace(err_msg("(%s) - new connection from client %s:%u", prog_name, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)));

		int exit_condition = 0;
		while (exit_condition == 0)
		{
			char get_buf[4] = "";
			int n_read = recv(connection, buf, MAXBUFL, 0);
			int n_arg = 0;
			if (n_read <= 0)
			{
				exit_condition = 1;
				break;
			}
			n_arg = sscanf(buf, "%s %s", get_buf, file_name);
			printf("\n\t--file name: %s,\n\t--command: %s \n", file_name, get_buf);
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
					Send(connection, err, strlen(err), 0);

					close(connection);
					exit_condition = 1;
					break;
				}
				else
				{
					char *file_buf;
					printf("\n\t--File opened %s", file_name); /* LETTURA FILE */
					fflush(stdout);
					rewind(F);
					file_buf = malloc(st.st_size * sizeof(char)); /* allocazione dinamica */
					int bytes_read = 0;
					bytes_read = fread(file_buf, 1, st.st_size, F);
					printf("\n\t--Bytes read: %d (previsti: %ld)\n", bytes_read, st.st_size);
					fflush(stdout);
					fclose(F);
					strcpy(response, "");
					sprintf(timestamp, "%u", ntohl(st.st_mtime));
					strcat(response, "+OK\r\n");
					Send(connection, response, strlen(response), 0); /* +ok */
					Send(connection, &len, sizeof(len), 0);
					start_memory = file_buf; /* IMPORTANTE */ /*dimensione*/
					size_t nleft;
					ssize_t nwritten = 0;
					for (nleft = bytes_read; nleft > 0;)
					{
						nwritten = send(connection, file_buf, nleft, 0); /*FILE */
						if (nwritten <= 0)
						{ /*ERRORE*/
							//	return (nwritten);
						}
						else
						{
							nleft -= nwritten;
							file_buf += nwritten;
						}
					}
					file_buf = start_memory; /* RIPORTO IL PUNTATORE ALL'INIZIO */
					Send(connection, &tim, 4, 0);
					printf("\n\t--sended %s (%ld) \n\n", file_name, nwritten);
					free(file_buf);
				}
			}
			else
			{
				Send(connection, err, strlen(err), 0);
				printf("\n\t--error number argument (sscanf)");
				exit_condition = 1;
				close(connection);
			}
		}
		sprintf(size, "any");
		sprintf(timestamp, "any");
		if (exit_condition == 1)
			close(connection);
	}
	return 0;
}
