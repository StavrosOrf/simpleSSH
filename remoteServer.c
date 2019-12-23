#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <inttypes.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <signal.h>
#include <limits.h>
#include <netdb.h>
#include <errno.h>

#define NUMBER_OF_COMMANDS_TO_SEND 10
#define BUFFER_SIZE NUMBER_OF_COMMANDS_TO_SEND*101 + 6 + 11 
#define SA struct sockaddr
#define MAX 10*100 
#define MAX_COUNT  200
#define PIPE_BUFFER_SIZE   118


// Function designed for chat between client and server. 
void accept_commands(int sockfd,int pipeWrite) 
{ 
    char buffer[BUFFER_SIZE]; 
    char pipeBuffer[PIPE_BUFFER_SIZE];
    int n,i,j,k,l,instrNumber;

    int instructionsSent[FD_SETSIZE]; 

    fd_set active_fd_set,read_fd_set;
    struct sockaddr_in clientname;
    int size,counter;
    char clientPort[6],strInstrNumber[11];

 	FD_ZERO(&active_fd_set);
    FD_SET (sockfd,&active_fd_set);
 
    while (1) { 
        bzero(buffer, sizeof(buffer)); 
        //printf("Sockets size %d socket: %d\n",FD_SETSIZE , sockfd);

        printf("=====================\n");

        read_fd_set = active_fd_set;
        if(select(FD_SETSIZE,&read_fd_set,NULL,NULL, NULL) < 0 ){
        	perror("select");
        	exit(EXIT_FAILURE);
        }

        for(i = 0; i<FD_SETSIZE;i++){
        	if(FD_ISSET(i,&read_fd_set)){
        		//check if it is a new connection
        		if( i == sockfd){
        			int new;
        			size = sizeof(clientname);
        			new = accept(sockfd,(SA*) &clientname,&size);

        			instructionsSent[new] = 0;
        			printf("new Connection %d \n",new);
        			if(new < 0){
        				exit(EXIT_FAILURE);
        			}
        			FD_SET(new,&active_fd_set);
        		
        		}else{
        			//socket is already connected

        			bzero(pipeBuffer, sizeof(pipeBuffer)); 
        			bzero(buffer, sizeof(buffer)); 

			        // read the message from client and copy it in buffer
			        read(i, buffer, sizeof(buffer)); 

			        //if socket was disconnected 
			        if(buffer[0] == '\0'){
			        	printf("--Socket %d closed...\n", i); 
			            close(i);
        				FD_CLR(i,&active_fd_set);
        				instructionsSent[i] = 0;
			        }

			        
			        for(j = 0; j < 6 ;j++){
			        	pipeBuffer[j] = buffer[j];
			        }

			        strncpy(strInstrNumber,&buffer[6],10);
			        instrNumber = atoi(strInstrNumber);

			        counter = instructionsSent[i];

			        for(k = 0; k < instrNumber ; k++){
			        	sprintf(strInstrNumber,"%10u",counter);
			        	strcpy(&pipeBuffer[7],strInstrNumber);

			        	// printf("Command --: %s\t \n", &buffer[instrNumber*101 + 17]);
			        	// strncpy(&pipeBuffer[17],&buffer[instrNumber*101 + 17],101);

			        	l = 17;
			        	for(j = k*101 + 17 ; j < (k+1)*101 + 17; j++ ){
			        		pipeBuffer[l] = buffer[j];
			        		l++;
			        	}

				    	for(j=0;j<PIPE_BUFFER_SIZE;j++){
				    		printf("%c",pipeBuffer[j]);
				    	}
				    	printf("\n");

				    	write(pipeWrite,pipeBuffer,PIPE_BUFFER_SIZE);
				    	//printf("Written %ld bytes from fd %d:%s\n ", write(pipeWrite,pipeBuffer,PIPE_BUFFER_SIZE),i,&pipeBuffer[17] );

			        	counter++;
			        }
			        // print buffer which contains the client contents 
			        // printf("From client: %s\t \n", &buffer[1]); 
			        // printf("From client2: %s\t \n", &buffer[17]);
			        // printf("From client3: %s\t \n", &buffer[17+101]);  
			    	

			        
			        n = 0; 

			        instructionsSent[i] += instrNumber;
			         
			        // TODO Write each unique command into the pipe using 100 bytes windows (to be implemented soon)
			        // TODO parse each line and determine the correct command
			        //while()
			       

			  		

			        // if msg contains "Exit" then server exit and chat ended. 
			        if (strncmp("exit", buffer, 4) == 0) { 
			            
			            printf("Server Exit...\n"); 
			            close(i);
        				FD_CLR(i,&active_fd_set);
			             
			        }         			

        			

        		}

        	}
        }

    } 
} 

int main(int argc, char *argv[]) {

	unsigned int numberOfChilds = (uintptr_t)atoi(argv[2]);
	unsigned int serverPort = (uintptr_t)atoi(argv[1]);

	pid_t  pid;
	pid_t  ppid = getpid();
	int p[2],nbytes;

	char inbuf[PIPE_BUFFER_SIZE]; 

	//Create Pipe

	if(pipe(p) != 0){
		printf("Error while creating pipe\n");
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN);

	printf("Parent pid %d %d\n", ppid,PIPE_BUF );

	if(argc == 1 || argc >3){
		printf("Wrong number of arguments\n");
		exit(1);
	}

	if(numberOfChilds <= 0 ){
		printf("Not enough childs\n");
		exit(1);
	}



    int i = 0;
    //Create child processes
    for(i = 0 ; i < numberOfChilds; i++){
    	fork();
    	//printf("%d %d\n", getpid(),numberOfChilds);
    	if(getpid() != ppid ){
    		break;
    	}
    }

    //printf("Out %d parent is %d\n", getpid(),getppid());

    //if parent wait for clients' commands
    if(getpid() == ppid){

    	int sockfd, connfd, len; 
    	struct sockaddr_in servaddr, client; 

    	// socket create and verification 
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd == -1) { 
        	printf("socket creation failed...\n"); 
        	exit(1);
        } 

	    bzero(&servaddr, sizeof(servaddr)); 
	  
	    // assign IP, PORT 
	    servaddr.sin_family = AF_INET; 
	    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	    servaddr.sin_port = htons(serverPort); 

	     // Binding newly created socket to given IP and verification 
	    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
	        printf("socket bind failed...\n"); 
	        exit(0); 
	    }
	    else
	        printf("Socket successfully binded..\n"); 
	  
	    // Now server is ready to listen and verification 
	    if ((listen(sockfd, 1)) != 0) { 
	        printf("Listen failed...\n"); 
	        exit(0); 
	    } 
	    else
	        printf("Server listening..\n"); 
	    len = sizeof(client); 
	  	printf("sockfd socket %d\n",sockfd );

	    accept_commands(sockfd,p[1]); 
	  
	    // After chatting close the socket 
	    close(sockfd); 

    }else{//if child wait parent for commands
    	
    	char tmp1[6],tmp2[11];
    	char path[1035];
    	int instrNumber,clientPort ;

    	FILE *fp;
    	
    	while(1){
    		bzero(inbuf,sizeof(inbuf));
    		while ((nbytes = read(p[0], inbuf, PIPE_BUFFER_SIZE)) > 0) {
    			printf("Child %d read %d bytes and  says %s\n",getpid(),nbytes, &inbuf[17]);
            	break; 
    		}

    		strncpy(tmp2,&inbuf[7],10);
    		int i;
    		
			instrNumber = atoi(tmp2);
    		
			strncpy(tmp1,inbuf,6);

			clientPort = atoi(tmp1);

        	//other approach

        	fp = popen(&inbuf[17], "r");
		  	if (fp == NULL) {
		    	printf("Failed to run command\n" );
		    	exit(1);
		    }
		    
		    //TODO Read and send output back to client

		    // send instruction number  and packet number along with 504 bytes of data
		    while (fgets(path, sizeof(path), fp) != NULL) {
    			printf("Child %d command %d to port %d: %s", getpid(),instrNumber,clientPort,path);
  			}


  			
  			

  			//TODO kallinteris enstablish udp connection on variables
  			// and send 512 bytes of whatever 


		    pclose(fp);
        	
    	}
    }
    printf("Out %d parent is %d\n", getpid(),getppid());

	return 0;
}