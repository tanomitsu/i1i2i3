#include "chatQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

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

    copyString(new->content, content);
    copyString(new->senderName, senderName);
    /*
    strcpy(new->content, content);
    strcpy(new->senderName, senderName);
    */
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

int copyString(char *target, const char *src) {
    int res = 0;
    for (int i = 0; i < minInt(CHAT_LEN, strlen(src)); i++) {
        target[i] = src[i];
        res++;
    }
    target[strlen(target)] = '\0';
    res = minInt(res, strlen(target));
    return res;
}