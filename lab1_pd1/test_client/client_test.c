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
    /* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=3 || strlen(argv[1]) > 16 || strlen(argv[1]) < 7)
    {
        err_quit ("errore parametri in ingresso,inserire indirizzo server e porta");
    }
		
	port=atoi(argv[2]);
    addr=argv[1];

    printf("\n client avviato ricevuto address: %s / %d \n",addr,port);
}