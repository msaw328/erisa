#!/bin/bash

# utility script for running a binary
# example: ./quickrun.sh erisa-exec

if [ -n "$1" ]; then
    LD_LIBRARY_PATH='./build/liberisa' ./build/$1/$1 $2
fi
