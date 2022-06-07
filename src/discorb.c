/*
    使い方:
        サーバー側: ./discorb.out <port>
        クライアント: ./discorb.out <ip> <port>
*/

#include <arpa/inet.h>
#include <fft.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 1024

int serverConnect(int port);
int clientConnect(char *ip, int port);
int call(int s);

int main(int argc, char **argv) {
    // connect
    int s = -1;
    if (argc == 2) {
        // server side
        // ./a.out <port>
        int port = atoi(argv[1]);
        s = serverConnect(port);
    } else if (argc == 3) {
        // client side
        // ./a.out <IP> <port>
        char *ip = argv[1];
        int port = atoi(argv[2]);
        s = clientConnect(ip, port);
    } else {
        fprintf(stderr, "usage: %s <ip> <port> or %s <port>\n", argv[0],
                argv[0]);
        exit(1);
    }
    call(s);
    close(s);
}
