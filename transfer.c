#include "transfer.h"

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
            printf("Error in credentials.\n");
            return 1;
        }
    } else { // Anonymmous url
        strcpy(user, "anonymous");
        strcpy(password, "anonymous");
        int result = sscanf(url, "%[^:]://%[^/]%s", protocol, host, path);
        if (result < 3) {
            printf("Error in anonymous credentials.\n");
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

int readMessage(int sockfd, char *response, size_t size) {
    bzero(response, size); 
    char buffer[1];
    int pos = 0;
    State state = Initial;
    while (state != End) {
        ssize_t bytesRead = read(sockfd, buffer, 1);

        //printf("Received byte: %c (0x%02X)\n", buffer[0], (unsigned char)buffer[0]);


        if (bytesRead < 0) {
            printf("Error reading from socket\n");
            exit(EXIT_FAILURE);
            return 1;
        }
        
        response[pos++] = buffer[0];

        switch (buffer[0])
        {
            case ' ':
                if (state == Numbers) state = Last;
                else if (state == Initial) state = Line;
                break;

            case '-':
                if(state == Numbers) state = Line;
                break;
            
            case '\n':
                if (state == Last || state == Initial) state = End;
                else state = Initial;
            
            default:
                if (state == Initial && buffer[0] >= '0' && buffer[0] <= '9') state = Numbers;
                break;
        }        
       

    }

    int statusCode = 0;
    sscanf(response, "%3d", &statusCode);
    printf("Response: %s\n", response);
    return statusCode;
}

void closeSocket(int sockfd) {
    if (close(sockfd) < 0) {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}

void parsePASVResponse(char *response, char *ip, int *port) {
    int h1, h2, h3, h4, p1, p2;
    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2);
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    *port = p1 * 256 + p2;
}

void extractFilename(const char *path, char *filename) {
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        strcpy(filename, lastSlash + 1);
    } else {
        strcpy(filename, path);
    }
}

int main(int argc, char *argv[]) {

    char protocol[BUFFER_SIZE], user[BUFFER_SIZE], password[BUFFER_SIZE], host[BUFFER_SIZE], path[BUFFER_SIZE];
    char response[BUFFER_SIZE], passiveIp[BUFFER_SIZE];
    char *ip = malloc(INET_ADDRSTRLEN * sizeof(char));

    if (argc != 2 || parseURL(argv[1], protocol, user, password, host, path) != 0) {
        printf("Write in format: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
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

    connectToServer(control_sockfd, &server_addr);
    printf("Connected to server !\n");

    
    if (readMessage(control_sockfd, response, BUFFER_SIZE) != LogIn){
        printf("Error trying to log in the server. \n");
        exit(EXIT_FAILURE);
    }

    // Login
    char command[BUFFER_SIZE];
    snprintf(command, BUFFER_SIZE + 10, "user %s\r\n", user);
    sendMessage(control_sockfd, command);

    
    if (readMessage(control_sockfd, response, BUFFER_SIZE) != NeedPassword){
        printf("Error trying to send user name. \n");
        exit(EXIT_FAILURE);
    }


    snprintf(command, BUFFER_SIZE + 10, "pass %s\r\n", password);
    sendMessage(control_sockfd, command);

    
    if (readMessage(control_sockfd, response, BUFFER_SIZE) != UserIn){
        printf("Error trying to send password. \n");
        exit(EXIT_FAILURE);
    }

    // Enter passive mode
    snprintf(command, BUFFER_SIZE, "pasv\r\n");
    sendMessage(control_sockfd, command);
    
    
    if (readMessage(control_sockfd, response, BUFFER_SIZE) != EnterPassive){
        printf("Error trying to enter passive mode. \n");
        exit(EXIT_FAILURE);
    }

    int newPort;
    parsePASVResponse(response, passiveIp, &newPort);
    
    int data_sockfd = createSocket();
    struct sockaddr_in data_addr;
    initServerAddress(&data_addr, ip);
    data_addr.sin_port = htons(newPort);
    connectToServer(data_sockfd, &data_addr);

    snprintf(command, BUFFER_SIZE + 10, "retr %s\r\n", path);
    sendMessage(control_sockfd, command);

    
    int control = readMessage(control_sockfd, response, BUFFER_SIZE);
    if ( control != FileOkay && control != FileOkay2){
        printf("Error trying to start receiving file. \n");
        exit(EXIT_FAILURE);
    }

    char filename[256];
    extractFilename(path, filename);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;
    while ((bytesRead = read(data_sockfd, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytesRead, f);
    }

    fclose(f);
    
    if (readMessage(control_sockfd, response, BUFFER_SIZE) != CloseConnection){
        printf("Error in closing connection. \n");
        exit(EXIT_FAILURE);
    }

    closeSocket(data_sockfd);

    // Close control connection
    snprintf(command, BUFFER_SIZE, "quit\r\n");
    sendMessage(control_sockfd, command);
    if (readMessage(control_sockfd, response, BUFFER_SIZE)!= Bye){
        printf("Error trying to end. \n");
        exit(EXIT_FAILURE);
    }
    closeSocket(control_sockfd);

    printf("File downloaded successfully!\n");
    free(ip);

    return 0;
}