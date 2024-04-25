CC = gcc
CFLAGS = -Wall

all: Part_A_client Part_A_server

Part_A_client:
	$(CC) $(CFLAGS) -o ./client ./client.c

Part_A_server:
	$(CC) $(CFLAGS) -o ./server ./server.c

clean:
	rm -f ./client ./server
