make all: ht.o ht

ht.o: ht.c
	gcc -c ht.c -o ht.o

ht: ht.o
	gcc ht.o -o ht


