#include <arpa/inet.h>
#include <communicate.h>
#include <connect.h>
#include <fft.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <util.h>

int main(int argc, char **argv) {
    // connect
    int call_s = -1;
    int chat_s = -1;
    const int callPort = 55555;
    const int chatPort = 50000;
    if (argc == 1) {
        // server side
        // ./discorb.out
        call_s = serverConnect(callPort);
        // chat_s = serverConnect(chatPort);
    } else if (argc == 2) {
        // client side
        // ./a.out <IP>
        char *ip = argv[1];
        call_s = clientConnect(ip, callPort);
        // chat_s = clientConnect(ip, chatPort);
    } else {
        fprintf(stderr, "usage: %s <ip> or %s \n", argv[0], argv[0]);
        exit(1);
    }

    // set up multi thread
    pthread_t callThread, chatThread;
    int callRet, chatRet;
    callRet = pthread_create(&callThread, NULL, (void *)&call, (void *)&call_s);
    if (callRet != 0) die("thread");
    chatRet = pthread_create(&chatThread, NULL, (void *)&chat, (void *)&chat_s);
    if (chatRet != 0) die("thread");

    // multi thread後片付け
    pthread_join(callThread, NULL);
    pthread_join(chatThread, NULL);
    close(call_s);
    close(chat_s);
}
