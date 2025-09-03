# SudoVDA Tools Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = 

TARGETS = sudovda-ctl

all: $(TARGETS)

sudovda-ctl: sudovda-ctl.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

install: all
	install -m 755 sudovda-ctl /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/sudovda-ctl

.PHONY: all clean install uninstall
