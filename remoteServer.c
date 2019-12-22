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
    int n,i; 

    fd_set active_fd_set,read_fd_set;
    struct sockaddr_in clientname;
    size_t size;

 	FD_ZERO(&active_fd_set);
    FD_SET (sockfd,&active_fd_set);


    // infinite loop for chat 
    while (1) { 
        bzero(buff, MAX); 
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
        			printf("new Connection %d \n",new);
        			if(new < 0){
        				exit(EXIT_FAILURE);
        			}
        			//fprintf(stderr, "Server : connect from host %s ,port %hd.\n",inet_ntoa(clientname.sin_addr),ntohs(clientname.sin_port) );
        			FD_SET(new,&active_fd_set);
        		}else{
        			//socket is already connected

			        // read the message from client and copy it in buffer
			        // read(sockfd, buff, sizeof(buff)); 
			        read(i, buff, sizeof(buff)); 
			        // print buffer which contains the client contents 
			        printf("From client: %s\t \n", buff); 
			        strcpy(tmpBuff,buff);
			        bzero(buff, MAX); 
			        n = 0; 

			         
			        // TODO Write each unique command into the pipe using 100 bytes windows (to be implemented soon)
			        // TODO parse each line and determine the correct command
			        //while()
			       

			  		printf("Written %d bytes from fd %d: %s\n ", write(pipeWrite,tmpBuff,BUF_SIZE),i,tmpBuff );

			        // and send that buffer to client 
			        write(i, "OK \n", sizeof(buff)); 

			  		// while(1){
			  		// 	;
			  		// }
			        // if msg contains "Exit" then server exit and chat ended. 
			        if (strncmp("exit", buff, 4) == 0) { 
			            
			            printf("Server Exit...\n"); 
			            close(i);
        				FD_CLR(i,&active_fd_set);
			             
			        }         			

        			//no more data

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

	//extern int make_socket(uint16_t port);

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
	    //Accept the data packet from client and verification 
	    // connfd = accept(sockfd, (SA*)&client, &len);
	    // printf("CONN socket %d\n",connfd );
	    // if (connfd < 0) { 
	    //     printf("server acccept failed...\n"); 
	    //     exit(0); 
	    // } 
	    // else
	    //     printf("server acccept the client...\n"); 
	  
	    // Function for chatting between client and server 


	    //sockfd = make_socket(serverPort);
	    // if(listen(sockfd,1) <0){
	    // 	perror("listen");
	    // 	exit(EXIT_FAILURE);
	    // }
	    accept_commands(sockfd,p[1]); 
	  
	    // After chatting close the socket 
	    close(sockfd); 

    }else{//if child wait parent for commands
    	
    	char tempfileName[16] = "";
    	char path[1035];

    	// sprintf(tempfileName,"%d",getpid());
    	// strcat(tempfileName,".txt");
    	// printf("tmp file name :%s\n",tempfileName);

    	FILE *fp;
    	

    	//strcat(tempfileName,)
    	while(1){

    		while ((nbytes = read(p[0], inbuf, BUF_SIZE)) > 0) {
    			printf("Child %d read %d bytes and  says %s\n",getpid(),nbytes, inbuf);
            	break; 
    		}

    		//TODO set up client udp connection variables (port,address)
    		int clientPort = 8082;

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

		    // send instruction number  and packet number along with 504 bytes of data
		    while (fgets(path, sizeof(path), fp) != NULL) {
    			printf("%s", path);
  			}


  			int clientPort = 0;
  			int instructionNumber = 0;
  			//TODO kallinteris enstablish udp connection on variables
  			// and send 512 bytes of whatever 


		    pclose(fp);
        	
    	}
    }
    printf("Out %d parent is %d\n", getpid(),getppid());

	return 0;
}