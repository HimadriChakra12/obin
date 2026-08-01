CC      = clang
CFLAGS  = -Wall -Wextra -Os -flto
PREFIX  ?= /usr/local
BD  = $(PREFIX)/bin
BIN     = sping

.PHONY: all build install uninstall clean

all: build

build:
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN)

install: build
	strip $(BIN)
	install -Dm755 $(BIN) $(BD)/$(BIN)
	setcap cap_net_raw+ep $(BD)/$(BIN)

uninstall:
	rm -f $(BD)/$(BIN)

clean:
	rm -f $(BIN)
