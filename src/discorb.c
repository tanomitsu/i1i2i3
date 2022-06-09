#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chatQueue.h"
#include "communicate.h"
#include "connect.h"
#include "fft.h"
#include "util.h"

int main(int argc, char **argv) {
    // connect
    int call_s = -1;
    int chat_s = -1;
    const int callPort = 55556;
    const int chatPort = 55557;
    char stopProgram = 0;  // 1になったらプログラムを終了する
    if (argc == 1) {
        // server side
        // ./discorb.out
        call_s = serverConnect(callPort);
        chat_s = serverConnect(chatPort);
    } else if (argc == 2) {
        // client side
        // ./a.out <IP>
        char *ip = argv[1];
        call_s = clientConnect(ip, callPort);
        usleep(1000);  // サーバー側がlistenするまで待つ
        chat_s = clientConnect(ip, chatPort);
    } else {
        fprintf(stderr, "usage: %s <ip> or %s \n", argv[0], argv[0]);
        exit(1);
    }

    // mutex lock用の変数
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    // get char before enter is pressed
    system("/bin/stty raw onlcr");

    // properties to pass to pthread functions
    CallProps _callProps =
        (CallProps){.s = call_s, .stopProgram = &stopProgram, .mutex = &mutex};
    SendChatProps _sendChatProps = (SendChatProps){
        .s = chat_s, .stopProgram = &stopProgram, .mutex = &mutex};
    RecvChatProps _recvChatProps = (RecvChatProps){
        .s = chat_s, .stopProgram = &stopProgram, .mutex = &mutex};

    // set up multi thread
    pthread_t callThread, sendChatThread, recvChatThread;
    int callRet =
        pthread_create(&callThread, NULL, (void *)&call, (void *)&_callProps);
    int sendChatRet = pthread_create(&sendChatThread, NULL, (void *)&sendChat,
                                     (void *)&_sendChatProps);
    int recvChatRet = pthread_create(&recvChatThread, NULL, (void *)&recvChat,
                                     (void *)&_recvChatProps);

    // error handling
    if (callRet != 0) die("thread/call");
    if (sendChatRet != 0) die("thread/sendChat");
    if (recvChatRet != 0) die("thread/recvChat");

    // multi thread後片付け
    pthread_mutex_destroy(&mutex);
    pthread_join(callThread, NULL);
    pthread_join(sendChatThread, NULL);
    pthread_join(recvChatThread, NULL);
    close(call_s);
    close(chat_s);
    system("/bin/stty cooked");
}
