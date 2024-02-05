// Analyzer for Pack Rat v3 archives

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>

// Pack Rat v3 - File signature 'PR'
#define PACKRAT_HEADER_SIG 0x5250
#define PACKRAT_OFFSET_BITSPOS 16
#define PACKRAT_OFFSET_BITSLEN 1
struct packrat_header {
	uint64_t pr: 16;
	uint64_t id: 32;

	uint64_t bitsPos: 5; // 0-31 -> 16-47. 64 KiB - 128 TiB for total data
	uint64_t bitsLen: 5; // 0-31 -> 1-32. 4 B - 4 GiB per file (Zero variant only)
	uint64_t unused: 5;
	uint64_t allowMod: 1; // If 0, the library refuses to modify existing data.
};

static int div_floor(const int a, const int b) {
	return (a - (a % b)) / b;
}

//static int div_ceil(const int a, const int b) {
//	return (a % b == 0) ? a / b : div_floor(a, b) + 1;
//}

static uint64_t extractPruint(unsigned char * const src, const int startBit, const int pruintBits) {
	union {
		uint64_t u64;
		unsigned char c[8];
	} r;
	r.u64 = 0;

	for (int i = 0; i < pruintBits; i++) {
		const int t = startBit + i;
		const int tRemainder = t % 8;
		const int iRemainder = i % 8;

		if ((src[(t - tRemainder) / 8] & (1 << (7 - tRemainder))) != 0) {
			r.c[(i - iRemainder) / 8] |= 1 << iRemainder;
		}
	}

	return r.u64;
}

static char humanReadableSize(double * const humanSize) {
	char humanUnit = 'B';
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'K';}
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'M';}
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'G';}
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'T';}
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'P';}
	if (*humanSize > 1023) {*humanSize /= 1024; humanUnit = 'E';}

	return humanUnit;
}

int main(int argc, char *argv[]) {
	puts("Pack Rat Analyzer, for Pack Rat v3 archives (file signature 'PR').");

	if (argc < 2) {
		printf(
			"Usage:\n"
			"Short analysis (index only): %s index.pri\n"
			"Full analysis (index and data): %s index.pri data.prd\n"
		, argv[0], argv[0]);

		return 0;
	}

	puts("\n== PRI (Index): Header");

	int pri = open(argv[1], O_RDONLY);
	if (pri == -1) {
		perror("Failed to open PRI file");
		return 1;
	}

	struct packrat_header header;
	ssize_t bytesRead = read(pri, (unsigned char*)&header, sizeof(struct packrat_header));
	if (bytesRead == -1) {
		perror("Failed reading PRI file");
	} else if (bytesRead != sizeof(struct packrat_header)) {
		puts("Invalid PRI file format.");
		return 1;
	}

	if (header.pr != PACKRAT_HEADER_SIG) {
		puts("Invalid file signature.");
		return 1;
	}

	const int bitsPos = header.bitsPos + PACKRAT_OFFSET_BITSPOS;
	const int bitsLen = (header.bitsLen == 0) ? 0 : header.bitsLen + PACKRAT_OFFSET_BITSLEN;

	if (bitsLen == 0) {
		puts("Variant: Compact");
		printf("Bits used for position: %d\n", bitsPos);
	} else {
		puts("Variant: Zero");
		printf("Bits used for position: %d\n", bitsPos);
		printf("Bits used for length: %d\n", bitsLen);
		printf("Total bits per entry: %d\n", bitsPos + bitsLen);
	}

	if (header.bitsPos == 0) {
		puts("Invalid bitsPos.");
		return 1;
	}

	puts("\n== PRI (Index): Entries");

	off_t indexSize = lseek(pri, 0, SEEK_END);
	if (indexSize == -1) {
		perror("Failed lseek()");
	}
	indexSize -= sizeof(struct packrat_header);

	const int entryCount = div_floor(indexSize * 8, bitsPos + bitsLen);
	printf("Number of entries: %d\n", entryCount);

	if (entryCount == 0) {
		puts("Empty archive.");
		return 0;
	}

//	int phCount = 0;
//	uint64_t expectedSize = 0;

	unsigned char * const raw = malloc(indexSize);
	if (raw == NULL) {
		perror("Failed malloc()");
		return 1;
	}

	if (pread(pri, raw, indexSize, sizeof(struct packrat_header)) != (ssize_t)indexSize) {
		puts("Failed reading PRI file.");
		free(raw);
		return 1;
	}

	if (bitsLen == 0) {
		int posNow = 0;

		for (int i = 0; i < entryCount - 1; i++) {
			const uint64_t posNext = extractPruint(raw, (i + 1) * bitsPos, bitsPos);

			double humanSize = posNext - posNow;
			const char humanUnit = humanReadableSize(&humanSize);
			printf("#%.4d: %3.d %c | %d-%lu\n", i, (int)humanSize, humanUnit, posNow, posNext);

			posNow = posNext;
		}

		printf("#%.4d:       | %d-EOF\n", entryCount - 1, posNow);
	} else {
		for (int i = 0; i < entryCount; i++) {
			const int sz = extractPruint(raw, i * (bitsLen + bitsPos), bitsLen);
			const int pos = extractPruint(raw, i * (bitsLen + bitsPos) + bitsLen, bitsPos);

			double humanSize = sz;
			const char humanUnit = humanReadableSize(&humanSize);

			printf("#%.4d: %3.d %c | %d-%d\n", i, (int)humanSize, humanUnit, pos, pos + sz);
		}
	}

	free(raw);
	close(pri);
	puts("");

	if (argc < 3) {
		puts("PRD (Data) analysis not requested.");
	} else {
		puts("TODO: PRD (Data) analysis.");
	}

	puts("Analysis complete.");
	return 0;
}
