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
    // const double ratio = 0;

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
    int cycle = 32;
    complex double pastsend[cycle][BUFSIZE];
    int cnt = 0;
    float rate = 0.5;
    // initialize
    for (int i = 0; i < BUFSIZE; i++) recvY[i] = 0.0;
    for (int j = 0; j < cycle; j++) {
        for (int i = 0; i < BUFSIZE; i++) pastsend[j][i] = 0;
    }

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

        double pastProduct = 0;
        for (int i = 0; i < BUFSIZE; i++)
            pastProduct += creal(pastsend[cnt][i]) * creal(pastsend[cnt][i]) +
                           cimag(pastsend[cnt][i]) * cimag(pastsend[cnt][i]);
        double pastProduct31 = 0;
        for (int i = 0; i < BUFSIZE; i++)
            pastProduct31 += creal(pastsend[(cnt + 1) % cycle][i]) *
                                 creal(pastsend[(cnt + 1) % cycle][i]) +
                             cimag(pastsend[(cnt + 1) % cycle][i]) *
                                 cimag(pastsend[(cnt + 1) % cycle][i]);
        double Product = 0;  // 32 cycle mae
        for (int i = 0; i < BUFSIZE; i++)
            Product += creal(sendY[i]) * creal(pastsend[cnt][i]) +
                       cimag(sendY[i]) * cimag(pastsend[cnt][i]);
        double Product31 = 0;  // 31 cycle mae
        for (int i = 0; i < BUFSIZE; i++)
            Product31 +=
                creal(sendY[i]) * creal(pastsend[(cnt + 1) % cycle][i]) +
                cimag(sendY[i]) * cimag(pastsend[(cnt + 1) % cycle][i]);

        double r = 0;
        if (pastProduct > 0.1) r = Product / pastProduct;
        double r31 = 0;
        if (pastProduct31 > 0.1) r31 = Product31 / pastProduct31;
        if (abs(r) >= abs(r31) && abs(r) > 0.1) {
            for (int i = 0; i < BUFSIZE; i++){
                double complex diff=pastsend[cnt][i] * r;
                pastsend[cnt][i] = sendY[i];
                sendY[i] -= diff;
            }
        } else if (abs(r31) > abs(r) && abs(r31) > 0.1) {
            cnt = (cnt + 1) % cycle;
            for (int i = 0; i < BUFSIZE; i++){
                double complex diff=pastsend[cnt][i] * r31;
                pastsend[cnt][i] = sendY[i];
                sendY[i] -= diff;
            }
        }
        for(int i=0;i<30; i++)printf("%f\n", cabs(pastsend[cnt][i]));
        cnt = (cnt + 1) % cycle;
        for(int i=0;i<30; i++)printf("%f\n", cabs(sendY[i]));

        // double maxAmp = 0;
        // int maxHz = -1;
        // for (int i = 0; i < BUFSIZE; i++) {
        //     if (cabs(sendY[i]) > maxAmp) {
        //         maxAmp = cabs(sendY[i]);
        //         maxHz = i;
        //     }
        // }
        // printf("%d %f\n", maxHz, maxAmp);
        
        send(s, sendY, sendNum * sizeof(complex double), 0);

        // receive sound
        int recvNum = recv(s, recvY, sizeof(complex double) * BUFSIZE, 0);
        for (int i = 0; i < BUFSIZE; i++) {
            // recvBeforeY[cnt][i] -= (recvBeforeY[(cnt+1)%33][i]*rate +
            // recvBeforeY[(cnt+2)%33][i]*(1-rate));
        }
        ifft(recvY, recvX, BUFSIZE);
        complex_to_sample(recvX, recvBuf, BUFSIZE);
        fwrite(recvBuf, sizeof(short), BUFSIZE, soundOut);
    }

    pclose(soundIn);
    pclose(soundOut);

    return 0;
}