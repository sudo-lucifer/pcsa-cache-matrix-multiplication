#
# Makefile that builds btest and other helper programs for the CS:APP data lab
# 
CC = gcc
CFLAGS = -pthread -O3 -march=native -ffast-math -fopenmp -Wall -m64

all: mm

mm: mm-mt.c mm-mt.h 
	$(CC) $(CFLAGS) -o mm mm-mt.c 

clean:
	rm -f *.o mm
