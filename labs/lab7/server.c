#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number: %s. Using default %d.\n", argv[1], DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return 1;
    }

    struct sockaddr_in ser;
    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_addr.s_addr = INADDR_ANY;
    ser.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&ser, sizeof(ser)) < 0) {
        perror("bind");
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, 10) < 0) {
        perror("listen");
        close(socket_fd);
        return 1;
    }

    printf("Server running on port %d\n", port);

    int COUNTER = 0;
    char buffer[BUFFER_SIZE];

    while (1) {
        struct sockaddr_in client_address;
        socklen_t addr_len = sizeof(client_address);
        int client_fd = accept(socket_fd, (struct sockaddr *)&client_address, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';

            if (strncmp(buffer, "GET", 3) == 0 || strncmp(buffer, "POST", 4) == 0) {
                COUNTER++;
                
                char response_body[256];
                int body_len = snprintf(response_body, sizeof(response_body), "Site downloads: %d", COUNTER);

                char response_header[512];
                int header_len = snprintf(response_header, sizeof(response_header),
                    "HTTP/1.1 200 OK\r\n"
                    "Server: Zajeciowy serwer SO\r\n"
                    "Content-Type: text/plain; charset=utf-8\r\n"
                    "Connection: close\r\n"
                    "Cache-Control: no-store\r\n"
                    "Content-Length: %d\r\n\r\n",
                    body_len);

                write(client_fd, response_header, header_len);
                write(client_fd, response_body, body_len);

            }
            else if (strncmp(buffer, "TASK", 4) == 0) {
                int wartość = 0;
                if (sscanf(buffer, "TASK %d", &wartość) == 1) {
                    COUNTER += wartość;
                }

                char response_body[256];
                int body_len = snprintf(response_body, sizeof(response_body), "Site downloads: %d", COUNTER);

                write(client_fd, response_body, body_len);
            }
        }

        close(client_fd);
    }

    close(socket_fd);
    return 0;
}
