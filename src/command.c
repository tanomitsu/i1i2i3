#include "command.h"

#include <pthread.h>
#include <string.h>

Action interpretCommand(char *cmd, int *startPos) {
    if (cmd[0] == '/') {
        int check = -1;
        if ((check = checkCommand(cmd, "name")) >= 0) {
            *startPos = check;
            return CHANGE_NAME;
        }
        if ((check = checkCommand(cmd, "mute")) >= 0) {
            *startPos = check;
            return TOGGLE_MUTE;
        }
        if ((check = checkCommand(cmd, "clear")) >= 0) {
            *startPos = check;
            return CLEAR_CHAT;
        }
        return SEND_CHAT;
    } else
        return SEND_CHAT;
}

/*
    コマンドと一致するか調べる関数
    -1: 不一致
    それ以外: startPos
*/
int checkCommand(char *cmd, char *target) {
    int cur = 1;
    for (int i = 0; i < strlen(target); i++) {
        if (cmd[cur++] != target[i]) return -1;
    }
    if (cmd[cur] != ' ' && cmd[cur] != '\0') return -1;
    while (cur < strlen(cmd) && cmd[cur] != '\0' && cmd[cur] == ' ') cur++;
    return cur;
}