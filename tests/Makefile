# Makefile for Linux using GCC
# 
# For compiling with micorosft MSVC, use make.bat

CC = gcc
OPTFLAG = -Ofast
CFLAGS = -Wall $(OPTFLAG)
OBJS = crc_timing.o pentominos.o 3d-pentomino.o perftest.o
OUT = perftest

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) -o $(OUT) $(OBJS)

perftest.o: perftest.c perftest.h Makefile
	$(CC) $(CFLAGS) -DOPTFLAG=\"$(OPTFLAG)\" -c perftest.c

crc_timing.o: crc_timing.c perftest.h Makefile
	$(CC) $(CFLAGS) -c crc_timing.c

pentominos.o: pentominos.c perftest.h Makefile
	$(CC) $(CFLAGS) -DTEST_MODULE=1 -c pentominos.c

3d-pentomino.o: 3d-pentomino.c perftest.h Makefile
	$(CC) $(CFLAGS) -DTEST_MODULE=1 -c 3d-pentomino.c

clean:
	rm -f *.o $(OUT)
