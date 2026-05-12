CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Isrc/include
SRC=$(wildcard src/core/*.c) $(wildcard src/commands/*.c) src/main.c
BIN=bin/minigit

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

BINDIR ?= $(HOME)/.local/bin

install: all
	mkdir -p $(BINDIR)
	cp bin/minigit $(BINDIR)/minigit
	chmod +x $(BINDIR)/minigit

uninstall:
	rm -f $(BINDIR)/minigit

clean:
	rm -rf bin
