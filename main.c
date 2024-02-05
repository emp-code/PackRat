#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "packrat.h"

static int printHelp(const char * const cmd) {
	printf("\
Pack Rat archive tool, for Pack Rat v3 archives\n\n\
Mode:\n\
-c, --create  Create a new archive\n\
-h, --help    Show this help\n\
-r, --read    Read a file from an archive\n\
-u, --update  Update (replace) a file in an archive\n\
-w, --write   Write a file to an archive\n\
\n\
Global options:\n\
-d FILE, --data=FILE   Pack Rat Data file to use\n\
-i FILE, --index=FILE  Pack Rat Index file to use\n\
\n\
Create options:\n\
-l NUM, --lenbits=NUM  Bits to use for length (0 for Compact variant)\n\
-p NUM, --posbits=NUM  Bits to use for position\n\
\n\
Read options:\n\
-f FILE, --file=FILE  Output file path (standard output if omitted)\n\
-n NUM, --num=NUM     File number to read (starts from zero)\n\
\n\
Update options:\n\
-f FILE, --file=FILE  Input file path (standard input if omitted)\n\
-n NUM, --num=NUM     File number to update (starts from zero)\n\
\n\
Write options:\n\
-f FILE, --file=FILE  Input file path (standard input if omitted)\n\
\n\
Examples:\n\n\
Create a new Pack Rat Zero archive under the files 'example.prd' and 'example.pri', using 40 bits for the position and 24 bits for the length\n\
%s --create --data=example.prd --index=example.pri --posbits=40 --lenbits=24\n\
%s -c -d example.prd -i example.pri -p 40 -l 24\n\
\n\
Write the file 'test.jpg' to 'example.prd' and 'example.pri'\n\
%s --write --data=example.prd --index=example.pri --file=test.jpg\n\
%s -w -d example.prd -i example.pri -f test.jpg\n\
\n\
Read file number 42 from 'example.prd' and 'example.pri', writing to 'test.jpg'\n\
%s --read --data=example.prd --index=example.pri --num=42 --file=test.jpg\n\
%s -r -d example.prd -i example.pri -n 42 -f test.jpg\n\
\n\
Replace file number 25 with 'test.jpg' in 'example.prd' and 'example.pri'\n\
%s --update --data=example.prd --index=example.pri --num=25 --file=test.jpg\n\
%s -u -d example.prd -i example.pri -n 25 -f test.jpg\n\
", cmd, cmd, cmd, cmd, cmd, cmd, cmd, cmd);

	return 0;
}

static int readFile(const char * const path, unsigned char ** const data) {
	const int fd = open(path, O_RDONLY);
	if (fd < 0) return -1;

	const off_t lenData = lseek(fd, 0, SEEK_END);

	lseek(fd, 0, SEEK_SET);

	*data = malloc(lenData);
	const ssize_t ret = read(fd, *data, lenData);

	close(fd);
	return (ret == lenData) ? lenData : -2;
}

static int writeFile(const char * const path, const unsigned char * const data, const size_t lenData) {
	const int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0) return -1;

	if (write(fd, data, lenData) != (ssize_t)lenData) puts("Failed writing output file");

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

static void printError(const int e) {
	switch (e) {
		// General
		case PACKRAT_ERROR_ALLOC: puts("Error: Allocation error"); break;
		case PACKRAT_ERROR_PARAM: puts("Error: Incorrect parameters"); break;

		// Data
		case PACKRAT_ERROR_FORMAT: puts("Error: Invalid archive format"); break;
		case PACKRAT_ERROR_HEADER: puts("Error: Invalid archive header"); break;
		case PACKRAT_ERROR_MISMATCH: puts("Error: The PRD/PRI headers do not match"); break;
		case PACKRAT_ERROR_TOOBIG: puts("Error: Requested file is too large"); break;

		// I/O
		case PACKRAT_ERROR_LOCK: puts("Error: Failed to lock file"); break;
		case PACKRAT_ERROR_OPEN: puts("Error: Failed to open file"); break;
		case PACKRAT_ERROR_READ_PRD: puts("Error: Failed to read the PRD file"); break;
		case PACKRAT_ERROR_READ_PRI: puts("Error: Failed to read the PRI file"); break;
		case PACKRAT_ERROR_WRITE: puts("Error: Failed to write file"); break;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) return printHelp(argv[0]);

	char mode = 0;

	char *prd = NULL;
	char *pri = NULL;
	const char *path = "-";

	int bitsPos = -1;
	int bitsLen = -1;
	int fileNum = -1;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			printf("Invalid option '%s'. Use -h for help.\n", argv[i]);
			return 1;
		}

		const int n = (argv[i][1] == '-') ? 2 : 1;

		switch(argv[i][n]) {
			case 'c': if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--create") == 0) {mode = 'c';} break;
			case 'r': if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--read")   == 0) {mode = 'r';} break;
			case 'w': if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--write")  == 0) {mode = 'w';} break;
			case 'u': if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0) {mode = 'u';} break;

			case 'd': prd  = getArgStr(argv, n, "--data=",  &i); break;
			case 'i': pri  = getArgStr(argv, n, "--index=", &i); break;
			case 'f': path = getArgStr(argv, n, "--file=",  &i); break;

			case 'n': fileNum = getArgInt(argv, n, "--num=",     &i); break;
			case 'p': bitsPos = getArgInt(argv, n, "--posbits=", &i); break;
			case 'l': bitsLen = getArgInt(argv, n, "--lenbits=", &i); break;

			case 'h': return printHelp(argv[0]);

			default:
				printf("Invalid option '%s'. Use -h for help.\n", argv[i]);
				return 1;
		}
	}

	switch (mode) {
		case 'c': { // Create
			if (prd == NULL || pri == NULL || bitsPos < 1) {
				puts("Invalid options. Use -h for help.");
				return 0;
			}

			const int ret = packrat_create(pri, prd, bitsPos, bitsLen);
			if (ret != 0)
				printError(ret);
			else
				puts("Created OK");
		break;}

		case 'r': { // Read
			if (fileNum == -1) {
				puts("Please specify the file number");
				break;
			}

			unsigned char *buf;
			const int lenFile = packrat_read(pri, prd, fileNum, &buf);

			if (lenFile < 1) {
				printError(lenFile);
				return EXIT_FAILURE;
			}

			if (strcmp(path, "-") == 0)
				printf("%s\n", buf);
			else
				writeFile(path, buf, (size_t)lenFile);

			free(buf);
		break;}

		case 'w': { // Write
			unsigned char *data = NULL;
			int lenData;

			if (strcmp(path, "-") == 0) {
				puts("TODO: stdin");
				return 1;
			} else {
				lenData = readFile(path, &data);
				if (lenData < 1) {
					puts("Failed reading input file");
					if (data != NULL) free(data);
					return 1;
				}
			}

			const int ret = packrat_write(pri, prd, data, lenData);
			if (ret < 0) {
				printError(ret);
				return EXIT_FAILURE;
			}
//			puts("File added to archive successfully.");

			free(data);
		break;}

		case 'u': { // Update (replace)
			unsigned char *data = NULL;
			int lenData;

			if (strcmp(path, "-") == 0) {
				puts("TODO: stdin");
				return 1;
			} else {
				lenData = readFile(path, &data);
				if (lenData < 1) {
					puts("Failed reading input file");
					if (data != NULL) free(data);
					return 1;
				}
			}

			const int ret = 0;//packrat_update(pri, prd, fileNum, data, lenData);
			if (ret < 0) {
				printf("%d\n", ret);
			}

			free(data);
		break;}
	}

	return 0;
}
