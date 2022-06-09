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
    const double ratio = 0;

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

    int howling_step=0;
    double pastAmp[31];
    for(int i=0;i<31;i++)pastAmp[i] = -1;
    FILE* fp=fopen("recv1.txt", "w");
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
        //for (int i = 0; i < BUFSIZE; i++) sendY[i] -= ratio * recvBeforeY[i];

        double maxAmp=0;
        int maxHz=-1;
        for (int i = 0; i < BUFSIZE; i++){
            if(cabs(sendY[i]) > maxAmp){
                maxAmp=cabs(sendY[i]);
                if(maxAmp>2500)maxHz=i;
            }
        }
        //printf("%f\n", maxAmp);
        fprintf(fp, "%d\n", maxHz);
        printf("%d\n", maxHz);
        if(pastAmp[howling_step]<0){
            pastAmp[howling_step]=maxAmp;
        }
       else if(maxAmp>pastAmp[howling_step]){
            for (int i = 0; i < BUFSIZE; i++)sendY[i]=sendY[i]/3.0;
            pastAmp[howling_step]=maxAmp/3.0;
        }
        else pastAmp[howling_step]=maxAmp;
        //printf("%f\n",pastAmp[howling_step]);
        howling_step=(howling_step+1)%31;
        double limit=5000;
        if(maxAmp>limit){
            double ratioamp=limit/maxAmp;
            //for (int i = 0; i < BUFSIZE; i++) sendY[i] = sendY[i]*ratioamp;
            //for (int i = 0; i < BUFSIZE; i++)sendY[i]=0;
            
        }
        for (int i = 0; i < BUFSIZE; i++){if(cabs(sendY[i])<100)sendY[i]=0;}
        for (int i = 0; i < BUFSIZE; i++) {
            double f = i / (double)BUFSIZE * 44100;
            if (f < 50 || f > 2000) sendY[i] = 0;
        }

        // send data
        send(s, sendY, sendNum * sizeof(complex double), 0);

        // receive sound
        int recvNum = recv(s, recvBeforeY, sizeof(complex double) * BUFSIZE, 0);

        // write(1, recvBuf, recvNum);
        double rmax=0;
        for (int i = 0; i < BUFSIZE; i++){if(cabs(recvBeforeY[i]) > rmax)rmax=cabs(recvBeforeY[i]);}
        //printf("%f\n",rmax);
        ifft(recvBeforeY, recvBeforeX, BUFSIZE);
        complex_to_sample(recvBeforeX, recvBuf, BUFSIZE);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);
    }
    // fprintf(stderr, "Ended connection.\n");
    pclose(soundIn);
    pclose(soundOut);
    fclose(fp);
    return 0;
}