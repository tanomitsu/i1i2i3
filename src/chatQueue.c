#include "chatQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

ChatQueue *createChatQueue() {
    ChatQueue *new = (ChatQueue *)malloc(sizeof(ChatQueue));
    new->front = NULL;
    new->back = NULL;
    new->size = 0;
    return new;
}

int chatPushBack(ChatQueue *q, char *content, char *senderName) {
    ChatItem *new = (ChatItem *)malloc(sizeof(ChatItem));

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
        new->prev = q->back;
        q->back->next = new;
        q->back = new;
        q->size += 1;
        return 0;
    } else {
        // queue doesn't have any ChatItem
        new->prev = NULL;
        q->front = new;
        q->back = new;
        q->size += 1;
        return 0;
    }
    return 1;
}

int chatPopFront(ChatQueue *q) {
    if (q->size == 0) return 1;
    ChatItem *poppedItem = q->front;
    q->front->next->prev = NULL;
    q->front = q->front->next;
    free(poppedItem);
    q->size -= 1;
    return 0;
}

int copyString(char *target, const char *src) {
    int res = 0;
    int cur = 0;
    for (cur = 0; cur < minInt(CHAT_LEN, strlen(src)); cur++) {
        target[cur] = src[cur];
        res++;
    }
    target[cur] = '\0';
    res = minInt(res, strlen(target));
    return res;
}

void clearQueue(ChatQueue *q) {
    ChatItem *cur1 = q->front;
    ChatItem *cur2;
    while (cur1 != NULL) {
        cur2 = cur1->next;
        free(cur1);
        cur1 = cur2;
    }
    q->back = NULL;
    q->front = NULL;
    q->size = 0;
    return;
}