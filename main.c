#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "packrat.h"

static int printHelp(const char * const cmd) {
	printf("\
Pack Rat archive tool, for Pack Rat v3 archives\n\n\
Mode:\n\
-h, --help    Show this help\n\
-c, --create  Create a new archive\n\
-a, --add     Add a file to the archive\n\
-g, --get     Get a file from the archive\n\
\n\
Global options:\n\
-d FILE, --data=FILE   Data file (.prd) of the archive\n\
-i FILE, --index=FILE  Index file (.pri) of the archive\n\
\n\
Create-mode options:\n\
-l NUM, --lenbits=NUM  Bits to use for length (0 for Compact variant)\n\
-p NUM, --posbits=NUM  Bits to use for position\n\
\n\
Add-mode options:\n\
-f FILE, --file=FILE  Input file path (standard input if omitted, or '-')\n\
\n\
Get-mode options:\n\
-f FILE, --file=FILE  Output file path (standard output if omitted, or '-')\n\
-n NUM,  --num=NUM    File number to extract (starts from zero)\n\
\n\
Examples:\n\n\
Create a new Pack Rat Zero archive with the paths 'example.prd' and 'example.pri', using 40 bits for the position and 24 bits for the length\n\
%s --create --data=example.prd --index=example.pri --posbits=40 --lenbits=24\n\
%s -c -d example.prd -i example.pri -p 40 -l 24\n\
\n\
Add the file 'test.jpg' to the archive 'example.prd' and 'example.pri'\n\
%s --add --data=example.prd --index=example.pri --file=test.jpg\n\
%s -a -d example.prd -i example.pri -f test.jpg\n\
\n\
Get file number 42 from 'example.prd' and 'example.pri' and write it to 'test.jpg'\n\
%s --get --data=example.prd --index=example.pri --num=42 --file=test.jpg\n\
%s -g -d example.prd -i example.pri -n 42 -f test.jpg\n\
\n\
", cmd, cmd, cmd, cmd, cmd, cmd);

	return 0;
}

static int readFile(const char * const path, unsigned char ** const data) {
	const int fd = open(path, O_RDONLY);
	if (fd < 0) return -1;

	const off_t lenData = lseek(fd, 0, SEEK_END);
	if (lenData == 0) return 0;

	*data = malloc(lenData);
	if (*data == NULL) return -1;

	const ssize_t ret = pread(fd, *data, lenData, 0);
	if (ret != lenData) {
		close(fd);
		free(*data);
		return -1;
	}

	close(fd);
	return lenData;
}

static int writeFile(const char * const path, const unsigned char * const data, const size_t lenData) {
	if (strcmp(path, "-") == 0) {
		return (write(STDOUT_FILENO, data, lenData) == (ssize_t)lenData) ? 0 : -1;
	}

	const int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if (fd < 0) return -1;

	const ssize_t r = write(fd, data, lenData);

	close(fd);
	return (r == (ssize_t)lenData) ? 0 : -1;
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

	int mode = 0;

	char *prd = NULL;
	char *pri = NULL;
	const char *path = "-";

	int fileNum = -1;
	int bitsPos = -1;
	int bitsLen = -1;

	for(;;) {
		static struct option longOpts[] = {
			{"help",    no_argument,       NULL, 'h'},
			{"create",  no_argument,       NULL, 'c'},
			{"add",     no_argument,       NULL, 'a'},
			{"get",     no_argument,       NULL, 'g'},
			{"data",    required_argument, NULL, 'd'},
			{"index",   required_argument, NULL, 'i'},
			{"file",    required_argument, NULL, 'f'},
			{"num",     required_argument, NULL, 'n'},
			{"lenbits", required_argument, NULL, 'l'},
			{"posbits", required_argument, NULL, 'p'},
			{0, 0, 0, 0}
		};

		const int c = getopt_long(argc, argv, "hcagd:i:f:n:l:p:", longOpts, NULL);
		if (c == -1) break;

		switch (c) {
			case 'c': mode = 'c'; break;
			case 'a': mode = 'a'; break;
			case 'g': mode = 'g'; break;

			case 'd': prd = optarg; break;
			case 'i': pri = optarg; break;
			case 'f': path = optarg; break;

			case 'n': fileNum = strtol(optarg, NULL, 10); break;
			case 'p': bitsPos = strtol(optarg, NULL, 10); break;
			case 'l': bitsLen = strtol(optarg, NULL, 10); break;

			default: return printHelp(argv[0]);
		}
	}

	switch (mode) {
		case 'c': { // Create
			const int ret = packrat_create(pri, prd, bitsPos, bitsLen);
			if (ret != 0)
				printError(ret);
			else
				puts("Created OK");
		break;}

		case 'a': { // Add
			unsigned char *data = NULL;
			int lenData = -1;

			if (strcmp(path, "-") == 0) {
				// Standard input
				const int r = ioctl(STDIN_FILENO, FIONREAD, &lenData);
				if (r != 0 || lenData < 1) {
					puts("Failed reading standard input");
					return 1;
				}

				data = malloc(lenData);
				if (data == NULL) return 1;

				lenData = read(STDIN_FILENO, data, lenData);
			} else {
				// File input
				lenData = readFile(path, &data);
				if (lenData < 0) {
					puts("Failed reading input file");
					return 1;
				}
			}

			const int ret = packrat_add(pri, prd, data, lenData);
			if (lenData > 0) free(data);

			if (ret < 0) {
				printError(ret);
				return EXIT_FAILURE;
			}

			if (lenData == 0) puts("Placeholder added successfully."); else puts("File added to archive successfully.");
		break;}

		case 'g': { // Get
			if (fileNum == -1) {
				puts("Please specify the file number");
				break;
			}

			unsigned char *buf;
			const int lenFile = packrat_get(pri, prd, fileNum, &buf);

			if (lenFile < 1) {
				printError(lenFile);
				return EXIT_FAILURE;
			}

			writeFile(path, buf, (size_t)lenFile);
			free(buf);
		break;}
	}

	return 0;
}
