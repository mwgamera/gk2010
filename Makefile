CC=gcc
CFLAGS=-O4 -Wall -Wextra -pedantic -ansi -msse3 `pkg-config --cflags xcb`
LIBS=-lm `pkg-config --libs xcb`

all: gk2010

gk2010: *.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

.PHONY: clean
clean:
	rm gk2010
