#include "client.h"

int client_error(const char *msg)
{
	perror(msg);
	return -1;
}

int client_init(int argc, char *argv[])
{	
	int client_socket_fd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	// Read command line arguments, need to get the host IP address and port
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		return -1;
	}

	// Convert the arguments to the appropriate data types
	portno = atoi(argv[2]);

	// setup the socket
	client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	// check if the socket was created successfully. If it wasnt, display an error and exit
	if(client_socket_fd < 0) {
		return client_error("ERROR opening socket");
	}

	// check if the IP entered by the user is valid 
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		return -1;
	}

	// clear our the serv_addr buffer
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	// set up the socket 
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	// try to connect to the server
	if (connect(client_socket_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ 
		return client_error("ERROR connecting");
	}

	return client_socket_fd;
}
