// Author: Daniel Green, greendan@oregonstate.edu
// Date: 2 June 2019
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


static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig) {
	// int savedErrno;             /* Save 'errno' in case changed here */
	// savedErrno = errno;
 	while (waitpid(-1, NULL, WNOHANG) > 0)
 		continue;
  childcnt--;
	// errno = savedErrno;
}

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

void 
ctrl_send(const char* status, int sock){
	char buf[16];
	memset(buf, '\0', 16);

	// sprintf(buf, status);
	send(sock, status, strlen(status), 0);
}


void 
ctrl_recieve(int socket, char* buffer){	
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

int data_connect(int *sock, struct sockaddr_in* csa_cmd, int port)
{
	struct sockaddr_in csa;

	*sock = socket(AF_INET, SOCK_STREAM, 0);

	memset((char *)&csa, '\0', sizeof(csa)); // initialize address struct
	csa.sin_family = AF_INET;						   // Create a network-capable socket
	csa.sin_port = htons(port);			   // Store the port number
	csa.sin_addr.s_addr = csa_cmd->sin_addr.s_addr;				   // Any address is allowed for connection to this process

	if (connect(*sock, (SA *)&csa, sizeof(csa)) < 0)
	{
		puts("SERVER: cannot connect to client for data connection");
		return 0;
	}
	else
	{
		puts("Connected to client for data transfer");
		return 1;
	}

}


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


char* readDirectory(const char* path, size_t* length)
{
	// int stat(const char *path, struct stat *buf);
	char* buf = 0;
	struct stat stat_buf;
	int i, j;

	DIR* dirFd;
	struct dirent* dirInfo;

	if ((dirFd = opendir(path)) == NULL)
	{
		error("SERVER: error readDirectory()");
	}
	else
	{
		// read contents into buffer
		for (i = 0, j = 0; (dirInfo = readdir(dirFd)); i = j)
		{
			if(stat(dirInfo->d_name, &stat_buf) == -1)
			{
				error("SERVER: error reading dirInfo");
			}

			// sum the total length of direntry name strings
			j += strlen(dirInfo->d_name);

			// expand buffer to hold new direntry, +2 for newline and directory slash char
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

		closedir(dirFd);

		buf[j - 1] = '\0'; // replace last newline with null character
		*length = j;	// store length
	}

	return buf;
}


// returns: 1 if found, O if not found, -1 if file error, -2 if file is directory
int getFile(char** buf, const char *path, size_t *length, char* file_name)
{
	// int stat(const char *path, struct stat *buf);
	// unsigned char* buf = 0;
	struct stat stat_buf;
	int found;
	FILE *fp;
	DIR *dirFd;
	struct dirent *dirInfo;

	if ((dirFd = opendir(path)) == NULL)
	{
		error("SERVER: error readDir()");
	}
	else
	{
		found = 0;
		// read contents into buffer
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
					found = 1;
					fp = fopen(file_name, "rb");

					//Get file length
					fseek(fp, 0, SEEK_END);
					*length = ftell(fp);
					fseek(fp, 0, SEEK_SET);

					//Allocate memory
					if (!(*buf = (char *)malloc(*length + 1)))
					{
						return -1;
					}

					//Read file contents into buf
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

// void readData(FILE* fp, char* dest, int len)
// {
// 	size_t n = 100;

// 	if (fp)
// 	{
// 		while (n)
// 		{
// 			size_t n_read = fread(head, sizeof(double), n, file);
// 			head += n_read;
// 			n -= n_read;
// 			if (feof(file) || ferror(file))
// 				break;
// 		}
// 		processData(array, head - array);
// 		fclose(file);
// 	}
// }

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



#endif
