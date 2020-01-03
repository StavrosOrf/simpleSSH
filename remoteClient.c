#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>

#define SOCKET struct sockaddr
#define NUMBER_OF_COMMANDS_TO_SEND 10
#define BUFFER_SIZE NUMBER_OF_COMMANDS_TO_SEND*101 + 6 + 11
#define SA struct sockaddr
#define UDP_PACKAGE_SIZE 512
int finishedSending = 0;

typedef struct node {
	FILE* fp;
	int instrNumber;
	int receivedPackages;
	int totalPackages;
	struct node * next;
} fdNode;


void signal_handler(int sig){
	//fprintf(stderr,"Finished sending Commands process with pid : %d  \n", getpid());
	finishedSending = 1;
}

void sendCommands(int sockfd ,int receivePORT,char* fileName){
	char buffer[BUFFER_SIZE];
	char line[101];
	char strPort[6];
	char strInstr[11];
	int terminatingFlag = 0,i;

	//open command file
	FILE* file = fopen(fileName, "r");

	sprintf(strPort,"%5u",receivePORT);

	int n;
	int numberOfInstructions ;


	while(!terminatingFlag) {
		numberOfInstructions = 0;
		bzero(buffer, sizeof(buffer));
		n = 0;

		strcpy(buffer,strPort);

		while (n <10){
			bzero(line,sizeof(line));
			if (fgets(line, sizeof(line), file) == NULL ){
				terminatingFlag = 1;
				break;
			}

			// if command larger that 100 characters, write an empty string to buffer
			if(line[99] != '\0'){
				//printf("%s\n",line );
				bzero(line,sizeof(line));
				//printf("Too many characters\n");

				//flush the remaining line and move the pointer to the new line
				while(1){
					fgets(line, sizeof(line), file);
					if(line[99] == '\0')
						break;
					bzero(line,sizeof(line));
				}
				bzero(line,sizeof(line));
				line[0] = '\0';
			}

			//copy command to buffer
			strcpy(&buffer[101*n + 17],line);

			numberOfInstructions ++;
			n++;
		}

		sprintf(strInstr,"%10u",numberOfInstructions);
		strncpy(&buffer[5],strInstr,10);

		for(i=0;i<BUFFER_SIZE;i++){
			if(i==17 || (i-17)%101 == 0)
				printf("\n");
			printf("%c",buffer[i]);
		}
		printf("\n");

		//Send to Server through TCP socket
		write(sockfd, buffer, sizeof(buffer));

		if(!terminatingFlag)
			sleep(5);
	}

	fclose(file);
	kill(getppid(),SIGUSR1);
}

int countLines(char* filename){

	int count = 0;
	char c;

	FILE *fp;
	fp = fopen(filename, "r");

	// Check if file exists
	if (fp == NULL){
		printf("Could not open file %s", filename);
		return 0;
	}


	for (c = getc(fp); c != EOF; c = getc(fp))
		if (c == '\n') // Increment count if this character is newline
			count = count + 1;

	fclose(fp);
	printf("The file %s has %d lines\n ", filename, count);
	return count;
}

int main(int argc, char *argv[]){
	if(argc != 5){
		printf("Wrong number of arguments\n");
		exit(EXIT_FAILURE);
	}

	char* serverName = argv[1];
	unsigned int serverPORT = (uintptr_t)atoi(argv[2]);
	unsigned int receivePORT = (uintptr_t)atoi(argv[3]);
	char* inputFileWithCommands = argv[4];
	FILE * fPtr;
	fdNode* head = NULL;
	fdNode* tmphead;
	fdNode* tmpNode;
	fdNode* prevNode;

	signal(SIGUSR1,signal_handler);

	if(receivePORT > 65535 || serverPORT > 65535){
		printf("Invalid Port\n");
		exit(EXIT_FAILURE);
	}

	pid_t ppid = getpid();

	int commandNumber = countLines(inputFileWithCommands);
	fork();

	if(getpid() == ppid){
		int sockfd;
		char buffer[UDP_PACKAGE_SIZE];
		struct sockaddr_in servaddr, cliaddr;
		memset(&cliaddr, 0, sizeof(cliaddr));

		//init socket
		{
			// Creating socket file descriptor
			if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
				perror("socket creation failed");
				exit(EXIT_FAILURE);
			}

			memset(&servaddr, 0, sizeof(servaddr));

			// Filling server information
			servaddr.sin_family	   = AF_INET; // IPv4
			servaddr.sin_addr.s_addr = INADDR_ANY;
			servaddr.sin_port = htons(receivePORT);

			// Bind the socket with the server address
			if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
		}

		int n, receivedCommands = 0;
		socklen_t len;
		char strInstr[11],strPackage[11],strTmp[11],fileName[100];
		int instrNumber,packageNumber,tmp;

		len = sizeof(cliaddr);
		//head != NULL ||
		start:
		while(receivedCommands < commandNumber || finishedSending == 0){

			//printf("\n===================================================\nReceived commands %d/%d\n",receivedCommands,commandNumber );
			//printf("Status %d\n",finishedSending );
			tmpNode = head;
			//printf("head -- %d\n",head );
			while(tmpNode != NULL)
				//printf("\n---------\nNODE %d	received %d/%d \n------------\n",tmpNode->instrNumber,tmpNode->receivedPackages,tmpNode->totalPackages );
				tmpNode = tmpNode->next;

			bzero(buffer,sizeof(buffer));

			n = recvfrom(sockfd, (char *)buffer, UDP_PACKAGE_SIZE,
					MSG_WAITALL, ( struct sockaddr *) &cliaddr,
					&len);

			buffer[n] = '\0';

			strncpy(strInstr,buffer,10);
			instrNumber = atoi(strInstr);
			instrNumber++;

			strncpy(strPackage ,&buffer[10],10);
			packageNumber = atoi(strPackage);
			//generate fileName
			sprintf(fileName,"output.%d,%d",receivePORT,instrNumber);
			//printf("Instruction %d packageNumber %d\n",instrNumber, packageNumber );
			// Package number = 0 means that all output is within one package
			if(packageNumber == 0){
				//printf("--1\n");
				//create file
				fPtr = fopen(fileName, "w");


				if(fPtr == NULL){
					printf("Unable to create file.%s \n",fileName);
					exit(EXIT_FAILURE);
				}
				//fseek(fPtr,512*instrNumber,SEEK_SET);
				fputs(&buffer[20], fPtr);
				fclose(fPtr);
				receivedCommands ++;
			}else{
				tmpNode = NULL;
				prevNode = NULL;

				if(head != NULL){
					tmpNode = head;
					prevNode = NULL;
					while(tmpNode != NULL){
						//found node
						if(tmpNode->instrNumber == instrNumber){
							//set file pointer
							fPtr = tmpNode->fp;

							//final package, set total packages to wait
							if(packageNumber == 0xFFFFFFFF){
								//printf("Setting--------\n");
								strncpy(strTmp,&buffer[20],10);
								tmp = atoi(strTmp);

								tmpNode->totalPackages = tmp;
								//if all packages arrived, close file
								if(tmpNode->receivedPackages == tmp){
									//printf("Arrived--------\n");
									receivedCommands ++;
									fclose(fPtr);

									//and delete node from list
									if(prevNode != NULL){
										//printf("Arrived--------1\n");
										prevNode->next = tmpNode->next;
										free(tmpNode);
										goto start;
									}else{
										//printf("Arrived--------2\n");
										head = tmpNode->next;
										free(tmpNode);
										goto start;
									}
								}
							//else if it is a normal package
							}else{
								tmpNode->receivedPackages ++;
								//printf("\n\nWriting package %d %d/%d\n\n",packageNumber,tmpNode->receivedPackages,tmpNode->totalPackages);
								//write to specific position in file
								fseek(fPtr,(512-20)*(packageNumber-1),SEEK_SET);
								fputs(&buffer[20], fPtr);

								//if all packages arrived, close file
								if(tmpNode->receivedPackages == tmpNode->totalPackages){
									//printf("\n\nClosing command %d\n\n",instrNumber );
									receivedCommands ++;
									fclose(fPtr);

									//and delete node from list
									if(prevNode != NULL){
										prevNode->next = tmpNode->next;
										free(tmpNode);
										goto start;
									}else{
										head = tmpNode->next;
										free(tmpNode);
										goto start;
									}
								}
							}
							goto start;
						}
						prevNode = tmpNode;
						tmpNode = tmpNode->next;
					}
				}


				//Node doesnt exist, first package of instruction arrived

				//printf("----------------------------\n\n\n\nNEW\n\n\n\n");
				tmphead = malloc(sizeof(fdNode));
				//printf("pointer : %d\n",tmphead );
				//if list is empty


				tmphead->instrNumber = instrNumber;
				tmphead->next = NULL;
				tmphead->receivedPackages = 0;
				tmphead->totalPackages = -1;
				tmphead->fp = fopen(fileName, "w");

				if(tmphead->fp == NULL){
					perror("Error: ");
					printf("--Unable to create file.%s \n",fileName);
					exit(EXIT_FAILURE);
				}

				fPtr = tmphead->fp;
				//check package type
				if(packageNumber == 0xFFFFFFFF){
					//printf("Setting--------NEW\n");
					strncpy(strTmp,&buffer[20],10);
					tmp = atoi(strInstr);

					tmphead->totalPackages = tmp;

				//else if it is a normal package
				}else{
					//printf("????\n");
					tmphead->receivedPackages ++;
					//write to specific position in file
					fseek(fPtr,(512-20)*(packageNumber-1),SEEK_SET);
					fputs(&buffer[20], fPtr);

				}
				if(prevNode == NULL){
					//printf("\n\n\nNew 1\n\n\n");
					head = tmphead;
				}else{
					//printf("New 2\n");
					prevNode->next = tmphead;
				}
			}
		}
		close(sockfd);
	}else{//child
		printf("%s %s\n",serverName,inputFileWithCommands);

		int sockfd;
		struct sockaddr_in servaddr;

		// socket create and varification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			printf("socket creation failed...\n");
			exit(0);
		}
		else
			printf("Socket successfully created..\n");

		bzero(&servaddr, sizeof(servaddr));

		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(serverName);
		servaddr.sin_port = htons(serverPORT);

		// connect the client socket to server socket
		if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
			printf("connection with the server failed...\nPossibly -Wrong serverName or serverPORT\n or \t -Server socket Queue Full\n");
			exit(0);
		}
		else
			printf("connected to the server..\n");

		// function for chat
		sendCommands(sockfd,receivePORT,inputFileWithCommands);

		// close the socket
		close(sockfd);
	}

	printf("(CLIENT)Terminating process %d \n", getpid());

}



/*
Questions

-what is server name ?

-Are we allowed to spawn one child of the client
	so the parent can wait for the results
	while the kid sends the commands to the server and then terminate ?


- UDP by default assures us that any package could be lost. Do we have to think that a package could not be succesfully sent to the server?



*/
