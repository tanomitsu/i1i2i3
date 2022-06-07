#pragma once

#define CHAT_LEN 100
#define NAME_LEN 100

typedef struct chatqueue chatQueue;
typedef struct chatitem chatItem;

struct chatqueue {
    chatItem *front;
    chatItem *back;
    int size;
};

struct chatitem {
    char content[CHAT_LEN];
    char senderName[NAME_LEN];
    chatItem *next;
};

/*
空のchatQueueを作成して返す関数
freeが必要！
*/
chatQueue *createChatQueue();

/*
chatQueueにpush_backして、整数を返す
0: 成功
1: 失敗
*/
int chatPushBack(chatQueue *q, char *content, char *senderName);