#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "packrat.h"

// Pack Rat v3 - File signature 'PR'
#define PACKRAT_OFFSET_BITSPOS 16
#define PACKRAT_OFFSET_BITSLEN 1

#define PACKRAT_HEADER_SIG 0x5250
#define PACKRAT_HEADER_LEN 8
union packrat_header {
	uint64_t u64;
	unsigned char chr[8];

	struct {
		uint64_t pr: 16;
		uint64_t id: 32;

		uint64_t bitsPos: 5; // 0-31 -> 16-47. 64 KiB - 128 TiB for total data
		uint64_t bitsLen: 5; // 0-31 -> 1-32. 4 B - 4 GiB per file (Zero variant only)
		uint64_t unused: 5;
		uint64_t allowMod: 1; // If 0, the library refuses to modify existing data.
	} fields;
};

static int div_floor(const int a, const int b) {
	return (a - (a % b)) / b;
}

static int div_ceil(const int a, const int b) {
	return (a % b == 0) ? a / b : div_floor(a, b) + 1;
}

// pruint: Pack Rat Unsigned Integer: 1-48 bits, little endian bit and byte order
static void appendPruint(unsigned char * const target, const int bitsTarget, const uint64_t new, const int bitsNew) {
	for (int i = 0; i < bitsNew; i++) {
		const int t = bitsTarget + i;
		const int tRemainder = t % 8;
		const int iRemainder = i % 8;

		if (*((const unsigned char * const)&new + (i - iRemainder) / 8) & (1 << iRemainder)) target[(t - tRemainder) / 8] |= 1 << (7 - tRemainder);
	}
}

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

static bool headersMatch(const union packrat_header pri_hdr, const int fd) {
	union packrat_header prd_hdr;
	if (read(fd, prd_hdr.chr, PACKRAT_HEADER_LEN) != PACKRAT_HEADER_LEN) return false;

	if ((pri_hdr.fields.id & 1) != 1 && (prd_hdr.fields.id & 1) != 0) return false;
	prd_hdr.fields.id |= 1;
	return (pri_hdr.u64 == prd_hdr.u64) ? true : false;
}

static int pr_get_zero(const int pri, const int prd, const unsigned long fileNum, const int bitsLen, const int bitsPos, unsigned char ** const data) {
	const int64_t priPos = PACKRAT_HEADER_LEN + div_floor((bitsLen + bitsPos) * fileNum, 8); // Byte number to start reading from
	const long skipBits = ((bitsLen + bitsPos) * fileNum) % 8; // Number of irrelevant bits in the first byte, to be skipped
	const long priBytes = div_ceil(skipBits + bitsLen + bitsPos, 8);

	unsigned char raw[priBytes];
	ssize_t r = pread(pri, raw, priBytes, priPos);
	if (r < 0) return PACKRAT_ERROR_READ_PRI;

	const uint64_t readLen = extractPruint(raw, skipBits, bitsLen);
	const uint64_t readPos = extractPruint(raw, skipBits + bitsLen, bitsPos);

	if (readLen > (1UL << bitsPos)) return PACKRAT_ERROR_TOOBIG;

	*data = malloc(readLen);
	if (*data == NULL) return PACKRAT_ERROR_ALLOC;

	r = pread(prd, *data, readLen, PACKRAT_HEADER_LEN + readPos);
	if (r != (ssize_t)readLen) {
		free(*data);
		*data = NULL;
		return PACKRAT_ERROR_READ_PRD;
	}

	return readLen;
}

static int pr_get_compact(const int pri, const int prd, const unsigned long fileNum, const int bitsPos, unsigned char ** const data) {
	const int64_t priPos = PACKRAT_HEADER_LEN + div_floor(bitsPos * fileNum, 8); // Byte number to start reading from
	const long skipBits = (bitsPos * fileNum) % 8; // Number of irrelevant bits in the first byte, to be skipped
	const long priBytes = div_ceil(skipBits + (bitsPos * 2), 8);

	unsigned char raw[priBytes];
	ssize_t r = pread(pri, raw, priBytes, priPos);
	if (r < 0) return PACKRAT_ERROR_READ_PRI;

	const uint64_t readPos = extractPruint(raw, skipBits, bitsPos);
	size_t readLen = 0;

	if (r == priBytes) {
		const uint64_t readEnd = extractPruint(raw, skipBits + bitsPos, bitsPos);
		if (readEnd < readPos) return PACKRAT_ERROR_FORMAT;
		readLen = readEnd - readPos;
	} else if (r == div_ceil(skipBits + bitsPos, 8)) {
		const off_t prdSize = lseek(prd, 0, SEEK_END);
		if (prdSize < 0) return PACKRAT_ERROR_READ_PRI;
		readLen = (prdSize - PACKRAT_HEADER_LEN) - readPos;
	} else {
		return PACKRAT_ERROR_READ_PRI;
	}

	if (readLen > (1UL << bitsPos)) return PACKRAT_ERROR_TOOBIG;

	*data = malloc(readLen);
	if (*data == NULL) return PACKRAT_ERROR_ALLOC;

	r = pread(prd, *data, readLen, PACKRAT_HEADER_LEN + readPos);
	if (r != (ssize_t)readLen) {
		free(*data);
		*data = NULL;
		return PACKRAT_ERROR_READ_PRD;
	}

	return readLen;
}

int packrat_get(const char * const pathPri, const char * const pathPrd, const unsigned long fileNum, unsigned char ** const data) {
	if (pathPri == NULL || pathPrd == NULL || data == NULL) return -1;

	const int pri = open(pathPri, O_RDONLY | O_NOCTTY);
	if (pri < 0) return PACKRAT_ERROR_OPEN;
	if (flock(pri, LOCK_EX | LOCK_NB) != 0) {close(pri); return PACKRAT_ERROR_LOCK;}

	const int prd = open(pathPrd, O_RDONLY | O_NOCTTY);
	if (prd < 0) return PACKRAT_ERROR_OPEN;
	if (flock(prd, LOCK_EX | LOCK_NB) != 0) {close(pri); close(prd); return PACKRAT_ERROR_LOCK;}

	union packrat_header header;
	if (read(pri, header.chr, PACKRAT_HEADER_LEN) != PACKRAT_HEADER_LEN) {close(prd); close(pri); return PACKRAT_ERROR_READ_PRI;}
	if (!headersMatch(header, prd)) return PACKRAT_ERROR_MISMATCH;

	const int ret = (header.fields.bitsLen == 0) ? pr_get_compact(pri, prd, fileNum, header.fields.bitsPos + PACKRAT_OFFSET_BITSPOS, data) : pr_get_zero(pri, prd, fileNum, header.fields.bitsLen + PACKRAT_OFFSET_BITSLEN, header.fields.bitsPos + PACKRAT_OFFSET_BITSPOS, data);

	flock(pri, LOCK_UN);
	close(pri);
	flock(prd, LOCK_UN);
	close(prd);

	return ret;
}

static int pr_add_zero(const int pri, const int prd, int bitsLen, int bitsPos, const unsigned char * const data, const int lenData) {
	if (lenData >= (1L << bitsLen)) return PACKRAT_ERROR_TOOBIG;

	const off_t priSize = lseek(pri, 0, SEEK_END);
	if (priSize < PACKRAT_HEADER_LEN) return PACKRAT_ERROR_READ_PRI;

	const int entries = div_floor((priSize - PACKRAT_HEADER_LEN) * 8, bitsLen + bitsPos);
	const int usedBits = ((bitsLen + bitsPos) * entries) % 8; // Number of currently-used bits in the last byte

	const int lenTbw = div_ceil(usedBits + bitsLen + bitsPos, 8);
	unsigned char tbw[lenTbw];
	bzero(tbw, lenTbw);

	if (usedBits > 0 && pread(pri, tbw, 1, priSize - 1) != 1) return PACKRAT_ERROR_READ_PRI;

	// Zero variant stores the length and position of each file
	const uint64_t raw1 = lenData;
	appendPruint(tbw, usedBits, raw1, bitsLen);

	if (lenData > 0) {
		// Not a placeholder
		const off_t prdSize = lseek(prd, 0, SEEK_END);
		if (prdSize < PACKRAT_HEADER_LEN) return PACKRAT_ERROR_READ_PRD;
		if (prdSize - PACKRAT_HEADER_LEN + lenData >= (1L << bitsPos)) return PACKRAT_ERROR_TOOBIG;

		const uint64_t raw2 = prdSize - PACKRAT_HEADER_LEN;
		appendPruint(tbw, usedBits + bitsLen, raw2, bitsPos);
		if (write(prd, data, lenData) != (ssize_t)lenData) return PACKRAT_ERROR_WRITE;
	}

	return (pwrite(pri, (unsigned char*)&tbw, lenTbw, (usedBits > 0) ? priSize - 1 : priSize) == lenTbw) ? PACKRAT_OK : PACKRAT_ERROR_WRITE;
}

static int pr_add_compact(const int pri, const int prd, int bitsPos, const unsigned char * const data, const int lenData) {
	if (lenData < 1) return PACKRAT_ERROR_PARAM;
	if (bitsPos > 47) return PACKRAT_ERROR_HEADER;

	const off_t prdSize = lseek(prd, 0, SEEK_END);
	if (prdSize < PACKRAT_HEADER_LEN) return PACKRAT_ERROR_READ_PRD;
	if (prdSize - PACKRAT_HEADER_LEN + lenData >= (1L << bitsPos)) return PACKRAT_ERROR_TOOBIG;

	const off_t priSize = lseek(pri, 0, SEEK_END);
	if (priSize < PACKRAT_HEADER_LEN) return PACKRAT_ERROR_READ_PRI;

	const int entries = div_floor((priSize - PACKRAT_HEADER_LEN) * 8, bitsPos);
	const int usedBits = (bitsPos * entries) % 8; // Number of currently-used bits in the last byte

	const int lenTbw = div_ceil(usedBits + bitsPos, 8);
	unsigned char tbw[lenTbw];
	bzero(tbw, lenTbw);

	if (usedBits > 0 && pread(pri, tbw, 1, priSize - 1) != 1) return PACKRAT_ERROR_READ_PRI;

	// Compact variant stores the position (starting byte) of each file
	const uint64_t raw = prdSize - PACKRAT_HEADER_LEN;
	appendPruint(tbw, usedBits, raw, bitsPos);

	if (pwrite(pri, (unsigned char*)&tbw, lenTbw, (usedBits > 0) ? priSize - 1 : priSize) != lenTbw) return PACKRAT_ERROR_WRITE;
	if (write(prd, data, lenData) != (ssize_t)lenData) return PACKRAT_ERROR_WRITE;

	return PACKRAT_OK;
}

int packrat_add(const char * const pathPri, const char * const pathPrd, const unsigned char * const data, const int lenData) {
	if (pathPri == NULL || pathPrd == NULL || lenData < 0) return PACKRAT_ERROR_PARAM;

	const int pri = open(pathPri, O_RDWR | O_NOCTTY);
	if (pri < 0) return PACKRAT_ERROR_OPEN;
	if (flock(pri, LOCK_EX | LOCK_NB) != 0) {close(pri); return PACKRAT_ERROR_LOCK;}

	const int prd = open(pathPrd, ((lenData > 0) ? O_RDWR : O_RDONLY) | O_NOCTTY | O_APPEND);
	if (prd < 0) return PACKRAT_ERROR_OPEN;
	if (flock(prd, LOCK_EX | LOCK_NB) != 0) {close(pri); close(prd); return PACKRAT_ERROR_LOCK;}

	union packrat_header header;
	if (read(pri, header.chr, PACKRAT_HEADER_LEN) != PACKRAT_HEADER_LEN) {close(prd); close(pri); return PACKRAT_ERROR_READ_PRI;}
	if (!headersMatch(header, prd)) return PACKRAT_ERROR_MISMATCH;

	const int ret = (header.fields.bitsLen == 0) ? pr_add_compact(pri, prd, header.fields.bitsPos + PACKRAT_OFFSET_BITSPOS, data, lenData) : pr_add_zero(pri, prd, header.fields.bitsLen + PACKRAT_OFFSET_BITSLEN, header.fields.bitsPos + PACKRAT_OFFSET_BITSPOS, data, lenData);

	flock(pri, LOCK_UN);
	close(pri);
	flock(prd, LOCK_UN);
	close(prd);

	return ret;
}

int packrat_create(const char * const pathPri, const char * const pathPrd, const int bitsPos, const int bitsLen) {
	if (pathPri == NULL || pathPrd == NULL || bitsPos < 16 || bitsPos > 47 || bitsLen == 1 || bitsLen > 32 || bitsPos < bitsLen) return PACKRAT_ERROR_PARAM;

	const int prd = open(pathPrd, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (prd < 0) return PACKRAT_ERROR_OPEN;
	if (flock(prd, LOCK_EX | LOCK_NB) != 0) {close(prd); return PACKRAT_ERROR_LOCK;}

	const int pri = open(pathPri, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (pri < 0) return PACKRAT_ERROR_OPEN;
	if (flock(pri, LOCK_EX | LOCK_NB) != 0) {close(pri); return PACKRAT_ERROR_LOCK;}

	union packrat_header header;
	header.fields.pr = PACKRAT_HEADER_SIG;
	header.fields.id = arc4random() | 1; // PRI: id & 1 == 1
	header.fields.bitsPos = bitsPos - PACKRAT_OFFSET_BITSPOS;
	header.fields.bitsLen = (bitsLen == 0) ? 0 : bitsLen - PACKRAT_OFFSET_BITSLEN;
	header.fields.unused = 0;
	header.fields.allowMod = 1;

	bool ok = false;
	if (write(pri, header.chr, PACKRAT_HEADER_LEN) == PACKRAT_HEADER_LEN) {
		header.fields.id--; // PRD: id & 1 == 0

		if (write(prd, header.chr, PACKRAT_HEADER_LEN) == PACKRAT_HEADER_LEN) {
			ok = true;
		}
	}

	flock(pri, LOCK_UN);
	close(pri);
	flock(prd, LOCK_UN);
	close(prd);

	return ok? 0 : -1;
}
