#!/usr/bin/bash

mkdir -p build
clang src/tetris.c -o build/tetris -g -ggdb3 -O0 -std=c23 -Wall -Wextra -I./include -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
# clang src/tetris.c -o build/tetris -O3 -std=c23 -Wall -Wextra -I./include -L./lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

