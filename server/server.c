/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include "server.h"

int server_error(const char *msg)
{
	perror(msg);
	return -1;
}

void get_ip_addr(char ip_addr_str[], struct sockaddr_in* connection_addr)
{
	sprintf(ip_addr_str, inet_ntoa(connection_addr->sin_addr));
}

int server_init(int portno, int timeout_seconds)
{
	int server_socket_fd;
	struct sockaddr_in serv_addr;
	struct timeval timeout;
	CONNECTION* server;

	// setup socket
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket_fd < 0) {	
		return server_error("ERROR opening socket");
	}

	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = 0;
	if (setsockopt(server_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
		return server_error("setsockopt failed\n");
	}

	// setup server information
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// bind the socket to an address
	if (bind(server_socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		return server_error("ERROR on binding");
	}

	// listen for incoming connections
	// accept at most 5 connections before refusing them
	listen(server_socket_fd, 5);

	server = malloc(sizeof(CONNECTION));
	server->sockfd = server_socket_fd;
	get_ip_addr(server->ip_addr_str, &serv_addr);
	server->port = portno;

	printf("server init: %s:%d\n", server->ip_addr_str, server->port);

	return (int) server;
}

int server_accept_connection(int server_socket_fd)
{
	int client_socket_fd;
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	CONNECTION* client;

	clilen = sizeof(cli_addr);

	// block until there is a new connection. When one is received, accept it
	client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &cli_addr, &clilen);
	if (client_socket_fd < 0) {
		return server_error("ERROR on accept");
	}

	client = malloc(sizeof(CONNECTION));
	client->sockfd = client_socket_fd;
	get_ip_addr(client->ip_addr_str, &cli_addr);
	client->port = -1;

	printf("accepted connection from %s\n", client->ip_addr_str);

	return (int) client;
}