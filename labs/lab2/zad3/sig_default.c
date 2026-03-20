#include "definitions.h"

void sig_default() {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, SIG_DFL) == SIG_ERR) {
        perror("sig_default signal failed");
    }
}
