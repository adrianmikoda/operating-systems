#include "definitions.h"

#define MAXCLIENTS 10

void handle_init(struct msgbuf *message, int *client_count, int client_msg_queues[]){
    int client_msg_queue = message->value;

    if (*client_count >= MAXCLIENTS) {
        message->mtype = FAIL;
        msgsnd(client_msg_queue, message, sizeof(*message) - sizeof(long), IPC_NOWAIT);
        printf("failed client connection\n");
        fflush(stdout);
        return;
    }

    int new_client_id = (*client_count)++;
    client_msg_queues[new_client_id] = client_msg_queue;
    message->mtype = INIT;
    message->value = new_client_id;
    msgsnd(client_msg_queue, message, sizeof(*message) - sizeof(long), IPC_NOWAIT);
    printf("connected new client\n");
    fflush(stdout);
}

void handle_msg(struct msgbuf *message, int *client_count, int client_msg_queues[]){
    printf("received: \"%s\" id: %d\n", message->text, message->value);
    fflush(stdout);
    int from_id = message->value;
    for (int i = 0 ; i < *client_count; ++i) {
        if (i != from_id) {
            message -> mtype = MSG;
            msgsnd(client_msg_queues[i], message, sizeof(*message) - sizeof(long), IPC_NOWAIT);
        }
    }
}

int main(void) {
    printf("server started\n");
    fflush(stdout);
    key_t key = ftok("./server", 'a');
    int msg_queue_id = msgget(key, IPC_CREAT | 0600);

    int client_msg_queues[MAXCLIENTS];
    int client_count = 0;
    struct msgbuf message;

    while(1) {
        if (msgrcv(msg_queue_id, &message, sizeof(message) - sizeof(long), 0, 0) >= 0) {
            switch (message.mtype) {
                case INIT:
                    handle_init(&message, &client_count, client_msg_queues);
                    break;
                case MSG:
                    handle_msg(&message, &client_count, client_msg_queues);
            }
        }
    }

    printf("server stopped\n");
    msgctl(msg_queue_id, IPC_RMID, NULL);

    return 0;
}
