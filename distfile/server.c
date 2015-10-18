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
void putFile();
void listFiles();
int checkUser();
void checkServer();
void checkFileCurrServ();
int requestFileCheck();

typedef struct user{
    char name[128];
    char passwd[128];
} user;

//Global Values
int port;
char serverDir[128] = ".";
user * users;
int connfd;  

int main(int argc, char **argv) {

    printf("Just getting started!\n");

    //Parse Arguments and Create Directories
    if(argc != 3){
        printf("Invalid Arguments!");
        return -1;
    }

    strcat(serverDir, argv[1]);

    strcat(serverDir, "/");

    mkdir(serverDir, 0770);

    port = atoi(argv[2]);

    //Listen at the specified locaiton
    int listenfd, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;

    //Read Config for users
    getConfig();

    listenfd = open_listenfd(port);


    //Call function to wait to process requests
    while (1) {

        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);         
        request(connfd);
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

        // strcpy(dir,serverDir);

        printf("Line: %s", line);

        //Tokenize Username and Passwd
        token = strtok(line, " ");

        strcpy(users[i].name, token);

        // strcat(dir, token);

        token = strtok(NULL, " ");
        strcpy(users[i].passwd,token);

        i++;

    }

    fclose(fp);
    if (line)
        free(line);
}

//Process Request: Check Command, Username and Password
void request(int connfd) {

    char command[MAXLINE] = ""; 
    char * token;
    char username[256];
    char passwd[256];
    int n;

    while(1) {

        //Read Command, Host and Keep-Alive
        readline(connfd, command, MAXLINE);

        // printf("Line: %s\n", command);

        //Grab UserName and Password
        token = strtok(command, " ");

        token = strtok(NULL, " ");
        if (token == NULL){
            write(connfd, "Invalid Username/Password. Please try again.\n", 45);            close(connfd);
            close(connfd);
            return;
        }
        strcpy(username, token);

        token = strtok(NULL, " ");
        if (token == NULL){
            write(connfd, "Invalid Username/Password. Please try again.\n", 45);
            close(connfd);
            return;
        }

        strcpy(passwd, token);

        token = strtok(NULL, " ");

        //TODO Check Username and Password

        // Check Command
        if (strncmp(command, "GET", 3) == 0) {
            //Token File Name
            printf("GET CALLED!\n");
            getFile(command, connfd);
        } else if(strncmp(command, "LIST", 4) == 0) {
            //List Call
            printf("LIST Called!\n");
            listFiles(username);
        } else if(strncmp(command, "PUT", 3) == 0){
            //Put Call
            printf("PUT Called!\n");

        } else if(strncmp(command, "CHECK", 5) == 0){
            //Put Call
            checkServer(token);
            printf("CHECK Called!\n");

        }else {

            printf("Unsupported Command: %s\n", command);
            close(connfd);
            return;

        }

    }

}

//LIST: LIST the possible files
void listFiles(char * username){

    printf("Getting Files for: %s\n", username);

    DIR * d;
    struct dirent *dir;
    struct stat filedets;
    int status;

    char path[PATH_MAX];
    char directory[MAXLINE];

    strcpy(directory, serverDir);
    strcat(directory, username);

    d = opendir(directory);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(".", dir->d_name) == 0){
                //Skip Current Dir
            } else if (strcmp("..", dir->d_name) == 0){
                //Skip Prev Dir
            } else {
                sprintf(path, "%s/%s", directory, dir->d_name);
                status = lstat(path, &filedets);
                if(S_ISDIR(filedets.st_mode)) {
                    //Skip Directories
                } else {
                    printf("%s\n", dir->d_name);
                    checkFileCurrServ(dir->d_name);
                    //TODO: Check for Pieces on Other Servers
                }
            }
        }

        closedir(d);
    } else {

        printf("Directory Doesn't Exist. Creating!\n");

        mkdir(directory, 0770);
    }
}


//GET: Grab the Content Request by the Client
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

//PUT: Read the file from the client and same to dir
void putFile(){

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

//Checks if a valid usersname and password supplied
int checkUser(char * username) {
    //TODO Implement This

    return 1;
}

void checkServer(char * filename){

    char currpart[256] = "";
    int fd;

    strcat(currpart, serverDir);
    strcat(currpart, filename);

    if (fd = open(filename, O_RDONLY) != -1){
        write(connfd, "1\n", 2);
        close(connfd);
    } else {
        write(connfd, "0\n", 2);
        close(connfd);
    }

}

int requestFileCheck(char * filename){

    //Connect to Each Server


    //1 sec timeout


    //If file check other servers or return 1 when found

    return 0;
}

void checkFileCurrServ(char * filename){

    // printf("File: %s\n", filename);
    
    char ext[8] = "";
    char filenopart[256] = "";
    char currpart[256] = "";
    char part[2] = "";

    int fd;

    strcpy(filenopart, strtok (filename, "."));

    strcpy(ext, strtok (NULL, "."));

    if (strtok (NULL, ".") != NULL){
        sprintf(filenopart, "%s.%s", filenopart, ext);
    } 

    // printf("Filename: %s\n",filenopart);

    int part1 = 0;
    int part2 = 0;
    int part3 = 0;
    int part4 = 0;
    int currport = port; 

    //Check Current Server for Part
    strcat(currpart, serverDir);
    strcat(currpart, filenopart);
    strcpy(filenopart, currpart);
    strcat(currpart, ".1");


    if ((fd = open(currpart, O_RDONLY) != -1)){
        part1 = 1;  
    } else {
        if (requestFileCheck(currpart)){
            part1 = 1;
        }
    }

    strcpy(currpart, filenopart);
    strcat(currpart, ".2");

    if (fd = open(currpart, O_RDONLY) != -1){
        part2 = 1;  
    } else {
         if (requestFileCheck(currpart)){
            part2 = 1;
        }
    }

    strcpy(currpart, filenopart);
    strcat(currpart, ".3");

    if ((fd = open(currpart, O_RDONLY) != -1)){
        part3 = 1;  
    } else {
         if (requestFileCheck(currpart)){
            part3 = 1;
        }
    }

    strcpy(currpart, filenopart);
    strcat(currpart, ".4");

    if ((fd = open(currpart, O_RDONLY) != -1)){
        part4 = 1;  
    } else {
         if (requestFileCheck(currpart)){
            part4 = 1;
        }
    }

    if ((part1+part2+part3+part4) == 4){
        write(connfd, filenopart, strlen(filenopart));
        write(connfd, "\n", 1);
    } else {
        write(connfd, filenopart, strlen(filenopart));
        write(connfd, " [incomplete]", 13);
        write(connfd, "\n", 1);
    }

}
