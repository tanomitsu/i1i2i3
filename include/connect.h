#pragma once
#include "communicate.h"

typedef enum connectmode { CLIENT, SERVER } ConnectMode;

int serverConnect(int port);
int clientConnect(char *ip, int port);
int autoConnect(char *ip, int port, ConnectMode connectMode);
int callConnect(void *arg);
int chatConnect(void *arg);

typedef struct callCoonectProps {
    char *ip;
    int port;
    int *s;  // 出力用
    ConnectMode connectMode;
} CallConnectProps;

typedef struct chatConnectProps {
    char *ip;
    int port;
    int *s;  // 出力用
    ConnectMode connectMode;
} ChatConnectProps;

typedef struct stateConnectProps {
    char *ip;
    int port;
    int *s;
    ConnectMode connectMode;
} StateConnectProps;
