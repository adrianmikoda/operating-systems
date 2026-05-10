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
    sem_t *id_sems[2] = {open_semaphore(semaphore_filenames[PRIORITY]), open_semaphore(semaphore_filenames[NORMAL])};
    int fds[2] = {open_memory(memory_filenames[PRIORITY]), open_memory(memory_filenames[NORMAL])};
    memory *mems[2] = {map_memory(fds[PRIORITY]), map_memory(fds[NORMAL])};
    close(fds[PRIORITY]);
    close(fds[NORMAL]);

    usleep(rand()%4000001 + 1000000);
    
    while (1) {
        if (rand() % 100 < 50) {
            char *task = NULL;
            for (int i = 0; i < 2; ++i) {
                memory *mem = mems[i];
                sem_t *id_sem = id_sems[i];
            
                sem_wait(id_sem);
                task = read_task(mem);
                sem_post(id_sem);
            
                if (task) { break; }
            }

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

    munmap(mems[PRIORITY], sizeof(memory));
    munmap(mems[NORMAL], sizeof(memory));
    sem_close(id_sems[PRIORITY]);
    sem_close(id_sems[NORMAL]);
}
