#
# Makefile that builds btest and other helper programs for the CS:APP data lab
# 
CC = gcc
CFLAGS = -O2 -Wall -m64

all: mm

mm: mm.c mm.h 
	$(CC) $(CFLAGS) -o mm mm.c 

clean:
	rm -f *.o mm
