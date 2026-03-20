#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

void handle_invalid_argument() {
    printf("Wywołanie: ./main default|mask|ignore|handle\n");
    exit(0);
}

void sig_default() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    if (signal(SIGUSR1, SIG_DFL) == SIG_ERR) {
        perror("sig_default signal failed");
    }
}

void sig_mask() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    sigset_t usr1_set;
    sigemptyset(&usr1_set);
    sigaddset(&usr1_set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &usr1_set, NULL) < 0) {
        perror("sig_mask sigprocmask failed");
    }
}

void sig_ignore() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
        perror("sig_ignore signal failed");
    }
}

void handler(int signo) {
    printf("Wywołano handler dla sygnału %d\n", signo);
}

void sig_handle() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    if (signal(SIGUSR1, handler) == SIG_ERR) {
        perror("sig_handle signal failed");
    }
}

void sig_unblock() {
    printf("Wywołano funkcję: '%s()'\n", __func__);
    sigset_t usr1_set;
    sigemptyset(&usr1_set);
    sigaddset(&usr1_set, SIGUSR1);
    if (sigprocmask(SIG_UNBLOCK, &usr1_set, NULL) < 0) {
        perror("sig_unblock sigprocmask failed");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        handle_invalid_argument();
    }

    setbuf(stdout, NULL);

    char* argument = argv[1];

    if (strcmp(argument, "default") == 0) {
        sig_default();
    } else if (strcmp(argument, "mask") == 0) {
        sig_mask();
    } else if (strcmp(argument, "ignore") == 0) {
        sig_ignore();
    } else if (strcmp(argument, "handle") == 0) {
        sig_handle();
    } else {
        handle_invalid_argument();
    }

    sigset_t pending;

    for (int i = 1; i <= 20; ++i) {
        printf("%d\n", i);
        fflush(stdout);
        if (i == 5 || i == 15) {
            printf("Wysyłam sygnał USR1\n");
            raise(SIGUSR1);
        }
        if (i == 10) {
            if (sigpending(&pending) == -1) {
                perror("sigpending");
                return 1;
            }
            if (sigismember(&pending, SIGUSR1)) {
                printf("Odblokowuję USR1\n");
                sig_unblock();
            }
        }
        sleep(1);
    }
    printf("Pętla została wykonana w całości\n");

    return 0;
}
