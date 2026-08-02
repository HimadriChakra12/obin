CC      = clang
CFLAGS  = -Wall -Wextra -Wpedantic -O2 -flto
PREFIX ?= /usr/local
BINDIR  = $(PREFIX)/bin
BIN     = sping

.PHONY: all install uninstall clean

all: $(BIN)

$(BIN): $(BIN).c
	$(CC) $(CFLAGS) $< -o $@
	strip $@

install: $(BIN)
	install -Dm755 $(BIN) $(BINDIR)/$(BIN)
	setcap cap_net_raw+ep $(BINDIR)/$(BIN)

uninstall:
	rm -f $(BINDIR)/$(BIN)

clean:
	rm -f $(BIN)
