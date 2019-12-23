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
#define BUFFER_SIZE NUMBER_OF_COMMANDS_TO_SEND*101 + 6 + 11 
#define SA struct sockaddr 

void sendCommands(int sockfd ,int receivePORT,char* fileName) 
{ 
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
         	//printf("-%c\n",line[99] );
         	// if command larger that 100 characters, write an empty string to buffer
         	if(line[99] != '\0'){
         		//printf("%s\n",line );
         		bzero(line,sizeof(line));
         		printf("Too many characters\n");

         		//flush the remaining line and move the pointer to the new line
         		while(1){
         			fgets(line, sizeof(line), file);
        			if(line[99] == '\0'){
        				break;
 	        		}     			
 					bzero(line,sizeof(line));
         		}
         		bzero(line,sizeof(line));
         	}
         	
         	

         	//copy command to buffer
           	strcpy(&buffer[101*n + 17],line);

         	numberOfInstructions ++;
         	n++;
         }

        sprintf(strInstr,"%10u",numberOfInstructions);
        strncpy(&buffer[5],strInstr,10);

        i =0;
    	for(i=0;i<BUFFER_SIZE;i++){
    		if(i==17 || (i-17)%101 == 0){
    			printf("\n");
    		}
    		printf("%c ",buffer[i]);
    	}
    	printf("\n");


    	//Send to Server through TCP socket
        write(sockfd, buffer, sizeof(buffer)); 
        

        if ((strncmp(buffer, "exit", 4)) == 0) { 
            printf("Client Exit...\n"); 
            break; 
        }
        if(!terminatingFlag)
        	sleep(5);
    } 

    fclose(file);
} 
  
int main(int argc, char *argv[]) 
{ 
	char* serverName = argv[1];
	unsigned int serverPORT = (uintptr_t)atoi(argv[2]);
	unsigned int receivePORT = (uintptr_t)atoi(argv[3]);
	char* inputFileWithCommands = argv[4];

	if(receivePORT > 65535 || serverPORT > 65535){
		printf("Invalid Port\n");
		exit(1);
	}

	pid_t ppid = getpid();
	printf("Parent pid %d \n", getpid() );


	pid_t childPid = fork();

	if(getpid() == ppid){
		printf("Child pid %d \n", childPid );
		//TODO KALLINTERIS create udp connection and receive data (512 bytes)
		//and write each instruction to a tmp file, consider that every instruction returns at max 512 bytes

		// dont exit ,just wait for data
		while(1){

		}
 
	}else{
		printf("PID = %d and pid = %d\n",getpid(), ppid);

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

	printf("Out %d parent is %d\n", getpid(),getppid());
	
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