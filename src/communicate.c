#include <arpa/inet.h>
#include <communicate.h>
#include <connect.h>
#include <fft.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int call(int s) {
    // parameters
    //const double ratio = 0;

    // start recording
    FILE *soundIn = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    FILE *soundOut = popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");

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

    double pastAmp[47];//BPF(50-2000)なのでindex(2-46)
    for(int i=0;i<47;i++)pastAmp[i] = -1;
    int cnt = 0;
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
        for (int i = 0; i < BUFSIZE; i++){if(cabs(sendY[i])<100)sendY[i]=0;}
        for(int i = 0; i < BUFSIZE; i++) {
                double f = i / (double)BUFSIZE * 44100;
                if (f < 50 || f > 2000) sendY[i] = 0;
        }
        //for (int i = 0; i < BUFSIZE; i++) sendY[i] -= ratio * recvBeforeY[i];
        /*
       else if(maxAmp>pastAmp[howling_step]){
            for (int i = 0; i < BUFSIZE; i++)sendY[i]=sendY[i]/3.0;
            pastAmp[howling_step]=maxAmp/3.0;
        }
        */
        double maxAmp=0;
        int maxHz=-1;
        for (int i = 0; i < BUFSIZE; i++){
            if(cabs(sendY[i]) > maxAmp){
                maxAmp=cabs(sendY[i]);
                maxHz=i;
            }
        }
        if(maxAmp>10000){
            for (int i = 0; i < BUFSIZE; i++)sendY[i]=0;
            maxAmp=-1;
        }
        else{
            if(pastAmp[maxHz]<0)pastAmp[maxHz]=maxAmp;
            else{
                if(maxAmp>pastAmp[maxHz] && maxAmp>5000){
                    for (int i = 0; i < BUFSIZE; i++)sendY[i]*=0.5;
                    maxAmp*=0.5;
                }
                else pastAmp[maxHz]=maxAmp;
            }
        }
        // send data
        printf("%f %d\n", maxAmp, maxHz);
        //for(int i=0;i<BUFSIZE; i++)printf("%f\n", cabs(sendY[i]));
        send(s, sendY, sendNum * sizeof(complex double), 0);

        // receive sound
        int recvNum = recv(s, recvBeforeY, sizeof(complex double) * BUFSIZE, 0);

        // write(1, recvBuf, recvNum);
        double rmax=0;
        for (int i = 0; i < BUFSIZE; i++){if(cabs(recvBeforeY[i]) > rmax)rmax=cabs(recvBeforeY[i]);}
        ifft(recvBeforeY, recvBeforeX, BUFSIZE);
        complex_to_sample(recvBeforeX, recvBuf, BUFSIZE);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);
    }
    // fprintf(stderr, "Ended connection.\n");
    pclose(soundIn);
    pclose(soundOut);
    return 0;
}