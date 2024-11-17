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

void readMessage(int sockfd, char *response, size_t size) {
    bzero(response, size);
    if (read(sockfd, response, size - 1) < 0) {
        perror("Error reading from socket");
        exit(EXIT_FAILURE);
    }
}

void closeSocket(int sockfd) {
    if (close(sockfd) < 0) {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}

void parsePASVResponse(char *response, char *ip, int *port) { // CHAT GPT nao sei oq é isto
    int h1, h2, h3, h4, p1, p2;
    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    *port = p1 * 256 + p2;
}

int main(int argc, char *argv[]) {

    char protocol[BUFFER_SIZE], user[BUFFER_SIZE], password[BUFFER_SIZE], host[BUFFER_SIZE], path[BUFFER_SIZE];
    char response[BUFFER_SIZE], passiveIp[BUFFER_SIZE];
    int ip;

    if (argc != 2 || parseURL(argv[1], protocol, user, password, host, path) != 0) {
        printf(stderr, "Write in format: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((ip = getIp(host)) != 0){
        printf("Error trying to get the IP address \n");
        exit(EXIT_FAILURE);
    }
    
    int control_sockfd;
    struct sockaddr_in server_addr;
    initServerAddress(&server_addr);
    control_sockfd = createSocket();
    connectToServer(control_sockfd, &server_addr);

    // Login
    char command[BUFFER_SIZE];
    snprintf(command, BUFFER_SIZE, "USER %s\n", user);
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    snprintf(command, BUFFER_SIZE, "PASS %s\n", password);
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    // Enter Passive Mode
    snprintf(command, BUFFER_SIZE, "PASV\n");
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    int passivePort;
    parsePASVResponse(response, passiveIp, &passivePort);
    
    int data_sockfd = createSocket();
    struct sockaddr_in data_addr;
    initServerAddress(&data_addr);
    data_addr.sin_port = htons(passivePort);
    connectToServer(data_sockfd, &data_addr);

    snprintf(command, BUFFER_SIZE, "RETR %s\n", path);
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    FILE *f = fopen("pipe.txt", "wb");
    if (!f) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(data_sockfd, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytesRead, f);
    }

    fclose(f);
    closeSocket(data_sockfd);

    // Close Control Connection
    snprintf(command, BUFFER_SIZE, "QUIT\n");
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    closeSocket(control_sockfd);

    printf("File downloaded successfully!\n");
    return 0;
}