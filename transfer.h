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

/**
 * getIp - Resolves a hostname to its corresponding IP address.
 * @host: The hostname.
 * @ip: A buffer to store the IP address.
 * Returns 0 on success, or 1 on failure.
 */
int getIp(char host[], char* ip);

/**
 * parseURL - Parses a URL and extracts its main components.
 * @url: The full URL.
 * @protocol: Buffer to store the protocol.
 * @user: Buffer to store the username.
 * @password: Buffer to store the password.
 * @host: Buffer to store the hostname.
 * @path: Buffer to store the file path.
 * Returns 0 on success, or 1 if the URL is malformed.
 */
int parseURL(const char *url, char *protocol, char *user, char *password, char *host, char *path);

/**
 * initServerAddress - Initializes a sockaddr_in structure with server details.
 * @server_addr: Pointer to the sockaddr_in structure to initialize.
 * @ip: The server's IP address.
 */
void initServerAddress(struct sockaddr_in *server_addr, char* ip);

/**
 * createSocket - Creates a socket for server communication.
 * Returns the socket descriptor on success, or 1 on failure.
 */
int createSocket();

/**
 * connectToServer - Establishes a connection to the specified server.
 * @sockfd: The socket descriptor.
 * @server_addr: Structure containing the server's address details.
 * Exits the program if the connection fails.
 */
void connectToServer(int sockfd, struct sockaddr_in *server_addr);

/**
 * sendMessage - Sends a message to the server through the socket.
 * @sockfd: The socket descriptor.
 * @message: The message to be sent.
 * Exits the program if the message fails.
 */
void sendMessage(int sockfd, const char *message);

/**
 * readMessage - Reads a response from the server into a buffer.
 * @sockfd: The socket descriptor.
 * @response: Buffer to store the received message.
 * @size: The size of the buffer.
 * Returns the status code from the server, or 1 on failure.
 */
int readMessage(int sockfd, char *response, size_t size);

/**
 * closeSocket - Closes the socket connection.
 * @sockfd: The socket descriptor to close.
 */
void closeSocket(int sockfd);

/**
 * parsePASVResponse - Parses the server's PASV response to extract IP and port.
 * @response: The PASV response string received from the server.
 * @ip: Buffer to store the extracted IP address.
 * @port: Pointer to an integer to store the extracted port number.
 */
void parsePASVResponse(char *response, char *ip, int *port);

/**
 * extractFilename - Extracts the filename from a given file path.
 * @path: The full file path.
 * @filename: Buffer to store the extracted filename.
 */
void extractFilename(const char *path, char *filename);
