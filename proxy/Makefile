CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -L /usr/lib/happycoders -lnsl -lrt -lsocket -lcrypto

OBJS = nethelp.o webproxy.o 

all: webproxy 

nethelp.o: nethelp.c
	$(CC) $(CFLAGS) -c nethelp.c

webproxy.o: webproxy.c
	$(CC) $(CFLAGS) -c webproxy.c

webproxy: webproxy.o nethelp.o
	$(CC) -o $@ webproxy.o nethelp.o $(LDFLAGS)

clean:
	rm -f webproxy *.o *~ core
