#pragma once

#include <pthread.h>

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