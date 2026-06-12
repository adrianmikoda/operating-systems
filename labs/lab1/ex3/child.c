#include "definitions.h"

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        return 1;
    }

    FILE* file = fopen(OUTPUT_FILE, "a");
    if (!file) {
        perror("open");
        return 1;
    }

    int fd = fileno(file);

    if (flock(fd, LOCK_EX) == -1) {
        perror("lock");
        return 1;
    }

    char buffer[128];
    char* M = argv[1];
    int m = atoi(M);
    for (int j = 0; j < m; j++) {
        int length = sprintf(buffer, "Potomek (%d)\n", getpid());
        fwrite(buffer, sizeof(char), length, file);
        fflush(file);
        sleep(0.25);
    }

    if (flock(fd, LOCK_UN) == -1) {
        perror("unlock");
        return 1;
    }

    fclose(file);

    return 0;
}
