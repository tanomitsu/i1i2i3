#include "communicate.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chatQueue.h"
#include "connect.h"
#include "fft.h"
#include "visualize.h"

int call(void *arg) {
    // parameters
    int s = *((int *)arg);
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
    }
    fprintf(stderr, "Ended connection.\n");
    system("/bin/stty cooked");
    pclose(soundIn);
    pclose(soundOut);
    return 0;
}

int sendChat(void *arg) {
    // parameters
    int s = *((int *)arg);
    char cmd[COMMAND_LEN];
    for (;;) {
        if (fgets(cmd, COMMAND_LEN, stdin) == NULL) break;  // チャット送信終了
        // 入力した文字を送信
        send(s, &cmd, sizeof(char) * COMMAND_LEN, 0);
    }
    return 1;
}

int recvChat(void *arg) {
    chatQueue *q = createChatQueue();
    int s = *((int *)arg);
    char recvBuf[COMMAND_LEN];
    sleep(1);
    display(q);
    for (;;) {
        int recvNum = recv(s, recvBuf, sizeof(char) * COMMAND_LEN, 0);
        if (recvNum == 0) die("thread/recvChat");

        // chatのqueueにpushするが、長さが5を超えた場合5に揃える
        chatPushBack(q, recvBuf, "guest");
        if (q->size > CHAT_MAX) chatPopFront(q);
        display(q);
    }
    return 0;
}