#include "common.h"

void cleanup_memory(char *filename) {
    shm_unlink(filename);
    printf("cleaned up %s file\n", filename);
}

void cleanup_semaphore(char *filename) {
    sem_unlink(filename);
    printf("cleaned up %s file\n", filename);
}

int main(void) {
    for (int i = 0; i < 2; ++i) {
        cleanup_memory(memory_filenames[i]);
        cleanup_semaphore(semaphore_filenames[i]);
    }
}
