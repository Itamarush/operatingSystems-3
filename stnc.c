#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define MAX_CONN 1

void server_mode(int port, int progress, int quiet);

void client_mode(const char *ip, int port, char* type, char* param);

void client_ipv6_tcp(const char *ip, int port, const char *filename);

void client_ipv6_udp(const char *ip, int port, const char *filename);

void server_ipv6_udp(int port, const char *filename);

void server_ipv6_tcp(int port, const char *filename);

void server_ipv4_tcp(int port, const char* filename);

void client_ipv4_tcp(const char* ip, int port, const char* filename);

void server_ipv4_udp(int port, const char* filename);

void client_ipv4_udp(const char* ip, int port, const char* filename);

void handle_connection(int sockfd);

unsigned short calculateChecksum(const char *data, int length);

int main(int argc, char *argv[]) {
    // if wrong input has been given
    if (argc < 3) {
        fprintf(stderr, "Usage:\n for server side: %s -s PORT \n for client side: %s -c IP PORT\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    //if the user is server side
    if (strcmp(argv[1], "-s") == 0) {
        int port = atoi(argv[2]);
        int progress = 0;
        int quiet = 0;
        if (argc > 3 && strcmp(argv[3], "-p") == 0) {
            progress = 1;
        }
        if (argc > 4 && strcmp(argv[4], "-q") == 0) {
            quiet = 1;
        }
        server_mode(port, progress, quiet);
    }
    //if the user is client side
    else if (strcmp(argv[1], "-c") == 0) {
        const char *ip = argv[2];
        int port = atoi(argv[3]);
        if (argc > 4 && strcmp(argv[4], "-p") == 0) {
            client_mode(ip, port, argv[5], argv[6]);
        } else {
            client_mode(ip, port, NULL, NULL);
        }
    } else {
        fprintf(stderr, "Invalid option: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    return 0;
}

void server_mode(int port, int progress, int quiet) {
    char buffer[BUFFER_SIZE];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
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
    int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("Connected to client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    if (progress) {
        if (quiet) {
            ssize_t n = recv(connfd, buffer, BUFFER_SIZE, 0);
            if (n > 0) {
                buffer[n] = '\0';
                printf("\nReceived: %s\n", buffer);

                struct timeval start, end; // Variables for time measurement
                gettimeofday(&start, NULL); // Start time measurement

                // Calculate and print the checksum
                unsigned short checksum = calculateChecksum(buffer, n);
                printf("Checksum: %u\n", checksum);

                if (strcmp("ipv6 tcp", buffer) == 0) {
                    server_ipv6_tcp(port + 1, "100MB");
                } else if (strcmp("ipv6 udp", buffer) == 0) {
                    server_ipv6_udp(port + 1, "100MB");
                } else if (strcmp("ipv4 tcp", buffer) == 0) {
                    server_ipv4_tcp(port + 1, "100MB");
                } else if (strcmp("ipv4 udp", buffer) == 0) {
                    server_ipv4_udp(port + 1, "100MB");
                }

                gettimeofday(&end, NULL); // End time measurement
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
                printf("Time taken: %.2f seconds.\n", elapsed);
            }
        }
    } else {
        handle_connection(connfd);
    }

    close(connfd);
    close(sockfd);
}

void client_mode(const char *ip, int port, char *type, char *param) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server: %s:%d\n", ip, port);
    printf("Sending file 100MB.txt of size 104857600 bytes\n");

    if (type) {
        if (strcmp(type, "ipv4") == 0) {
            if (strcmp(param, "tcp") == 0) {
                sleep(1);
                int a = send(sockfd, "ipv4 tcp", 9, 0);
                printf("\nSending: ipv4 tcp\n");
                client_ipv4_tcp("127.0.0.1", port + 1, "100MB");
            } else if (strcmp(param, "udp") == 0) {
                send(sockfd, "ipv4 udp", 8, 0);
                printf("\nSending: ipv4 udp\n");
                client_ipv4_udp("127.0.0.1", port + 1, "100MB");
            }

        } else if (strcmp(type, "ipv6") == 0) {
            if (strcmp(param, "tcp") == 0) {
                sleep(1);
                int a = send(sockfd, "ipv6 tcp", 9, 0);
                send(sockfd, "ipv6 tcp", 8, 0);
                printf("\nSending: ipv6 tcp\n");
                client_ipv6_tcp("::1", port + 1, "100MB");

            } else if (strcmp(param, "udp") == 0)
                sleep(1);
                int a = send(sockfd, "ipv6 udp", 9, 0);
                send(sockfd, "ipv6 udp", 8, 0);
                printf("\nSending: ipv6 udp\n");
                client_ipv6_udp("::1", port + 1, "100MB");

            }
        } else if (strcmp(type, "pipe") == 0) {
            if (strcmp(param, "filename") == 0) {
                sleep(1);
                int a = send(sockfd, "pipe filename", 9, 0);
                send(sockfd, "pipe filename", 8, 0);
                printf("\nSending: pipe filename\n");
            }
        } else if (strcmp(type, "mmap") == 0) {
            if (strcmp(param, "filename") == 0) {
                send(sockfd, "mmap filename", 8, 0);
                printf("\nSending: mmap filename\n");
            }
        } else if (strcmp(type, "uds") == 0) {
            if (strcmp(param, "dgram") == 0) {
                send(sockfd, "uds dgram", 8, 0);
                printf("\nSending: uds dgram\n");
            } else if (strcmp(param, "stream") == 0) {
                send(sockfd, "uds stream", 8, 0);
                printf("\nSending: uds stream\n");
            }
        }
    else {
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

//################### ipV6 tcp ######################//

void server_ipv6_tcp(int port, const char *filename) {
    int serv_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(port);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (client_sock < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen("newIpv6Tcp.txt", "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t n;
    while ((n = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, file);
    }

    fclose(file);
    close(client_sock);
    close(serv_sock);
    printf("File received and saved as newIpv6Tcp.txt\n");
}

    void client_ipv6_tcp(const char *ip, int port, const char *filename) {
        int client_sock;
        struct sockaddr_in6 serv_addr;
        int connection_result;

        do {
            sleep(1);  // Delay before retrying

            client_sock = socket(AF_INET6, SOCK_STREAM, 0);
            if (client_sock < 0) {
                perror("socket");
                exit(EXIT_FAILURE);
            }

            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin6_family = AF_INET6;
            if (inet_pton(AF_INET6, ip, &serv_addr.sin6_addr) <= 0) {
                perror("inet_pton");
                exit(EXIT_FAILURE);
            }
            serv_addr.sin6_port = htons(port);

            connection_result = connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        } while (connection_result < 0);

        FILE *file = fopen("100MB.txt", "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            send(client_sock, buffer, n, 0);
        }

        fclose(file);
        close(client_sock);
        printf("File sent successfully.\n");
    }

    //################### ipV6 udp ######################//

    void server_ipv6_udp(int port, const char *filename) {
        int serv_sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (serv_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in6 serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin6_family = AF_INET6;
        serv_addr.sin6_addr = in6addr_any;
        serv_addr.sin6_port = htons(port);

        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("newIpv6Udp.txt", "wb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        struct sockaddr_in6 client_addr;
        socklen_t addr_size = sizeof(client_addr);

        while ((n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
            fwrite(buffer            , 1, n, file);
        }

        fclose(file);
        close(serv_sock);
        printf("File received and saved as newIpv6Udp.txt\n");
    }

    void client_ipv6_udp(const char *ip, int port, const char *filename) {
        int client_sock = socket(AF_INET6, SOCK_DGRAM, 0);
        if (client_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in6 serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ip, &serv_addr.sin6_addr) <= 0) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        serv_addr.sin6_port = htons(port);

        FILE *file = fopen("100MB.txt", "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;

        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            sendto(client_sock, buffer, n, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        }

        fclose(file);
        close(client_sock);
        printf("File sent successfully.\n");
    }

    // ################### IPv4 TCP ######################

    void server_ipv4_tcp(int port, const char *filename) {
        int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (serv_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        if (listen(serv_sock, 10) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("newIpv4Tcp.txt", "wb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, n, file);
        }

        fclose(file);
        close(client_sock);
        close(serv_sock);
        printf("File received and saved as newIpv4Tcp.txt\n");
    }

    void client_ipv4_tcp(const char *ip, int port, const char *filename) {
        int client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
            if (connect(client_sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("100MB.txt", "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            send(client_sock, buffer, n, 0);
        }

        fclose(file);
        close(client_sock);
        printf("File sent successfully.\n");
    }

    // ################### IPv4 UDP ######################

    void server_ipv4_udp(int port, const char *filename) {
        int serv_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (serv_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("newIpv4Udp.txt", "wb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);

        while ((n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
            fwrite(buffer, 1, n, file);
        }

        fclose(file);
        close(serv_sock);
        printf("File received and saved as newIpv4Udp.txt\n");
    }

    void client_ipv4_udp(const char *ip, int port, const char *filename) {
        int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        serv_addr.sin_port = htons(port);

        FILE *file = fopen("100MB.txt", "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;

        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            sendto(client_sock, buffer, n, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        }

        fclose(file);
        close(client_sock);
        printf("File sent successfully.\n");
    }

    // Function to calculate checksum
    unsigned short calculateChecksum(const char *data, int length) {
    unsigned int sum = 0;
    unsigned short *ptr = (unsigned short *)data;

    // Calculate the sum of 16-bit chunks
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }

    // If length is odd, add the last byte
    if (length > 0) {
        sum += *((unsigned char *)ptr);
    }

    // Add the carry bits to the sum
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Take the one's complement of the sum
    unsigned short checksum = ~sum;

    return checksum;
}

