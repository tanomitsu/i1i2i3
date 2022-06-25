#pragma once
#include "communicate.h"

typedef enum connectmode
{
    CLIENT,
    SERVER
} ConnectMode;

int serverConnect(int port);
int clientConnect(char *ip, int port);
int autoConnect(char *ip, int port, ConnectMode connectMode);
int connectThread(void *arg);

typedef struct connectThreadProps
{
    char *ip;
    int port;
    int *s;
    ConnectMode connectMode;
} ConnectThreadProps;
