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
#include "connect.h"
#include "fft.h"
#include "visualize.h"

int call(void *arg) {
    // properties passed
    CallProps props = *((CallProps *)arg);
    int s = props.s;
    char *stopProgram = props.stopProgram;
    pthread_mutex_t *mutex = props.mutex;
    ChatQueue *q = props.q;
    char *inputString = props.inputString;
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
    char *cmd = props.inputString;
    ChatQueue *q = props.q;

    DisplayProps _displayProps = (DisplayProps){
        .inputString = cmd,
        .mutex = mutex,
        .q = q,
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
            // 入力した文字を送信
            send(s, cmd, sizeof(char) * COMMAND_LEN, 0);
            cmdIndex = 0;
            cmd[0] = '\0';
            pthread_mutex_unlock(mutex);
        } else {
            pthread_mutex_lock(mutex);
            cmd[cmdIndex++] = c;
            cmd[cmdIndex] = '\0';
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
    char *inpuString = props.inputString;
    ChatQueue *q = props.q;

    DisplayProps _displayProps = (DisplayProps){
        .inputString = inpuString,
        .mutex = mutex,
        .q = q,
    };

    char recvBuf[COMMAND_LEN];
    sleep(1);
    display(_displayProps);
    for (;;) {
        int recvNum = recv(s, recvBuf, sizeof(char) * COMMAND_LEN, 0);
        if (recvNum == 0) die("thread/recvChat");

        // chatのqueueにpushするが、長さが5を超えた場合5に揃える
        pthread_mutex_lock(mutex);
        chatPushBack(q, recvBuf, "guest");
        if (q->size > CHAT_MAX) chatPopFront(q);
        pthread_mutex_unlock(mutex);
        display(_displayProps);
        if (*stopProgram) break;
    }
    return 0;
}