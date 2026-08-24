# Makefile for Multithreaded Magic Square Validator

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread

all: mmsv

mmsv: mmsv.c
	$(CC) $(CFLAGS) -o mmsv mmsv.c

clean:
	rm -f mmsv
