CC=gcc

all: remoteServer remoteClient

remoteServer: remoteServer.o 
	$(CC) -o remoteServer remoteServer.o

remoteClient: remoteClient.o 
	$(CC) -o remoteClient remoteClient.o

clean :
	rm *.o *.txt remoteServer remoteClient 

cr:
	make
	./remoteServer 8080	2 
# 	./remoteClient