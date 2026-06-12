#include "common.h"

void cleanup_memory() {
    shm_unlink(memory_filename);
    printf("cleaned up %s file\n", memory_filename);
}

void cleanup_semaphore() {
    sem_unlink(semaphore_filename);
    printf("cleaned up %s file\n", semaphore_filename);
}

int main(void) {
    cleanup_memory();
    cleanup_semaphore();
}
