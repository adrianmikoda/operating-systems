#include "definitions.h"

void sig_mask() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
        perror("sig_mask sigprocmask failed");
    }
}
