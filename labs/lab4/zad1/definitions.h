#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>

#define INIT 1
#define MSG 2
#define FAIL 3

struct msgbuf {
    long mtype;
    int value;
    char text[256];
};

#endif
