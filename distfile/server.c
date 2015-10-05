/* **************************************************************
 * 
 * CSCI 4273 - Network Systems
 * Distributed File Server 
 * This server splits files into 4 parts and copies the parts to 
 *  4 different servers.
 * 
 * Author: Jacob Resman
 * 01 Octoberr 2015
 * Based on: echoserver.c - A sequential request server from
 *  from http://www.csc.villanova.edu/~mdamian/sockets/echoC.htm
 * 
 ****************************************************************
 */

#include "nethelp.h"

//Initalize Functions
void request(int connfd);
void getConfig();
void sendFile();
void getFile();
void listFiles();

typedef struct user{

    char name[128];
    char passwd[128];
} user;

//Global Values
int port;
char documentRoot[128];
user * users;  

int main(int argc, char **argv) {

    printf("Just getting started!\n");

    //Listen at the specified locaiton

    //Read Config for users

    //Call function to wait to process requests

    if(argc != 3){
        printf("Invalid Arguments!");
        return -1;
    }

    strcpy(documentRoot, argv[2]);

    port = atoi(argv[1]);

    int listenfd, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;

    getConfig();

    listenfd = open_listenfd(port);

    while (1) {

        int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);         
        request(connfd);
        close(connfd);
    }

    return 0;
}

//Read the Configuration File
void getConfig() {

    FILE * fp; 
    char * line = NULL;
    char * token;
    size_t len = 0;
    ssize_t read;
    int content = 0; //If content Comment Found
    int num_users = 0;
    int i=0;

    fp = fopen(SERVER_CONF, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // printf("We are congfiguring\n");

    //Grab number of users
    getline(&line, &len, fp);

    num_users = atoi(line);
    
    users = malloc(num_users * sizeof(user));

    printf("%d\n", num_users);

    //Read the config File
    while ((read = getline(&line, &len, fp)) != -1) {

        printf("Line: %s", line);

        //Tokenize Username and Passwd
        token = strtok(line, " ");

        strcpy(users[i].name, token);

        token = strtok(NULL, " ");
        strcpy(users[i].passwd,token);
        
        i++;

    }
      
    fclose(fp);
    if (line)
        free(line);
}

//Read the File
void request(int connfd) {

    char command[MAXLINE] = ""; 

    int n;

    //Read Command, Host and Keep-Alive
    readline(connfd, command, MAXLINE);

    // printf("Line 1: %s\n", command);

    // Check Command
    if (strncmp(command, "GET", 3) == 0) {
        getFile(command, connfd);
        // printf("GET CALLED!\n");
    } else if(n == -1) {
        //Intentionally Left Blank

    }else {
        printf("Unsupported Command: %s\n", command);
        return;
    }


    // printf("HTTP REQUEST PROCESSED!\n");

    // printf("Closing Connection: %d\n", connfd);
    close(connfd);
}

//Grab the Content Request by the Client
void getFile( char * file, int connfd) {
    // printf("%s\n", file);

   // char * token  = strtok(file, " ");
   // char * location;
   // char * version;
   // char path[1024] = {};
   // int fd;  //file descriptor
   // char * ext;
   // int i = 0;

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
