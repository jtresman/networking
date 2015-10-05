CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -L /usr/lib/happycoders -lnsl -lrt -lsocket

OBJS = nethelp.o server.o client.o

all: server client

nethelp.o: nethelp.c
		$(CC) $(CFLAGS) -c nethelp.c

server.o: server.c
		$(CC) $(CFLAGS) -c server.c

client.o: client.c
		$(CC) $(CFLAGS) -c client.c

server: server.o nethelp.o
		$(CC) $(LDFLAGS) -o $@ server.o nethelp.o

client: client.o nethelp.o
		$(CC) $(LDFLAGS) -o $@ client.o nethelp.o

clean:
		rm -rf server client *.o *~ core ./DFS1 ./DFS2 ./DFS3 ./DFS4