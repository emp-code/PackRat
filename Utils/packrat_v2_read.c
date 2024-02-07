#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "packrat_v2_read.h"

// Reader for Pack Rat v2 archives (pre-2024, file signature 'Pr')

static const uint64_t pow2[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216,33554432,67108864,134217728,268435456,536870912,1073741824,2147483648,4294967296,8589934592,17179869184,34359738368,68719476736,137438953472,274877906944,549755813888,1099511627776,2199023255552,4398046511104,8796093022208,17592186044416,35184372088832,70368744177664,140737488355328,281474976710656,562949953421312,1125899906842624,2251799813685248,4503599627370496,9007199254740992,18014398509481984,36028797018963968,72057594037927936,144115188075855872,288230376151711744,576460752303423488,1152921504606846976,2305843009213693952,4611686018427387904,9223372036854775808ULL};

#define bytesInBits_UP 1
__attribute__((warn_unused_result, const))
static int bytesInBits(const int bits, const int dir) {
	const int modulus = bits % 8;
	return ((bits - modulus) / 8) + ((dir == bytesInBits_UP && modulus > 0) ? 1 : 0);
}

static uint64_t pruint_fetch(const char * const source, const int skipBits, const int bits) {
	int bit = skipBits % 8;
	int byte = (skipBits - bit) / 8;
	uint64_t result = 0;

	for (int i = 0; i < bits; i++) {
		if (1 & (source[byte] >> (7 - bit))) result += pow2[i];

		bit++;
		if (bit > 7) {
			byte++;
			bit = 0;
		}
	}

	return result;
}

static uint64_t getPos(const int pri, const int infoBytes, const int id, const int bitsPos) {
	char info[infoBytes];
	const ssize_t bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	return (bytesRead != infoBytes) ? UINT64_MAX : pruint_fetch(info, 0, bitsPos);
}

// Read: Pack Rat Compact (PrC)
static int packrat_v2_read_compact(const int pri, const int bitsPos, const char *pathPrd, const int id, char ** const data) {
	if (pri < 0) return PACKRAT_ERROR_MISC;

	const off_t endPri = lseek(pri, 0, SEEK_END);
	const int infoBytes = bytesInBits(bitsPos, bytesInBits_UP);

	uint64_t len;
	int prd;

	const uint64_t pos = getPos(pri, infoBytes, id, bitsPos);

	if (pos == UINT64_MAX) {close(pri); return PACKRAT_ERROR_MISC;}
	if (data == NULL) {close(pri); return PACKRAT_ERROR_NODATA;}

	// Pack Rat Data: File contents
	if ((5 + (id + 1) * infoBytes) >= endPri) {
		// Request is for the last file stored
		close(pri);

		prd = open(pathPrd, O_RDONLY);
		if (prd == -1) return PACKRAT_ERROR_OPEN;

		// Length is eof - start
		const uint64_t endPrd = lseek(prd, 0, SEEK_END);
		len = endPrd - pos;
	} else {
		const uint64_t end = getPos(pri, infoBytes, id + 1, bitsPos);
		len = end - pos;
		close(pri);

		if (len < 1) return PACKRAT_ERROR_EMPTY;

		prd = open(pathPrd, O_RDONLY);
		if (prd == -1) return PACKRAT_ERROR_OPEN;
	}

	*data = malloc(len + 1);
	if (*data == NULL) return PACKRAT_ERROR_ALLOC;

	const ssize_t bytesRead = pread(prd, *data, len, pos);
	close(prd);

	if (bytesRead != (ssize_t)len) {
		free(*data);
		return PACKRAT_ERROR_READWRITE;
	}

	return len;
}

// Read: Pack Rat Zero (Pr0)
static int packrat_v2_read_zero(const int pri, const int bitsPos, const int bitsLen, const char * const pathPrd, const int id, char ** const data) {
	if (pri < 0) return PACKRAT_ERROR_MISC;

	const int infoBytes = bytesInBits(bitsPos + bitsLen, bytesInBits_UP);
	const int readPos = 5 + (id * infoBytes);

	// Pack Rat Index: Error checking
	const off_t priSize = lseek(pri, 0, SEEK_END);
	if (priSize < 6) return PACKRAT_ERROR_CORRUPT;
	if (readPos + infoBytes > priSize) return PACKRAT_ERROR_ID;

	// Pack Rat Index: Position and Length
	char info[infoBytes];
	ssize_t bytesRead = pread(pri, info, infoBytes, readPos);
	if (bytesRead != infoBytes) {
		close(pri);
		return PACKRAT_ERROR_READWRITE;
	}

	const uint64_t pos = pruint_fetch(info, 0,       bitsPos);
	const uint64_t len = pruint_fetch(info, bitsPos, bitsLen);
	close(pri);

	if (len == 0) return PACKRAT_ERROR_EMPTY;
	if (data == NULL) return PACKRAT_ERROR_NODATA;

	// Pack Rat Data: File contents
	const int prd = open(pathPrd, O_RDONLY);
	if (prd == -1) return PACKRAT_ERROR_OPEN;

	const off_t prdSize = lseek(prd, 0, SEEK_END);
	if (prdSize < 1) {
		close(prd);
		return PACKRAT_ERROR_READWRITE;
	} else if (pos + len > (unsigned long)prdSize) {
		close(prd);
		return PACKRAT_ERROR_CORRUPT;
	}

	*data = malloc(len + 1);
	if (*data == NULL) return PACKRAT_ERROR_ALLOC;

	bytesRead = pread(prd, *data, len, pos);
	close(prd);

	if (bytesRead != (ssize_t)len) {
		free(*data);
		return PACKRAT_ERROR_READWRITE;
	}

	return len;
}

// Returns length of file on success, or a negative error code on failure
int packrat_v2_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data) {
	if (pathPri == NULL || pathPrd == NULL || id < 0) return -1;

	const int pri = open(pathPri, O_RDONLY);
	if (pri == -1) return PACKRAT_ERROR_OPEN;

	char header[5];
	const ssize_t bytesRead = read(pri, header, 5);
	if (bytesRead != 5) return PACKRAT_ERROR_READWRITE;

	if (header[0] != 'P' || header[1] != 'r') return PACKRAT_ERROR_FILESIG;

	const int bitsPos = header[3];
	const int bitsLen = header[4];

	int ret = -1;
	if (header[2] == '0') ret = packrat_v2_read_zero(pri, bitsPos, bitsLen, pathPrd, id, data);
	if (header[2] == 'C') ret = packrat_v2_read_compact(pri, bitsPos, pathPrd, id, data);

	close(pri);
	return ret;
}
