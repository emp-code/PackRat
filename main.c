#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "packrat.h"


static int printHelp() {
	printf("\
Pack Rat Archival System by EMPcode\n\n\
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
-l NUM, --lenbits=NUM  Bits to use for length (ignored if Compact type)\n\
-p NUM, --posbits=NUM  Bits to use for position\n\
-t NUM, --type=TYPE    Type of archive to make; '0' for Zero, 'C' for Compact\n\
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
packrat --create --data=example.prd --index=example.pri --posbits=40 --lenbits=24 --type=0\n\
packrat -c -d example.prd -i example.pri -p 40 -l 24 -t 0\n\
\n\
Write the file 'test.jpg' to 'example.prd' and 'example.pri'\n\
packrat --write --data=example.prd --index=example.pri --file=test.jpg\n\
packrat -w -d example.prd -i example.pri -f test.jpg\n\
\n\
Read file number 42 from 'example.prd' and 'example.pri', writing to 'test.jpg'\n\
packrat --read --data=example.prd --index=example.pri --num=42 --file=test.jpg\n\
packrat -r -d example.prd -i example.pri -n 42 -f test.jpg\n\
\n\
Replace file number 25 with 'test.jpg' in 'example.prd' and 'example.pri'\n\
packrat --update --data=example.prd --index=example.pri --num=25 --file=test.jpg\n\
packrat -u -d example.prd -i example.pri -n 25 -f test.jpg\n\
");

	return 0;
}

static int readFile(const char * const path, char ** const data) {
	const int fd = open(path, O_RDONLY);
	if (fd < 0) return -1;

	const off_t lenData = lseek(fd, 0, SEEK_END);

	lseek(fd, 0, SEEK_SET);

	*data = malloc(lenData);
	const ssize_t ret = read(fd, *data, lenData);

	close(fd);
	return (ret == lenData) ? lenData : -2;
}

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

static char getArgChar(char * const argv[], const int n, const char * const longname, int * const i) {
	if (n == 1) { // short form
		(*i)++;
		if (argv[*i][1] != 0x00) return 0x00;
		return argv[*i][0];
	} else { // long form
		const size_t l = strlen(longname);
		if (strncmp(argv[*i], longname, l) != 0) return 0x00;
		return argv[*i][l];
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) return printHelp();

	char mode = 0;
	char type = 0;

	char *prd = NULL;
	char *pri = NULL;
	char *path = "-";

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

			case 't': type = getArgChar(argv, n, "--type=", &i); break;

			case 'n': fileNum = getArgInt(argv, n, "--num=",     &i); break;
			case 'p': bitsPos = getArgInt(argv, n, "--posbits=", &i); break;
			case 'l': bitsLen = getArgInt(argv, n, "--lenbits=", &i); break;

			case 'h': return printHelp();

			default:
				printf("Invalid option '%s'. Use -h for help.\n", argv[i]);
				return 1;
		}
	}

	switch(mode) {
		case 'c': { // Create
			if (prd == NULL || pri == NULL || type == 0 || bitsPos < 1 || (type == '0' && bitsLen < 0)) {
				puts("Invalid options. Use -h for help.");
				return 0;
			}

			if (type != 'C' && type != '0') {printf("Invalid type '%c'\n", type); return 1;}
			packrat_create(pri, prd, bitsPos, bitsLen, type);
		break;}

		case 'r': { // Read
			char *buf;
			const int lenFile = packrat_read(pri, prd, fileNum, &buf);

			if (lenFile < 1) {
				return 1;
			}

			if (strcmp(path, "-") == 0)
				printf("%s\n", buf);
			else
				writeFile(path, buf, lenFile);

			free(buf);
		break;}

		case 'w': { // Write
			char *data = NULL;
			int lenData;

			if (strcmp(path, "-") == 0) {
				puts("TODO: stdin");
				return 1;
			} else {
				lenData = readFile(path, &data);
				if (lenData < 1) {
					if (data != NULL) free(data);
					return 1;
				}
			}

			const int ret = packrat_write(pri, prd, data, lenData);
			if (ret < 0) {
				printf("%d\n", ret);
			}

			free(data);
		break;}

		case 'u': { // Update (replace)
			char *data = NULL;
			int lenData;

			if (strcmp(path, "-") == 0) {
				puts("TODO: stdin");
				return 1;
			} else {
				lenData = readFile(path, &data);
				if (lenData < 1) {
					if (data != NULL) free(data);
					return 1;
				}
			}

			const int ret = packrat_update(pri, prd, fileNum, data, lenData);
			if (ret < 0) {
				printf("%d\n", ret);
			}

			free(data);
		break;}
	}

	return 0;
}
