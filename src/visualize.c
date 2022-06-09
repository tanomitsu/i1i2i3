#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"
#include "communicate.h"

void display(DisplayProps prop) {
    ChatItem* cur = prop.q->front;
    State* state = prop.state;

    // 画面を更新
    system("clear");
    printf("\r[Chat] ------------------------------------\r\n");
    while (cur != NULL) {
        printf("%s: %s\r\n", cur->senderName, cur->content);
        cur = cur->next;
    }
    for (int i = 0; i < CHAT_MAX - prop.q->size; i++) printf("\r\n");
    printf("-------------------------------------------\r\n");
    printf("[%s] > %s_\r\n", state->myName, prop.inputString);
    return;
}