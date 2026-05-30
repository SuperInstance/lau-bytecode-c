CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -g
INCLUDES = -Iinclude

.PHONY: all clean test

all: build lib/liblau_bytecode.a tests/test_basic

build:
	mkdir -p build

lib:
	mkdir -p lib

lib/liblau_bytecode.a: src/lau_bytecode.c include/lau_bytecode.h | build lib
	$(CC) $(CFLAGS) $(INCLUDES) -c src/lau_bytecode.c -o build/lau_bytecode.o
	ar rcs $@ build/lau_bytecode.o

tests/test_basic: tests/test_basic.c lib/liblau_bytecode.a | build
	$(CC) $(CFLAGS) $(INCLUDES) -Llib tests/test_basic.c -o tests/test_basic -llau_bytecode -lm

test: tests/test_basic
	./tests/test_basic

clean:
	rm -rf build lib tests/test_basic
