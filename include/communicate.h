#pragma once
#define BUFSIZE 1024
#define COMMAND_LEN 100

#include <pthread.h>
#include <visualize.h>

// sへのポインタを引数にとり、通話を行う
int call(void *arg);

int sendChat(void *arg);

int recvChat(void *arg);

typedef struct callProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    ChatQueue *q;
    char *inputString;
} CallProps;

typedef struct sendChatProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    ChatQueue *q;
    char *inputString;
} SendChatProps;

typedef struct recvChatProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    ChatQueue *q;
    char *inputString;
} RecvChatProps;