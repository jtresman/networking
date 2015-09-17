/*
 * echoserver.c - A sequential httpRequest server
 */

#include "nethelp.h"

//Initalize Functions
void httpRequest(int connfd);
void getConfig();
int checkFileType();
char * checkDefaultFile();
void sendFile();
void getContent();
void sendHeader();

//Global Values
int port;
char fileExt[9][6];
char fileHeader[9][32];
char * documentRoot = "";
char defaultPage[64];

int main(int argc, char **argv) {

    printf("Just getting started!\n");
    int listenfd, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pid_t childPID;

    getConfig();

    listenfd = open_listenfd(port);

    while (1) {
        
        int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen); 
       
        childPID = fork();
        
        if (childPID == 0){
            httpRequest(connfd);    /* Service client */
            exit(0);        /* code */
        }  
        close(connfd);
    }

    return 0;
}

//Read the Configuration File
void getConfig() {

    FILE * fp; 
    char * line = NULL;
    char * tok;
    size_t len = 0;
    ssize_t read;
    int i = 0;
    int content = 0; //If content Comment Found

    fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // printf("We are congfiguring\n");
    //Read the config File
    while ((read = getline(&line, &len, fp)) != -1) {

        //printf("Line: %s\n", line);

        if (strcmp("#serviceport number\n", line) == 0){
            content = 1;
        }  else if (strcmp("#document root\n", line) == 0){
            content = 2;
        } else if (strcmp("#default web page\n", line) == 0){
            content = 3;
        } else if (strcmp("#Content-Type\n", line) == 0){
            content = 4;
        } else {

            switch(content){

                case 1:
                    
                    port = atoi(line);
                    printf("Service Port: %d\n",port);
                    break;

                case 2:
                
                    documentRoot = line;
                    documentRoot++;
                    //Remove the Newline and Quotes
                    documentRoot[strlen(documentRoot)-2] = 0;
                    printf("Document Root: %s\n", documentRoot);
                    break;

                case 3:

                    strcpy(defaultPage, line);
                    //Remove the Newline
                    defaultPage[strlen(defaultPage)-1] = 0;
                    printf("Default Page: %s\n", defaultPage);
                    break;

                case 4: 
                    
                    tok = strtok(line, " ");
                    strcpy(fileExt[i], tok);
                    printf("File Ext: %s\n",tok);
                    tok = strtok(NULL, " ");
                    strcpy(fileHeader[i], tok); 
                    printf("File Header: %s\n",tok);
                    ++i;
                    break;

                default:
                    break;

            }
        }
    }

    fclose(fp);
    if (line)
        free(line);
}

//Read the Request Page and Information
void httpRequest(int connfd) {
    
    char command[MAXLINE] = ""; 
    char host[MAXLINE] = ""; 
    char keepAlive[MAXLINE] = ""; 
    
    //Read First 3 Line
    readline(connfd, command, MAXLINE);
    readline(connfd, host, MAXLINE);
    readline(connfd, keepAlive, MAXLINE);

    printf("Line 1: %s\n", command);
    printf("Line 2: %s\n", host);
    printf("Line 3: %s\n", keepAlive);
    // printf("Line Num: %d\n", n);

    // Check if GET Command
    if (strncmp(command, "GET", 3) == 0) {

        getContent(command, connfd);
        // printf("GET CALLED!\n");
    } else {

        printf("Unsupported Command: %s\n", command);
        return;
    }
    
    //Check for keep-alive
    if (strcmp(keepAlive, "Connection: keep-alive\n") == 0) {

         printf("DO KEEP ALIVE STUFF\n");

    } 

    printf("HTTP REQUEST PROCESSED!\n");

    printf("Closing Connection: %d\n", connfd);
    close(connfd);
}

//Grab the Content Request by the Client
void getContent( char * file, int connfd) {
    // printf("%s\n", file);

    char * token  = strtok(file, " ");
    char * location;
    char path[1024] = documentRoot;
    int fd;  //file descriptor
    char * ext;
    int i = 0;


    //Grab Location Token
    token = strtok(NULL, " ");
    location =  token;

    if(location == NULL)
    {
        printf("No Location Specified\n");
        return;
    }

    printf("Location: %s\n", location);
    // printf("Cmp: %d\n", strcmp(location, "/")); 

    //Determine File Type
    if (strcmp(location, "/") == 0){
        
        printf("Path: %s\n", path);
        printf("defaultPage: %s\n",defaultPage );

        i = 0;
        strcat(path, "/");
        strcat(path, defaultPage);
        printf("Path: %s12\n", path);

    } else {

        //Grab Extension
        ext = strrchr(location, '.');

        if( (i = checkFileType(ext)) != -1){

            strcat(path, location);

        } else {

            //Unsupportedd File Type
            write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
            write(connfd, "text/plain\n", 11);
            write(connfd, ERROR_501, strlen(ERROR_501));
            write(connfd, location, strlen(location));
            write(connfd, "\r\n", 2); 
        }
    } 

    printf("File: %s\n", path);

    //Check if the File Exists and Send it.
    fd = open(path, O_RDONLY);

    printf("File Des: %d\n", fd );

    if (fd == -1) {
         
        return;
    } else {
        sendHeader(fd, i, connfd);
        sendFile(fd, connfd);
    }
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

//Process and Send the Neccesary Header to the Client
void sendHeader(int fd, int i, int connfd) {

    char strLength[BUFSIZ];
    struct stat fileStat; 

    fstat(fd, &fileStat);

    sprintf(strLength, "%d\n", fileStat.st_size);

    //Write Headers to Server
    write(connfd, STATUS_200, strlen(STATUS_200));
    write(connfd, CONTENT_LENGTH, strlen(CONTENT_LENGTH));
    write(connfd, strLength, strlen(strLength)); 
    write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
    write(connfd, fileHeader[i], strlen(fileHeader[i]));
    write(connfd, "\n", 1);

    printf("File Header: %s\n", fileHeader[i]);
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

    write(connfd, CONTENT_TYPE, strlen(CONTENT_TYPE));
    write(connfd, "text/plain\n", 11);

    switch (err) {
        case 400:
            break;
        case 404:
            write(connfd, ERROR_404, strlen(ERROR_404));
            write(connfd, loc, strlen(loc));
            write(connfd, "\r\n", 1);
            break;
        case 501:
            //Unsupportedd File Type
            write(connfd, ERROR_501, strlen(ERROR_501));
            write(connfd, loc, strlen(loc));
            write(connfd, "\r\n", 2); 
            break;
        case 500:
            break;
        default:
            break;
    }
}


