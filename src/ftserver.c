/******************************************************************************
** Author: Daniel Green, greendan@oregonstate.edu
** Date: 2 June 2019
** Description: starts a ftp server that accepts list and get commands.
******************************************************************************/
/*
sources: https://linux.die.net/man/,
         https://docs.python.org/3.7/library/socket.html
         https://beej.us/guide/bgnet/
*/

#include "ft.h"

int main(int argc, char *argv[])
{
	int sock, cmdSock, dataSock, dataPort;
   char buffer[MAX_BUF],
        dd_addr_buff[ADDR_LEN],
        status[MAX_BUF],
        cmd[CMD_LEN];
   socklen_t clientLen;
   struct sockaddr_in csa_cmd;

   if (argc < 2) { fprintf(stderr," ***ERROR usage: %s PORT=<port>", "make server"); exit(1); } // Check usage & args

   start_server(&sock, atoi(argv[1]));

   // Enter server main loop
   while(1){

      // Get the size of the address for the client that will connect
      clientLen = sizeof(csa_cmd); 

      // Accept a connection, blocking if one is not available until one connects
      if((cmdSock = accept(sock, (SA*)&csa_cmd, &clientLen)) > -1){
         
         // save client address as string
         inet_ntop(AF_INET, &csa_cmd.sin_addr, dd_addr_buff, clientLen);
         // acknowledge the client has connected, print the address and port
         printlnStr("SERVER: accepted connection from %s ", dd_addr_buff);
         
         // send success status greeting to client
         ctrl_send("220", cmdSock);

         while (1)
         {
            // memset(buffer, '\0', MAX_BUF);
            memset(status, '\0', MAX_BUF);

            // recv command from client on cmd port
            ctrl_recieve(cmdSock, buffer);
            printlnStr("Recieved from client: %s", buffer);

            // Only accept legal commands
            if (strcmp(buffer, "-l") == 0 || strcmp(buffer, "-g") == 0)
            {
               // save command
               strcpy(cmd, buffer);

               // send client the "pending further information" status
               ctrl_send("350", cmdSock);
               // recieve data port from client   
               ctrl_recieve(cmdSock, buffer);
               dataPort = atoi(buffer);

               // after recieving data port, inform client more info is needed
               ctrl_send("350", cmdSock);
               //recieve OK ready for connection status from client
               ctrl_recieve(cmdSock, buffer);
               
               // data_connect: establishes data connection on dataPort
               data_connect(&dataSock, &csa_cmd, dataPort);

               // handle_cmd: execs command, sends data, and closes dataSocket
               handle_cmd(cmd, &cmdSock, &dataSock);
            }
            else
            {
               ctrl_send("502", cmdSock);
               close(cmdSock);
               memset(buffer, '\0', MAX_BUF);
               puts("Connection closed by client");
               break;
               // printlnStr("Recieved from client: %s", buffer);
            }
         }
      }
   }

   puts("closing listen socket\n");
   close(sock); // Close the listening socket
   return 0;
}

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


// scratch code

// struct sigaction sigact;

// sigemptyset(&sigact.sa_mask);
// sigact.sa_flags = SA_RESTART;
// sigact.sa_handler = grimReaper;
// if (sigaction(SIGCHLD, &sigact, NULL) == -1) {
// 	error("SERVER: Error from sigaction()");
// 	exit(EXIT_FAILURE);
// }