CC = gcc
CFLAGS = -Wall

all: Part_B_client Part_B_server

Part_B_client:
	$(CC) $(CFLAGS) -o ./client ./client.c

Part_B_server:
	$(CC) $(CFLAGS) -o ./server ./server.c

clean:
	rm -f ./client ./server
