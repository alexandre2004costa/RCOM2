#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 21
#define BUFFER_SIZE 1024

int getIp(char host[], char* ip){
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        return 1;
    }

    strcpy(ip, inet_ntoa(*((struct in_addr *) h->h_addr)));
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));

    return 0;
}

int parseURL(const char *url, char *protocol, char *user, char *password, char *host, char *path) {
    if (strchr(url, '@') != NULL) { // We have user and password
        int result = sscanf(url, "%[^:]://%[^:]:%[^@]@%[^/]/%s", protocol, user, password, host, path);
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
    printf("protocol : %s\n", protocol);
    printf("user: %s\n", user);
    printf("password: %s\n", password);
    printf("host: %s\n", host);
    printf("path: %s\n", path);
    return 0;
}

void initServerAddress(struct sockaddr_in *server_addr, char* ip) {
    bzero((char *)server_addr, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = inet_addr(ip); 
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
    printf("AQUI\n");
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
        printf("SENDING: %s", message);
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
    char *ip = malloc(INET_ADDRSTRLEN * sizeof(char));

    if (argc != 2 || parseURL(argv[1], protocol, user, password, host, path) != 0) {
        printf(stderr, "Write in format: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((getIp(host, ip)) != 0) {
        printf("Error trying to get the IP address \n");
        exit(EXIT_FAILURE);
    }
    
    int control_sockfd;
    struct sockaddr_in server_addr;
    initServerAddress(&server_addr, ip); 
    control_sockfd = createSocket();
    printf("%i\n", control_sockfd);
    printf("Socket file descriptor: %d\n", control_sockfd);
    printf("Server IP: %s\n", inet_ntoa(server_addr.sin_addr)); // Converte o endereço para string
    printf("Server Port: %d\n", ntohs(server_addr.sin_port));   // Converte porta para formato legível
    connectToServer(control_sockfd, &server_addr);
    printf("Connected to server !\n");

    sleep(1);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    // Login
    char command[BUFFER_SIZE];
    snprintf(command, BUFFER_SIZE, "user %s\n", user);
    sendMessage(control_sockfd, command);

    sleep(1);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);


    snprintf(command, BUFFER_SIZE, "pass %s\n", password);
    sendMessage(control_sockfd, command);

    sleep(1);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    // Enter passive mode
    snprintf(command, BUFFER_SIZE, "pasv\n");
    sendMessage(control_sockfd, command);
    
    sleep(1);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    int newPort;
    parsePASVResponse(response, passiveIp, &newPort);
    
    int data_sockfd = createSocket();
    struct sockaddr_in data_addr;
    initServerAddress(&data_addr, ip);
    data_addr.sin_port = htons(newPort);
    connectToServer(data_sockfd, &data_addr);

    snprintf(command, BUFFER_SIZE, "retr %s\n", path);
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    FILE *f = fopen("result", "wb");
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

    // Close control connection
    snprintf(command, BUFFER_SIZE, "quit\n");
    sendMessage(control_sockfd, command);
    readMessage(control_sockfd, response, BUFFER_SIZE);
    printf("Response: %s\n", response);

    closeSocket(control_sockfd);

    printf("File downloaded successfully!\n");
    free(ip);

    return 0;
}