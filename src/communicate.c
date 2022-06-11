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
    pthread_mutex_t *mutex = props.mutex;
    State *state = props.state;
    const double ratio = 0.;

    // start recording
    FILE *soundIn = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    FILE *soundOut =
        popen("play -t raw -b 16 -c 1 -e s -r 44100 - 2> bin/err.out", "w");

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

        ifft(sendY, sendX, BUFSIZE);
        complex_to_sample(sendX, sendBuf, BUFSIZE);

        // send data
        if (state->isMeMuted) {
            // if MUTE, dont send anything
            for (int i = 0; i < BUFSIZE; i++) sendBuf[i] = 0;
        }
        send(s, sendBuf, sendNum * sizeof(short), 0);

        // receive sound
        recv(s, recvBuf, sizeof(short) * BUFSIZE, 0);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);

        // save pre-received data for noise cancellation
        sample_to_complex(recvBuf, recvBeforeX, BUFSIZE);
        fft(recvBeforeX, recvBeforeY, BUFSIZE);
        pthread_mutex_lock(mutex);
    }
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
        if (c == 127) {
            // backspace key
            if (cmdIndex > 0) {
                pthread_mutex_lock(mutex);
                for (int i = cmdIndex - 1; i < COMMAND_LEN - 1; i++) {
                    cmd[i] = cmd[i + 1];
                    if (cmd[i + 1] == '\0') break;
                }
                cmdIndex--;
                state->curPos--;
                pthread_mutex_unlock(mutex);
            }
        } else if (c == 13) {
            // enter key
            pthread_mutex_lock(mutex);
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
            } else if (action == TOGGLE_MUTE) {
                state->isMeMuted = 1 - state->isMeMuted;
            } else if (action == CLEAR_CHAT) {
                state->scrolledUp = 0;
                clearQueue(state->q);
            } else if (action == QUIT_PROGRAM) {
                *stopProgram = 1;
                pthread_mutex_unlock(mutex);
            }
            cmdIndex = 0;
            state->curPos = 0;
            memset(cmd, 0, COMMAND_LEN);
            pthread_mutex_unlock(mutex);
        } else if (c == 27) {
            // arrow keys
            c = getchar();
            c = getchar();
            if (c == 65) {
                // UP key
                if (state->scrolledUp + CHAT_MAX < state->q->size) {
                    pthread_mutex_lock(mutex);
                    state->scrolledUp++;
                    pthread_mutex_unlock(mutex);
                }
            }
            if (c == 66) {
                // DOWN key
                if (state->scrolledUp > 0) {
                    pthread_mutex_lock(mutex);
                    state->scrolledUp--;
                    pthread_mutex_unlock(mutex);
                }
            }
            if (c == 67) {
                // RIGHT key
                if (cmdIndex < strlen(cmd) && state->curPos < strlen(cmd)) {
                    pthread_mutex_lock(mutex);
                    cmdIndex++;
                    state->curPos++;
                    pthread_mutex_unlock(mutex);
                }
            }
            if (c == 68) {
                // LEFT key
                if (cmdIndex > 0 && state->curPos > 0) {
                    pthread_mutex_lock(mutex);
                    cmdIndex--;
                    state->curPos--;
                    pthread_mutex_unlock(mutex);
                }
            }
        } else {
            pthread_mutex_lock(mutex);
            if (cmd[cmdIndex] != '\0') {
                // insert
                char c1 = c;
                char c2 = cmd[cmdIndex];
                for (int i = cmdIndex; i < CHAT_LEN - 1; i++) {
                    cmd[i] = c1;
                    if (c1 == '\0') break;
                    c1 = c2;
                    c2 = cmd[i + 1];
                }
            } else {
                cmd[cmdIndex] = c;
            }
            state->curPos++;
            cmdIndex++;
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
        recv(s, &recvItem, sizeof(ChatItem), 0);

        // chatのqueueにpushする
        pthread_mutex_lock(mutex);
        chatPushBack(q, recvItem.content, recvItem.senderName);
        pthread_mutex_unlock(mutex);
        display(_displayProps);
    }
    return 0;
}

int sendRcvState(void *arg) {
    SendRcvStateProps props = *((SendRcvStateProps *)arg);
    int s = props.s;
    State *state = props.state;
    for (;;) {
        CallState sendData = (CallState){.stopProgram = *props.stopProgram};
        if (*props.stopProgram) {
            system("/bin/stty cooked");
            send(s, &sendData, sizeof(CallState), 0);
            system("clear");
            sleep(1);
            pthread_cancel(*state->threads.recvChatThread);
            pthread_cancel(*state->threads.sendChatThread);
            pthread_cancel(*state->threads.callThread);
            pthread_cancel(*state->threads.stateThread);
        } else {
            send(s, props.stopProgram, sizeof(char), 0);
        }

        CallState recvData;
        recv(s, &recvData, sizeof(CallState), 0);
        if (recvData.stopProgram) {
            system("/bin/stty cooked");
            system("clear");
            // end program
            pthread_cancel(*state->threads.recvChatThread);
            pthread_cancel(*state->threads.sendChatThread);
            pthread_cancel(*state->threads.callThread);
            pthread_cancel(*state->threads.stateThread);
        }
    }
}
