#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(){
    long double a, b;
    int scanned = scanf("%Lf %Lf", &a, &b);
    if (scanned != 2) {
        return 1;
    }

    const char *filename = "stream";
    if (access(filename, F_OK) == -1) {
        if (mkfifo(filename, 0666) == -1) {
            perror("mkfifo");
            return 1;
        }
    }

    if (fork() == 0) {
        execl("./main", "main", "1000000000", "6", NULL);
    }

    int fd = open(filename, O_WRONLY);
    if (fd == -1) { 
        perror("open FIFO"); 
        return 1; 
    }
    if (write(fd, &a, sizeof(long double)) < sizeof(long double) ||
        write(fd, &b, sizeof(long double)) < sizeof(long double)) {
        perror("write");
        return 1;
    }
    close(fd);

    while (wait(NULL) > 0) {};
}
