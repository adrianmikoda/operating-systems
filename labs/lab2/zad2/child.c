#include "definitions.h"

int mode;

void sig_default() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, SIG_DFL) == SIG_ERR) {
        perror("sig_default signal failed");
    }
}

void sig_mask() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
        perror("sig_mask sigprocmask failed");
    }
}

void sig_ignore() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
        perror("sig_ignore signal failed");
    }
}

void handler(int sig_id) {
    printf("[%d] Wywołano handler dla sygnału %d\n", getpid(), sig_id);
}

void sig_handle(void (*function)(int)) {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, function) == SIG_ERR) {
        perror("sig_handle signal failed");
    }
}

void sig_unblock() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    sigset_t usr1_set;
    sigemptyset(&usr1_set);
    sigaddset(&usr1_set, SIGUSR1);
    if (sigprocmask(SIG_UNBLOCK, &usr1_set, NULL) < 0) {
        perror("sig_unblock sigprocmask failed");
        exit(1);
    }
}

void sigusr2_unblock() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    sigset_t usr2_set;
    sigemptyset(&usr2_set);
    sigaddset(&usr2_set, SIGUSR2);
    if (sigprocmask(SIG_UNBLOCK, &usr2_set, NULL) < 0) {
        perror("sig_unblock sigprocmask failed");
        exit(1);
    }
}

void sigusr2_handler(int signom, siginfo_t* info, void* extra) {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    mode = info->si_value.sival_int;
}

void mode_handler() {
    switch (mode) {
        case 0:
            sig_default();
            break;
        case 1:
            sig_mask();
            break;
        case 2:
            sig_ignore();
            break;
        case 3:
            sig_handle(handler);
            break;
    }
}

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;     
    action.sa_sigaction = &sigusr2_handler;
    sigaction(SIGUSR2, &action, NULL);

    sigusr2_unblock();

    sigset_t wait_mask;
    sigfillset(&wait_mask);
    sigdelset(&wait_mask, SIGUSR2);
    sigpending(&wait_mask);

    mode_handler();

    sigset_t pending;

    for (int i = 1; i <= 20; ++i) {
        printf("[%d] %d\n", getpid(), i);
        fflush(stdout);
        if (i == 5 || i == 15) {
            printf("[%d] Wysyłam sygnał USR1\n", getpid());
            raise(SIGUSR1);
        }
        if (i == 10) {
            if (sigpending(&pending) == -1) {
                perror("sigpending");
                return 1;
            }
            if (sigismember(&pending, SIGUSR1)) {
                printf("[%d] Odblokowuję USR1\n", getpid());
                sig_unblock();
            }
        }
        sleep(1);
    }
    printf("[%d] Pętla została wykonana w całości\n", getpid());
}
