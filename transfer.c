#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"
#define BUFFER_SIZE 1024

int getIp(char host[]){
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        return 1;
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

    return 0;
}

int parseURL(const char *url, char *protocol, char *user, char *password, char *host, char *path) {
    if (strchr(url, '@') != NULL) { // We have user and password
        int result = sscanf(url, "%[^:]://%[^:]:%[^@]@%[^/]%s", protocol, user, password, host, path);
        if (result < 5) {
            printf(stderr, "Error in credentials.\n");
            return 1;
        }
    } else { // Anonymmous url
        strcpy(user, "anonymous");
        strcpy(password, "anonymous");
        int result = sscanf(url, "%[^:]://%[^/]%s", protocol, host, path);
        if (result < 3) {
            printf(stderr, "Error in anonymous credentials.\n");
            return 1;
        }
    }
    return 0;
}

void initServerAddress(struct sockaddr_in *server_addr) {
    bzero((char *)server_addr, sizeof(struct sockaddr_in));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(SERVER_ADDR); 
    server_addr->sin_port = htons(SERVER_PORT);           
}

int createSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void connectToServer(int sockfd, struct sockaddr_in *server_addr) {
    if (connect(sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("connect()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void sendMessage(int sockfd, const char *message) {
    ssize_t bytes = write(sockfd, message, strlen(message));
    if (bytes > 0) {
        printf("Bytes escritos: %ld\n", bytes);
    } else {
        perror("write()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void closeSocket(int sockfd) {
    if (close(sockfd) < 0) {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char *argv[]) {

    char protocol[BUFFER_SIZE], user[BUFFER_SIZE], password[BUFFER_SIZE], host[BUFFER_SIZE], path[BUFFER_SIZE];

    if (argc != 2 || parseURL(argv[1], protocol, user, password, host, path) != 0) {
        printf(stderr, "Write in format: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (getIp(host) != 0){
        printf("Error trying to get the IP address \n");
        exit(EXIT_FAILURE);
    }
    
    int *sockfd;
    

}