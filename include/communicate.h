#pragma once
#define BUFSIZE 1024
#define COMMAND_LEN 100

// sへのポインタを引数にとり、通話を行う
int call(void *arg);

int sendChat(void *arg);

int recvChat(void *arg);