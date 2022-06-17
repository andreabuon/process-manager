mytop: mytop.c aux.c
	gcc -o mytop mytop.c -g

all: mytop

clean:
	rm mytop

.phony: clean