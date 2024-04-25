#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    int client_id = atoi(argv[1]);
    int socket_fd;
    struct sockaddr_in server_address;
    char buffer[1024];
    int bytes_received;

    printf("This is client #%d\n", client_id);

    // Create client socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Set server IP address
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    // Read the server status
    bytes_received = recv(socket_fd, buffer, 1024, 0);
    buffer[bytes_received] = '\0';
    if (strcmp(buffer, "FULL") == 0) {
        printf("Server is full\n");
        close(socket_fd);
        return 0;
    }

    // Send client ID to server
    char client_id_str[10];
    sprintf(client_id_str, "%d", client_id);
    send(socket_fd, client_id_str, strlen(client_id_str), 0);

    while (1) {
        printf("Enter request (negative to terminate): ");
        fgets(buffer, 1024, stdin);
        buffer[strlen(buffer) - 1] = '\0';
         int num = atoi(buffer);
    if (num < 0) {
        send(socket_fd, buffer, strlen(buffer), 0);
        printf("\tWill terminate\n");
        break;
    }

    send(socket_fd, buffer, strlen(buffer), 0);

    bytes_received = recv(socket_fd, buffer, 1024, 0);
    buffer[bytes_received] = '\0';
    printf("Result: %s\n", buffer);
}

close(socket_fd);
return 0;
}