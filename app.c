#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_ADDR "192.168.28.96"
#define SERVER_PORT 6000

typedef struct{
    char user[128];
    char password[128];
    char host[128];
    char path[128];
    char ip[128];
    char filename[256];
    char host_name[128];
} DATA;

int divideURL(char* link, DATA *url);
int getPathAndFileName(char *path, char *fileName);
int getIP(char *address, DATA *url);
int startSocket(int *sockt, char *ip, int port);
int readResponse(FILE *socketResp, char *response);
int sendCommand(int socket, char *command);
int getPort(FILE *socket, char *response, char *ip, int *port);
int saveFile(char *fileName, int socket, int fileSize);



int main(int argc, char **argv){

    DATA url;
    char url_copy[256];
    char command[256];
    char response[512];
    int socket;
    int socket_b;
    char ip[32];
    int port;
    

    strcpy(url_copy, argv[1]);

    if(divideURL(url_copy, &url) != 0){
        printf("Error: divideURL()\n");
        return -1;
    }

    printf("user:%s\npassword:%s\nhost:%s\npath:%s\nip:%s\nfilename:%s\nhost_name:%s\n", url.user, url.password, url.host, url.path, url.ip, url.filename, url.host_name);


    if(startSocket(&socket, url.ip, 21) != 0){
        printf("Error starting socket\n");
        return -1;
    }

    FILE *socketfile = fdopen(socket, "r");

    if(readResponse(socketfile, response) != 0){
        return -1;
    }

    sprintf(command, "user %s\n", url.user);

    if(sendCommand(socket, command) != 0){
        return -1;
    }

    if(readResponse(socketfile, response) != 0){
        return -1;
    }

    sprintf(command, "pass %s\n", url.password);

    if(sendCommand(socket, command) != 0){
        return -1;
    }

    if(readResponse(socketfile, response) != 0){
        return -1;
    }

    sprintf(command, "pasv\n");

    if(sendCommand(socket, command) != 0){
        return -1;
    }

    if(getPort(socketfile, response, ip, &port) != 0){
        return -1;
    }

    printf("ip: %s  port: %d\n", ip, port);

    if(startSocket(&socket_b, ip, port) != 0){
        printf("Error: startSocket()\n");
        return -1;
    }

    sprintf(command, "retr %s\n", url.path);

    if(sendCommand(socket, command) != 0){
        return -1;
    }

    if(readResponse(socketfile, response) != 0){
        return -1;
    }

    strtok(response, "(");
    char* size = strtok(NULL, " ");
    int sizeOfFile = atoi(size);

    if(saveFile(url.filename, socket_b, sizeOfFile) != 0){
        return -1;
    }

    sprintf(command, "quit");

    if(readResponse(socketfile, response) != 0){
        return -1;
    }

    if(sendCommand(socket, command) != 0){
        return -1;
    }

    if(close(socket) < 0){
        perror("close()");
        exit(-1);
    }

    if(close(socket_b) < 0){
        perror("close()");
        exit(-1);
    }

    return 0;

}

int divideURL(char* link, DATA *url){

    char *ftp;
    char *userpasshost;
    char *path;
    char *user;
    char *pass;
    char *host;

    ftp = strtok(link , "/");
    userpasshost = strtok(NULL, "/");
    path = strtok(NULL, "");

    strcpy(url->path, path);

    if(strcmp(ftp, "ftp:") != 0){
        printf("Error: Must be an ftp\n");
        return 1;
    }

    user = strtok(userpasshost, ":");
    pass = strtok(NULL, "@");

    if(pass == NULL){
        user = "anonymous";
        pass = "something";
        strcpy(url->host, userpasshost);
    }
    else{
        host = strtok(NULL, "");
        strcpy(url->host, host);
    }

    strcpy(url->user, user);
    strcpy(url->password, pass);

    if(getIP(url->host, url) != 0){
        printf("Error: couldn't get the IP address\n");
        return -1;
    }

    if(getPathAndFileName(url->path, url->filename) != 0){
        printf("Error: getPathAndFileName\n");
        return -1;
    }
    
    return 0;

}

int getIP(char *address, DATA *url){

    struct hostent *h;

    if((h = gethostbyname(address)) == NULL){

        printf("Error: goethostbyname()\n");
        return -1;
    }

    strcpy(url->ip, inet_ntoa(*((struct in_addr *)h->h_addr)));
    strcpy(url->host_name, h->h_name);

    return 0;
    
    
}

int getPathAndFileName(char *path, char *fileName)
{
    char strtoken[256];
    
    strcpy(strtoken, path);
    char *token = strtok(strtoken, "/");
    
    while (token != NULL)
    {
        strcpy(fileName, token);
        token = strtok(NULL, "/");
    }
    return 0;
}

int startSocket(int *sockt, char *ip, int port)
{

    struct sockaddr_in server_addr;
    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
    //switch byte order from host to network
    //and add port to tcp
    server_addr.sin_port = htons(port); 

    
    /*open a TCP socket*/
    if ((*sockt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: open TCP socket!\n");
        return -1;
    }
    /*connect to the server*/
    if (connect(*sockt,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        printf("Error: connect to the server!\n");
        return -1;
    }

    return 0;
}

int readResponse(FILE *socketResp, char *response){
    
    int nr_code;

    while(fgets(response, 512, socketResp)){

        printf("%s", response);

        if(response[3] == ' '){
            nr_code = atoi(response);
            if(nr_code == 530 || nr_code == 550){
                printf("Error: command\n");
                return -1;
            }
            if(nr_code == 501 || nr_code == 500){
                printf("Error: Syntax\n");
                return -1;
            }
            if(nr_code == 552){
                printf("Error: aborted\n");
            }
            break;
            
        }
    }

    return 0;
}

int sendCommand(int socket, char *command)
{
    size_t bytes = write(socket, command, strlen(command));
    if (bytes > 0)
    {
        printf("%s", command);
    }
    else
    {
        perror("write()");
        return -1;
    }

    return 0;
}

int getPort(FILE *socket, char *response, char *ip, int *port){

    while(fgets(response, 512, socket)){
        printf("%s", response);
        if(response[3] == ' '){
            break;
        }
    }

    strtok(response, "(");
    char *ip1, *ip2, *ip3, *ip4, *port1, *port2;

    ip1 = strtok(NULL, ",");
    ip2 = strtok(NULL, ",");
    ip3 = strtok(NULL, ",");
    ip4 = strtok(NULL, ",");

    port1 = strtok(NULL, ",");
    port2 = strtok(NULL, ")");

    sprintf(ip, "%s.%s.%s.%s", ip1, ip2, ip3, ip4);
    *port = atoi(port1) * 256 + atoi(port2);
    return 0;
}


int saveFile(char *fileName, int socket, int fileSize)
{
    static int currentSize = 0;
    int fileDescriptor = open(fileName, O_WRONLY | O_CREAT, 0777);
    if (fileDescriptor < 0)
    {
        perror("open file");
        return -1;
    }

    size_t bytes;
    char buffer[2048];

    do
    {
        bytes = read(socket, buffer, sizeof(buffer));
        if (bytes > 0)
        {
            currentSize += bytes;
            write(fileDescriptor, buffer, bytes);
        }
    } while (bytes != 0);

    close(fileDescriptor);

    return 0;
}