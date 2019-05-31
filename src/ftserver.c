/******************************************************************************
** Author: Daniel Green, greendan@oregonstate.edu
** Description: starts a ftp server that accepts list and get commands.
******************************************************************************/
/*
sources: https://linux.die.net/man/,
         https://docs.python.org/3.7/library/socket.html
*/
#include "ft.h"

int main(int argc, char *argv[])
{
	int sock, cmdSock, dataSock, n;
   size_t dirlen;
   char buffer[MAX_BUF],
        msg[MAX_BUF],
        dd_addr_buff[20],
        status[MAX_BUF],
        cmd[CMD_LEN];
   char* dir_buffer = 0;
	socklen_t clientLen;
   pid_t childpid;
	// struct sigaction sigact;
   struct sockaddr_in csa_cmd;
   DIR* dir;
   struct dirent* dir_entry;

   if (argc < 2) { fprintf(stderr," ***ERROR usage: %s PORT=<port>", "make server"); exit(1); } // Check usage & args

   start_server(&sock, atoi(argv[1]));
	// sigemptyset(&sigact.sa_mask);
	// sigact.sa_flags = SA_RESTART;
	// sigact.sa_handler = grimReaper;
	// if (sigaction(SIGCHLD, &sigact, NULL) == -1) {
	// 	error("SERVER: Error from sigaction()");
	// 	exit(EXIT_FAILURE);
	// }

   
   childcnt = 0;

   // Enter server main loop
   while(1){

      if(childcnt > MAX_CNCT){ 
         error("Maximum number of conccurent connections reached\n"); break; 
      }

      clientLen = sizeof(csa_cmd); // Get the size of the address for the client that will connect

      // Accept a connection, blocking if one is not available until one connects
      if((cmdSock = accept(sock, (SA*)&csa_cmd, &clientLen)) > -1){
         
         // save client address as string
         inet_ntop(AF_INET, &csa_cmd.sin_addr, dd_addr_buff, clientLen);
         // acknowledge the client has connected, print the address and port
         printlnStr("SERVER: accepted connection from %s ", dd_addr_buff);
         
         // send success status greeting to client
         sprintf(status, "220");
         send(cmdSock, status, strlen(status), 0);

         while (1)
         {
            memset(buffer, '\0', MAX_BUF);
            memset(status, '\0', MAX_BUF);

            // recv command from client on cmd port
            recieve(cmdSock, buffer);

            // Only accept legal commands
            if (strcmp(buffer, "-l") == 0 || strcmp(buffer, "-g") == 0)
            {
               // save command
               strcpy(cmd, buffer);

               // send client the connect status
               sprintf(status, "150");
               send(cmdSock, status, strlen(status), 0);

               // recieve data port from client   
               recieve(cmdSock, buffer);
               data_connect(&dataSock, atoi(buffer));

               // handle cmd
               handle_cmd(&cmd, &buffer, &cmdSock, &dataSock);
               // if (strcmp(cmd, "-l") == 0)
               // {
               //    dir_buffer = readDir(".", &dirlen);
               //    sprintf(buffer, "%d", strlen(dir_buffer));
               //    send(cmdSock, buffer, strlen(buffer), 0);
               //    recieve(cmdSock, buffer);
               //    if(strcmp(buffer, "220") == 0)
               //       send(dataSock, dir_buffer, strlen(dir_buffer), 0);
               // }
               // else
               // {
               //    puts("Client wants file transfer");
               // }
            }
            else
            {
               printlnStr("Recieved from client: %s", buffer);
               // send client error status
               sprintf(status, "502");
               send(cmdSock, status, strlen(status), 0);
            }
         }
      }



      
   }

   puts("closing listen socket\n");
   close(sock); // Close the listening socket
   return 0;
}


   //    childcnt++;
   //    childpid = fork();
   //
   //    switch(childpid){
   //       case -1:
   //          fflush(stdout);
   //          printf("fork error\n");
   //          fflush(stdout);
   //          close(cmdSock);
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
   //          recvData(cmdSock, &buffer, dataLength);
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


// int recvData(int sockfd, void** buffer, int dataLength){
// 	int charsRead, charsSent, charsRemain, n;

// 	charsRead = 0;
// 	charsRemain = dataLength;

//    *buffer = malloc(dataLength);
//    memset(*buffer, '\0', dataLength);

//    while(charsRead < dataLength){
//       n = recv(sockfd, buffer + dataLength, charsRemain, 0);
//       if (charsRead < 0) { error("ERROR reading from socket"); }
//       charsRead+=n;
//       charsRemain-=n;
//    }

// 	// Send a Success message back to the client
// 	charsSent = send(sockfd, buffer, dataLength, 0); // Send message back
// 	if (charsSent < 0) { error("ERROR writing to socket"); }

// 	return 1;
// }


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
