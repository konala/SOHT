make all: ht.o ht clean

ht.o: ht.c
	gcc -c ht.c -o ht.o

ht: ht.o
	gcc ht.o -o ht

clean:
	-rm -f ht.o


