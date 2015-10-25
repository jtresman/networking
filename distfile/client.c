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

void getConfig();
void processRequest();
int verifyUser();
void putFile();
void listFiles();

char userName[128];
char passwd[128];

typedef struct server{

    char host[20];
    int port;
    int fd;

} server;

server * servers; 

int servfd1, servfd2, servfd3, servfd4;
int userVerified;

int main(int argc, char **argv){
    
    char *conf, buf[MAXLINE];

    if (argc != 2) {
        fprintf(stderr, "Please Specify a Config File.");
        exit(0);
    }

    conf = argv[1];
    
    getConfig(conf);

    servers[0].fd = open_clientfd(servers[0].host, servers[0].port);
    servers[1].fd = open_clientfd(servers[1].host, servers[1].port);
    servers[2].fd = open_clientfd(servers[2].host, servers[2].port);
    servers[3].fd = open_clientfd(servers[3].host, servers[3].port);

    // printf("Server 1: FD %d Host: %s Port: %d\n", servers[0].fd, servers[0].host, servers[0].port);
    // printf("Server 2: FD %d Host: %s Port: %d\n", servers[1].fd, servers[1].host, servers[1].port);
    // printf("Server 3: FD %d Host: %s Port: %d\n", servers[2].fd, servers[2].host, servers[2].port);
    // printf("Server 4: FD %d Host: %s Port: %d\n", servers[3].fd, servers[3].host, servers[3].port);

    printf("Please Enter A Command: \n");

    //TODO Find a Server that is avalible

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        // printf("The line was: %s\n", buf);
        processRequest(buf);
        printf("Please Enter a Command: \n");
    }

    close(servers[0].fd);
    close(servers[1].fd);
    close(servers[2].fd);
    close(servers[3].fd);

    exit(0);
}

//Read the Configuration File
void getConfig(char * confile){

    FILE * fp; 
    char * line;
    char * token;
    size_t len = 0;
    ssize_t read;
    int i=0;
    userVerified = 0;

    servers = malloc(NUM_SERVERS * sizeof(server));


    fp = fopen(confile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // printf("We are congfiguring\n");

    //Read the config File
    while ((read = getline(&line, &len, fp)) != -1) {

        // printf("Line: %s", line);

        //Tokenize Hostname, Port, Username and Password
        token = strtok(line, " ");

        if (strcmp("Server", token) == 0){
            token = strtok(NULL, ":");
            // printf("Host: %s\n", token);
            strcpy(servers[i].host,token);
            token = strtok(NULL, " ");
            // printf("Port: %s\n", token);
            servers[i].port = atoi(token);
        } else if(strcmp("Username", token) == 0){
            //Username
            token = strtok(NULL, "\n");
            strcpy(userName, token);
        } else if(strcmp("Password", token) == 0){
            token = strtok(NULL, "\n");
            strcpy(passwd, token);
            // printf("We found the Password!\n");
        }

        i++;
    }

    //Verify that the user is Valid      
    userVerified = verifyUser();

    fclose(fp);
}

//Process a Request
void processRequest(char * buf){

    char * token;

    // Check Command
    if (strncmp(buf, "GET", 3) == 0) {
        //Token File Name
        printf("GET CALLED!\n");
        //Call Get Function

    } else if(strncmp(buf, "LIST", 4) == 0) {
        //List Call
        listFiles();
    } else if((strncmp(buf, "PUT", 3) == 0)) {
        printf("PUT Called!\n");
        if (userVerified){
            strtok(buf, " ");
            token = strtok(NULL, "\n");
            putFile(token);
        } else {
            printf("User not Verified! Please Enter the Corret Credentials!\n");
        }

    } else {
        printf("Unsupported Command: %s\n", buf);
        return;
    }
}

//LIST
void listFiles(){

    char command[MAXLINE];
    char buf[MAXLINE];

    int i = 0;

    sprintf(command, "LIST %s %s\n", userName, passwd);

    //TODO Try All Servers Incase one is down
    write(servers[i].fd, command, strlen(command));

    while(readline(servers[i].fd, buf, MAXLINE)){

        // printf("Line %s\n", buf );

        if(strncmp(buf, "Done", 4) == 0){
            // printf("Done Reading!\n");
            break;
        } else {
            printf("%s", buf);
        }
    }
}

//GET 

//PUT 
void putFile(char * filename){

    int fd;
    long int len;
    int part, finalpart;
    struct stat fileStat; 

    char command[MAXLINE];
    char path[MAXLINE];
    char strLength[BUFSIZ];
    char partlen[MAXLINE];

    sprintf(command, "PUT %s %s %s.1\n", userName, passwd, filename);

    // printf("Command: %s\n", command );

    //Location Each Part is going
    int p1a,p1b,p2a,p2b,p3a,p3b,p4a,p4b;

    //Add Path to File Name
    strcat(path, "./");
    strcat(path, filename);

    // printf("File Name: %s \n", path );

    //Open File for Reading
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file!\n");
        exit(1);
    }

    //Split File Into Parts
    fstat(fd, &fileStat);

    sprintf(strLength, "%d\n", fileStat.st_size);

    len = atoi(strLength);

    if(len>NUM_SERVERS){
        part = len/NUM_SERVERS;
    }


    finalpart = (len - (part*NUM_SERVERS)) + part;

    sprintf(partlen, "%d\n", part);
    
    unsigned char buf[part];

    int nread = read(fd, buf, part);
    
    //TODO Determine Mapping for File Part to Server

    //Write Part 1
    if(nread > 0) {

        p1a = 0;
        p1b = 3;

        // printf("Sending: %s %d\n", command, servers[p1a].fd);
        write(servers[p1a].fd, command, strlen(command));
        write(servers[p1a].fd, partlen, strlen(partlen));
        write(servers[p1a].fd, buf, nread);
        write(servers[p1b].fd, command, strlen(command));
        write(servers[p1b].fd, partlen, strlen(partlen));
        write(servers[p1b].fd, buf, nread);
    }

    //Write Part 2
    sprintf(command, "PUT %s %s %s.2\n", userName, passwd, filename);
    nread = read(fd, buf, part);
    if(nread > 0) {

        p2a = 0;
        p2b = 1;

        printf("Sending: %s %d %d\n", command, servers[p2b].fd, nread);
        write(servers[p2a].fd, command, strlen(command));
        write(servers[p2a].fd, partlen, strlen(partlen));
        write(servers[p2a].fd, buf, nread);
        write(servers[p2b].fd, command, strlen(command));
        write(servers[p2b].fd, partlen, strlen(partlen));
        write(servers[p2b].fd, buf, nread);
    }


    //Write Part 3
    sprintf(command, "PUT %s %s %s.3\n", userName, passwd, filename);
    nread = read(fd, buf, part);
    if(nread > 0) {

        p3a = 1;
        p3b = 2;

        // printf("Sending: %s %d\n", command, servers[p1a].fd);
        write(servers[p3a].fd, command, strlen(command));
        write(servers[p3a].fd, partlen, strlen(partlen));
        write(servers[p3a].fd, buf, nread);
        write(servers[p3b].fd, command, strlen(command));
        write(servers[p3b].fd, partlen, strlen(partlen));
        write(servers[p3b].fd, buf, nread);
    }


    //Write Part 4
    sprintf(command, "PUT %s %s %s.4\n", userName, passwd, filename);
    nread = read(fd, buf, finalpart);
    sprintf(partlen, "%d\n", finalpart);

    if(nread > 0) {

        p4a = 2;
        p4b = 3;

        // printf("Sending: %s %d\n", command, servers[p1a].fd);
        write(servers[p4a].fd, command, strlen(command));
        write(servers[p4a].fd, partlen, strlen(partlen));
        write(servers[p4a].fd, buf, nread);
        write(servers[p4b].fd, command, strlen(command));
        write(servers[p4b].fd, partlen, strlen(partlen));
        write(servers[p4b].fd, buf, nread);
    }


    // write(s.fd, "\0", 1);
    close(fd);

    printf("Finished PUTing\n");
}

//Verify the Username is Corret
int verifyUser(){

    int i = 0;

    int servfd = 0;

    char command[MAXLINE];
    char res[3];

    sprintf(command, "VERIFY %s %s\n", userName, passwd);


    printf("Verify User! %s %s\n", userName, passwd );

    while ((servfd = open_clientfd("localhost", servers[i].port)) < 0 && i<NUM_SERVERS){

        i++;

    }

    if (servfd < 0){
        write(servfd, command, strlen(command));

        readline(servfd, res, 2);

        printf("We read: %s %d\n", res, strncmp(res, "1", 1));

        if(strncmp(res, "1", 1) == 0){
            printf("User Verified!\n");
            return 1;
        } 
    } else {
        printf("We could not connect to any server to verify user!\n");
    }

    close(servfd);

    return 0;
}
