#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 9000

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IPv4 adress> <port> <number>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int number = atoi(argv[3]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number: %s. Using default %d.\n", argv[2], DEFAULT_PORT);
        port = DEFAULT_PORT;
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in ser;
    
    memset(&ser, 0, sizeof(ser));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(port);
    ser.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(socket_fd, (struct sockaddr *)&ser, sizeof(ser)) < 0) {
        perror("connect");
        close(socket_fd);
        return 1;
    }

    char request[32];
    int req_len = snprintf(request, sizeof(request), "TASK %d", number);

    if (write(socket_fd, request, req_len) < 0) {
        perror("write");
        close(socket_fd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    bytes_read = read(socket_fd, buffer, sizeof(buffer));
    fwrite(buffer, 1, bytes_read, stdout);
    fwrite("\n", 1, 1, stdout);
    fflush(stdout);

    if (bytes_read < 0) {
        perror("read");
        close(socket_fd);
        return 1;
    }

    close(socket_fd);
    return 0;
}
