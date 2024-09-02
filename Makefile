IDIR=./include
TARGET=bin/main

CC=gcc
CFLAGS=-I./include
LDFLAGS=

DEPS=$(wildcard $(IDIR)/*.h)
SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c, build/%.o, $(SRCS))


.PHONY: debug release clean

debug: CFLAGS += -g
debug: $(TARGET)

release: CFLAGS += -O2
release: $(TARGET)


build/%.o: src/%.c $(DEPS)
	mkdir -p build
	$(CC) -c -o $@ $< $(CFLAGS)


$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)


clean:
	rm -rf build bin

