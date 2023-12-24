# Makefile for c-markdown parser
CC = gcc
GCC = -ansi -Wpedantic -fstack-protector-all -Wall -pedantic-errors -W -g

main.o: main.c
	$(CC) $(GCC) main.c -o main.o
