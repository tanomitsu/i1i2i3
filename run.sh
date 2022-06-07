#!/bin/sh

make clean
make

if [ $# -eq 0 ]; then
    # server side
    ./bin/discorb.out
elif [ $# -eq 1 ]; then
    #client side
    ./bin/discorb.out $1
else
    echo "usage: ./run.sh   or   ./run.sh <ip>" 1>&2
    exit 1
fi
