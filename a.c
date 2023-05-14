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
        printf("wy1");
        while ((n = recvfrom(serv_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_size)) > 0) {
            fwrite(buffer, sizeof(char), n, file);
        }
        printf("wy2");
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