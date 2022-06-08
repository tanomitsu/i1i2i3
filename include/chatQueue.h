#pragma once

#define CHAT_LEN 100
#define NAME_LEN 100
#define CHAT_MAX 5

typedef struct chatqueue ChatQueue;
typedef struct chatitem ChatItem;

struct chatqueue {
    ChatItem *front;
    ChatItem *back;
    int size;
};

struct chatitem {
    char content[CHAT_LEN];
    char senderName[NAME_LEN];
    ChatItem *next;
};

/*
空のchatQueueを作成して返す関数
freeが必要！
*/
ChatQueue *createChatQueue();

/*
chatQueueにpush_backして、整数を返す
0: 成功
1: 失敗
*/
int chatPushBack(ChatQueue *q, char *content, char *senderName);

/*
chatQueueからpopするが、q->sizeが0の時は何もしない
0: 成功
1: 失敗
*/
int chatPopFront(ChatQueue *q);

/*
文字列をコピーできるだけコピーし、コピーした文字数を返す関数
*/
int copyString(char *target, const char *src);