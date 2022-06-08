#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"

void display(chatQueue* q) {
    chatItem* cur = q->front;

    // 画面を更新
    // system("/bin/stty cooked");
    // printf("\033[%dA", CHAT_MAX + 2);
    system("clear");
    printf("[Chat] ----------------\r\n");
    while (cur != NULL) {
        printf("%s: %s\r\n", cur->senderName, cur->content);
        cur = cur->next;
    }
    for (int i = 0; i < CHAT_MAX - q->size; i++) printf("\r\n");
    printf("-----------------------\r\n");
    // system("/bin/stty raw onlcr");
    return;
}