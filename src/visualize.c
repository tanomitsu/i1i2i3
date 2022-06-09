#include "visualize.h"

#include <stdio.h>
#include <stdlib.h>

#include "chatQueue.h"

void display(DisplayProps prop) {
    pthread_mutex_lock(prop.mutex);
    ChatItem* cur = prop.q->front;

    // 画面を更新
    system("clear");
    printf("[Chat] ----------------\r\n");
    while (cur != NULL) {
        printf("%s: %s\r\n", cur->senderName, cur->content);
        cur = cur->next;
    }
    for (int i = 0; i < CHAT_MAX - prop.q->size; i++) printf("\r\n");
    printf("-----------------------\r\n");
    pthread_mutex_unlock(prop.mutex);
    return;
}