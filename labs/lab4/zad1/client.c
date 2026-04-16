#include "definitions.h"

int main(void) {
    int server_msg_queue = msgget(ftok("./server", 'a'), 0600);

    struct msgbuf message;
    int msg_queue = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    message.mtype = INIT;
    message.value = msg_queue;
    msgsnd(server_msg_queue, &message, sizeof(struct msgbuf) - sizeof(long), 0);

    int id;
    if (msgrcv(msg_queue, &message, sizeof(struct msgbuf) - sizeof(long), 0, 0) <= 0){
        exit(1);
    }
    if (message.mtype == FAIL) {
        printf("failed connection\n");
        fflush(stdout);
        exit(1);
    }
    id = message.value;
    printf("received new id: %d\n", id);
    fflush(stdout);

    pid_t pid = fork();
    if (pid == 0) {
        while(1) {
            if (msgrcv(msg_queue, &message, sizeof(struct msgbuf) - sizeof(long), MSG, 0) > 0) {
            printf("received: %s\n",  message.text);
            fflush(stdout);
            } 
        }
    }

    while(1) {
        message.mtype = MSG;
        message.value = id;
        if(fgets(message.text, sizeof(message.text), stdin) == NULL || strlen(message.text) <= 1) {
            continue;
        }
        message.text[strcspn(message.text, "\n")] = 0;
        msgsnd(server_msg_queue, &message, sizeof(struct msgbuf) - sizeof(long), 0);
    }
}
