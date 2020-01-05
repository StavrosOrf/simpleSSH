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

client:
	./remoteClient 127.0.0.1 50000 50001 ./tests/unit-test-EOL
