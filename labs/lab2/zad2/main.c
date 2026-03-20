#include "definitions.h"

void handle_invalid_argument() {
    printf("Wywołanie: ./main default|mask|ignore|handle\n");
    exit(0);
}

void sigusr2_block() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    sigset_t usr2_set;
    sigemptyset(&usr2_set);
    sigaddset(&usr2_set, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &usr2_set, NULL) < 0) {
        perror("sigusr2_block sigprocmask failed");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        handle_invalid_argument();
    }

    setbuf(stdout, NULL);
    char* argument = argv[1];
    union sigval toSend;
    if (strcmp(argument, "default") == 0) {
            toSend.sival_int = 0;
    } else if (strcmp(argument, "mask") == 0) {
            toSend.sival_int = 1;
    } else if (strcmp(argument, "ignore") == 0) {
            toSend.sival_int = 2;
    } else if (strcmp(argument, "handle") == 0) {
            toSend.sival_int = 3;
    } else {
        handle_invalid_argument();
    }

    sigusr2_block();

    pid_t pid = fork();
    if (pid == 0) {
        if (execl("./child", "child", (char*) NULL) == -1) {
            perror("ececl");
        }
    }
    printf("(%d) Stworzono proces child [%d]\n", getpid(), pid);
    printf("(%d) Wysyłano sygnał SIGUSR2 do procesu %d\n", getpid(), pid);
    if (sigqueue(pid, SIGUSR2, toSend) == -1) {
        perror("sigqueue");
    }
    while(wait(NULL) > 0) {};

    return 0;
}
