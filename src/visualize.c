#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"

void display(chatQueue* q) {
    chatItem* cur = q->front;

    // 画面を更新
    printf("\033[%dA", CHAT_MAX + 2);
    system("clear");
    puts("[Chat] ----------------");
    while (cur != NULL) {
        printf("%s: %s", cur->senderName, cur->content);
        cur = cur->next;
    }
    for (int i = 0; i < CHAT_MAX - q->size; i++) putchar('\n');
    puts("-----------------------");
    return;
}