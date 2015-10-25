CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -L/usr/lib/happycoders -lnsl -lrt -lsocket -lcrypto

OBJS = nethelp.o server.o client.o

all: server client

nethelp.o: nethelp.c
		$(CC) $(CFLAGS) -c nethelp.c

server.o: server.c
		$(CC) $(CFLAGS) -c server.c

client.o: client.c
		$(CC) $(CFLAGS) -c client.c

server: server.o nethelp.o
		$(CC) -o $@ server.o nethelp.o $(LDFLAGS)

client: client.o nethelp.o
		$(CC) -o $@ client.o nethelp.o $(LDFLAGS) 

clean:
		rm -rf server client *.o *~ core ./DFS1 ./DFS2 ./DFS3 ./DFS4