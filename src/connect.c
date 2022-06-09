#include "connect.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fft.h"

int serverConnect(int port) {
    // Prepare
    int ss = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(ss, (struct sockaddr *)&addr, sizeof(addr));

    // listen and accept incoming connections
    fprintf(stderr, "waiting for connections...\n");
    listen(ss, 10);
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss, (struct sockaddr *)&client_addr, &len);
    close(ss);
    return s;
}

int clientConnect(char *ip, int port) {
    // connect & set up
    int s = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;  // IPv4のIPアドレスを使うことを明示
    inet_aton(ip, &addr.sin_addr);  // IPアドレスの指定
    if (addr.sin_addr.s_addr == 0) {
        // 不正なIPアドレス
        perror("Invalid IP");
        exit(EXIT_FAILURE);
    }
    addr.sin_port = htons(port);  // ポートの指定
    fprintf(stderr, "Successfully selected the IP address.\n");

    fprintf(stderr, "start connecting to IP address...\n");
    int ret = connect(s, (struct sockaddr *)&addr, sizeof(addr));  // connect
    if (ret == -1) {
        perror("connection");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Successfully connected to the IP adress.\n");
    return s;
}

int autoConnect(char *ip, int port, ConnectMode connectMode) {
    if (connectMode == SERVER) {
        int s = serverConnect(port);
        return s;
    }
    if (connectMode == CLIENT) {
        int s = clientConnect(ip, port);
        return s;
    }
    return -1;
}

int callConnect(void *arg) {
    CallConnectProps props = *((CallConnectProps *)arg);
    char *ip = props.ip;
    int port = props.port;
    int *s = props.s;
    ConnectMode connectMode = props.connectMode;
    *s = autoConnect(ip, port, connectMode);
    return 0;
}

int chatConnect(void *arg) {
    ChatConnectProps props = *((ChatConnectProps *)arg);
    char *ip = props.ip;
    int port = props.port;
    int *s = props.s;
    ConnectMode connectMode = props.connectMode;
    *s = autoConnect(ip, port, connectMode);
    return 0;
}
