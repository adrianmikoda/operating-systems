#include "definitions.h"

char *memory_filenames[2] = {"priority_memory", "normal_memory"};
char *semaphore_filenames[2] = {"priority_semaphore", "normal_semaphore"};

sem_t *open_semaphore(char filename[]) {
    sem_t *id_sem = sem_open(filename, 1);
    if (id_sem == SEM_FAILED) {
        printf("semaphore file does not exist, run setup first\n");
        fflush(stdout);
        perror("sem_open"); 
        exit(1); 
    }
    
    return id_sem;
}

int open_memory(char filename[]) {
    int fd = shm_open(filename, O_RDWR, 0644);
    if (fd == -1) {
        if (errno == ENOENT) {
            printf("shared memory file does not exist, run setup first\n");
            fflush(stdout);
        }
        perror("shm_open");
        exit(1);
    }

    return fd;
}

memory *map_memory(int fd) {
    memory *mem = mmap(NULL, sizeof(memory), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (mem == MAP_FAILED) { perror("mmap"); exit(1); }

    return mem;
}

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
