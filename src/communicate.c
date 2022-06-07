#include <communicate.h>
#include <complex.h>
#include <stdio.h>

int call(int s) {
    // start recording
    FILE *fp = popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");

    // after the client has connected
    fprintf(stderr, "A client has connected to your port.\n");
    fprintf(stderr, "Input what you want to send.\n");
    short sendBuf[BUFSIZE];
    short recvBuf[BUFSIZE];

    for (;;) {
        // send sound
        int sendNum = fread(sendBuf, sizeof(short), BUFSIZE, fp);
        if (sendNum == 0) break;
        if (sendNum < 0) {
            perror("send");
            return 1;
        }

        complex double *X = calloc(sizeof(complex double), BUFSIZE);
        complex double *Y = calloc(sizeof(complex double), BUFSIZE);
        /* 複素数の配列に変換 */
        sample_to_complex(sendBuf, X, BUFSIZE);
        /* FFT -> Y */
        fft(X, Y, BUFSIZE);  // X=t-axis

        ifft(Y, X, BUFSIZE);
        /* 標本の配列に変換 */
        complex_to_sample(X, sendBuf, BUFSIZE);

        send(s, sendBuf, sendNum * sizeof(short), 0);

        // receive sound
        int recvNum = recv(s, recvBuf, sizeof(short) * BUFSIZE, 0);
        write(1, recvBuf, recvNum);
    }
    // fprintf(stderr, "Ended connection.\n");
    pclose(fp);
    return 0;
}