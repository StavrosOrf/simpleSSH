CC=gcc
CFLAGS=-g

all: remoteServer remoteClient

remoteServer: remoteServer.c
	$(CC) $(CFLAGS) -o remoteServer remoteServer.c

remoteClient: remoteClient.c
	$(CC) $(CFLAGS) -o remoteClient remoteClient.c

clean :
	rm *.o output* remoteServer remoteClient *.out

cr:
	make
	./remoteServer 50000	5
# 	./remoteClient 127.0.0.0 8080 8082 inputFile1
