#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"

void display(DisplayProps prop) {
    ChatItem* cur = prop.q->front;

    // 画面を更新
    system("clear");
    printf("\r[Chat] ------------------------------------\r\n");
    while (cur != NULL) {
        printf("%s: %s\r\n", cur->senderName, cur->content);
        cur = cur->next;
    }
    for (int i = 0; i < CHAT_MAX - prop.q->size; i++) printf("\r\n");
    printf("-------------------------------------------\r\n");
    printf("> %s_\r\n", prop.inputString);
    return;
}