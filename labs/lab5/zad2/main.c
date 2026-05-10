#include "definitions.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("usage: ./main N M K\n");
        fflush(stdout);
        exit(1);
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    int K = atoi(argv[3]);

    if (K > MAX_TASK_COUNT) {
        printf("K must be <= MAX_TASK_COUNT(%d)\n", MAX_TASK_COUNT);
        fflush(stdout);
        exit(1);
    }

    printf("running setup\n\n");
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        execl("./setup", "setup", (char *)NULL);
    }

    while (wait(0) > 0) { };

    int fds[2] = {open_memory(memory_filenames[PRIORITY]), open_memory(memory_filenames[NORMAL])};
    memory *mems[2] = {map_memory(fds[PRIORITY]), map_memory(fds[NORMAL])};
    close(fds[PRIORITY]);
    close(fds[NORMAL]);

    memory *mem;
    for(int i = 0; i < 2; ++i) {
        mem = mems[i];
        mem->max_task_count = K;
        munmap(mem, sizeof(memory));
    }

    printf("\nspawning %d producers\n", N);
    fflush(stdout);
    for (int i = 0; i < N; ++i) {
        printf("spawning producer #%d\n", i+1);
        fflush(stdout);
        pid = fork();
        if (pid == 0) {
            execl("./producer", "producer", (char *)NULL);
        }
    }
    printf("\n");
    fflush(stdout);

    printf("spawning %d consumers\n", M);
    for (int i = 0; i < M; ++i) {
        printf("spawning consumer #%d\n", i+1);
        fflush(stdout);
        pid = fork();
        if (pid == 0) {
            execl("./consumer", "consumer", (char *)NULL);
        }
    }
    printf("\n");
    fflush(stdout);

    while (wait(0) > 0) { };
}
