#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"
#include "communicate.h"

void display(DisplayProps prop) {
    ChatItem* cur = prop.q->back;
    State* state = prop.state;

    // 画面を更新
    system("clear");
    printf("\r[Chat] ------------------------------------\r\n");
    // 一旦上まで戻る
    for (int i = 0; i < state->scrolledUp + CHAT_MAX - 1; i++) {
        if (cur == NULL || cur->prev == NULL) break;
        cur = cur->prev;
    }

    int printCount = 0;
    for (int i = 0; i < CHAT_MAX; i++) {
        if (cur == NULL) break;
        printf("%s: %s\r\n", cur->senderName, cur->content);
        cur = cur->next;
        printCount++;
    }
    while (printCount < CHAT_MAX) {
        printf("\r\n");
        printCount++;
    }
    printf("-------------------------------------------\r\n");
    printf("[\e[92m%s\e[39m] > ", state->myName);
    // printf("%s\e[92m|\e[39m\r\n", prop.inputString);
    for (int i = 0; i < CHAT_LEN; i++) {
        if (i == state->curPos) printf("\e[92m|\e[39m");
        if (prop.inputString[i] == '\0') break;
        putchar(prop.inputString[i]);
    }
    if (state->curPos == -1) {
        // empty string

    } else {
    }
    printf("%s\r\n", (state->isMeMuted ? "\e[91mMUTED\e[39m" : "     "));
    return;
}