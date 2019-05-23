

// ========================================================================== //
// === Author: Daniel Green, greendan@oregonstate.edu
// === Date: 2 May 2019
// === Description: starts a client program that sends plaintext a chat server
// ========================================================================== //

#define _POSIX_C_SOURCE 1
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <math.h>
#define MAX 500
#define MAX_STATUS 8
#define HANDLE_LEN 10
#define h_addr h_addr_list[0] /* for backward compatibility */


void error(const char *msg) { perror(msg);} // Error function used for reporting issues


int comm(int sockfd, char* data, char* response){
	int charsWritten, charsRead;

  // send data to server
	charsWritten = send(sockfd, data, strlen(data), 0); // Write to the server
	if ( charsWritten < 0) { error("CLIENT: ERROR writing to socket"); return 1; }
  if ( strcmp(data, "\\quit\n") == 0){ return 1; } // if client sent "quit", exit and return 1

	// Get response message from server
	charsRead = recv(sockfd, response, MAX, 0); // Read data from the socket
	if ( charsRead < 0 ) { error("CLIENT: ERROR reading from socket"); return 1; }
  if ( strcmp(response, "\\quit") == 0){ return 1; } // if server sends "quit", exit and return 1

  // print response from server
  printf("Hal> %s\n", response);
  memset(response, '\0', MAX);
	return 0;
}


int main(int argc, char *argv[])
{
	int socketFD, port, mCnt, keyCnt, nread, commResult;
	size_t maxHandleLen = HANDLE_LEN;
	char handle[10];
	char message[MAX];
  char response[MAX];
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	if(argc < 3){ error("CLIENT: ERROR, usage: chatclient <server_hostname> <server_port>"); exit(1); }

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	port = atoi(argv[2]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(port); // Store the port number
	serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {error("CLIENT: ERROR opening socket"); exit(1); }

	// Get users handle and initialize message array
	printf("Enter a handle that you would like to be identified by: ");
	fgets(handle, maxHandleLen, stdin);
	handle[strlen(handle) -1] = '\0';
  memset(message, '\0', MAX);
  memset(response, '\0', MAX);

	// Connect to server via socketFD
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		error("CLIENT: ERROR connecting"); exit(1);
	}

  // send User handle to server
	comm(socketFD, handle, response);

  while(1){
    printf("%s> ", handle);
    fgets(message, MAX, stdin);

    if(strcmp(message, "\\quit\n") == 0){
      printf("%s\n", "Closing socket");
      commResult = comm(socketFD, message, response);
      close(socketFD);
      return 0;
    }

    commResult = comm(socketFD, message, response);

    if(commResult > 0){
      printf("%s\n", "Server closed socket");
      close(socketFD);
      return 0;
    }
    memset(message, '\0', MAX);
  }


}
