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
int serverPort;
char serverDir[128] = ".";
user * users;
int numUsers;
user currUser;

char files[20][128];

int main(int argc, char **argv) {

    printf("Just getting started!\n");

    pid_t childPID;

    //Parse Arguments and Create Directories
    if(argc != 3){
        printf("Invalid Arguments!");
        return -1;
    }

    strcat(serverDir, argv[1]);

    strcat(serverDir, "/");

    mkdir(serverDir, 0770);

    serverPort = atoi(argv[2]);

    //Listen at the specified locaiton
    int listenfd, clientlen=sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;

    //Read Config for users
    getConfig();

    listenfd = open_listenfd(serverPort);


    //Call function to wait to process requests
    while (1) {

        int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        // printf("Connected! %d\n", serverPort );
        childPID = fork();
        
        if (childPID == 0){
            request(connfd);    
            exit(0);        
        }  
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
    int num_users = 0;
    int i=0;

    fp = fopen(SERVER_CONF, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // printf("We are congfiguring\n");

    //Grab number of users
    getline(&line, &len, fp);

    num_users = atoi(line);

    numUsers = num_users;

    users = malloc(num_users * sizeof(user));

    // printf("%d\n", num_users);

    //Read the config File
    while ((read = getline(&line, &len, fp)) != -1) {

        // strcpy(dir,serverDir);

        // printf("Line: %s\n", line);

        //Tokenize Username and Passwd
        token = strtok(line, " ");

        strcpy(users[i].name, token);

        // strcat(dir, token);

        token = strtok(NULL, " ");
        strcpy(users[i].passwd,token);

        // printf("Name: %s Password: %s\n",users[i].name, users[i].passwd);

        i++;

    }

    fclose(fp);
    if (line)
        free(line);
}

//Process Request: Check Command, Username and Password
//Requests must be in format: Command Username Password File
void request(int connfd) {

    char command[MAXLINE] = ""; 
    char * token;
    char username[256];
    char passwd[256];

    while(1) {

        // printf("Read a line!\n");

        //Read Command, Host and Keep-Alive
        readline(connfd, command, MAXLINE);

        // printf("Line: %s\n", command);

        //Grab UserName and Password
        token = strtok(command, " ");

        token = strtok(NULL, " ");
        if (token == NULL){
            write(connfd, "Invalid Command Format. Please try again.\n", 42);    
            close(connfd);
            return;
        }
        strcpy(username, token);

        token = strtok(NULL, " ");
        if (token == NULL){
            write(connfd, "Invalid Command Format. Please try again.\n", 42);
            close(connfd);
            return;
        }

        strcpy(passwd, token);

        token = strtok(NULL, "\n");

        //Check Username and Password
        if (checkUser(username, passwd) == 0){
            write(connfd, "Invalid Username/Password. Please try again.\n", 45);
            close(connfd);
            return;
        } else if(strncmp(command, "VERIFY", 6) == 0) {
            write(connfd, "1\n", 2);
            return;
        }

        // Check Command
        if (strncmp(command, "GET", 3) == 0) {
            //Token File Name
            printf("GET CALLED!\n");
            getFile(command, connfd);
        } else if(strncmp(command, "LIST", 4) == 0) {
            //List Call
            printf("LIST Called!\n");
            listFiles(username, connfd);
        } else if(strncmp(command, "PUT", 3) == 0){
            //Put Call
            printf("PUT Called!\n");
            putFile(token, connfd);
        } else if(strncmp(command, "CHECK", 5) == 0){
            //Put Call
            checkServer(token, connfd);
            printf("CHECK Called!\n");

        } else if(strncmp(command, "VERIFY", 6) == 0){
            printf("VERIFY Called!\n");

        } else {
            printf("Unsupported Command: %s\n", command);
            close(connfd);
            return;
        }

    }
}

//LIST: LIST the possible files
void listFiles(char * username, int connfd){

    printf("Getting Files for: %s\n", username);

    DIR * d;
    struct dirent *dir;
    struct stat filedets;

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
                lstat(path, &filedets);
                if(S_ISDIR(filedets.st_mode)) {
                    //Skip Directories
                } else {
                    // printf("%s\n", dir->d_name);
                    checkFileCurrServ(dir->d_name, connfd);
                }
            }
        }

        write(connfd, "Done\n", 5);

        closedir(d);


    } 

    printf("List Done!\n");
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
void putFile(char * filename, int connfd){

    char buf[MAXLINE] = "";

    char path[PATH_MAX];

    //Add Path onto File Name
    sprintf(path, "%s%s/%s", serverDir, currUser.name, filename);

    printf("Path: %s\n", path );

    //Open File for Writing
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    readline(connfd, buf, 15);

    int len = atoi(buf);

    // printf("File Len: %d\n", len);

    //Keep reading lines from client
    read(connfd, buf, len);

    //Write Line to File
    fprintf(f, "%s", buf);

    // }

    //Close File
    fclose(f);
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
    write(connfd, "\n", 1);
    close(fd);
}

//Checks if a valid usersname and password supplied
int checkUser(char * username, char * password) {

    int i;
    DIR * d;

    char directory[MAXLINE];

    strcpy(directory, serverDir);
    strcat(directory, username);

    if (strtok(password, "\n") != NULL){
        // printf("We Must Strip!\n");
        password = strtok(password, "\n"); 
    }

    // printf("IName: %s IPass: %s\n", username, password );

    for (i = 0; i < numUsers; i++){
        // printf("Name: %s Pass: %s\n", users[i].name, users[i].passwd);
        if (strncmp(users[i].name, username, strlen(users[i].name)) == 0 
                && strncmp(users[i].passwd, password, strlen(users[i].passwd)) == 0){
            currUser = users[i];
            // printf("We found the correct user!\n");
            d = opendir(directory);

            if(!d) {

                printf("Directory Doesn't Exist. Creating!\n");
                // write(connfd, "Directory Doesn't Exist. Creating!\n", 35);

                mkdir(directory, 0770);
            }

            return 1;
        }
    }

    return 0;
}

//Check for another server
void checkServer(char * filename, int connfd){

    char currpart[256] = "";
    int fd;

    // printf("Fil: %s\n", filename );

    sprintf(currpart, "%s%s/", serverDir, currUser.name);
    strcat(currpart, filename);

    strcpy(currpart, strtok(currpart, "\n"));

    // printf("File: %s FD: %d\n", currpart, access( currpart, F_OK ));

    if ((fd = access( currpart, F_OK )) != -1){
        // printf("We found the file!\n");
        write(connfd, "1\n", 2);
        close(connfd);
    } else {
        // printf("We didn't find the file!\n");
        write(connfd, "0\n", 2);
        close(connfd);
    }
}

//Request a Check on another Server
int requestFileCheck(char * filename){

    //Connect to Each Servers
    // char host[9] = "localhost";
    static int currport = 10001;
    char res[2];
    int servfd = 0;

    char command[MAXLINE];

    sprintf(command, "CHECK %s %s %s\n", currUser.name, currUser.passwd, filename);

    // printf("Command RFC: %s\n", command);

    for(currport = 10001; currport<10005; currport++){

        // printf("Port: %d\n", currport);    

        if (currport == serverPort){

        } else {
            //Connect to Other Server
            // printf("Trying to connect to server: %d\n", currport);

            servfd = open_clientfd("localhost", currport);

            //TODO 1 sec timeout

            write(servfd, command, strlen(command));
            readline(servfd, res, 2);

            // printf("We read: %s %d\n", res, strncmp(res, "1", 1));

            if(strncmp(res, "1", 1) == 0){
                // printf("WE FOUND IT!\n");
                return 1;
            }
            close(servfd);
        }
    }

    return 0;
}

//Check the Current Server for the File
//Request Check on other Server if not found
void checkFileCurrServ(char * filename, int connfd){

    // printf("File: %s\n", filename);

    char ext[8] = "";
    char filenopart[256] = "";
    char currpart[256] = "";
    char path[256] = "";

    int fd;

    strcpy(filenopart, strtok (filename, "."));

    strcpy(ext, strtok (NULL, "."));

    if (strtok (NULL, ".") != NULL){
        sprintf(filenopart, "%s.%s", filenopart, ext);
    } 

    //TODO Check to See if we Already Saw that File

    printf("Filename: %s\n",filenopart);

    int part1 = 0;
    int part2 = 0;
    int part3 = 0;
    int part4 = 0;

    //Check Current Server for Part
    sprintf(path, "%s%s/", serverDir, currUser.name);

    strcpy(currpart, filenopart);
    strcat(currpart, ".1");

    strcat(path, currpart);

    // printf("Path: %s FD: %d\n", path, access( path, F_OK ));

    if ((fd = open(path, O_RDONLY)) != -1){
        part1 = 1;  
        // printf("CWe found part1\n");

    } else {
        if (requestFileCheck(currpart)){
            // printf("SWe found part1\n");
            part1 = 1;
        }
    }
    close(fd);

    sprintf(path, "%s%s/", serverDir, currUser.name);

    strcpy(currpart, filenopart);
    strcat(currpart, ".2");

    strcat(path, currpart);

    if ((fd = open(path, O_RDONLY)) != -1){
        part2 = 1;  
        // printf("CWe found part3\n");
    } else {
        if (requestFileCheck(currpart)){
            part2 = 1;
            // printf("SWe found part3\n");
        }
    }
    close(fd);


    sprintf(path, "%s%s/", serverDir, currUser.name);

    strcpy(currpart, filenopart);
    strcat(currpart, ".3");

    strcat(path, currpart);

    if ((fd = open(path, O_RDONLY) != -1)){
        part3 = 1;  
        // printf("CWe found part3\n");

    } else {
        if (requestFileCheck(currpart)){
            part3 = 1;
            // printf("SWe found part3\n");

        }
    }
    close(fd);

    sprintf(path, "%s%s/", serverDir, currUser.name);

    strcpy(currpart, filenopart);
    strcat(currpart, ".4");

    strcat(path, currpart);

    if ((fd = open(path, O_RDONLY) != -1)){
        part4 = 1;  
        // printf("CWe found part4\n");

    } else {
        if (requestFileCheck(currpart)){
            part4 = 1;
            // printf("SWe found part4\n");
        }
    }
    close(fd);


    //Add File to Check List
    if ((part1+part2+part3+part4) == 4){
        printf("File Found!: %s\n", filenopart );
        write(connfd, filenopart, strlen(filenopart));
        write(connfd, "\n", 1);
    } else {
        write(connfd, filenopart, strlen(filenopart));
        write(connfd, " [incomplete]", 13);
        write(connfd, "\n", 1);
    }

}
