#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int sock, port;
    struct sockaddr_in server_addr;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    int num, result;
    ssize_t bytes_read;
    ssize_t bytes_written;

    printf("Enter a number: ");
    scanf("%d", &num);


    bytes_written = write(sock, &num, sizeof(num));

    if (bytes_written < 0) {
        perror("Error writing to socket");
        return 1;
    }

    bytes_read = read(sock, &result, sizeof(result));

    if (bytes_read < 0) {
        perror("Error reading from socket");
        return 1;
    }

    printf("\tResult: %d\n", result);

    close(sock);

    return 0;
}
