#include "server.h"
////////////////////////////////////////////////
// Code for this was obtained and slightly modified from the textbook (O'Halleren)
// Here we create a socket descriptor, and binds the created socket descriptor to a specific port (when we call server)
// We bind() the socket descriptor that we make with socket() to the given port that the server listens to.
// When it is successfully done, listen() is called to get the socket ready so that
// we can accept() the connection (to the user).
////////////////////////////////////////////////


int open_listenfd(int port){
	int listenfd, optval=1; // initialization variables
	struct sockaddr_in serveraddr;

	// make a new socket descriptor
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}

	// making sure that the address is not already in use and wont accidentally show up as such either
	 if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int)) < 0){
	 	return -1;
	 }

	 // set all the bytes of serveraddr to 0, setting other variables properties to serveraddr
	 bzero((char *) &serveraddr, sizeof(serveraddr));
	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	 serveraddr.sin_port = htons((unsigned short)port);
	 if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){ // call bind to bind it to the given port
	 	return -1; // or not be able to find it
	 }
	 // We get the socket ready to be able to call accept()
	 // We can accept a max number of 20 connections onto the queue (listening queue)
	 // until accept() is recalled.
	 if (listen(listenfd, 20) < 0){
	 	return -1;
	 }
	// return the socket descriptor
	 return listenfd;
}