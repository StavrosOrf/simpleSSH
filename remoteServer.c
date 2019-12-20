#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <inttypes.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <signal.h>
#include <limits.h>

#define MAX_NUMBER_OF_CLIENTS_IN_QUEUE 100	
#define SA struct sockaddr
#define MAX 10*100 
#define MAX_COUNT  200
#define BUF_SIZE   100


// Function designed for chat between client and server. 
void accept_commands(int sockfd,int pipeWrite) 
{ 
    char buff[MAX]; 
    char tmpBuff[MAX];
    int n; 
    // infinite loop for chat 
    while (1) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t \n", buff); 
        strcpy(tmpBuff,buff);
        bzero(buff, MAX); 
        n = 0; 

         
        // TODO Write each unique command into the pipe using 100 bytes windows (to be implemented soon)
        //while()
       
  		printf("Written %d bytes : %s\n ", write(pipeWrite,tmpBuff,BUF_SIZE),tmpBuff );

        // and send that buffer to client 
        write(sockfd, "OK \n", sizeof(buff)); 

  		
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 

int main(int argc, char *argv[]) {

	unsigned int numberOfChilds = (uintptr_t)atoi(argv[2]);
	unsigned int port = (uintptr_t)atoi(argv[1]);

	pid_t  pid;
	pid_t  ppid = getpid();
	int p[2],nbytes;

	char inbuf[120]; 

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
	    servaddr.sin_port = htons(port); 

	     // Binding newly created socket to given IP and verification 
	    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
	        printf("socket bind failed...\n"); 
	        exit(0); 
	    }
	    else
	        printf("Socket successfully binded..\n"); 
	  
	    // Now server is ready to listen and verification 
	    if ((listen(sockfd, MAX_NUMBER_OF_CLIENTS_IN_QUEUE)) != 0) { 
	        printf("Listen failed...\n"); 
	        exit(0); 
	    } 
	    else
	        printf("Server listening..\n"); 
	    len = sizeof(client); 
	  
	    // Accept the data packet from client and verification 
	    connfd = accept(sockfd, (SA*)&client, &len); 
	    if (connfd < 0) { 
	        printf("server acccept failed...\n"); 
	        exit(0); 
	    } 
	    else
	        printf("server acccept the client...\n"); 
	  
	    // Function for chatting between client and server 
	    accept_commands(connfd,p[1]); 
	  
	    // After chatting close the socket 
	    close(sockfd); 

    }else{//if child wait parent for commands
    	
    	char tempfileName[16] = "";
    	char path[1035];

    	sprintf(tempfileName,"%d",getpid());

    	FILE *fp;
    	
    	strcat(tempfileName,".txt");
    	printf("tmp file name :%s\n",tempfileName);
    	//strcat(tempfileName,)
    	while(1){
    		while ((nbytes = read(p[0], inbuf, BUF_SIZE)) > 0) {
    			printf("Child %d read %d bytes and  says %s\n",getpid(),nbytes, inbuf);
            	break; 
    		}

    		// strtok(inbuf,"\n");
      //       //we redirect command output to a file 
    		// sprintf(inbuf,"%s >> %s",inbuf,tempfileName);
      //   	printf("Child %d Finished reading the command %s\n",getpid(),inbuf);

      //   	system(inbuf); // inbuf + ">> tmp.childpid.txt"


        	//other approach

        	fp = popen(inbuf, "r");
		  	if (fp == NULL) {
		    	printf("Failed to run command\n" );
		    	exit(1);
		    }
		    
		    //TODO Read and send output back to client
		    while (fgets(path, sizeof(path), fp) != NULL) {
    			printf("%s", path);
  			}

		    pclose(fp);
        	// TODO Execute the command and store result to a local file
    	}
    }
    printf("Out %d parent is %d\n", getpid(),getppid());

	return 0;
}