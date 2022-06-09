#include "communicate.h"

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
#include "command.h"
#include "connect.h"
#include "fft.h"
#include "visualize.h"

int call(void *arg) {
    // properties passed
    CallProps props = *((CallProps *)arg);
    int s = props.s;
    char *stopProgram = props.stopProgram;
    pthread_mutex_t *mutex = props.mutex;
    const double ratio = 0.;

    // start recording
    FILE *soundIn = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    FILE *soundOut =
        popen("play -t raw -b 16 -c 1 -e s -r 44100 - 2> bin/err.out", "w");

    // after the client has connected
    fprintf(stderr, "A client has connected to your port.\n");
    fprintf(stderr, "Input what you want to send.\n");

    // definition of variables
    short sendBuf[BUFSIZE];
    short recvBuf[BUFSIZE];
    complex double recvBeforeX[BUFSIZE];
    complex double recvBeforeY[BUFSIZE];
    complex double sendX[BUFSIZE];
    complex double sendY[BUFSIZE];

    // initialize
    for (int i = 0; i < BUFSIZE; i++) recvBeforeY[i] = 0.0;

    for (;;) {
        // send sound
        int sendNum = fread(sendBuf, sizeof(short), BUFSIZE, soundIn);
        if (sendNum == 0) break;
        if (sendNum < 0) {
            perror("send");
            return 1;
        }

        // noise calcelling
        sample_to_complex(sendBuf, sendX, BUFSIZE);
        fft(sendX, sendY, BUFSIZE);
        for (int i = 0; i < BUFSIZE; i++) sendY[i] -= ratio * recvBeforeY[i];
        double maxAmp = 0;
        for (int i = 0; i < BUFSIZE; i++) {
            if (cabs(sendY[i]) > maxAmp) maxAmp = cabs(sendY[i]);
        }
        /*
        for (int i = 0; i < BUFSIZE; i++) sendY[i] = sendY[i] / maxAmp *
        5000;
        for (int i = 0; i < BUFSIZE; i++) {
            double f = i / (double)BUFSIZE * 44100;
            if (f < 20 || f > 5000) sendY[i] = 0;
        }
        */
        ifft(sendY, sendX, BUFSIZE);
        complex_to_sample(sendX, sendBuf, BUFSIZE);

        // send data
        send(s, sendBuf, sendNum * sizeof(short), 0);

        // receive sound
        recv(s, recvBuf, sizeof(short) * BUFSIZE, 0);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);

        // save pre-received data for noise cancellation
        sample_to_complex(recvBuf, recvBeforeX, BUFSIZE);
        fft(recvBeforeX, recvBeforeY, BUFSIZE);
        pthread_mutex_lock(mutex);
        if (*stopProgram) {
            pthread_mutex_unlock(mutex);
            break;
        } else
            pthread_mutex_unlock(mutex);
    }
    fprintf(stderr, "Ended connection.\n");
    system("/bin/stty cooked");
    pclose(soundIn);
    pclose(soundOut);
    return 0;
}

int sendChat(void *arg) {
    // properties passsed
    SendChatProps props = *((SendChatProps *)arg);
    int s = props.s;
    char *stopProgram = props.stopProgram;
    pthread_mutex_t *mutex = props.mutex;
    State *state = props.state;
    ChatQueue *q = state->q;
    char *cmd = state->cmd;

    DisplayProps _displayProps = (DisplayProps){
        .inputString = cmd,
        .mutex = mutex,
        .q = q,
        .state = state,
    };

    int cmdIndex = 0;
    char c;
    for (;;) {
        c = getchar();
        printf("\033[1K \033[1K");
        if (c == ':') {
            pthread_mutex_lock(mutex);
            *stopProgram = 1;  // end program
            pthread_mutex_unlock(mutex);
            break;
        } else if ((int)c == 127) {
            // back space
            if (cmdIndex > 0) {
                pthread_mutex_lock(mutex);
                cmd[--cmdIndex] = '\0';
                pthread_mutex_unlock(mutex);
            }
        } else if ((int)c == 13) {
            pthread_mutex_lock(mutex);
            cmd[cmdIndex++] = '\0';

            // interpret command and choose action
            int startPos = 0;
            // send input string
            Action action = interpretCommand(cmd, &startPos);
            if (action == SEND_CHAT) {
                ChatItem sendItem = (ChatItem){.next = NULL};
                copyString(sendItem.content, cmd);
                copyString(sendItem.senderName, state->myName);
                send(s, &sendItem, sizeof(ChatItem), 0);
            } else if (action == CHANGE_NAME) {
                copyString(state->myName, cmd + startPos);
            }
            cmdIndex = 0;
            memset(cmd, 0, COMMAND_LEN);
            pthread_mutex_unlock(mutex);
        } else {
            pthread_mutex_lock(mutex);
            cmd[cmdIndex++] = c;
            pthread_mutex_unlock(mutex);
        }
        display(_displayProps);
        if (*stopProgram) break;
    }
    return 1;
}

int recvChat(void *arg) {
    // properties passsed
    RecvChatProps props = *((RecvChatProps *)arg);
    int s = props.s;
    char *stopProgram = props.stopProgram;
    pthread_mutex_t *mutex = props.mutex;
    State *state = props.state;
    char *cmd = state->cmd;
    ChatQueue *q = state->q;

    DisplayProps _displayProps = (DisplayProps){
        .inputString = cmd,
        .mutex = mutex,
        .q = q,
        .state = state,
    };

    sleep(1);
    display(_displayProps);
    for (;;) {
        ChatItem recvItem;
        int recvNum = recv(s, &recvItem, sizeof(ChatItem), 0);
        if (recvNum == 0) die("thread/recvChat");

        // chatのqueueにpushするが、長さが5を超えた場合5に揃える
        pthread_mutex_lock(mutex);
        chatPushBack(q, recvItem.content, recvItem.senderName);
        if (q->size > CHAT_MAX) chatPopFront(q);
        pthread_mutex_unlock(mutex);
        display(_displayProps);
        if (*stopProgram) break;
    }
    return 0;
}