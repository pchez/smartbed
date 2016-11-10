/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

typedef struct {
	int sockfd;
	char ip_addr_str[INET_ADDRSTRLEN];
	int port;
	float pitchBuffer[151];
	float rollBuffer[151];
} CONNECTION;

void get_ip_addr(char ip_addr_str[], struct sockaddr_in* connection_addr);

int server_error(const char *msg);
int server_init(int portno, int timeout_seconds);
int server_accept_connection(int server_socket_fd);

