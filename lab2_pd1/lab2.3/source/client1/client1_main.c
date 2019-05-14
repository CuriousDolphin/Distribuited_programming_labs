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
#define CHUNK_SIZE 1500
#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif
char *prog_name;

int read_n(int, char *, size_t);
int main(int argc, char *argv[])
{
	int id_socket;
	int port;
	int res_addr;
	char *addr;
	char request[MAXBUFL];
	char *command_buf;
	char *file_buf;
	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);
	command_buf = calloc(6, sizeof(char));
	prog_name = argv[0];
	if (argc < 4)
	{
		err_quit("wrong params usage: %s need <address> <port>\n ", prog_name);
	}

	port = atoi(argv[2]);
	addr = argv[1];
	id_socket = Socket(AF_INET, SOCK_STREAM, 0);
	/* CONFIGURAZIONE SOCKET */
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(port);								 /* CONVERSIONE NETWORK ORDER SHORT*/
	res_addr = inet_aton(addr, &cliaddr.sin_addr); /* inserimento indirizzo da stringa dotted */
	if (res_addr == 0)
	{
		printf("indirizzo non valido inet_aton() failed");
	}
	Connect(id_socket, (struct sockaddr *)&cliaddr, cliaddrlen);
	int i;

	for (i = 3; i < argc; i++) /* ITERAZIONE NUMERO ARGOMENTI */
	{
		strcpy(request, "");
		strcat(request, "GET ");
		strcat(request, argv[i]);
		strcat(request, "\r\n");
		Send(id_socket, request, strlen(request), 0);
		struct timeval tval;
		fd_set cset;							//insieme di socket su cui agisce la SELECT
		FD_ZERO(&cset);						//azzero il set
		FD_SET(id_socket, &cset); //ASSOCIO IL SOCKET ALL'INSIEME
		int times = 15;
		tval.tv_sec = times;
		tval.tv_usec = 0; //imposto il tempo nell astruttura
		int res_sel;
		/* SELECT */
		res_sel = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
		if (res_sel == -1)
		{
			printf("select() failed");
			return -1;
		}
		if (res_sel > 0)
		{
			uint32_t len;
			uint32_t timest;
			uint32_t timestamp;
			uint32_t file_len;

			Recv(id_socket, command_buf, 5, 0); /* COMMAND */
			if (strcmp(command_buf, "+OK\r\n") == 0)
			{
				printf("\t--Received Response OK");
				Recv(id_socket, &len, 4, 0); /* LUNGHEZZA */
				file_len = ntohl(len);
				printf("\t--Received file len: '%u' byte", file_len);
				fflush(stdout);
				file_buf = malloc(CHUNK_SIZE * sizeof(char));
				FILE *F;
				F = fopen(argv[i], "w");
				if (F == NULL)
				{
					printf("errore salvataggio file");
					fflush(stdout);
					return -1;
				}
				/* RICEZIONE FILE */
				ssize_t nread = 0;
				size_t nleft;
				nleft = file_len;
				int cont = 0;

				if (file_len < CHUNK_SIZE) //RICEVO IN UNA VOLTA SOLA
				{
					nread = Recv(id_socket, file_buf, file_len, 0);
					cont = 1;
					fwrite(file_buf, 1, nread, F);
				}
				else
				{

					size_t size_to_download = CHUNK_SIZE;
					while (nleft > 0) // RICEVO e scrivo 1500 byte alla volta
					{
						if (nleft < CHUNK_SIZE)
						{
							size_to_download = nleft;
						}
						nread = Recv(id_socket, file_buf, size_to_download, 0);
						if (nread > 0)
						{
							nleft -= nread;
							cont++;

							fwrite(file_buf, 1, nread, F);
						}
						else
						{
							printf("\n recv error");
							fflush(stdout);
							break;
						}
					}
				}
				fclose(F);
				struct stat st;
				stat(argv[i], &st);
				Recv(id_socket, &timest, 4, 0);
				timestamp = htonl(timest);
				printf("\n\t---'%s' downloaded %ld byte (%d) chunks", argv[i], st.st_size, cont);
				printf("\n\t--Received  timestamp: %u  \n\n", timestamp);
				free(file_buf);
			}
			else
			{
				printf("\t--Server error\n");
				fflush(stdout);
				close(id_socket);
				break;
			}
		}
		else
		{
			printf("---timeout exceded %d seconds\n", times);
			close(id_socket);
		}
	}
	free(command_buf);
	close(id_socket);
	return 0;
}

int read_n(int s, char *ptr, size_t len)
{
	ssize_t nread;
	size_t nleft;

	for (nleft = len; nleft > 0;)
	{
		nread = recv(s, ptr, nleft, 0);
		if (nread > 0)
		{
			nleft -= nread;
			ptr += nread;
		}
		else if (nread == 0) /* conn. closed by party */
			break;
		else
			/* error */
			return (nread);
	}
	return (len - nleft);
}