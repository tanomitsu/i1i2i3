#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int state_s = -1;
    const int callPort = 55555;
    const int chatPort = 55556;
    const int statePort = 55557;
    char stopProgram = 0;
    char *ip;
    State state = (State){
        .myName = "guest",
        .isMeMuted = 0,
        .scrolledUp = 0,
        .curPos = 0,
    };
    memset(state.cmd, 0, COMMAND_LEN * sizeof(char));
    state.q = createChatQueue();
    ConnectMode connectMode;

    if (argc == 1) {
        // server side
        // ./discorb.out
        connectMode = SERVER;
    } else if (argc == 2) {
        // client side
        // ./a.out <IP>
        ip = argv[1];
        connectMode = CLIENT;
    } else {
        fprintf(stderr, "usage: %s <ip> or %s \n", argv[0], argv[0]);
        exit(1);
    }

    // threads for connecting
    pthread_t callConnectThread, chatConnectThread, stateConnectThread;
    CallConnectProps _CallConnectProps = (CallConnectProps){
        .ip = ip,
        .port = callPort,
        .s = &call_s,
        .connectMode = connectMode,
    };
    ChatConnectProps _ChatConnectProps = (ChatConnectProps){
        .ip = ip,
        .port = chatPort,
        .s = &chat_s,
        .connectMode = connectMode,
    };
    StateConnectProps _StateConnectProps = (StateConnectProps){
        .ip = ip,
        .port = statePort,
        .s = &state_s,
        .connectMode = connectMode,
    };

    // connect
    int chatConnectRet =
        pthread_create(&chatConnectThread, NULL, (void *)&chatConnect,
                       (void *)&_ChatConnectProps);
    int callConnectRet =
        pthread_create(&callConnectThread, NULL, (void *)&chatConnect,
                       (void *)&_CallConnectProps);
    int stateConnectRet =
        pthread_create(&stateConnectThread, NULL, (void *)&callConnect,
                       (void *)&_StateConnectProps);

    // error handling for connecting threads
    if (chatConnectRet != 0) die("thread/chatConnectThread");
    if (callConnectRet != 0) die("thread/callConnectThread");
    if (stateConnectRet != 0) die("thread/stateConnectThread");

    // connect用threadを後片付け
    pthread_join(chatConnectThread, NULL);
    pthread_join(callConnectThread, NULL);
    pthread_join(stateConnectThread, NULL);

    // mutex lock用の変数
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    // get char before enter is pressed
    system("/bin/stty raw onlcr");

    // properties to pass to pthread functions
    CallProps _callProps = (CallProps){
        .s = call_s,
        .stopProgram = &stopProgram,
        .mutex = &mutex,
        .state = &state,
    };
    SendChatProps _sendChatProps = (SendChatProps){
        .s = chat_s,
        .stopProgram = &stopProgram,
        .mutex = &mutex,
        .state = &state,
    };
    RecvChatProps _recvChatProps = (RecvChatProps){
        .s = chat_s,
        .stopProgram = &stopProgram,
        .mutex = &mutex,
        .state = &state,
    };

    SendRcvStateProps _sendRcvStateProps = (SendRcvStateProps){
        .s = state_s,
        .stopProgram = &stopProgram,
        .mutex = &mutex,
        .state = &state,
    };

    // set up multi thread
    pthread_t callThread, sendChatThread, recvChatThread, sendRcvStateThread;
    state.threads.callThread = &callThread;
    state.threads.sendChatThread = &sendChatThread;
    state.threads.recvChatThread = &recvChatThread;
    state.threads.stateThread = &sendRcvStateThread;

    int callRet =
        pthread_create(&callThread, NULL, (void *)&call, (void *)&_callProps);
    int sendChatRet = pthread_create(&sendChatThread, NULL, (void *)&sendChat,
                                     (void *)&_sendChatProps);
    int recvChatRet = pthread_create(&recvChatThread, NULL, (void *)&recvChat,
                                     (void *)&_recvChatProps);
    int sendRcvStateRet =
        pthread_create(&sendRcvStateThread, NULL, (void *)&sendRcvState,
                       (void *)&_sendRcvStateProps);

    // error handling
    if (callRet != 0) die("thread/call");
    if (sendChatRet != 0) die("thread/sendChat");
    if (recvChatRet != 0) die("thread/recvChat");
    if (sendRcvStateRet != 0) die("thread/sendState");

    // multi thread後片付け
    pthread_mutex_destroy(&mutex);
    pthread_join(recvChatThread, NULL);
    pthread_join(sendChatThread, NULL);
    pthread_join(callThread, NULL);
    pthread_join(sendRcvStateThread, NULL);
    close(call_s);
    close(chat_s);
    close(state_s);
    system("/bin/stty cooked");
}
