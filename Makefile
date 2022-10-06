GCC = gcc

BINDIR=$(HOME)/bin

GDAL=-I/usr/include/gdal -L/usr/lib -Wl,-rpath=/usr/lib
LDGDAL=-lgdal

CFLAGS=-fopenmp -O3 
#CFLAGS=-g -Wall -fopenmp 

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

string: src/string.c
	$(GCC) $(CFLAGS) -c src/string.c -o string.o

vutils: src/vutils.c
	$(GCC) $(CFLAGS) -c src/vutils.c -o vutils.o

fire-spread: alloc angle date focalfuns queue string vutils src/fire-spread.c
	$(GCC) $(CFLAGS) $(GDAL) -o fire-spread src/fire-spread.c *.o -lm $(LDGDAL)

install:
	cp fire-spread $(BINDIR) ; chmod 755 $(BINDIR)/fire-spread

clean:
	rm -f fire-spread *.o
