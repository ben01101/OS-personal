SRCS = wc.c map.c main.c
TGTS = wc map main
CFLAGS = -ggdb -Wall -std=c99

.PHONY: compile, clean

default: compile

compile: $(TGTS)

wc: wc.c
	gcc -g wc.c -o wc

map: map.c
	gcc -g map.c -o map

main: main.c
	gcc -g main.c -o main

clean: 
	rm -f $(TGTS)