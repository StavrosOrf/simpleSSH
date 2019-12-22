
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

#define NUMBER_OF_COMMANDS_TO_SEND 10
#define MAX NUMBER_OF_COMMANDS_TO_SEND*100 
#define SA struct sockaddr 

void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    while(1) { 
        bzero(buff, sizeof(buff)); 

        printf("Enter the string : "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n') 
        	;

        write(sockfd, buff, sizeof(buff)); 
        
        bzero(buff, sizeof(buff)); 
        read(sockfd, buff, sizeof(buff)); 
        printf("From Server : %s", buff); 
        if ((strncmp(buff, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        } 
    } 
} 
  
int main(int argc, char *argv[]) 
{ 
	char* serverName = argv[1];
	unsigned int serverPORT = (uintptr_t)atoi(argv[2]);
	unsigned int receivePORT = (uintptr_t)atoi(argv[3]);
	char* inputFileWithCommands = argv[4];

	pid_t ppid = getpid();
	printf("Parent pid %d \n", getpid() );


	fork();

	if(getpid() == ppid){
		//TODO KALLINTERIS create udp connection and receive data (512 bytes)
		//and write each instruction to a tmp file, consider that every instruction returns at max 512 bytes

		// dont exit ,just wait for data
 
	}else{
		printf("PID = %d and pid = %d\n",getpid(), pid);

	if(argc == 1 || argc >5){
		printf("Wrong number of arguments\n");
		exit(1);
	}
	printf("%s %s\n",serverName,inputFileWithCommands );

    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
  
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
        printf("connection with the server failed...\nPossibly -Wrong serverName or serverPORT\n or \t-Server socket Queue Full\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // function for chat 
    func(sockfd); 
  
    // close the socket 
    close(sockfd); 
	}
	
} 



/*
Questions

-what is server name ?

-Are we allowed to spawn one child of the client
	so the parent can wait for the results 
	while the kid sends the commands to the server and then terminate ?

-Mutex allowed for pipe synchronization ?( though i dont think it is necessary to use them)

- UDP by default assures us that any package could be lost. Do we have to think that a package could not be succesfully sent to the server?



*/