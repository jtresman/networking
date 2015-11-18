#ifndef __NET_HELP
#define __NET_HELP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h> // for open
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <openssl/md5.h>

#define MAXLINE  1024  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */
#define PATH_MAX 128

#define CONFIG_FILE      "/home/jacob/Documents/Networking/proxy/proxy.conf"
static char * ERROR_400      = "HTML/1.1 400 BAD REQUEST\r\n";
static char * STATUS_200     = "HTTP/1.1 200 OK\n";
static char * CONTENT_TYPE   = "Content-Type: ";
static char * CONTENT_LENGTH = "Content-length: "; 
static char * ERROR_404 = "HTTP/1.1 404 NOT FOUND\r\n";
static char * ERROR_501 = "HTTP/1.1 501 NOT IMPLEMENTED\r\n";
static char * ERROR_500 = "HTTP/1.1 500 INTERNAL SERVER ERRROR\r\n";
static int DEFAULT_WEB_PORT = 80;

/* 
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure 
 */
int open_listenfd(int port);

/*
 * open_clientfd - open connection to server at <hostname, port>
 *   and return a socket descriptor ready for reading and writing.
 *   Return <0 in case of failure.
 */
int open_clientfd(char *hostname, int port);

/* 
 * readline - read a line of text
 * return the number of characters read
 * return -1 if error
 */
int readline(int fd, char * buf, int maxlen);

#endif




