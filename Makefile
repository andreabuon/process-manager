FLAGS=-Wall -g -DDEBUG

all: mytop

mytop: mytop.o process.o handlers.o util.o list.o
	gcc $(FLAGS) -o mytop *.o `pkg-config --libs gtk4`

mytop.o: mytop.c
	gcc $(FLAGS) -c mytop.c `pkg-config --cflags gtk4`

handlers.o: handlers.c handlers.h
	gcc $(FLAGS) -c handlers.c

process.o: process.c process.h
	gcc $(FLAGS) -c process.c

util.o: util.c util.h
	gcc $(FLAGS) -c util.c 

list.o: list.c list.h
	gcc $(FLAGS) -c list.c

clean:
	rm mytop *.o

.phony: clean all