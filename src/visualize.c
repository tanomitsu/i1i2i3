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
    printf("[%s] > %s_\r\n", state->myName, prop.inputString);
    printf("MUTE: %s\r\n",
           (state->isMeMuted ? "\x1b[32mEnabled\x1b[39m"
                             : "\x1b[31mDisabled	\x1b[39m"));
    return;
}