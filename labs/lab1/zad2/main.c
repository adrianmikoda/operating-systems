#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        return 1;
    }

    char* N = argv[1];
    int n = atoi(N);
    char* M = argv[2];

    for (int i = 0; i < n; ++i) {
        int pid = fork();
        
        if (pid == 0) {
            execl("./child", "child", M, (char *)NULL);
        }
    }

    while (wait(0) > 0) {};
    
    printf("Rodzic  (%d)\n", getpid());

    return 0;
}
