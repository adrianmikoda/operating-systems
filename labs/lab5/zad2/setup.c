#include "common.h"

void setup_memory(char *filename) {
    int fd = shm_open(filename, O_CREAT | O_EXCL | O_RDWR, 0644);
    if (fd == -1) {
        perror("shm_open"); exit(1);
    } else {
        if (ftruncate(fd, sizeof(memory)) == -1) { perror("ftruncate"); exit(1); };
    }

    printf("set up %s file\n", filename);
}

void setup_semaphore(char *filename) {
    sem_t *id_sem = sem_open(filename, O_CREAT, 0600, 1);
    if (id_sem == SEM_FAILED) { perror("sem_open"); exit(1); }
    
    printf("set up %s file\n", filename);
}

int main(void) {
    for (int i = 0; i < 2; ++i) {
        setup_memory(memory_filenames[i]);
        setup_semaphore(semaphore_filenames[i]);
    }
}
