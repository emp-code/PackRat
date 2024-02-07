#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "packrat_v2_read.h"

// packrat-v2reader: Tool to extract individual files from Pack Rat v2 archives (pre-2024, file signature 'Pr')

static int writeFile(const char * const path, const char * const data, const size_t lenData) {
	const int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd < 0) return -1;

	write(fd, data, lenData);

	close(fd);
	return 0;
}

static int getArgInt(char * const argv[], const int n, const char * const longname, int * const i) {
	if (n == 1) { // short form
		(*i)++;
		return strtol(argv[*i], NULL, 10);
	} else { // long form
		const size_t l = strlen(longname);
		return (strncmp(argv[*i], longname, l) == 0) ? strtol(argv[*i] + l, NULL, 10) : -1;
	}
}

static char *getArgStr(char * const argv[], const int n, const char * const longname, int * const i) {
	if (n == 1) { // short form
		(*i)++;
		return argv[*i];
	} else { // long form
		const size_t l = strlen(longname);
		if (strncmp(argv[*i], longname, l) != 0) return NULL;
		return argv[*i] + l;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s --data=example.prd --index=example.pri --num=0 --file=out.put\n", argv[0]);
		return 1;
	}

	char *path = NULL;
	char *prd = NULL;
	char *pri = NULL;
	int fileNum = -1;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			printf("Usage: %s --data=example.prd --index=example.pri --num=0 --file=out.put\n", argv[0]);
			return 1;
		}

		const int n = (argv[i][1] == '-') ? 2 : 1;

		switch(argv[i][n]) {
			case 'f': path  = getArgStr(argv, n, "--file=",  &i); break;
			case 'd': prd  = getArgStr(argv, n, "--data=",  &i); break;
			case 'i': pri  = getArgStr(argv, n, "--index=", &i); break;
			case 'n': fileNum = getArgInt(argv, n, "--num=", &i); break;

			default:
				printf("Usage: %s --data=example.prd --index=example.pri --num=0 --file=out.put\n", argv[0]);
				return 1;
		}
	}

	char *buf;
	const int lenFile = packrat_v2_read(pri, prd, fileNum, &buf);

	if (lenFile < 1) {
		switch (lenFile) {
			case PACKRAT_ERROR_MISC: puts("Error: Miscellaneous error"); break;
			case PACKRAT_ERROR_ALLOC: puts("Error: Allocating memory failed (out of memory?)"); break;
			case PACKRAT_ERROR_FILESIG: puts("Error: File signature does not match (not a Pack Rat archive?)"); break;
			case PACKRAT_ERROR_CORRUPT: puts("Error: Index file corrupted"); break;
			case PACKRAT_ERROR_OPEN: puts("Error: Failed to open file"); break;
			case PACKRAT_ERROR_READWRITE: puts("Error: Failed to read file"); break;
			case PACKRAT_ERROR_EMPTY: puts("Error: The requested file is empty"); break;
			case PACKRAT_ERROR_ID: puts("Error: The requested file does not exist"); break;
		}

		return EXIT_FAILURE;
	}

	if (strcmp(path, "-") == 0) {
		write(STDOUT_FILENO, buf, lenFile);
	} else {
		writeFile(path, buf, lenFile);
		printf("Extracted file #%d to %s\n", fileNum, path);
	}

	free(buf);
	return 0;
}
