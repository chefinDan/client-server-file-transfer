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

#define MAX_BUF 512
#define MAX_CNCT 5
#define STAT_LEN 4
#define CMD_LEN 2
#define ASCII_CAP_START 65
#define ASCII_SPACE 32
#define SA struct sockaddr
// #define family sin_family
// #define port sin_port
// #define addr sin_addr

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

void printlnStr(char* str, char* data){
	char* msg = malloc((strlen(str)*sizeof(char)) + (strlen(data)*sizeof(char)));
	memset(msg, '\0', strlen(msg));
	sprintf(msg, str, data);
	puts(msg);
	fflush(stdout);
	free(msg);
}

void printlnInt(char *str, int data){
	char *msg = malloc((strlen(str) * sizeof(char))+8);
	memset(msg, '\0', strlen(msg));
	sprintf(msg, str, data);
	puts(msg);
	fflush(stdout);
	free(msg);
}


void recieve(int socket, char* buffer){
	int i, n;
	for (i = 0;; ++i)
	{
		n = recv(socket, &buffer[i], 1, 0);
		if (n <= 0)
			return;
		if (buffer[i] == '\n'){
			buffer[i] = '\0';
			return;
		}
	}
}


int data_connect(int* sock, int port){
	struct sockaddr_in csa;

	*sock = socket(AF_INET, SOCK_STREAM, 0);

	memset((char *)&csa, '\0', sizeof(csa)); // initialize address struct
	csa.sin_family = AF_INET;						   // Create a network-capable socket
	csa.sin_port = htons(port);			   // Store the port number
	csa.sin_addr.s_addr = INADDR_ANY;				   // Any address is allowed for connection to this process

	if(connect(*sock, (SA *)&csa, sizeof(csa)) < 0)
	{
		puts("SERVER: attempting to connect to client for data connection");
	}

	puts("Connected to client for data transfer");
	return 1;
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

	if (listen(*sock, MAX_CNCT) < 0){ // Flip the socket on - it can now receive up to 5 connections
		close(*sock);
		error("ERROR on listen server socket");
	}

	printlnInt("Server listening on localhost at port %d", port);
	
	return 1;
}

char* readDir(const char* path, int* length)
{
	// int stat(const char *path, struct stat *buf);
	char* buf = 0;
	struct stat stat_buf;
	int i, j;

	DIR* dirFd;
	struct dirent* dirInfo;

	if ((dirFd = opendir(path)) == NULL)
	{
		error("SERVER: error readDir()");
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

int handle_cmd(char* cmd, char* buf, int* cmdSock, int* dataSock){
	char* dir_buffer = 0;
	size_t dirlen;
	int n;

	if (strcmp(cmd, "-l") == 0)
	{
		dir_buffer = readDir(".", &dirlen);
		sprintf(buf, "%d", strlen(dir_buffer));
		send(*cmdSock, buf, strlen(buf), 0);
		recieve(*cmdSock, buf);
		if (strcmp(buf, "220") == 0){
			puts("client ready");
			if((n = send(*dataSock, dir_buffer, strlen(dir_buffer), 0)) == -1){
				return 0;
			}
		}
		else{
			return 0;
		}
	}
	else{
		puts("Client wants file transfer");
	}

	return 1;
}

#endif
