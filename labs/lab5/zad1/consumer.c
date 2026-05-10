#include "definitions.h"

char *read_task(memory* mem) {
    if (mem->task_count <= 0) {
        return NULL;
    }
    char *task = malloc(TASK_LENGTH + 1);
    if (!task) {
        printf("malloc error");
        return NULL;
    }
    strcpy(task, mem->tasks[mem->read_index]);
    mem->task_count--;
    mem->read_index = (mem->read_index + 1) % mem->max_task_count;    
    return task;
}

int main(void) {
    srand(getpid());

    sem_t *id_sem = open_semaphore(semaphore_filename);
    int fd = open_memory(memory_filename);
    memory *mem = map_memory(fd);

    usleep(rand()%4000001 + 1000000);

    while (1) {
        if (rand() % 100 < 50) {
            sem_wait(id_sem);
            char *task = read_task(mem);
            sem_post(id_sem);

            if (task) {
                printf("[%d]: read task \"%s\"\n", getpid(), task);
                fflush(stdout);
                int n = strlen(task);
                for (int i = 0; i < n ; ++i) {
                    printf("[%d]: read \"%c\"\n", getpid(), task[i]);
                    fflush(stdout);
                    usleep(300000);
                }
                free(task);
            } else {
                printf("[%d]: no task to read\n", getpid());
            }
        }

        usleep(rand()%4000001 + 1000000);
    }

    munmap(mem, sizeof(memory));
    sem_close(id_sem);
}
