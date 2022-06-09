#pragma once
#include "chatQueue.h"
#include "communicate.h"

typedef struct displayProps {
    ChatQueue *q;
    pthread_mutex_t *mutex;
    char *inputString;
    struct state *state;
} DisplayProps;
void display(DisplayProps props);
