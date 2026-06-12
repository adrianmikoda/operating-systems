#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

extern char *memory_filenames[2];
extern char *semaphore_filenames[2];

#define MAX_TASK_COUNT 100
#define TASK_LENGTH 10
#define PRIORITY 0
#define NORMAL 1

typedef struct {
    int read_index;
    int write_index;
    int task_count;
    int max_task_count;
    char tasks[MAX_TASK_COUNT][TASK_LENGTH + 1];
} memory;
