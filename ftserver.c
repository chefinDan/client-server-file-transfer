/******************************************************************************
** Author: Daniel Green, greendan@oregonstate.edu
** Description: starts a ftp server that accepts list and get commands.
******************************************************************************/
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
#include <signal.h>
#include <arpa/inet.h>

#include "ft.h"

#define MAX_BUF 512
#define MAX_CNCT 5
#define STAT_LEN 4
#define CMD_LEN 2
#define ASCII_CAP_START 65
#define ASCII_SPACE 32
#define list "-l"
#define get "-g"

int charsExpected = -1;
static int childcnt;

int recvDataSize(int sockfd);
int recvData(int sockfd, void**, int);
int sendDecoded(int sockfd, char **);
int decode(char *, char *, char **);
void error(const char *msg);


static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig) {
	// int savedErrno;             /* Save 'errno' in case changed here */
	// savedErrno = errno;
 	while (waitpid(-1, NULL, WNOHANG) > 0)
 		continue;
  childcnt--;
	// errno = savedErrno;
}

int main(int argc, char *argv[])
{
	int sock, newSock, n;
   char buffer[MAX_BUF],
        dd_addr_buff[20],
        status[STAT_LEN];
	socklen_t clientLen;
   pid_t childpid;
	struct sigaction sigact;
   struct sockaddr_in ssa_cmd, csa_cmd;

   if (argc < 2) { fprintf(stderr," ***ERROR usage: %s PORT=<port>", "make server"); exit(1); } // Check usage & args

	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_RESTART;
	sigact.sa_handler = grimReaper;
	if (sigaction(SIGCHLD, &sigact, NULL) == -1) {
		error("SERVER: Error from sigaction()");
		exit(EXIT_FAILURE);
	}

	// Set up the address struct for the server
	memset((char *)&ssa_cmd, '\0', sizeof(ssa_cmd)); // Clear out the address struct
	ssa_cmd.family = AF_INET; // Create a network-capable socket
	ssa_cmd.port = htons(atoi(argv[1])); // Store the port number
	ssa_cmd.addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up a socket
	sock = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if (sock < 0) { error("ERROR opening socket"); }

	// Enable the socket to begin listening
	if ( bind(sock, (SA*)&ssa_cmd, sizeof(ssa_cmd)) < 0) // Connect socket to port
		error("ERROR on binding server cmd port");

   listen(sock, MAX_CNCT); // Flip the socket on - it can now receive up to 5 connections
   printf("Server listening on localhost at port %s\n", argv[1]);
   childcnt = 0;

   // Enter server listening loop
   while(1){
      if(childcnt > MAX_CNCT){ error("Maximum number of conccurent connections reached\n"); break; }
      clientLen = sizeof(csa_cmd); // Get the size of the address for the client that will connect

      // Accept a connection, blocking if one is not available until one connects
      newSock = accept(sock, (SA*)&csa_cmd, &clientLen); // Accept
      if (newSock < 0) { error("ERROR on accept"); }

      // save client address as string
      inet_ntop(AF_INET, &csa_cmd.addr, dd_addr_buff, clientLen);
      // acknowledge the client has connected, print the address and port
      printf("SERVER: accepted connection from %s on %d \n", dd_addr_buff, ssa_cmd.port);

      // send client success status
      sprintf(status, "220");
      send(newSock, status, strlen(status), 0);

      while(1){
         memset(buffer, '\0', MAX_BUF);
         recv(newSock, buffer, CMD_LEN, 0);
         if(strcmp(buffer, list) == 0 || strcmp(buffer, get) == 0){
            sprintf(status, "150");
            send(newSock, status, strlen(status), 0);
         }
      }


   //    childcnt++;
   //    childpid = fork();
   //
   //    switch(childpid){
   //       case -1:
   //          fflush(stdout);
   //          printf("fork error\n");
   //          fflush(stdout);
   //          close(newSock);
   //          close(sock);
   //          exit(1);
   //
   //       case 0:
   //       {
   //          void* buffer;
   //          int dataLength = 32;
   //          // encoded = cypher = decoded = NULL;
   //          close(sock); // close the child's copy of the listening socket;
   //          // begin communicating with client over establishedConnectionFD,
   //          recvData(newSock, &buffer, dataLength);
   //          // recvData(establishedConnectionFD, &cypher);
   //          // decode(encoded, cypher, &decoded);
   //          // sendDecoded(establishedConnectionFD, &decoded);
   //          exit(EXIT_SUCCESS);
   //          break;
   //       }
   //       default:
   //          close(establishedConnectionFD); // Close the parents copy of the connected socket
   //          break; // begin listening again for new connections
   //    }
	}

	printf("closing listen socket\n");
	close(sock); // Close the listening socket
	return 0;
}


int recvData(int sockfd, void** buffer, int dataLength){
	int charsRead, charsSent, charsRemain, n;

	charsRead = 0;
	charsRemain = dataLength;

   *buffer = malloc(dataLength);
   memset(*buffer, '\0', dataLength);

   while(charsRead < dataLength){
      n = recv(sockfd, buffer + dataLength, charsRemain, 0);
      if (charsRead < 0) { error("ERROR reading from socket"); }
      charsRead+=n;
      charsRemain-=n;
   }

	// Send a Success message back to the client
	charsSent = send(sockfd, buffer, dataLength, 0); // Send message back
	if (charsSent < 0) { error("ERROR writing to socket"); }

	return 1;
}


// int sendDecoded(int sockfd, char** buffer){
//    int charsWritten, charsSent, charsRemain, len, n;
//    char status[MAX_STATUS];
//
//   len = strlen(*buffer);
//   charsRemain = len;
// 	charsWritten = 0;
//
// 	while(charsWritten < len){
// 		n = send(sockfd, *buffer + charsWritten, charsRemain, 0); // Write to the server
// 		if (n < 0) { error("SERVER: ERROR writing to socket"); return 1; }
// 		charsWritten+=n;
// 		charsRemain-=n;
// 	}
//
//   // recv client status
// 	n = recv(sockfd, status, MAX_STATUS, 0);
//   if (n < 0) { error("SERVER: ERROR recieving from socket"); return 1; }
//
// 	return 1;
// }
//
//
//
// int recvDataSize(int sockfd){
// 	char buffer[MAX_STATUS];
// 	int charsRead, charsSent, n;
// 	memset(buffer, '\0', MAX_STATUS);
//
// 	n = recv(sockfd, buffer, MAX_STATUS, 0);
// 	if(n < 0){ error("SERVER: cannot recieve data size"); exit(1); }
// 	send(sockfd, "200", MAX_STATUS, 0);
// 	return (int)strtol(buffer, NULL, 10);
// }
//
// int decode(char* encoded, char *cypher, char **decoded){
//   int i, enc, cyp, dec;
//
//   *decoded = (char*)malloc(strlen(encoded)+1);
//   memset(*decoded, '\0', strlen(encoded)+1);
//
//   for(i = 0; *(encoded+i); i++){
//     if(*(encoded+i) == ASCII_SPACE) { enc = 26; }
//     else{ enc = *(encoded+i) - ASCII_CAP_START; }
//     // printf("decode: enc: %d\n", enc);
//
//     if(*(cypher+i) == ASCII_SPACE){ cyp = 26; }
//     else{ cyp = *(cypher+i) - ASCII_CAP_START; }
//
//     dec = enc-cyp;
//     if(dec < 0){ dec+=27; }
//     dec=dec%27;
//
//     *(*decoded+i) = KEYS[dec];
//   }
// }

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
