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
#include <sys/wait.h>
#include <ctype.h>

#define NUMBER_OF_COMMANDS_TO_SEND 10
#define BUFFER_SIZE NUMBER_OF_COMMANDS_TO_SEND*101 + 6 + 11 
#define SOCKET struct sockaddr
#define MAX 10*100 
#define MAX_COUNT  200
#define PIPE_BUFFER_SIZE   118
char* COMMANDS[5] = {"ls","cut","tr","cat","grep"};
pid_t ppid;
int * childrenPID;
unsigned int numberOfChilds;

void signal_handlerEnd(int sig){
		
	for(int i = 0; i<FD_SETSIZE;i++){
		if(i != 2){
			close(i);			
		}


	}
	fprintf(stderr,"Terminated process with pid : %d  \n", getpid());
	close(2);
	exit(0);
}

void signal_handlerStop(int sig){
	//printf("Pid : %d Signal Stop\n",getpid() );
	int status = 0;

	//if parent, wait for all children to exit
	if(ppid == getpid()){
        for(int i = 0 ; i < numberOfChilds; i++){
    		kill(childrenPID[i],SIGUSR2);
    	}

		while(wait(&status) > 0); 
	}
	
	

	for(int i = 0; i<FD_SETSIZE;i++){
		if(i != 2){
			close(i);			
	}


	}
	fprintf(stderr,"Terminated process with pid : %d  \n", getpid());
	close(2);
	exit(0);

}
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
        			new = accept(sockfd,(SOCKET*) &clientname,&size);

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

			        n = 0; 

			        instructionsSent[i] += instrNumber;
        			

        		}

        	}
        }

    } 
} 

int main(int argc, char *argv[]) {

	numberOfChilds = (uintptr_t)atoi(argv[2]);
	unsigned int serverPort = (uintptr_t)atoi(argv[1]);

	pid_t  pid;
	ppid = getpid();
	int p[2],nbytes;

	char inbuf[PIPE_BUFFER_SIZE]; 

	// link signal handlers
	signal(SIGUSR2,signal_handlerStop);
	signal(SIGUSR1,signal_handlerEnd);
	//Create Pipe

	if(pipe(p) != 0){
		printf("Error while creating pipe\n");
		exit(1);
	}
	signal(SIGPIPE, SIG_IGN);

	printf("Parent pid %d \n", ppid );

	if(argc == 1 || argc >3){
		printf("Wrong number of arguments\n");
		exit(1);
	}

	if(numberOfChilds <= 0 ){
		printf("Not enough childs\n");
		exit(1);
	}

	childrenPID = malloc(sizeof(int)*numberOfChilds);

    int i = 0,k;
    //Create child processes
    for(i = 0 ; i < numberOfChilds; i++){
    	k = fork();
    	//printf("%d %d\n", getpid(),numberOfChilds);

    	if(getpid() != ppid ){
    		break;
    	}
    	childrenPID[i] = k;
    }




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
	    if ((bind(sockfd, (SOCKET*)&servaddr, sizeof(servaddr))) != 0) { 
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

	    accept_commands(sockfd,p[1]); 
	  
	    // After chatting close the socket 
	    close(sockfd); 

    }else{//if child wait parent for commands
    	
    	char tmp1[6],tmp2[11],tmp3[PIPE_BUFFER_SIZE],tmp4[PIPE_BUFFER_SIZE],tmp5[PIPE_BUFFER_SIZE],finalCmd[PIPE_BUFFER_SIZE];
    	char path[1035];
    	int instrNumber,clientPort ;
    	char* token;
    	char* token2;
    	char* token3;
    	char **saveptr1, **saveptr2, **saveptr3;
    	const char d[2] = ";",d1[2] = "|",d2[2] = " ";
    	int n,i,flag;

    	FILE *fp;
    	
    	while(1){
    		start:
    		n = 0;
    		bzero(inbuf,sizeof(inbuf));
    		//printf("Process %d ,Gonna block\n",getpid());
    		while ((nbytes = read(p[0], inbuf, PIPE_BUFFER_SIZE)) > 0) {
    			printf("Child %d read %d bytes and says %s\n",getpid(),nbytes, &inbuf[17]);
            	break; 
    		}

    		strncpy(tmp4,&inbuf[17],100);
    		//printf("test1\n");
    		strncpy(tmp2,&inbuf[7],10);
    		//printf("test2\n");
			instrNumber = atoi(tmp2);
    		//printf("test3\n");
			strncpy(tmp1,inbuf,6);
			//printf("test4\n");
			clientPort = atoi(tmp1);
			//printf("tokensInbuf:%s-\n",&inbuf[17] );
			//printf("tokens:%s-\n",tmp4 );

        	//Edit commands and accept only the ones from COMMANDS table--------------------------------

			token = strtok(tmp4,d2);
			//printf("token:%s-\n",token );
        	if (strcmp("end",token) == 0 || strcmp("end\n",token) == 0 ){
        	 	kill(getppid(),SIGUSR1);
        	 	goto start;

        	}else if(strcmp("timeToStop",token) == 0 || strcmp("timeToStop\n",token) == 0){
        	 	kill(getppid(),SIGUSR2);
        	 	goto start;
        	}


        	strncpy(tmp4,&inbuf[17],100);

        	bzero(finalCmd,sizeof(finalCmd));

			token = strtok(tmp4,d);

			strcpy(tmp4,token);
			//printf("=%s\n",tmp4 );

			strcpy(tmp3,tmp4);
			token = strtok(tmp3,d1);

			while(token != NULL){
				
				flag = 0;
				n = 0;
				bzero(tmp5,PIPE_BUFFER_SIZE);

				for(int i=0; i<100 ; i++){
					//printf("-+%c\n",token[i] );
					if(!isspace(token[i]) ){
						//printf("=-+%c\n",token[i] );
						flag = 1;
						tmp5[n] = token[i];
						n++;
					}else if(( token[i] == '\0' || isspace(token[i])) && flag){
						tmp5[n] = '\0';
						//printf("==%s\n",tmp5 );
						//check if command is a supported command 
						for(int j=0 ; j<5;j++){
							if(strcmp(tmp5,COMMANDS[j]) == 0){
								if(finalCmd[0] != '\0'){
									strcat(finalCmd,"|");
								}
								strcat(finalCmd,token);
								goto con;
							}
						}
						//if programm reach here , it means command is not supported
						//so every following pipe-command is discarded
						goto execute;
					}

					
				}
				con:
				//printf("FINAL = %s\n",finalCmd );

				token = strtok(NULL,d1);

			}

			// printf("Token1 %s\n",token );

			execute:

			printf("FINAL cmd to execute = %s\n",finalCmd );
        	fp = popen(finalCmd, "r");
		  	if (fp == NULL) {
		    	printf("Failed to run command\n" );
		    	exit(1);
		    }
		    
		    //TODO Read and send output back to client

		    // send instruction number  and packet number along with 504 bytes of data
		   //  while (fgets(path, sizeof(path), fp) != NULL) {
    	// 		printf("Child %d command %d to port %d: %s", getpid(),instrNumber,clientPort,path);
  			// }


  			
  			

  			//TODO kallinteris enstablish udp connection on variables
  			// and send 512 bytes of whatever 


		    pclose(fp);
        	
    	}
    }
    fprintf(stderr,"Terminated process with pid : %d  \n", getpid());

	return 0;
}