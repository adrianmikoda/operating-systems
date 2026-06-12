#include "definitions.h"

int mode;

void handler(int sig_id) {
    printf("[%d] Wywołano handler dla sygnału %d\n", getpid(), sig_id);
}

void sig_unblock(int signalno) {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, signalno);
    if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
        perror("sig_unblock sigprocmask failed");
    }
}

void sigusr2_handler(int signom, siginfo_t* info, void* extra) {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    mode = info->si_value.sival_int;
}

#ifdef DYNAMIC_LOAD

#include <dlfcn.h>

typedef void (*sig_fun_t)();
typedef void (*sig_handle_fun_t)(void (*)(int));

void mode_handler() {
    void *handle = dlopen("libsig.so", RTLD_LAZY);
    if (!handle) {
        perror("dlopen");
        exit(1);
    }
    printf("[%d] Using dynamic library loading\n", getpid());
    sig_fun_t function;
    sig_handle_fun_t handle_function;
    switch (mode) {
        case 0:
            function = (sig_fun_t)dlsym(handle, "sig_default");
            function();
            break;
        case 1:
            function = (sig_fun_t)dlsym(handle, "sig_mask");
            function();
            break;
        case 2:
            function = (sig_fun_t)dlsym(handle, "sig_ignore");
            function();
            break;
        case 3:
            handle_function = (sig_handle_fun_t)dlsym(handle, "sig_handle");
            handle_function(handler);
            break;
    }
}

#else

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

#endif

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);

    struct sigaction action;
    action.sa_flags = SA_SIGINFO;     
    action.sa_sigaction = &sigusr2_handler;
    sigaction(SIGUSR2, &action, NULL);

    sig_unblock(SIGUSR2);

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
                sig_unblock(SIGUSR1);
            }
        }
        sleep(1);
    }
    printf("[%d] Pętla została wykonana w całości\n", getpid());
}
