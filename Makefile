CC = clang
CFLAGS = -std=c23 -Wall -Wextra -g -ggdb3 -O0 -I./include
LDFLAGS = -L./lib
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

TARGET = tetris
SRCS = tetris.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

