mytop: mytop.c
	gcc -o mytop -g mytop.c `pkg-config --cflags --libs gtk4` 

all: mytop

.PHONY: clean
clean:
	rm mytop