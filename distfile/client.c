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
void writeFile();
int md5Hash();
void getFile();

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

    printf("Please Enter A Command: \n");

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

    char token[64] = "";

    // Check Command
    if (strncmp(buf, "GET", 3) == 0) {
        //Token File Name
        printf("GET CALLED!\n");
        //Call Get Function
        strtok(buf, " ");
        strcpy(token, strtok(NULL, "\n"));
        getFile(token);
    } else if(strncmp(buf, "LIST", 4) == 0) {
        //List Call
        listFiles();
    } else if((strncmp(buf, "PUT", 3) == 0)) {
        printf("PUT Called!\n");
        if (userVerified){
            strtok(buf, " ");
            strcpy(token, strtok(NULL, "\n"));
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
    int servfd;

    sprintf(command, "LIST %s %s\n", userName, passwd);

    //Find an avalible Server
    while ((servfd = open_clientfd("localhost", servers[i].port)) < 0 && i<NUM_SERVERS){
        i++;
    }

    if (servfd < 0){
        printf("No Servers avalible!\n");
        return;
    }

    write(servfd, command, strlen(command));

    while(readline(servfd, buf, MAXLINE)){

        // printf("Line %s\n", buf );

        if(strncmp(buf, "Done", 4) == 0){
            // printf("Done Reading!\n");
            break;
        } else if(strncmp(buf, "Error", 5) == 0){
            printf("%s\n", buf);
            break;
        } else {
            printf("%s", buf);
        }
    }

    close(servfd);
}

//GET 
void getFile(char * filename){

    unsigned char * buf;
    char path[PATH_MAX];
    char command[MAXLINE];

    int fd, i, len;
    int servfd = -1;

    //Add Path onto File Name
    strcat(path, "./");
    strcat(path, filename);


    //Send Command to Reposive Server
    while ((servfd = open_clientfd("localhost", servers[i].port)) < 0 && i<NUM_SERVERS){
        i++;
    }

    if (servfd < 0){
        printf("No Servers avalible!\n");
        exit(0);
    }

    sprintf(command, "GET %s %s %s\n", userName, passwd, filename);

    write(servers[i].fd, command, strlen(command));

    //Open File for Writing
    fd = open(filename, O_RDWR | O_CREAT, S_IRWXU);
    if (fd == -1) {
        printf("Error opening file!\n");
        exit(1);
    } 

    buf = (unsigned char *)malloc(sizeof(unsigned char)*15);

    //Read the File Length
    readline(servers[i].fd, buf, MAXLINE);
    if(strncmp(buf, "Error", 5) == 0){
        printf("%s", buf);
        free(buf);
        close(fd);
        return;
    }

    len = atoi(buf);

    //Read File from server
    buf = (unsigned char *) realloc(buf, sizeof(unsigned char)*len);

    read(servers[i].fd, buf, len);

    if(strncmp(buf, "Error", 5) == 0){
        printf("%s", buf);
        free(buf);
        close(fd);
        return;
    }

    //Write Line to File
    write(fd, buf, len);

    //Close File
    close(fd);
}

//PUT 
void putFile(char * filename){

    int fd;
    long int len;
    int part, finalpart;
    struct stat fileStat; 

    char command[MAXLINE] = "";
    char path[MAXLINE] = "";
    char strLength[BUFSIZ] = "";

    //Add Path to File Name
    strcat(path, "./");
    strcat(path, filename);

    printf("File Name: %s \n", path );

    //Open File for Reading
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file!\n");
        return;
    }

    //Split File Into Parts
    fstat(fd, &fileStat);

    sprintf(strLength, "%d\n", fileStat.st_size);

    len = atoi(strLength);

    if(len>NUM_SERVERS){
        part = len/NUM_SERVERS;
    }

    finalpart = (len - (part*NUM_SERVERS)) + part;

    //Determine Mapping for File Part to Server

    //Inital Location Each Part is going
    long int adjust = 0;

    adjust = md5Hash(filename);

    // printf("Sum: %d\n", adjust );

    adjust = adjust % 4;

    // printf("Adjust: %d\n", adjust );

    int p1a = (0+adjust) % 4;
    int p1b = (3+adjust) % 4;

    int p2a = (0+adjust) % 4;
    int p2b = (1+adjust) % 4;
    
    int p3a = (1+adjust) % 4;
    int p3b = (2+adjust) % 4;

    int p4a = (2+adjust) % 4;
    int p4b = (3+adjust) % 4;


    close(fd);

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file!\n");
        return;
    }

    //Write Part 1
    sprintf(command, "PUT %s %s %s.1\n", userName, passwd, filename);
    writeFile(p1a,p1b,fd,part,command);

    //Write Part 2
    sprintf(command, "PUT %s %s %s.2\n", userName, passwd, filename);
    writeFile(p2a,p2b,fd,part,command);

    //Write Part 3
    sprintf(command, "PUT %s %s %s.3\n", userName, passwd, filename);
    writeFile(p3a,p3b,fd,part,command);

    //Write Part 4
    sprintf(command, "PUT %s %s %s.4\n", userName, passwd, filename);
    writeFile(p4a,p4b,fd,finalpart,command);

    close(fd);

    // printf("Finished PUTing\n");
}

//Calcualte the MD5 Hash of the File
int md5Hash(char * filename){

    unsigned char c[15];

    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0){
        MD5_Update (&mdContext, data, bytes);
    }

    MD5_Final (c,&mdContext);
   
    fclose (inFile);

    return c[15];
}

//Write the File to the server
void writeFile(int ser1, int ser2, int fd, int part, char * command){
    int nread;

    char partlen[MAXLINE];
    char error[MAXLINE];

    unsigned char buf[part];

    sprintf(partlen, "%d\n", part);

    nread = read(fd, buf, part);
    if(nread > 0) {

        // printf("Sending: %s %d %d\n", command, servers[p2b].fd, nread);
        write(servers[ser1].fd, command, strlen(command));
        readline(servers[ser1].fd, error, MAXLINE);
        if(strncmp(error, "Error", 5) == 0){
            printf("%s", error);
            free(buf);
            return;
        } 
        write(servers[ser1].fd, partlen, strlen(partlen));
        write(servers[ser1].fd, buf, nread);
         
        write(servers[ser2].fd, command, strlen(command));
        readline(servers[ser2].fd, error, MAXLINE);
        if(strncmp(error, "Error", 5) == 0){
            printf("%s", error);
            free(buf);
            return;
        } 
        write(servers[ser2].fd, partlen, strlen(partlen));
        write(servers[ser2].fd, buf, nread);
    }
}

//Verify the Username is Corret
int verifyUser(){

    int i = 0;

    int servfd = 0;

    char command[MAXLINE];
    char res[3];

    sprintf(command, "VERIFY %s %s\n", userName, passwd);


    printf("Verifing User! %s %s\n", userName, passwd );

    while ((servfd = open_clientfd("localhost", servers[i].port)) < 0 && i<NUM_SERVERS){

        i++;

    }

    if (servfd > 0){
        write(servfd, command, strlen(command));

        readline(servfd, res, 2);

        // printf("We read: %s %d\n", res, strncmp(res, "1", 1));

        if(strncmp(res, "1", 1) == 0){
            printf("User Verified!\n");
            return 1;
        }  else {

            printf("User Not Verified! Please Check Your Credentials\n");

        }
    } else {
        printf("We could not connect to any server to verify user!\n");
        exit(0);
    }

    close(servfd);

    return 0;
}
