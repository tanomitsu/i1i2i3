#include <stdio.h>
#include <stdlib.h>
#include <util.h>

void die(char* s) {
    perror(s);
    exit(1);
}

int minInt(int a, int b) { return a < b ? a : b; }