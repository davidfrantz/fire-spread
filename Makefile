GCC = gcc

SRCDIR=$HOME/src/fire-spread
BINDIR=$HOME/bin

LIB=-I$HOME/libfrantz -L$HOME/temp/lib

CFLAGS=-fopenmp -O3 -g 
#CFLAGS=-g -Wall

.PHONY: all install clean

all: fire-spread install clean

fire-spread: fire-spread.c
	$(GCC) $(LIB) $(CFLAGS) -o fire-spread fire-spread.c -lfrantz -lm

install:
	cp fire-spread $(BINDIR) ; chmod 755 $(BINDIR)/fire-spread

clean:
	rm -f fire-spread
