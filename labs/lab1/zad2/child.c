#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        return 1;
    }

    char* M = argv[1];
    int m = atoi(M);
    
    for (int j = 0; j < m; j++) {
        printf("Potomek (%d)\n", getpid());
        sleep(0.25);
    }

    return 0;
}
