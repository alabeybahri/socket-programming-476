#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Structure to hold client_count
typedef struct {
    pthread_mutex_t mutex;
    int client_count;
} client_count_t;

void handle_client(int client_socket, client_count_t *client_count_shared) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int num;
    int client_id;

    // Receive client ID from client
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[bytes_received] = '\0';
    client_id = atoi(buffer);

    printf("(child #%d) Child created for incoming request\n", client_id);

    while (1) {
        if((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) == 0) {
            printf("(child #%d) Client disconnected\n", client_id);
            break;
        }

        buffer[bytes_received] = '\0';
        num = atoi(buffer);

        if (num < 0) {
            printf("(child #%d) Request=%d Will terminate\n", client_id, num);
            break;
        }

        printf("(child #%d) Request=%d\n", client_id, num);

        // Send response back to client
        int response = num * num;
        sprintf(buffer, "%d", response);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    
    close(client_socket);
    pthread_mutex_lock(&client_count_shared->mutex);
    (client_count_shared->client_count)--;
    pthread_mutex_unlock(&client_count_shared->mutex);
    exit(0);
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t addrlen = sizeof(client_address);
    pid_t pid;

    client_count_t *client_count_shared = mmap(NULL, sizeof(client_count_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (client_count_shared == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&client_count_shared->mutex, &attr);
    client_count_shared->client_count = 0;

    printf("(parent) Server has started\n");
    printf("(parent) Waiting for connections\n");

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        new_socket = accept(server_socket, (struct sockaddr *)&client_address, &addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&client_count_shared->mutex);
        if (client_count_shared->client_count < MAX_CLIENTS) {
            (client_count_shared->client_count)++;
            pthread_mutex_unlock(&client_count_shared->mutex);
            pid = fork();
            if (pid < 0) {
                perror("fork failed");
                pthread_mutex_lock(&client_count_shared->mutex);
                (client_count_shared->client_count)--;
                pthread_mutex_unlock(&client_count_shared->mutex);
                continue;
            } else if (pid == 0) { // Child process
                close(server_socket);
                send(new_socket, "OK\0", strlen("OK\0"), 0);
                handle_client(new_socket, client_count_shared);
            } else { // Parent process
                close(new_socket);
            }
        } else {
            pthread_mutex_unlock(&client_count_shared->mutex);
            send(new_socket, "FULL\0", strlen("FULL\0"), 0);
            close(new_socket);
        }
    }

    return 0;
}
