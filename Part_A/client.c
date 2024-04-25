#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 8080

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1025];

    if (argc < 2) {
       fprintf(stderr,"usage %s client_id\n", argv[0]);
       exit(0);
    }

    int client_id = atoi(argv[1]);
    portno = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    read(sockfd, buffer, 1024);
    
    if (strcmp(buffer, "FULL") == 0) {
        printf("Server is full\n");
        close(sockfd);
        return 0;
    }

    printf("This is client # %d \n", client_id);

    sprintf(buffer, "%d", client_id);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
         error("ERROR writing to socket");
    }

    do {
        printf("Enter request (negative to terminate): ");
        bzero(buffer, 1025);
        fgets(buffer, 1024, stdin);
        int number = atoi(buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");

        if (number < 0) {
            printf("\t Will terminate\n");
            break;
        }

        bzero(buffer, 1025);
        n = read(sockfd, buffer, 255);
        if (n < 0) 
             error("ERROR reading from socket");

        printf("\t Result: %s\n", buffer);
    } while (1);

    close(sockfd);
    return 0;
}