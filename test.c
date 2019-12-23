
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

int main(){
	int n = 8080;
	char bytes[4];

	bytes[0] = (n >> 24) & 0xFF;
	bytes[1] = (n >> 16) & 0xFF;
	bytes[2] = (n >> 8) & 0xFF;
	bytes[3] = n & 0xFF;

	printf("%x %x %x %x\n", (unsigned char)bytes[0],
                        (unsigned char)bytes[1],
                        (unsigned char)bytes[2],
                        (unsigned char)bytes[3]);
	return 0;
}
