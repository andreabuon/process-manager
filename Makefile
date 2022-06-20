mytop: mytop.c aux.c
	gcc -g -o mytop mytop.c `pkg-config --cflags --libs gtk4`

all: mytop

clean:
	rm mytop

.phony: clean