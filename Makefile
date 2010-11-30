CC=gcc
CFLAGS=-O3 -Wall -Wextra -pedantic -ansi -msse3 -ffast-math -freciprocal-math `pkg-config --cflags xcb`
LIBS=-lm `pkg-config --libs xcb`

all: gk2010

gk2010: *.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

.PHONY: clean
clean:
	rm gk2010
