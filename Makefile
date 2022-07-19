FLAGS=-Wall -g -DDEBUG

all: mytop

mytop: mytop.o process.o handlers.o util.o
	gcc $(FLAGS) -o mytop *.o `pkg-config --libs gtk4`

mytop.o: mytop.c mytop.h
	gcc $(FLAGS) -c mytop.c `pkg-config --cflags gtk4`

handlers.o: handlers.c handlers.h
	gcc $(FLAGS) -c handlers.c

process.o: process.c process.h
	gcc $(FLAGS) -c process.c

util.o: util.c util.h
	gcc $(FLAGS) -c util.c 

clean:
	rm mytop *.o

.phony: clean all