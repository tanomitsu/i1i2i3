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
    complex double recvBeforeY[33][BUFSIZE];
    complex double sendX[BUFSIZE];
    complex double sendY[BUFSIZE];
    int cnt = 0;
    float rate = 0.5;
    // initialize
    for (int i = 0; i < BUFSIZE; i++) recvBeforeY[0][i] = 0.0;

    // double pastAmp[47];//BPF(50-2000)なのでindex(2-46)
    // for(int i=0;i<47;i++)pastAmp[i] = -1;

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
        remove_small_sound(sendY, BUFSIZE);
        band_pass_filter(sendY, BUFSIZE);
        
        double maxAmp=0;
        int maxHz=-1;
        // for (int i = 0; i < BUFSIZE; i++){
        //     if(cabs(sendY[i]) > maxAmp){
        //         maxAmp=cabs(sendY[i]);
        //         maxHz=i;
        //     }
        // }
        // if(maxAmp>10000){
        //     for (int i = 0; i < BUFSIZE; i++)sendY[i]=0;
        //     maxAmp=-1;
        // }
        // else{
        //     if(pastAmp[maxHz]<0)pastAmp[maxHz]=maxAmp;
        //     else{
        //         if(maxAmp>pastAmp[maxHz] && maxAmp>5000){
        //             for (int i = 0; i < BUFSIZE; i++)sendY[i]*=0.5;
        //             maxAmp*=0.5;
        //         }
        //         else pastAmp[maxHz]=maxAmp;
        //     }
        // }
        
        // for(int i=0;i<BUFSIZE; i++)printf("%f\n", cabs(sendY[i]));
        send(s, sendY, sendNum * sizeof(complex double), 0);

        // receive sound
        int recvNum = recv(s, &recvBeforeY[cnt], sizeof(complex double) * BUFSIZE, 0);
        for (int i=0; i<BUFSIZE; i++) {
            recvBeforeY[cnt][i] -= (recvBeforeY[(cnt+1)%33][i]*rate + recvBeforeY[(cnt+2)%33][i]*(1-rate));
        }
        // double rmax=0;
        // for (int i = 0; i < BUFSIZE; i++){if(cabs(recvBeforeY[i]) > rmax)rmax=cabs(recvBeforeY[i]);}

        ifft(recvBeforeY, recvBeforeX, BUFSIZE);
        complex_to_sample(recvBeforeX, recvBuf, BUFSIZE);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);
    }

    pclose(soundIn);
    pclose(soundOut);
    
    return 0;
}