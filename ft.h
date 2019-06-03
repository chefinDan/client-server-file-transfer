// Author: Daniel Green, greendan@oregonstate.edu
// Last Modified: 2 June 2019
// Description: definitions of functions used by ftserver.c

#ifndef _FT_H
#define _FT_H

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
#include <dirent.h>
#include "cvector.h"

#define MAX_BUF 512
#define MAX_CNCT 5
#define ADDR_LEN 20
#define STAT_LEN 4
#define CMD_LEN 2
#define ASCII_CAP_START 65
#define ASCII_SPACE 32
#define SA struct sockaddr

int charsExpected = -1;
static int childcnt;

int recvDataSize(int sockfd);
int recvData(int sockfd, void**, int);
int sendDecoded(int sockfd, char **);
int decode(char *, char *, char **);
void error(const char *msg);



// utility function for easily printing a custom string with char data
void 
printlnStr(char* str, char* data){
	size_t str_len, data_len;
	char *msg;

	for (str_len = 0; str[str_len] != '\0'; ++str_len){}
	for (data_len = 0; data[data_len] != '\0'; ++data_len){}
	msg = malloc( (str_len + data_len) * sizeof(char));
	memset(msg, '\0', str_len + data_len);
	sprintf(msg, str, data);
	puts(msg);
	fflush(stdout);
	free(msg);
}


// utility function for easily printing a custom string with int data
void 
printlnInt(char *str, int data){
	size_t len;
	char* msg;

	for(len = 0; str[len] != '\0'; ++len){}
	msg = malloc((len * sizeof(char))+8);
	memset(msg, '\0', len);
	sprintf(msg, str, data);
	puts(msg);
	fflush(stdout);
	free(msg);
}


// =================================================================== //
// ===							ctrl_send()						   === //
// === Pre: 													   === //
// ===    Param0: a cstring containing a 3 digit ftp status        === //
// ===    code, such as "220". 									   === //
// ===    Param1: An active open tcp socket that the status code   === // 
// ===    will be sent to.										   === //
// === Post:													   === //
// ===    None													   === //
// === Description: This function is used to send ftp codes over   === //
// === the control socket.										   === // 
// =================================================================== //
void ctrl_send(const char* status, int sock){
	char buf[16];
	memset(buf, '\0', 16);

	// sprintf(buf, status);
	send(sock, status, strlen(status), 0);
}


// =================================================================== //
// ===						 ctrl_recieve()					       === //
// === Pre: 													   === //
// ===    Param0: An active open tcp socket that the status code   === //
// ===    will be recieved from.								   === //
// ===    Param1: An address to a stack allocated char buffer      === //
// ===    that will store the recived command/code.				   === //
// === Post:													   === //
// ===    buffer now contains a short status code or command.	   === //
// === Description: This function is used to recieve ftp codes     === //
// === and short commands over the control socket.				   === //
// =================================================================== //
void ctrl_recieve(int socket, char* buffer){	
	int i, n;

	for (i = 0;; ++i)
	{
		n = recv(socket, &buffer[i], 1, 0);
		if (n <= 0){
			// puts("bad recv");
			return;
		}

		if (buffer[i] == '\n')
		{
			buffer[i] = '\0';
			return;
		}
	}
}


// =================================================================== //
// ===						data_connect()						   
// === Pre: 
// ===   0. Param0: The address of an unitialized socket that will  
// ===    be used for data transmission.
// ===   1. Param1: The address of an unitialized sockaddr_in struct
// ===    that will be used to establish the data socket connection
// ===    to the client.
// ===   2. Param2: The port number that the server will attempt to        
// ===    connect on for the data connection.
// === Post:
// ===   0. The int address passed in at param0 will point to a connected
// ===    socket that can be used to transfer data.
// ===   1. The function returns 1 if connection was successful,
// ===    otherwise it returns 0.
// ================================================================== //
int data_connect(int *sock, struct sockaddr_in* csa_cmd, int port)
{
	// declare new sockaddr_in for data connection
	struct sockaddr_in csa;

	// initialize sock to be a tcp socket
	*sock = socket(AF_INET, SOCK_STREAM, 0);

	// setup sockaddr_in, assigning it's addr member to be the address of
	// the currently connected client. 
	memset((char *)&csa, '\0', sizeof(csa)); 
	csa.sin_family = AF_INET;	
	csa.sin_port = htons(port);			   
	csa.sin_addr.s_addr = csa_cmd->sin_addr.s_addr;

	// Attempt connection to listening client, if unsuccesful, return 0, otherwise return 1. 
	if (connect(*sock, (SA *)&csa, sizeof(csa)) < 0)
	{
		puts("SERVER: cannot connect to client for data connection");
		return 0;
	}
	
	puts("Connected to client for data transfer");
	return 1;

}


// =================================================================== //
// ===						start_server()
// === Pre:
// ===   0. Param0: The address of an unitialized socket that will
// ===    be used to accept connections.
// ===   1. Param1: The port that the server will listen on.
// === Post:
// ===   0. The int address passed in at param0 will point to a connected
// ===    socket that can be used to accept connections.
// ===   1. The program will enter listening phase on port passed at param1.
// ================================================================== //
int start_server(int* sock, int port){
	struct sockaddr_in ssa;

	// Set up the address struct for the server
	memset((char *)&ssa, '\0', sizeof(ssa)); // Clear out the address struct
	ssa.sin_family = AF_INET;					 // Create a network-capable socket
	ssa.sin_port = htons(port);		 // Store the port number
	ssa.sin_addr.s_addr = INADDR_ANY;			 // Any address is allowed for connection to this process

	// Set up a socket
	*sock = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (*sock < 0) {
		error("ERROR opening socket");
	}

	// Enable the socket to begin listening
	if (bind(*sock, (SA *)&ssa, sizeof(ssa)) < 0){ // Connect socket to port
		close(*sock);
		error("ERROR on binding server socket");
	}

	// Flip the socket on - it can now receive up to MAX_CNCT connections
	if (listen(*sock, MAX_CNCT) < 0){ 
		close(*sock);
		error("ERROR on listen server socket");
	}

	printlnInt("Server listening on localhost at port %d", port);
	
	return 1;
}


// =================================================================== //
// ===						readDirectory()
// === Pre:
// ===   0. Param0: A string literal of the directory path that will be read.
// ===   1. Param1: The address of a size_t that will store the length of 
// ===    the directory contents.
// === Post:
// ===   0. The size_t address passed in at param1 will point to a size_t
// ===    that represets the total length of the directory listing.
// ===    socket that can be used to accept connections.
// ===   1. Upon success, an address to a char array will be returned.
// ===    this array will contain a formatted string of the names of
// ===	  all entries in the directory pointed to by path.
// ===   2. Upon failure, null is returned.   
// ================================================================== //
char* readDirectory(const char* path, size_t* length)
{
	char* buf = 0;
	struct stat stat_buf;
	int i, j;
	DIR* dirFd;
	struct dirent* dirInfo;

	// Attempt to open the directory path
	if ((dirFd = opendir(path)) == NULL)
	{
		error("SERVER: error readDirectory()");
		return 0;
	}

	// read contents of dirFd into struct dirent* dirInfo
	for (i = 0, j = 0; (dirInfo = readdir(dirFd)); i = j)
	{
		// attempt to stat the current dir entry, and store in stat_buf
		if(stat(dirInfo->d_name, &stat_buf) == -1)
		{
			error("SERVER: error reading dirInfo");
		}

		// sum the total length of direntry name strings
		j += strlen(dirInfo->d_name);

		// expand buffer to hold new direntry, +2 for newline and directory slash symbol
		if (S_ISDIR(stat_buf.st_mode) && (buf = realloc(buf, (j + 2) * sizeof(char))) == NULL)
		{
			closedir(dirFd);
			fprintf(stderr, "readDir(): buffer realloc failed.\n");
			return 0;
		}

		// expand buffer to hold new direntry, +1 for newline
		if(S_ISREG(stat_buf.st_mode) && (buf = realloc(buf, (j + 1) * sizeof(char))) == NULL)
		{
			closedir(dirFd);
			fprintf(stderr, "readDir(): buffer realloc failed.\n");
			return 0;
		}

		// if the current direntry is a directory, append a slash to the end of
		// it's name in the buffer, followed by a newline. 
		if (S_ISDIR(stat_buf.st_mode)){
			strcpy(&buf[i], dirInfo->d_name);
			buf[j++] = '/'; // set directory slash identifier
			buf[j++] = '\n'; // set newline
		}
		else{
			strcpy(&buf[i], dirInfo->d_name);
			buf[j++] = '\n'; // set newline
		}

	}

	// close the directory
	closedir(dirFd);

	buf[j - 1] = '\0'; // replace last newline with null character
	*length = j;	// store length

	return buf;
}

// =================================================================== //
// ===							getFile()
// === Pre:
// ===   0. Param0: The address of a pointer to an unitialized char array.
// ===   1. Param1: A string literal of the directory path that will be read.
// ===   2. Param2: The address of a size_t that will hold the length of the file.
// ===   3. Param3: A initialized char array containing the name of the file
// ===    that will be read. 
// === Post:
// ===   0. *buf will contain the entire file contents of "file_name" 
// ===   1. *length will contain the total length of the file.
// ===   
// returns: 1 if found, O if not found, -1 if file error, -2 if file is directory
// ================================================================== //
int getFile(char** buf, const char *path, size_t *length, char* file_name)
{
	struct stat stat_buf;
	int found;
	FILE *fp;
	DIR *dirFd;
	struct dirent *dirInfo;

	// attempt to open the directory at "path"
	if ((dirFd = opendir(path)) == NULL)
	{
		error("SERVER: error readDir()");
	}
	else
	{
		// set a flag value to determine whether the file is found
		found = 0;

		// read contents into dirInfo
		while ((dirInfo = readdir(dirFd)))
		{
			// get file stats, if cannot stat file return -1
			if (stat(dirInfo->d_name, &stat_buf) == -1)
			{
				return -1;
			}

			// check if current direntry name matches user requested file_name 
			if(strcmp(dirInfo->d_name, file_name) == 0)
			{
				// if match is found and it's a directory, return -2
				if (S_ISDIR(stat_buf.st_mode))
				{	
					return -2;
				}
				else
				{
					// The file is found
					found = 1;

					// open the file in read/binary mode
					fp = fopen(file_name, "rb");

					//Get file length
					fseek(fp, 0, SEEK_END);
					*length = ftell(fp);
					fseek(fp, 0, SEEK_SET);

					//Allocate memory to *buf
					if (!(*buf = (char *)malloc(*length + 1)))
					{
						return -1;
					}

					//Read file contents into *buf
					fread(*buf, 1, *length, fp);
					fclose(fp);
				}
			}
		}
	}

	// if file was found and read, return 1
	closedir(dirFd);
	if(found)
	{
		return 1;
	}
	
	// else return 0 inidicating file was not found
	return 0;

}


// =================================================================== //
// ===							handleCmd()
// === Pre:
// ===   0. Param0: The cmd that was recieved from the client
// ===   1. Param1: The address of the control socket  
// ===   2. Param2: The address of the data socket.
// === Post:
// ===   0. If a legal command was recived then the cmd will be executed.
// ===   1. If -l was recieved, the directory is read, and the contents
// ===    are sent over the data socket. 
// ===   2. If -g was recieved, the file is searched for, and if found is
// ===    sent over the data socket.
// ===   3. After either outcome, the data socket is closed.
// ================================================================== //
int handle_cmd(char* cmd, int* cmdSock, int* dataSock){
	char* file_buffer = 0;
	char buffer[MAX_BUF]; 
	size_t dirlen;
	int n, result, bytesleft, total;

	if (strcmp(cmd, "-l") == 0)
	{
		// read local directory into buffer
		file_buffer = readDirectory(".", &dirlen);
		// read length of buffer and send to client
		sprintf(buffer, "%ld", strlen(file_buffer));
		ctrl_send(buffer, *cmdSock);
		// send(*cmdSock, *buf, strlen(*buf), 0);
		ctrl_recieve(*cmdSock, buffer);

		// If the client recived the expected data size and responded 220
		if (strcmp(buffer, "220") == 0){
			puts("client ready");
			if((n = send(*dataSock, file_buffer, strlen(file_buffer), 0)) == -1){
				// free(*buf);
				free(file_buffer);
				return 0;
			}
			else{
				// free(*buf);
				free(file_buffer);
				close(*dataSock);
				return 1;
			}
		}
		else{
			return 0;
		}
	}
	else if(strcmp(cmd, "-g") == 0)
	{
		puts("Client wants file transfer");
		// send client "pending further action" code
		ctrl_send("350", *cmdSock);

		
		// recieve the requested filename from client
		ctrl_recieve(*cmdSock, buffer);
		// puts("Client wants: ");
		// puts(buffer);

		// read directory and look for file, writing it's data to file_buffer
		result = getFile(&file_buffer, ".", &dirlen, buffer);
		if(result == 0){
			ctrl_send("550", *cmdSock);
		}
		else if(result == -1){
			ctrl_send("451", *cmdSock);
		}
		else if(result == -2){
			ctrl_send("550", *cmdSock);
		}
		else{
			// inform client that file was found
			ctrl_send("250", *cmdSock);
			ctrl_recieve(*cmdSock, buffer);

			// inform client of datasize
			sprintf(buffer, "%ld", dirlen);
			ctrl_send(buffer, *cmdSock);
			ctrl_recieve(*cmdSock, buffer);
			
			// If the client recived the expected data size and responded 220
			if (strcmp(buffer, "220") == 0)
			{
				bytesleft = dirlen;
				total = 0;
				while (total < dirlen)
				{
					fflush(stdout);
					printf("%d\n", total);
					n = send(*dataSock, file_buffer + total, dirlen, 0);
					if (n == -1)
					{
						puts("server send error");
						free(file_buffer);
						close(*dataSock);
						return 0;
					}
					total += n;
					bytesleft -= n;
				}

				puts("file sent");
				free(file_buffer);
				close(*dataSock);
				return 1;
			}
		}
	}
	else
	{
		error("SERVER: error, cmd not found");
	}

	return 1;
}

// static void /* SIGCHLD handler to reap dead child processes */
// grimReaper(int sig) {
// 	// int savedErrno;             /* Save 'errno' in case changed here */
// 	// savedErrno = errno;
//  	while (waitpid(-1, NULL, WNOHANG) > 0)
//  		continue;
//   childcnt--;
// 	// errno = savedErrno;
// }

#endif
