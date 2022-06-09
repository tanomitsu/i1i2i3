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

int main(int argc, char **argv) {
    // connect
    int s = -1;
    const int callPort = 55554;
    if (argc == 1) {
        // server side
        // ./discorb.out
        s = serverConnect(callPort);
    } else if (argc == 2) {
        // client side
        // ./a.out <IP>
        char *ip = argv[1];
        s = clientConnect(ip, callPort);
    } else {
        fprintf(stderr, "usage: %s <ip> or %s \n", argv[0], argv[0]);
        exit(1);
    }
    call(s);
    close(s);
}
