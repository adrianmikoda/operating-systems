#include "definitions.h"

void sig_ignore() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
        perror("sig_ignore signal failed");
    }
}
