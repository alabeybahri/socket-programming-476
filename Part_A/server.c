#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>

#define TRUE 1 
#define PORT 8080 
#define MAX_CLIENTS 3

int main(int argc , char *argv[]) 
{ 
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS] , activity, i , valread , sd, max_sd; 
	int max_clients = MAX_CLIENTS; 
	int connection_count = 0;
	struct sockaddr_in address; 
	int client_ids[MAX_CLIENTS];
	int client_id;
	char buffer[1025];
	fd_set readfds; 
	
	for (i = 0; i < max_clients; i++) 
	{ 
		client_socket[i] = 0; 
	} 
	
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
		sizeof(opt)) < 0 ) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	puts("Server has started"); 
	
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	
	//accept the incoming connection 
	addrlen = sizeof(address); 
	puts("Waiting for connections"); 
	
	while(TRUE) 
	{ 
		//clear the socket set 
		FD_ZERO(&readfds); 
		//add master socket to set 
		FD_SET(master_socket, &readfds); 
		max_sd = master_socket; 
		
		
		//add client sockets to set 
		for ( i = 0 ; i < max_clients ; i++) 
		{ 
			//socket descriptor 
			sd = client_socket[i]; 
			
			//if valid socket descriptor then add to read list 
			if(sd > 0) {
				FD_SET( sd , &readfds); 
			}

			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) {
				max_sd = sd; 
			}
		} 

		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
		
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
		
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds)) 
		{ 

			// Check if there are already 3 connections, if so reject the new connection
			if (connection_count >= MAX_CLIENTS) 
			{
				if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
				{ 
					perror("accept"); 
					exit(EXIT_FAILURE); 
				}

				send(new_socket, "FULL\0", strlen("FULL\0"), 0);

				close(new_socket);
				continue;
			}
			// If there are less than 3 connections, accept the new connection
			else {
			//accept the incoming connection 
			if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			send(new_socket, "OK\0", strlen("OK\0"), 0);
			int valread = recv(new_socket, buffer, sizeof(buffer), 0);
			buffer[valread] = '\0';
			printf("Incoming request from client # %s\n" , buffer);
			connection_count++;

			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( client_socket[i] == 0 ) 
				{ 
					client_socket[i] = new_socket; 
					client_ids[i] = atoi(buffer);
					break; 
				} 
			} 	
			}
		}
		
		//else its some IO operation on some other socket 
		for (i = 0; i < max_clients; i++) 
		{
			sd = client_socket[i]; 
			client_id = client_ids[i];

			if (FD_ISSET(sd , &readfds)) 
			{ 
				if ((valread = recv(sd, buffer, sizeof(buffer), 0)) == 0) 
				{
					printf("(client # %d) Disconnected\n" ,client_id );
					//remove from list of clients 
					client_socket[i] = 0; 
					client_ids[i] = 0;

					// Decrement the connection count
					connection_count--;
				} 

				else
				{
					//read the incoming message
					buffer[valread] = '\0';

					//convert the message to integer
					int num = atoi(buffer);

					//if the number is smaller than 0, close the connection
					if(num < 0)
					{
						printf("(client # %d) Request=%d dropping connection\n" ,client_id , num);
						close(sd);
						client_socket[i] = 0;
						client_ids[i] = 0;
						// Decrement the connection count
						connection_count--;
					}
					else
					{
						printf("(client # %d) Request=%s" ,client_id , buffer);

						//calculate the square of the number
						int square = num * num;

						//convert the square back to string
						char square_str[1024];
						sprintf(square_str, "%d", square);

						//send the square back to client
						if(send(sd, square_str, strlen(square_str), 0) != strlen(square_str)) 
						{
							printf("Error sending message to client\n"); 
						} 
					}
				}
			} 
		}
	} 
	
	return 0; 
}