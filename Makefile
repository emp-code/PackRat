CC=gcc
CFLAGS=-g -O1 -march=native -pipe -Wall -Wextra -Werror -Wno-comment -D_FORTIFY_SOURCE=2 -fsanitize=undefined -fstack-protector-strong -fcf-protection=full -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Wno-error=unused-result -Werror=implicit-fallthrough=0

objects = main.o packrat.o

packrat: $(objects)
	$(CC) $(CFLAGS) -o packrat $(objects)

main: main.c packrat.h
packrat.o: packrat.c

.PHONY: clean
clean:
	-rm $(objects)
