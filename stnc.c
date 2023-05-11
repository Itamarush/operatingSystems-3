#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MAX_CONN 1

void server_mode(int port, int progress, int quiet);

void client_mode(const char *ip, int port, char* type, char* param);

void handle_connection(int sockfd);

int main(int argc, char *argv[]) {
    // if wrong input has been given
    if (argc < 3) {
        fprintf(stderr, "Usage:\n for server side: %s -s PORT \n for client side: %s -c IP PORT\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
        //if the user is server side
    if (strcmp(argv[1], "-s") == 0) {
        int port = atoi(argv[2]);
        if (strcmp(argv[3], "-p") == 0)
        {
            if (strcmp(argv[4], "-q") == 0)
            {
                server_mode(port, 1, 1);
            }
            server_mode(port, 1, 0);
        }
        server_mode(port, 0, 0);
    } 
        //if the user is client side
    else if (strcmp(argv[1], "-c") == 0) {
        if (argc < 4 || argc > 7) {
            fprintf(stderr, "Usage: %s -c IP PORT\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        const char *ip = argv[2];
        int port = atoi(argv[3]);
        if (strcmp(argv[4], "-p") == 0)
        {
            printf("\nd\n");
            client_mode(ip, port, argv[5], argv[6]);
        }
        
        client_mode(ip, port, NULL, NULL);
    }
     else {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}

void server_mode(int port, int progress, int quiet)
{
    char buffer[BUFFER_SIZE];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) 
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, MAX_CONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
    if (connfd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Connected to client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
    if(progress)
    {
        if(quiet)
        {
            printf("im here\n");
            ssize_t n = recv(sockfd, buffer, BUFFER_SIZE, 0);
            printf("\nsize recieved: %ld\n", n);
        }
    }
    else
    {
        handle_connection(connfd);
    }
        close(connfd);
        close(sockfd);
    
}

void client_mode(const char *ip, int port, char* type, char* param) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
    {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server: %s:%d\n", ip, port);
    if (type)
    {
        if (strcmp(type, "ipv4") == 0)
        {
            if (strcmp(param, "tcp") == 0)
            {
                sleep(1);
                int a = send(sockfd, "ipv4 tcp", 8, 0);
                printf("\nsend size is: %d\n", a);
                printf("\nsend ipv4 tcp");
            }
            else if (strcmp(param, "udp") == 0)
            {
                send(sockfd, "ipv4 udp", 8, 0);
                printf("\nsend ipv4 udp");
            }
            
        }
        else if (type == "ipv6")
        {
            if (strcmp(param, "tcp") == 0)
            {
                send(sockfd, "ipv6 tcp", 8, 0);
                printf("\nsend ipv6 tcp");
            }
            else if (strcmp(param, "udp") == 0)
            {
                send(sockfd, "ipv6 udp", 8, 0);
                printf("\nsend ipv6 udp");
            }
        }
        else if (type == "pipe")
        {
            if (strcmp(param, "filename") == 0)
            {
                send(sockfd, "pipe filename", 8, 0);
                printf("\nsend pipe filename");
            }
        }
        else if (type == "mmap")
        {
            if (strcmp(param, "filename") == 0)
            {
                send(sockfd, "mmap pipename", 8, 0);
                printf("\nsend mmap pipename");
            }
        }
        else if (type == "uds")
        {
            if (strcmp(param, "dgram") == 0)
            {
                send(sockfd, "uds dgram", 8, 0);
                printf("\nsend uds dgram");
            }
            else if (strcmp(param, "stream") == 0)
            {
                send(sockfd, "uds stream", 8, 0);
                printf("\nsend uds stream");
            }
        }        
        
    }
    else
    {
        handle_connection(sockfd);
    }
        close(sockfd);
    
}

void handle_connection(int sockfd) {
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    bool running = true;

    while (running) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (ready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t n = recv(sockfd, buffer, BUFFER_SIZE, 0);
            if (n <= 0) {
                printf("Connection closed by peer\n");
                running = false;
            } else {
                buffer[n] = '\0';
                printf("Received: %s", buffer);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            ssize_t n = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (n < 0) {
                perror("read");
                exit(EXIT_FAILURE);
            } else if (n == 0) {
                running = false;
            } else {
                if (send(sockfd, buffer, n, 0) < 0) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}
