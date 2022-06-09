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
    complex double recvX[BUFSIZE];
    complex double recvY[BUFSIZE];
    complex double sendX[BUFSIZE];
    complex double sendY[BUFSIZE];
    int cycle=32;
    complex double pastsend[cycle][BUFSIZE];
    int cnt = 0;
    float rate = 0.5;
    // initialize
    for (int i = 0; i < BUFSIZE; i++) recvY[i] = 0.0;
    for(int j=0;j<cycle;j++){for (int i=0; i<BUFSIZE; i++)pastsend[j][i] = 0;}

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
        for (int i = 0; i < BUFSIZE; i++){
            if(cabs(sendY[i]) > maxAmp){
                maxAmp=cabs(sendY[i]);
                maxHz=i;
            }
        }
        //printf("%d %f\n", maxHz, maxAmp);
        if(maxAmp>5000){
            //for (int i = 0; i < BUFSIZE; i++)sendY[i]=0;
            //maxAmp=-1;
        }
        double pastProduct=0;
        for (int i = 0; i < BUFSIZE; i++)pastProduct+=creal(pastsend[cnt][i])*creal(pastsend[cnt][i])+cimag(pastsend[cnt][i])*cimag(pastsend[cnt][i]);
        double Product=0;//32 cycle mae
        for (int i = 0; i < BUFSIZE; i++)Product+=creal(sendY[i])*creal(pastsend[cnt][i])+cimag(sendY[i])*cimag(pastsend[cnt][i]);
        double ProductPast=0;// 31 cycle mae
        for (int i = 0; i < BUFSIZE; i++)ProductPast+=creal(sendY[i])*creal(pastsend[(cnt+1)%cycle][i])+cimag(sendY[i])*cimag(pastsend[(cnt+1)%cycle][i]);
        if(pastProduct>0.1){
            double r=Product/pastProduct;
            double rNext=ProductPast/pastProduct;
            if(r>rNext && r>0.5){
                for (int i = 0; i < BUFSIZE; i++){
                    sendY[i]-=pastsend[cnt][i]*r;
                    pastsend[cnt][i]=sendY[i];
                }
            }
            else if(rNext>r &&rNext>0.5){
                for (int i = 0; i < BUFSIZE; i++){
                    sendY[i]-=pastsend[(cnt+1)%cycle][i]*rNext;
                    pastsend[(cnt+1)%cycle][i]=sendY[i];
                }
                cnt=(cnt+1)%cycle;
            }
            //printf("%f\n", r);
        }
        for(int i = 0; i < BUFSIZE; i++)printf("%f\n", cabs(sendY[i]));
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
        cnt=(cnt+1)%cycle;
        // for(int i=0;i<BUFSIZE; i++)printf("%f\n", cabs(sendY[i]));
        send(s, sendY, sendNum * sizeof(complex double), 0);

        // receive sound
        int recvNum = recv(s, recvY, sizeof(complex double) * BUFSIZE, 0);
        for (int i=0; i<BUFSIZE; i++) {
            //recvBeforeY[cnt][i] -= (recvBeforeY[(cnt+1)%33][i]*rate + recvBeforeY[(cnt+2)%33][i]*(1-rate));
        }
        ifft(recvY, recvX, BUFSIZE);
        complex_to_sample(recvX, recvBuf, BUFSIZE);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);
    }

    pclose(soundIn);
    pclose(soundOut);
    
    return 0;
}