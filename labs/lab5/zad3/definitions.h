#include <mqueue.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "common.h"

sem_t *open_semaphore(char filename[]);
int open_memory(char filename[]);
memory *map_memory(int fd);
char *read_task(memory* mem);
void write_task(memory* mem, char* task);
