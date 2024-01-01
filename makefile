# Makefile for c-markdown parser
CC = gcc
GCC = -ansi -Wpedantic -fstack-protector-all -Wall -pedantic-errors -W -g

mmzs.o: mmzs.c mmzs.h
	$(CC) $(GCC) mmzs.c -o mmzs.o

test: 
	mmzs.o infile.txt >> out.html
