#include <stdio.h>
#include <stdlib.h>
#include <util.h>

void die(char* s) {
    perror(s);
    exit(1);
}