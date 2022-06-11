#pragma once
#define BUFSIZE 1024
#define COMMAND_LEN 1024

#include <pthread.h>
#include <visualize.h>

typedef struct threads {
    pthread_t *callThread;
    pthread_t *sendChatThread;
    pthread_t *recvChatThread;
} Threads;

typedef struct state {
    char myName[NAME_LEN];
    char cmd[COMMAND_LEN];
    ChatQueue *q;
    unsigned char isMeMuted;
    int scrolledUp;  // how many scrolled
    Threads threads;
    int curPos;  // chat cur's position
} State;

// sへのポインタを引数にとり、通話を行う
int call(void *arg);

int sendChat(void *arg);

int recvChat(void *arg);

typedef struct callProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    State *state;
} CallProps;

typedef struct sendChatProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    State *state;
} SendChatProps;

typedef struct recvChatProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    State *state;
} RecvChatProps;
