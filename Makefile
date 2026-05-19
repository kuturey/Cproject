CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Isrc/include
SRC=$(wildcard src/core/*.c) $(wildcard src/commands/*.c) src/main.c

ifeq ($(OS), Windows_NT)
    PLATFORM = windows
    BIN = bin/minigit.exe
    RM = del /Q /F
    RMDIR = rmdir /S /Q
    MKDIR = if not exist bin mkdir bin
else
    PLATFORM = macos/linux
    BIN = bin/minigit
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p bin
endif

all: $(BIN)
	@echo "Built for platform: $(PLATFORM)"

$(BIN): $(SRC)
	$(MKDIR)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	$(RMDIR) bin

BINDIR ?= $(HOME)/.local/bin

install: all
ifeq ($(PLATFORM), windows)
	@echo "Manual install: copy $(BIN) to a folder in your PATH"
else
	mkdir -p $(BINDIR)
	cp $(BIN) $(BINDIR)/minigit
	chmod +x $(BINDIR)/minigit
endif

uninstall:
ifeq ($(PLATFORM), windows)
	@echo "Manual uninstall: delete minigit.exe from your PATH folder"
else
	$(RM) $(BINDIR)/minigit
endif
