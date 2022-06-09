#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum action {
    QUIT_PROGRAM,
    SEND_CHAT,
    TOGGLE_MUTE,
    CHANGE_NAME,
} Action;

/*
    コマンドの種別を認識し返す
    第一引数の最初の位置のindexをstartPosに返す
*/
Action interpretCommand(char *cmd, int *startPos);
int checkCommand(char *cmd, char *target);