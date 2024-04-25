#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int client_sock = atoi(argv[1]);
    int num;
    ssize_t bytes_read;


    // Read the number from the client
    bytes_read = read(client_sock, &num, sizeof(num));
    if (bytes_read < 0) {
        perror("Error reading from client");
        return 1;
    }

    printf("(cube) Request=%d\n", num);
    // Calculate the cube
    int cubed = num * num * num;

    // Send the result back to the client
    bytes_read = write(client_sock, &cubed, sizeof(cubed));
    if (bytes_read < 0) {
        perror("Error writing to client");
        return 1;
    }

    printf("(cube) Reply sent as %d. Terminating...\n", cubed);

    close(client_sock);
    return 0;
}