/*******************************************************
 *
 * distclient.c
 *
 * This is the client for the distributed filer server.
 *  It puts and gets files from the different servers.
 *  
 *  Created: 01 October 2015
 *  Author: Jacob Resman
 *
 ******************************************************
 */

#include "nethelp.h"

int main(int argc, char **argv) 
{
    int clientfd, port;
    char *host, buf[MAXLINE];
    int n;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = atoi(argv[2]);

    clientfd = open_clientfd(host, port);

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        write(clientfd, buf, strlen(buf));
        n = readline(clientfd, buf, MAXLINE);
        write(1, buf, n);
    }
    close(clientfd);
    exit(0);
}

