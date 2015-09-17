CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -L /usr/lib/happycoders -pthread -lnsl -lrt -lsocket 

OBJS = nethelp.o echoserver.o 

all: echoserver 

nethelp.o: nethelp.c
	$(CC) $(CFLAGS) -c nethelp.c

echoserver.o: echoserver.c
	$(CC) $(CFLAGS) -c echoserver.c

echoserver: echoserver.o nethelp.o
	$(CC) $(LDFLAGS) -o $@ echoserver.o nethelp.o

clean:
	rm -f echoserver *.o *~ core
