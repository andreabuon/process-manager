all: mytop.o process.o util.o
	gcc -Wall -o mytop *.o `pkg-config --libs gtk4`

mytop.o: mytop.c
	gcc -Wall -c mytop.c `pkg-config --cflags gtk4`

process.o: process.c
	gcc -Wall -c process.c

util.o: util.c
	gcc -Wall -c util.c 

clean:
	rm mytop *.o

.phony: clean all