GCC = gcc
G11=g++ -std=c++11

BINDIR=$(HOME)/bin

GDAL=-I/usr/include/gdal -L/usr/lib -Wl,-rpath=/usr/lib
LDGDAL=-lgdal

CFLAGS=-fopenmp -O3 
CFLAGS=-g -Wall -fopenmp 

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

write: src/write.c
	$(G11) $(CFLAGS) $(GDAL) -c src/write.c -o write.o $(LDGDAL)

queue: src/queue.c
	$(GCC) $(CFLAGS) -c src/queue.c -o queue.o

spiral: src/spiral.c
	$(GCC) $(CFLAGS) -c src/spiral.c -o spiral.o

string: src/string.c
	$(GCC) $(CFLAGS) -c src/string.c -o string.o

vutils: src/vutils.c
	$(GCC) $(CFLAGS) -c src/vutils.c -o vutils.o

warp: src/warp.cpp
	$(G11) $(CFLAGS) $(GDAL) -c src/warp.cpp -o warp.o $(LDGDAL)

fire-funs: src/fire-funs.c
	$(GCC) $(CFLAGS) -c src/fire-funs.c -o fire-funs.o

fire-phase1: src/fire-phase1.c
	$(GCC) $(CFLAGS) -c src/fire-phase1.c -o fire-phase1.o

fire-phase2: src/fire-phase2.c
	$(GCC) $(CFLAGS) -c src/fire-phase2.c -o fire-phase2.o

fire-spread: alloc angle date focalfuns write queue spiral string vutils warp fire-funs fire-phase1 fire-phase2 src/fire-spread.c
	$(G11) $(CFLAGS) $(GDAL) -o fire-spread src/fire-spread.c *.o -lm $(LDGDAL)

install:
	cp fire-spread $(BINDIR) ; chmod 755 $(BINDIR)/fire-spread

clean:
	rm -f fire-spread *.o
