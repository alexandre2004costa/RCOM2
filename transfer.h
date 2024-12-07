#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 21
#define BUFFER_SIZE 1024
#define CloseConnection 226
#define UserIn 230
#define NeedPassword 331
#define FileOkay 150
#define FileOkay2 125
#define EnterPassive 227
#define LogIn 220
#define Bye 221

typedef enum {
    Initial,
    Line,
    Last,
    End,
    Numbers
} State;

int getIp(char host[], char* ip);

int parseURL(const char *url, char *protocol, char *user, char *password, char *host, char *path);

void initServerAddress(struct sockaddr_in *server_addr, char* ip);

int createSocket();

void connectToServer(int sockfd, struct sockaddr_in *server_addr);

void sendMessage(int sockfd, const char *message);

int readMessage(int sockfd, char *response, size_t size);

void closeSocket(int sockfd);

void parsePASVResponse(char *response, char *ip, int *port);

void extractFilename(const char *path, char *filename);