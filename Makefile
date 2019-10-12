CC=gcc
CFLAGS=-g -fsanitize=undefined -march=native -pipe -Wall -Werror=array-bounds -Werror=format-overflow=0 -Werror=format -Werror=implicit-function-declaration -Werror=address -Werror=implicit-int -Werror=incompatible-pointer-types -Wno-comment -Wno-switch -Wno-unused-variable
#CFLAGS=-O3 -march=native -pipe -Wall -Werror=array-bounds -Werror=format-overflow=0 -Werror=format -Werror=implicit-function-declaration -Werror=implicit-int -Werror=incompatible-pointer-types -Wno-comment -Wno-switch -Wno-unused-variable -lm

objects = main.o packrat.o

packrat: $(objects)
	$(CC) $(CFLAGS) -o packrat $(objects)

main: main.c packrat.h
packrat.o: packrat.c

.PHONY: clean
clean:
	-rm $(objects)
