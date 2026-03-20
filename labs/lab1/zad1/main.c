#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define M 5

int zmiennaGlobalna;

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        return 1;
    }
    char* N = argv[1];
    int n = atoi(N);
    pid_t pid;

    for (int i = 0; i < n; ++i) {
        // pid = fork();
        pid = vfork();
        if (pid == 0) {
            zmiennaGlobalna++;
            for (int j = 0; j < M; j++) {
                printf("Potomek (%d)\n", getpid());
                sleep(0.25);
            }
            // return 0;
            exit(0);
        }
    }

    while (wait(0) > 0) { };
    printf("Rodzic  (%d) zmiennaGlobalna=%d\n", getpid(), zmiennaGlobalna);

    return 0;
}
