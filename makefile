CC=gcc
CFLAGS=-g

all: remoteServer remoteClient

remoteServer: remoteServer.o 
	$(CC) $(CFLAGS) -o remoteServer remoteServer.o

remoteClient: remoteClient.o 
	$(CC) $(CFLAGS) -o remoteClient remoteClient.o

clean :
	rm *.o output* remoteServer remoteClient 

cr:
	make
	./remoteServer 8080	5
# 	./remoteClient 127.0.0.0 8080 8082 inputFile1
