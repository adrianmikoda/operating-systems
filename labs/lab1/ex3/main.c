#include "definitions.h"

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        return 1;
    }

    remove(OUTPUT_FILE);

    char* N = argv[1];
    char* M = argv[2];
    int n = atoi(N);

    for (int i = 0; i < n; ++i) {
        int pid = fork();
        
        if (pid == 0) {
            execl("./child", "child", M, (char*) NULL);
        }
    }

    while (wait(0) > 0) {};
    
    printf("Rodzic  (%d)\n", getpid());

    return 0;
}
