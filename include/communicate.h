#pragma once
#define BUFSIZE 1024
#define COMMAND_LEN 1024

#include <pthread.h>
#include <visualize.h>

typedef struct threads Threads;
typedef struct state State;
typedef struct callState CallState;

struct threads {
    pthread_t *callThread;
    pthread_t *sendChatThread;
    pthread_t *recvChatThread;
    pthread_t *sendStateThread;
    pthread_t *recvStateThread;
};

struct state {
    char myName[NAME_LEN];
    char cmd[COMMAND_LEN];
    ChatQueue *q;
    unsigned char isMeMuted;
    int scrolledUp;  // how many scrolled
    Threads threads;
    int curPos;  // chat cur's position
};

/*
相互通信して状態を共有する用の構造体
*/
struct callState {
    char stopProgram;
};

// sへのポインタを引数にとり、通話を行う
int call(void *arg);

int sendChat(void *arg);

int recvChat(void *arg);

int sendState(void *arg);

int recvState(void *arg);

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

typedef struct sendStateProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    State *state;
} SendStateProps;

typedef struct recvStateProps {
    int s;
    char *stopProgram;
    pthread_mutex_t *mutex;
    State *state;
} RecvStateProps;