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
#include <unistd.h> // for close
#include <assert.h>
#include <pthread.h>

#define MAXLINE  8192  /* max text line length */
#define MAXBUF   8192  /* max I/O buffer size */
#define LISTENQ  1024  /* second argument to listen() */

#define CONFIG_FILE      "/home/jacob/Documents/Networking/webserver/www/ws.conf"
static char * ERROR_400      = "HTML/1.1 400 Bad Request: ";
static char * BAD_METHOD     = "Invalid Method:  ";
static char * BAD_URI        = "Invalid URI:  ";
static char * BAD_HTTP_VER   = "Invalid HTTP-Version:  ";
static char * STATUS_200     = "HTTP/1.1 200 OK\n";
static char * CONTENT_TYPE   = "Content-Type: ";
static const char * CONTENT_LENGTH = "Content-length: "; 
static const char * ERROR_404 = "HTTP/1.1 404 Not Found: ";
static const char * ERROR_501 = "HTTP/1.1 501 Not Implemented ";
static const char * ERROR_500 = "500 Internal Server Error: cannot allocate memeory";

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




