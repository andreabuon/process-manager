all: mytop

mytop: mytop.o process.o util.o list.o
	gcc -Wall -o mytop *.o `pkg-config --libs gtk4`

mytop.o: mytop.c
	gcc -Wall -c mytop.c `pkg-config --cflags gtk4`

process.o: process.c process.h
	gcc -Wall -c process.c

util.o: util.c util.h
	gcc -Wall -c util.c 

list.o: list.c list.h
	gcc -Wall -c list.c

clean:
	rm mytop *.o

.phony: clean all