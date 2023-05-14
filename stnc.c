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
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>


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

void server_uds_stream(const char* path, const char* filename);

void server_uds_dgram(const char* path, const char* filename);

void client_uds_dgram(const char* path, const char* filename);

void client_uds_stream(const char* path, const char* filename);

void server_pipe_filename(const char* filename);

void client_pipe_filename(const char* filename);

void client_mmap_filename(const char* filename);

void server_mmap_filename(const char* filename);

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

    unsigned short calculateChecksumFile(const char *filename) {
        FILE *file = fopen(filename, "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        unsigned int sum = 0;
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            unsigned short *ptr = (unsigned short *)buffer;
            size_t length = bytesRead;

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
        }

        fclose(file);

        // Take the one's complement of the sum
        unsigned short checksum = ~sum;
        return checksum;
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

                struct timeval start, end; // Variables for time measurement
                gettimeofday(&start, NULL); // Start time measurement

                int checkSumRecieved = 0;
                if (strcmp("ipv6 tcp", buffer) == 0) {
                    server_ipv6_tcp(port + 1, "100MB");
                    checkSumRecieved = calculateChecksumFile("newIpv6Tcp.txt");
                    checkSumRecieved = htons(checkSumRecieved);

                } else if (strcmp("ipv6 udp", buffer) == 0) {
                    server_ipv6_udp(port + 1, "100MB");
                    checkSumRecieved = calculateChecksumFile("newIpv6Udp.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("ipv4 tcp", buffer) == 0) {
                    server_ipv4_tcp(port + 1, "100MB");
                    checkSumRecieved = calculateChecksumFile("newIpv4Tcp.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("ipv4 udp", buffer) == 0) {
                    server_ipv4_udp(port + 1, "100MB");
                    checkSumRecieved = calculateChecksumFile("newIpv4Udp.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("uds dgram", buffer) == 0) {
                    server_uds_dgram("socket.sock", "100MB");
                    checkSumRecieved = calculateChecksumFile("newUdsDgram.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("uds stream", buffer) == 0) {
                    server_uds_stream("socket.sock", "100MB");
                    checkSumRecieved = calculateChecksumFile("newUdsStream.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("pipe filename", buffer) == 0) {
                    server_pipe_filename("100MB");
                    checkSumRecieved = calculateChecksumFile("newPipeFilename.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                } else if (strcmp("mmap filename", buffer) == 0) {
                    server_mmap_filename("100MB");
                    checkSumRecieved = calculateChecksumFile("newMmapFilename.txt");
                    checkSumRecieved = htons(checkSumRecieved);
                }

                // Compare the received checksum with the calculated checksum of the file
                int  checksumOfSent;
                ssize_t checksumBytesReceived = recv(connfd, &checksumOfSent, sizeof(checksumOfSent), 0);
                if (checksumBytesReceived > 0) {
                    checksumOfSent = ntohs(checksumOfSent);

                    if (checkSumRecieved == checksumOfSent) {
                        printf("Checksums match. File transfer successful.\n");
                    } else {
                        printf("Checksums do not match. File transfer failed.\n");
                    }
                }

                else {
                    printf("Failed to receive checksum from the client.\n");
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
                    printf("\nSending: ipv4 tcp\n");
                    client_ipv4_tcp("127.0.0.1", port + 1, "100MB");
                } else if (strcmp(param, "udp") == 0) {
                    send(sockfd, "ipv4 udp", 9, 0);
                    printf("\nSending: ipv4 udp\n");
                    client_ipv4_udp("127.0.0.1", port + 1, "100MB");
                }
            } else if (strcmp(type, "ipv6") == 0) {
                if (strcmp(param, "tcp") == 0) {
                    sleep(1);
                    send(sockfd, "ipv6 tcp", 9, 0);
                    printf("\nSending: ipv6 tcp\n");
                    client_ipv6_tcp("::1", port + 1, "100MB");
                } else if (strcmp(param, "udp") == 0) {
                    sleep(1);
                    send(sockfd, "ipv6 udp", 9, 0);
                    printf("\nSending: ipv6 udp\n");
                    client_ipv6_udp("::1", port + 1, "100MB");
                }
            } else if (strcmp(type, "uds") == 0) {
                if (strcmp(param, "dgram") == 0) {
                    send(sockfd, "uds dgram", 10, 0);
                    client_uds_dgram("socket.sock", "100MB.txt");
                } else if (strcmp(param, "stream") == 0) {
                    send(sockfd, "uds stream", 11, 0);
                    client_uds_stream("socket.sock", "100MB.txt");
                }
            } else if (strcmp(type, "pipe") == 0) {
                if (strcmp(param, "filename") == 0) {
                    client_pipe_filename("100MB.txt");
                }
            } else if (strcmp(type, "mmap") == 0) {
                if (strcmp(param, "filename") == 0) {
                    client_mmap_filename("100MB.txt");
                }
            }
        } else {
            handle_connection(sockfd);
        }

        // Calculate and send the checksum of the file
        unsigned short checksum = calculateChecksumFile("100MB.txt");
        checksum = htons(checksum);
        
        // Send the checksum to the server
        ssize_t checksumBytesSent = send(sockfd, &checksum, sizeof(checksum), 0);
        if (checksumBytesSent < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        } else if (checksumBytesSent == 0) {
            printf("Failed to send checksum to the server.\n");
        } else {
            printf("Checksum sent to the server.\n");
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

    // Set the timeout
    struct timeval timeout;
    timeout.tv_sec = 1;  // After 1 sec
    timeout.tv_usec = 0; // and 0 microseconds

    if (setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen("newIpv6Udp.txt", "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);

    char buffer[BUFFER_SIZE];
    ssize_t n;
    struct sockaddr_in6 client_addr;
    socklen_t addr_size = sizeof(client_addr);
    fflush(stdout);
    while ((n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
    }

    // Check for timeout error
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Timeout occurred.\n");
        } else {
            perror("recvfrom");
        }
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
        printf("the port is: %d", port);
        fflush(stdout);
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

        if (listen(serv_sock, 1) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, (socklen_t*) &addr_size);
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
        sleep(1);
        int client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        printf("the port is: %d\n", port);
        fflush(stdout);
        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        int pton_result = inet_pton(AF_INET, ip, &serv_addr.sin_addr);
        if (pton_result <= 0) {
            if(pton_result == 0) {
                fprintf(stderr, "inet_pton: Invalid IP address\n");
            } else {
                perror("inet_pton");
            }
            exit(EXIT_FAILURE);
        }

        if (connect(client_sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        FILE *file = fopen("100MB.txt", "rb");  // you passed filename parameter, but didn't use it
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            ssize_t send_result = send(client_sock, buffer, n, 0);
            if(send_result == -1) {
                perror("send");
                exit(EXIT_FAILURE);
            }
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

        // Set the timeout
        struct timeval timeout;
        timeout.tv_sec = 1;  // After 1 sec
        timeout.tv_usec = 0; // and 0 microseconds

        if (setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("setsockopt");
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
        printf("ok1");
        fflush(stdout);
        while ((n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
            fwrite(buffer, sizeof(char), n, file);
        }

        // Check for timeout error
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("Timeout occurred.\n");
            } else {
                perror("recvfrom");
            }
        }

        printf("ok2");
        fflush(stdout);
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

    void server_uds_stream(const char* path, const char* filename) {
        int serv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serv_sock < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, path, sizeof(serv_addr.sun_path) - 1);

        unlink(path);  // Remove any existing socket file

        if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }

        if (listen(serv_sock, 10) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        printf("Server listening on Unix Domain Socket: %s\n", path);

        int client_sock = accept(serv_sock, NULL, NULL);
        if (client_sock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        FILE* file = fopen(filename, "wb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, sizeof(char), n, file);
        }

        fclose(file);
        close(client_sock);
        close(serv_sock);
        unlink(path);  // Remove the socket file after use
    }

    void client_uds_stream(const char* path, const char* filename) {
        int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, path, sizeof(serv_addr.sun_path) - 1);

        if (connect(sockfd, (const struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect");
            exit(EXIT_FAILURE);
        }

        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            send(sockfd, buffer, n, 0);
        }

        fclose(file);
        close(sockfd);
    }
    
    void server_uds_dgram(const char* path, const char* filename) {
    int serv_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (serv_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, path, sizeof(serv_addr.sun_path) - 1);

    unlink(path);  // Remove any existing socket file
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on Unix Domain Socket: %s\n", path);

    FILE* file = fopen("newUdsDgram.txt", "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t n;
    fd_set rfds;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(serv_sock, &rfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int retval = select(serv_sock + 1, &rfds, NULL, NULL, &timeout);
        if (retval == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        } else if (retval == 0) {
            // Timeout occurred, handle the timeout
            printf("Timeout occurred.\n");
            break; // Exit the loop or perform any other required action
        } else {
            // Data is available to receive, perform the receive operation
            n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, NULL, NULL);
            if (n > 0) {
                fwrite(buffer, sizeof(char), n, file);
            }
        }
    }

    fclose(file);
    close(serv_sock);
    unlink(path);  // Remove the socket file after use
}


    void client_uds_dgram(const char* path, const char* filename) {
        int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        printf("here!1");
        fflush(stdout);
        struct sockaddr_un serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, path, sizeof(serv_addr.sun_path) - 1);
        printf("here!2");
        fflush(stdout);
        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        printf("here!3");
        fflush(stdout);
        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            sendto(sockfd, buffer, n, 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));
        }
        printf("here!4");
        fflush(stdout);
        fclose(file);
        close(sockfd);
    }

    void server_pipe_filename(const char* filename) {
        FILE* file = fopen(filename, "wb");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process
            close(pipefd[1]);  // Close the write end of the pipe

            ssize_t n;
            char buffer[BUFFER_SIZE];
            while ((n = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
                fwrite(buffer, 1, n, file);
            }

            fclose(file);
            close(pipefd[0]);  // Close the read end of the pipe
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(pipefd[0]);  // Close the read end of the pipe

            FILE* source = fopen(filename, "rb");
            if (source == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }

            ssize_t n;
            char buffer[BUFFER_SIZE];
            while ((n = fread(buffer, 1, BUFFER_SIZE, source)) > 0) {
                write(pipefd[1], buffer, n);
            }

            fclose(source);
            close(pipefd[1]);  // Close the write end of the pipe
            wait(NULL);  // Wait for the child process to exit
        }
    }

    void client_pipe_filename(const char* filename) {
        int pipefd[2];
        if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close the read end of the pipe

        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        write(pipefd[1], buffer, n);
        }

        fclose(file);
        close(pipefd[1]);  // Close the write end of the pipe
        exit(EXIT_SUCCESS);
    } 
    else {
        // Parent process
        close(pipefd[1]);  // Close the write end of the pipe

        FILE* file = fopen(filename, "wb");
        if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
        }

        ssize_t n;
        char buffer[BUFFER_SIZE];
        while ((n = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, n, file);
        }

        fclose(file);
        close(pipefd[0]);  // Close the read end of the pipe
        wait(NULL);  // Wait for the child process to exit
    }

    }

    void server_mmap_filename(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    size_t size = BUFFER_SIZE * sizeof(char);
    char* buffer = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_FILE, -1, 0);
    if (buffer == (char*)MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        FILE* source = fopen(filename, "rb");
        if (source == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
        }

        ssize_t n;
        while ((n = fread(buffer, sizeof(char), size, source)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
        }

        fclose(source);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        ssize_t n;
        while ((n = fread(buffer, sizeof(char), size, stdin)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
        }

        fclose(file);
        wait(NULL);  // Wait for the child process to exit
    }

    munmap(buffer, size);
    }

    void client_mmap_filename(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    size_t size = BUFFER_SIZE * sizeof(char);
    char* buffer = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_FILE, -1, 0);
    if (buffer == (char*)MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
        ssize_t n;
    while ((n = fread(buffer, sizeof(char), size, file)) > 0) {
        fwrite(buffer, sizeof(char), n, stdout);
    }

    fclose(file);
    munmap(buffer, size);
    }
}