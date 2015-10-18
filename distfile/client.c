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

char userName[128];
char passwd[128];

typedef struct server{

    char host[20];
    int port;

} server;

server * servers; 


int main(int argc, char **argv) 
{
    int servfd1, servfd2, servfd3, servfd4;
    char *conf, buf[MAXLINE];
    int n;

    if (argc != 2) {
        fprintf(stderr, "Please Specify a Config File.");
        exit(0);
    }

    conf = argv[1];
    
    getConfig(conf);

    servfd1 = open_clientfd(servers[1].host, servers[1].port);
    servfd2 = open_clientfd(servers[2].host, servers[2].port);
    servfd3 = open_clientfd(servers[3].host, servers[3].port);
    servfd4 = open_clientfd(servers[4].host, servers[4].port);

    while (fgets(buf, MAXLINE, stdin) != NULL) {
        printf("The line was: %s\n", buf);
        processRequest(buf);
    }

    close(servfd1);
    close(servfd2);
    close(servfd3);
    close(servfd4);

    exit(0);
}

//Read the Configuration File
void getConfig(char * confile) {

    FILE * fp; 
    char * line = NULL;
    char * token;
    size_t len = 0;
    ssize_t read;
    int i=0;

    fp = fopen(confile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // printf("We are congfiguring\n");

    servers = malloc(NUM_SERVERS * sizeof(servers));

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
            servers[i].port= atoi(token);
        } else if(strcmp("Username", token) == 0){
            //Username
            token = strtok(NULL, " ");
            strcpy(userName, token);
        } else if(strcmp("Password", token) == 0){
            token = strtok(NULL, " ");
            strcpy(passwd, token);
            // printf("We found the Password!\n");
        }

        i++;
    }
      
    fclose(fp);
}

//Process a Request
void processRequest(char * buf){

    // Check Command
    if (strncmp(buf, "GET", 3) == 0) {
        //Token File Name
        printf("GET CALLED!\n");
    } else if(strncmp(buf, "LIST", 4) == 0) {
        //List Call

    } else if(strncmp(buf, "PUT", 3) == 0) {
        //Put Call
        //Token filename

    } else {
        printf("Unsupported Command: %s\n", buf);
        return;
    }

}

//LIST

//GET 

//PUT 

