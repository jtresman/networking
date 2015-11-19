/* **************************************************************
 * 
 * CSCI 4273 - Network Systems
 * Basic Webproxy
 * This webproxy accepts a request from a client and either 
 *  forwards it to the appropriate location or gets it from
 *  the cache.
 * 
 * Author: Jacob Resman
 * 15 November 2015
 * Based on: echoserver.c - A sequential httpRequest server from
 *  from http://www.csc.villanova.edu/~mdamian/sockets/echoC.htm
 * 
 ****************************************************************
 */

#include "nethelp.h"

//Initalize Functions
void request(int connfd);
void sendFile();
void getContent();
void sendHeader();
void errorCode();
void forwardRequest();
int checkCache();
int checkFileType();
void putFile();
void replaceChar(char * str, char cur, char rep);

//Global Values
int port;
char fileExt[9][6];
char fileHeader[9][32];
char proxyRoot[128];
char defaultPage[64];
char serverDir[128] = ".";
int timeout = 0;

int main(int argc, char **argv) {

    printf("Just getting started!\n");
    int listenfd, clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pid_t childPID;

    if(argc != 3){
        printf("Invalid Arguments!");
        return -1;
    }

    port = atoi(argv[1]);

    timeout = atoi(argv[2]);

    listenfd = open_listenfd(port);

    while (1) {
        
        int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen); 
       
        childPID = fork();
        
        if (childPID == 0){
            request(connfd);    /* Service client */
            close(connfd);
            exit(0);        /* code */
        }  
    }

    return 0;
}

//Read the Request Page and Information
void request(int connfd) {
    
    //Read the Request
    char command[MAXLINE] = ""; 
    char host[MAXLINE] = ""; 

    //Read Command, Host and Keep-Alive
    readline(connfd, command, MAXLINE);
    // readline(connfd, host, MAXLINE);
    
    printf("Line 1: %s\n", command);
    // printf("Line 2: %s\n", host);

    // Check if GET Command
    if (strncmp(command, "GET", 3) == 0) {
        //Check is the URL is Cached
        // printf("GET CALLED!\n");
        if (checkCache(command)){
            getContent(command, connfd);
        } else {
            forwardRequest(command, connfd);
        }        
    } else {
        errorCode(400, 1, command, connfd);
        printf("Unsupported Command: %s\n", command);
        return;
    }
    
    // printf("Closing Connection: %d\n", connfd);
}

//Grab the Content Request by the Client
void getContent( char * file, int connfd) {
    
    printf("Get Content From Cache\n");

    //Change to Request from Cache
    char * token  = strtok(file, " ");
    char * location;
    char * version;
    char path[1024] = {};
    int fd;  //file descriptor

    strcpy(path, proxyRoot);

    //Grab Location Token
    token = strtok(NULL, " ");
    location =  token;

    token = strtok(NULL, " ");
    version =  token;

    //Check if No Location Specified
    if(location == NULL || token == NULL || version == NULL)
    {
        printf("BAD URI!\n");
        errorCode(400, 2, file, connfd);
        return;
    }

    if (strcmp(version, "HTTP/1.0\r\n") != 0){
        printf("Invaid HTTP Version!\n");
        errorCode(400, 3, version, connfd);
        return;
    }

    printf("Location: %s\n", location);
    // printf("Cmp: %d\n", strcmp(location, "/")); 

    //Determine File Type
    replaceChar(location, '/', '_');

    sprintf(path, "./%s", location);

    printf("File: %s\n", path);

    //Check if the File Exists and Send it.
    fd = open(path, O_RDONLY);

    printf("File Des: %d\n", fd );

    if (fd == -1) {
         printf("File Doesn't Exists!\n");
         errorCode(404, 0, location, connfd);
         close(fd);
        return;
    } else {
        // sendHeader(fd, i, connfd);
        sendFile(fd, connfd);
    }
}

//Send the Requested File to the Client
void sendFile(int fd, int connfd) {
   // printf("Start Reading\n");

    while(1) {
        /* First read file in chunks of 256 bytes */
        unsigned char buf[256]={0};
        int nread = read(fd, buf, 256);
        // printf("Bytes read %d \n", nread);        

        /* If read was success, send data. */
        if(nread > 0) {
            // printf("Sending \n");
            write(connfd, buf, nread);
        }

        //Check if End of File was Reached
        if (nread < 256) {
           // printf("End of file\n");
            break;
        }
    }
    write(connfd, "\r\n", 2);
    close(fd);
}

//Send Content for Error Code
void errorCode(int err, int type, char * loc, int connfd) {

    printf("There was an ERROR!\n");
    char message[BUFSIZ];
    char strLength[BUFSIZ];

    //Construct Error Message
    //THIS IS SUPER UGLY!
    //I tried creating the message and doing all the writing at the end
    // however it never seemed to send the error correctly.

    switch (err) {
        case 400:
            printf("ERROR 400\n");
            
            switch (type) {
                case 1:
                    sprintf(message, "400 Bad Request: Invalid Method %s \r\n", loc);
                    write(connfd, ERROR_400, strlen(ERROR_400));
                    sprintf(strLength, "%d\n", strlen(message));
                    write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
                    write(connfd, strLength, strlen(strLength)); 
                    write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
                    write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
                    write(connfd, "\r\n", 2);
                    write(connfd, message, strlen(message));
                    break;
                case 2:
                    sprintf(message, "400 Bad Request: Invalid URI %s \r\n", loc);
                    write(connfd, ERROR_400, strlen(ERROR_400));
                    sprintf(strLength, "%d\n", strlen(message));
                    write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
                    write(connfd, strLength, strlen(strLength)); 
                    write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
                    write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
                    write(connfd, "\r\n", 2);
                    write(connfd, message, strlen(message));
                    break;
                case 3:
                    sprintf(message, "400 Bad Request: Invalid HTTP-Version %s \r\n", loc);
                    write(connfd, ERROR_400, strlen(ERROR_400));
                    sprintf(strLength, "%d\n", strlen(message));
                    write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
                    write(connfd, strLength, strlen(strLength)); 
                    write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
                    write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
                    write(connfd, "\r\n", 2);
                    write(connfd, message, strlen(message));
                    break;
            }
            break;
        case 404:
            printf("ERROR 404\n");
            sprintf(message, "404 Not Found: %s \r\n", loc);
            write(connfd, ERROR_404, strlen(ERROR_404));
            sprintf(strLength, "%d\n", strlen(message));
            write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
            write(connfd, strLength, strlen(strLength)); 
            write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
            write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
            write(connfd, "\r\n", 2);
            write(connfd, message, strlen(message));
            break;
        case 501:
            //Unsupported File Type
            printf("ERROR 501\n");
            sprintf(message, "501 Unsupported File: %s \r\n", loc);
            write(connfd, ERROR_501, strlen(ERROR_501));
            sprintf(strLength, "%d\n", strlen(message));
            write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
            write(connfd, strLength, strlen(strLength)); 
            write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
            write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
            write(connfd, "\r\n", 2);
            write(connfd, message, strlen(message));
            break;
        case 500:
            //All Other Errors
            printf("ERROR 500\n");
            sprintf(message, "500 Internal Server Error: cannot allocate memeoryr\n");
            write(connfd, ERROR_500, strlen(ERROR_500));
            sprintf(strLength, "%d\n", strlen(message));
            write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
            write(connfd, strLength, strlen(strLength)); 
            write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
            write(connfd, "text/plain\r\n", strlen("text/plain\r\n"));
            write(connfd, "\r\n", 2);
            write(connfd, message, strlen(message));
            break;
        default:
            break;
    }
}

//Check Cache to see if file Exists
int checkCache(char * command) { 

    printf("Check Cache!\n");

    char * token;
    char location[MAXBUF] = "";
    char path[128] = "";

    struct stat statbuf;

    strcpy(location, command);

    token = strtok(location, " ");

    //Grab Location Token
    token = strtok(NULL, " ");
    strcpy(location, token);

    //Replace / to _ 
    replaceChar(location, '/', '_');
    sprintf(path, "./%s", location);

    printf("Path: %s\n", path );

    if (stat(path, &statbuf) == -1) {
        perror(path);
        return 0;
    }
    
    printf("File Time: %d CurTime: %d Timeout: %d \n", statbuf.st_mtime, time(0), timeout );

    if (statbuf.st_mtime <= (time(0) + timeout)){
        return 0; 
    }

    return 1;
}

//Check for Supported File Type
int checkFileType(char * ext) {

    int i;

    //Determine if Supported Ext
    for (i = 0; i < sizeof(fileExt); ++i){
        if(strcmp(ext, fileExt[i]) == 0){
            break;
        }
    }

    //Unsupported File Type
    if (i == sizeof(fileExt)){
        return -1;
    }

    return i;
}

//Forward the Request to the corret host and port
void forwardRequest(char * command, int connfd) {

    /// printf("Forward Request! %s\n", command);

    char * token;
    char location[PATH_MAX];
    char buf[MAXBUF] = "";
    char host[128] = "";
    char file[128] = "";

    int serverfd;
    int isPort = 0;
    int port = DEFAULT_WEB_PORT;

    strcpy(buf, command);

    token = strtok(buf, " ");

    //Grab Location Token
    token = strtok(NULL, " ");
    strcpy(location, token);

    printf("Location: %s Port: %d \n", location, port );

    //Add Configurable Port
    strcpy(buf, location);
    strtok(buf, ":");
    strtok(NULL, ":");
    token = strtok(NULL, ":");
    if (token != NULL){
        port = atoi(token);
        printf("Location1: %s Port: %d \n", location, port );
        isPort = 1;
    } 

    //Strip Out Host
    strcpy(host, location);

    //Remove http://
    token = strtok(host, "/");
    token = strtok(NULL, "/");

    strcpy(host,token);

    printf("Host: %s\n", host );

    serverfd = open_clientfd(host, port);

    printf("Command: %s FD: %d\n", command, serverfd );

    //Intinal Page or A different Page
    token = strtok(NULL, " ");
    if (token == NULL && !isPort){
        sprintf(file, "GET %s/ HTTP/1.0\r\n\r\n", host);
    } else if (token != NULL && !isPort){
        sprintf(file, "GET %s/%s HTTP/1.0\r\n\r\n", host, token);
    } else if (isPort) {
        token = strtok(token, ":");
        printf("token: %s\n", token );
        if (atoi(token) != 0){
            sprintf(file, "GET %s/ HTTP/1.0\r\n\r\n", host);
        } else {
            sprintf(file, "GET %s/%s HTTP/1.0\r\n\r\n", host, token);
        }
    }

    printf("File: %s\n", file);

    write(serverfd, file, strlen(file));

    //Read response to a file
    putFile(location, connfd, serverfd);

    close(serverfd);

}

//PUT: Read the response from the website and wite to a file
void putFile(char * filename, int connfd, int serverfd){

    printf("Put File!\n");

    char path[PATH_MAX];
    int chunkRead;
    char data[512];

    //Add Path onto File Name
    replaceChar(filename, '/', '_');
    sprintf(path, "%s/%s", serverDir, filename);

    printf("Path: %s\n", path );

    //Open File for Writing
    remove(path);

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        write(connfd, "Error: Opening File!\n", 34);
        exit(1);
    } 

    while ((chunkRead = read(serverfd, data, sizeof(data)))!= (size_t)NULL)//send file
    {
        write(connfd, data, chunkRead);
        fprintf(f, "%s", data);
    }

    fprintf(f, "\r\n");

    //Close File
    fclose(f);
}

//Remove a Character from a string
void replaceChar(char * str, char cur, char rep){

    int j = 0;

    while (str[j] != '\0'){
        if (str[j] == cur) {
            str[j] = rep;
        }
        j++;
    }
}