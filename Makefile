#CC	= 	cc
CC	= 	gcc
CFLAGS	=	-g -Wall
COMPILE	=	$(CC) $(CFLAGS)

all:	client server

client3:	client.c
	$(COMPILE) -o client client.c

server3:	server.c
	$(COMPILE) -o server server.c -lpthread

clean:
	rm -rf *.o client server