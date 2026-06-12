#include "definitions.h"

void sig_handle(void (*function)(int)) {
    printf("[%d] Wywołano funkcję: '%s()'\n", getpid(), __func__);
    if (signal(SIGUSR1, function) == SIG_ERR) {
        perror("sig_handle signal failed");
    }
}
