#include "chatQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

chatQueue *createChatQueue() {
    chatQueue *new = (chatQueue *)malloc(sizeof(chatQueue));
    new->front = NULL;
    new->back = NULL;
    new->size = 0;
    return new;
}

int chatPushBack(chatQueue *q, char *content, char *senderName) {
    chatItem *new = (chatItem *)malloc(sizeof(chatItem));

    // set iniial values
    strcpy(new->content, content);
    strcpy(new->senderName, senderName);
    new->next = NULL;

    if (q->size > 0) {
        // queue alredy has chatItems
        q->back->next = new;
        q->back = new;
        q->size += 1;
        return 0;
    } else {
        // queue doesn't have any chatItem
        q->front = new;
        q->back = new;
        q->size += 1;
        return 0;
    }
    return 1;
}

int chatPopFront(chatQueue *q) {
    if (q->size == 0) return 1;
    chatItem *poppedItem = q->front;
    q->front = q->front->next;
    free(poppedItem);
    q->size -= 1;
    return 0;
}