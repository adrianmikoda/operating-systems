#include "definitions.h"

int main(void){
    sem_t *id_sems[2] = {open_semaphore(semaphore_filenames[PRIORITY]), open_semaphore(semaphore_filenames[NORMAL])};
    int fds[2] = {open_memory(memory_filenames[PRIORITY]), open_memory(memory_filenames[NORMAL])};
    memory *mems[2] = {map_memory(fds[PRIORITY]), map_memory(fds[NORMAL])};

    close(fds[PRIORITY]);
    close(fds[NORMAL]);

    while (1) {
        sleep(5);
        
        sem_wait(id_sems[PRIORITY]);
        sem_wait(id_sems[NORMAL]);

        char *task = read_task(mems[NORMAL]);
        if (task) {
            printf("[manager]: changed \"%s\" memory from NORMAL to PRIORITY\n", task);
            write_task(mems[PRIORITY], task);
        }

        printf("[manager]: PRIORITY memory task count: %d\n", mems[PRIORITY]->task_count);
        printf("[manager]: NORMAL memory task count: %d\n", mems[NORMAL]->task_count);
        fflush(stdout);

        sem_post(id_sems[PRIORITY]);
        sem_post(id_sems[NORMAL]);
    }

    munmap(mems[PRIORITY], sizeof(memory));
    munmap(mems[NORMAL], sizeof(memory));
    sem_close(id_sems[PRIORITY]);
    sem_close(id_sems[NORMAL]);
}
