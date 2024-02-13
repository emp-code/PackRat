CC=gcc
CFLAGS=-D_FILE_OFFSET_BITS=64 -O2 -march=native -pipe -std=gnu2x -Wall -Wextra -Wpedantic -Wno-comment -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fcf-protection=full -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -Werror=incompatible-pointer-types -Werror=implicit-function-declaration -Werror=discarded-array-qualifiers -Werror=alloc-zero -Wbidi-chars=any -Wduplicated-branches -Wfloat-equal -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align -Wlogical-op -Wmissing-declarations -Winvalid-utf8 -Wpadded -Wredundant-decls -Wstrict-prototypes -Wunused-macros -Wwrite-strings -Wpointer-arith -Wstack-usage=999999 -Wtrampolines -fanalyzer

all: packrat packrat-analyzer packrat-v2reader

packrat: main.c packrat.c
	$(CC) $(CFLAGS) -o packrat main.c packrat.c

packrat-analyzer: Utils/Analyzer.c
	$(CC) $(CFLAGS) -o packrat-analyzer Utils/Analyzer.c

packrat-v2reader: Utils/v2reader.c
	$(CC) $(CFLAGS) -o packrat-v2reader Utils/v2reader.c Utils/packrat_v2_read.c

.PHONY: clean
clean:
	-rm packrat packrat-analyzer packrat-v2reader
