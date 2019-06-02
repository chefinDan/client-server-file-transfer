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
	int sock, cmdSock, dataSock, dataPort, n;
   size_t dirlen;
   char buffer[MAX_BUF],
        msg[MAX_BUF],
        dd_addr_buff[20],
        status[MAX_BUF],
        cmd[CMD_LEN];
   char* dir_buffer = 0;
   const char *buf = 0;
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
            // memset(buffer, '\0', MAX_BUF);
            memset(status, '\0', MAX_BUF);

            // recv command from client on cmd port
            // recieve(cmdSock, buffer);
            recieve(cmdSock, &buf);
            printlnStr("Recieved from client: %s", buf);

            // printf("%s", buf);
            // Only accept legal commands
            if (strcmp(buf, "-l") == 0 || strcmp(buf, "-g") == 0)
            {
               // save command
               strcpy(cmd, buf);

               // send client the "pending further information" status
               sprintf(status, "350");
               send(cmdSock, status, strlen(status), 0);

               // recieve data port from client   
               recieve(cmdSock, &buf);
               dataPort = atoi(buf);
               send(cmdSock, status, strlen(status), 0);

               //recieve OK ready for connection status from client
               recieve(cmdSock, &buf);
               data_connect(&dataSock, dataPort);

               // handle cmd
               handle_cmd(&cmd, &buf, &cmdSock, &dataSock);
            }
            else
            {
               printlnStr("Recieved from client: %s", buf);
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

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
