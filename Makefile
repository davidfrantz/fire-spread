GCC = gcc

BINDIR=$(HOME)/bin

CFLAGS=-fopenmp -O3 
#CFLAGS=-g -Wall

.PHONY: all install clean

all: fire-spread install clean

alloc: src/alloc.c
	$(GCC) $(CFLAGS) -c src/alloc.c -o alloc.o

angle: src/angle.c
	$(GCC) $(CFLAGS) -c src/angle.c -o angle.o

date: src/date.c
	$(GCC) $(CFLAGS) -c src/date.c -o date.o

focalfuns: src/focalfuns.c
	$(GCC) $(CFLAGS) -c src/focalfuns.c -o focalfuns.o

queue: src/queue.c
	$(GCC) $(CFLAGS) -c src/queue.c -o queue.o

vutils: src/vutils.c
	$(GCC) $(CFLAGS) -c src/vutils.c -o vutils.o

fire-spread: alloc angle date focalfuns queue vutils src/fire-spread.c
	$(GCC) $(CFLAGS) -o fire-spread src/fire-spread.c *.o -lm

install:
	cp fire-spread $(BINDIR) ; chmod 755 $(BINDIR)/fire-spread

clean:
	rm -f fire-spread *.o
