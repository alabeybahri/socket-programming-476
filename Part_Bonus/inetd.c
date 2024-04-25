#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle_client(int client_sock, int service_port) {
    char *service;
    if (service_port == 5010) service = "./square";
    else if (service_port == 5020) service = "./cube";

    char client_sock_str[10];
    sprintf(client_sock_str, "%d", client_sock);

    if (fork() == 0) {
        execl(service, service, client_sock_str, (char *)NULL);
        exit(EXIT_FAILURE); // If execl fails
    }
    wait(NULL); // Wait for the child to terminate
    close(client_sock);
}

void setup_server(int port) {
    int socket_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error on binding");
        exit(1);
    }

    listen(socket_fd, 5);


    while (1) {
        client_sock = accept(socket_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_sock < 0) {
            perror("Error on accept");
            continue;
        }
        printf("(inetd) Connection request to %s service\n", (port == 5010) ? "square" : "cube");
        handle_client(client_sock, port);
    }
    close(socket_fd);
}

int main() {
    int ports[] = {5010, 5020};

    printf("(inetd) inetd has started\n");

    for (int i = 0; i < 2; i++) {
        if (fork() == 0) {
            setup_server(ports[i]);
            exit(0);  // terminate child process after server function
        }
    }
    printf("(inetd) Listening on ports %d & %d\n", ports[0], ports[1]);

    // parent waits for all child processes
    while (wait(NULL) > 0);

    return 0;
}
