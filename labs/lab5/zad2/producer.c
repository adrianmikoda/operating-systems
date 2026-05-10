#include "definitions.h"

char *random_string(int length) {
    if (length <= 0) {
        return NULL;
    }
    char* string = malloc(length + 1);
    if (!string) {
        return NULL;
    }
    for (int i = 0; i < length; ++i) {
        string[i] = 'a' + rand() % 26;
    }
    string[length]  = '\0';
    
    return string;
}

void write_task(memory* mem, char* task) {
    if (!task) {
        printf("[%d]: error saving task in memory", getpid());
        return;
    }
    strcpy(mem->tasks[mem->write_index], task);
    mem->task_count++;
    if (mem->task_count >= mem->max_task_count) {
        mem->read_index = (mem->read_index + 1) % mem->max_task_count;
    }
    mem->write_index = (mem->write_index + 1) % mem->max_task_count;
}

int main(void) {
    srand(getpid());

    sem_t *id_sems[2] = {open_semaphore(semaphore_filenames[PRIORITY]), open_semaphore(semaphore_filenames[NORMAL])};
    int fds[2] = {open_memory(memory_filenames[PRIORITY]), open_memory(memory_filenames[NORMAL])};
    memory *mems[2] = {map_memory(fds[PRIORITY]), map_memory(fds[NORMAL])};
    close(fds[PRIORITY]);
    close(fds[NORMAL]);

    sem_t *id_sem;
    memory *mem;
    char* mem_desc;

    usleep(rand()%4000001 + 1000000);

    while (1) {
        if (rand() % 100 < 50) {
            if (rand() % 100 < 30) {
                id_sem = id_sems[PRIORITY];
                mem = mems[PRIORITY];
                mem_desc = "PRIORITY";
            } else {
                id_sem = id_sems[NORMAL];
                mem = mems[NORMAL];
                mem_desc = "NORMAL";
            }

            sem_wait(id_sem);
            char *task = random_string(TASK_LENGTH);
            write_task(mem, task);
            printf("[%d]: saved \"%s\" in %s memory\n", getpid(), task, mem_desc);
            fflush(stdout);
            free(task);
            sem_post(id_sem);

            munmap(mem, sizeof(memory));
            sem_close(id_sem);
        }
        
        usleep(rand()%4000001 + 1000000);
    }
}
