#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "packrat.h"

// Pack Rat v2 - File signature 'Pr'

/* Pack Rat Unsigned Integer (Pruint) format

	Each bit, starting from left to right, represents a number to be added to the total (if the bit is on).
	The first bit is 1 (2^0), second is 2 (2^1), third is 4 (2^2), and so on. The final (64th) bit represents 9223372036854775808 (2^63).
	This means the integers can use any number of bits, from one to 64, while utilizing the maximum possible number range.
*/

static const uint64_t pow2[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288,1048576,2097152,4194304,8388608,16777216,33554432,67108864,134217728,268435456,536870912,1073741824,2147483648,4294967296,8589934592,17179869184,34359738368,68719476736,137438953472,274877906944,549755813888,1099511627776,2199023255552,4398046511104,8796093022208,17592186044416,35184372088832,70368744177664,140737488355328,281474976710656,562949953421312,1125899906842624,2251799813685248,4503599627370496,9007199254740992,18014398509481984,36028797018963968,72057594037927936,144115188075855872,288230376151711744,576460752303423488,1152921504606846976,2305843009213693952,4611686018427387904,9223372036854775808ULL};

#define bytesInBits_DOWN 0
#define bytesInBits_UP 1
__attribute__((warn_unused_result, const))
static int bytesInBits(const int bits, const int dir) {
	const int modulus = bits % 8;
	return ((bits - modulus) / 8) + ((dir == bytesInBits_UP && modulus > 0) ? 1 : 0);
}

void bitcpy(char * const target, const char * const source, const int targetBegin, const int sourceBegin, const int bits) {
	int targetBit = targetBegin % 8;
	int sourceBit = sourceBegin % 8;
	int targetByte = (targetBegin - targetBit) / 8;
	int sourceByte = (sourceBegin - sourceBit) / 8;

	for (int i = 0; i < bits; i++) {
		if (1 & (source[sourceByte] >> (7 - sourceBit))) {
			target[targetByte] |= (1 << (7 - targetBit)); // 1
		} else {
			target[targetByte] &= (UINT8_MAX ^ (1 << (7 - targetBit))); // 0
		}

		sourceBit++;
		if (sourceBit > 7) {
			sourceByte++;
			sourceBit = 0;
		}

		targetBit++;
		if (targetBit > 7) {
			targetByte++;
			targetBit = 0;
		}
	}
}

static void pruint_store(char * const target, uint64_t source, const int bitCount) {
	switch (bitCount) { // no breaks
		case 64: if (source >= pow2[63]) {target[7] |= 1 << 0; source -= pow2[63];} else target[7] &= (UINT8_MAX ^ 1 << 0);
		case 63: if (source >= pow2[62]) {target[7] |= 1 << 1; source -= pow2[62];} else target[7] &= (UINT8_MAX ^ 1 << 1);
		case 62: if (source >= pow2[61]) {target[7] |= 1 << 2; source -= pow2[61];} else target[7] &= (UINT8_MAX ^ 1 << 2);
		case 61: if (source >= pow2[60]) {target[7] |= 1 << 3; source -= pow2[60];} else target[7] &= (UINT8_MAX ^ 1 << 3);
		case 60: if (source >= pow2[59]) {target[7] |= 1 << 4; source -= pow2[59];} else target[7] &= (UINT8_MAX ^ 1 << 4);
		case 59: if (source >= pow2[58]) {target[7] |= 1 << 5; source -= pow2[58];} else target[7] &= (UINT8_MAX ^ 1 << 5);
		case 58: if (source >= pow2[57]) {target[7] |= 1 << 6; source -= pow2[57];} else target[7] &= (UINT8_MAX ^ 1 << 6);
		case 57: if (source >= pow2[56]) {target[7] |= 1 << 7; source -= pow2[56];} else target[7] &= (UINT8_MAX ^ 1 << 7);

		case 56: if (source >= pow2[55]) {target[6] |= 1 << 0; source -= pow2[55];} else target[6] &= (UINT8_MAX ^ 1 << 0);
		case 55: if (source >= pow2[54]) {target[6] |= 1 << 1; source -= pow2[54];} else target[6] &= (UINT8_MAX ^ 1 << 1);
		case 54: if (source >= pow2[53]) {target[6] |= 1 << 2; source -= pow2[53];} else target[6] &= (UINT8_MAX ^ 1 << 2);
		case 53: if (source >= pow2[52]) {target[6] |= 1 << 3; source -= pow2[52];} else target[6] &= (UINT8_MAX ^ 1 << 3);
		case 52: if (source >= pow2[51]) {target[6] |= 1 << 4; source -= pow2[51];} else target[6] &= (UINT8_MAX ^ 1 << 4);
		case 51: if (source >= pow2[50]) {target[6] |= 1 << 5; source -= pow2[50];} else target[6] &= (UINT8_MAX ^ 1 << 5);
		case 50: if (source >= pow2[49]) {target[6] |= 1 << 6; source -= pow2[49];} else target[6] &= (UINT8_MAX ^ 1 << 6);
		case 49: if (source >= pow2[48]) {target[6] |= 1 << 7; source -= pow2[48];} else target[6] &= (UINT8_MAX ^ 1 << 7);

		case 48: if (source >= pow2[47]) {target[5] |= 1 << 0; source -= pow2[47];} else target[5] &= (UINT8_MAX ^ 1 << 0);
		case 47: if (source >= pow2[46]) {target[5] |= 1 << 1; source -= pow2[46];} else target[5] &= (UINT8_MAX ^ 1 << 1);
		case 46: if (source >= pow2[45]) {target[5] |= 1 << 2; source -= pow2[45];} else target[5] &= (UINT8_MAX ^ 1 << 2);
		case 45: if (source >= pow2[44]) {target[5] |= 1 << 3; source -= pow2[44];} else target[5] &= (UINT8_MAX ^ 1 << 3);
		case 44: if (source >= pow2[43]) {target[5] |= 1 << 4; source -= pow2[43];} else target[5] &= (UINT8_MAX ^ 1 << 4);
		case 43: if (source >= pow2[42]) {target[5] |= 1 << 5; source -= pow2[42];} else target[5] &= (UINT8_MAX ^ 1 << 5);
		case 42: if (source >= pow2[41]) {target[5] |= 1 << 6; source -= pow2[41];} else target[5] &= (UINT8_MAX ^ 1 << 6);
		case 41: if (source >= pow2[40]) {target[5] |= 1 << 7; source -= pow2[40];} else target[5] &= (UINT8_MAX ^ 1 << 7);

		case 40: if (source >= pow2[39]) {target[4] |= 1 << 0; source -= pow2[39];} else target[4] &= (UINT8_MAX ^ 1 << 0);
		case 39: if (source >= pow2[38]) {target[4] |= 1 << 1; source -= pow2[38];} else target[4] &= (UINT8_MAX ^ 1 << 1);
		case 38: if (source >= pow2[37]) {target[4] |= 1 << 2; source -= pow2[37];} else target[4] &= (UINT8_MAX ^ 1 << 2);
		case 37: if (source >= pow2[36]) {target[4] |= 1 << 3; source -= pow2[36];} else target[4] &= (UINT8_MAX ^ 1 << 3);
		case 36: if (source >= pow2[35]) {target[4] |= 1 << 4; source -= pow2[35];} else target[4] &= (UINT8_MAX ^ 1 << 4);
		case 35: if (source >= pow2[34]) {target[4] |= 1 << 5; source -= pow2[34];} else target[4] &= (UINT8_MAX ^ 1 << 5);
		case 34: if (source >= pow2[33]) {target[4] |= 1 << 6; source -= pow2[33];} else target[4] &= (UINT8_MAX ^ 1 << 6);
		case 33: if (source >= pow2[32]) {target[4] |= 1 << 7; source -= pow2[32];} else target[4] &= (UINT8_MAX ^ 1 << 7);

		case 32: if (source >= pow2[31]) {target[3] |= 1 << 0; source -= pow2[31];} else target[3] &= (UINT8_MAX ^ 1 << 0);
		case 31: if (source >= pow2[30]) {target[3] |= 1 << 1; source -= pow2[30];} else target[3] &= (UINT8_MAX ^ 1 << 1);
		case 30: if (source >= pow2[29]) {target[3] |= 1 << 2; source -= pow2[29];} else target[3] &= (UINT8_MAX ^ 1 << 2);
		case 29: if (source >= pow2[28]) {target[3] |= 1 << 3; source -= pow2[28];} else target[3] &= (UINT8_MAX ^ 1 << 3);
		case 28: if (source >= pow2[27]) {target[3] |= 1 << 4; source -= pow2[27];} else target[3] &= (UINT8_MAX ^ 1 << 4);
		case 27: if (source >= pow2[26]) {target[3] |= 1 << 5; source -= pow2[26];} else target[3] &= (UINT8_MAX ^ 1 << 5);
		case 26: if (source >= pow2[25]) {target[3] |= 1 << 6; source -= pow2[25];} else target[3] &= (UINT8_MAX ^ 1 << 6);
		case 25: if (source >= pow2[24]) {target[3] |= 1 << 7; source -= pow2[24];} else target[3] &= (UINT8_MAX ^ 1 << 7);

		case 24: if (source >= pow2[23]) {target[2] |= 1 << 0; source -= pow2[23];} else target[2] &= (UINT8_MAX ^ 1 << 0);
		case 23: if (source >= pow2[22]) {target[2] |= 1 << 1; source -= pow2[22];} else target[2] &= (UINT8_MAX ^ 1 << 1);
		case 22: if (source >= pow2[21]) {target[2] |= 1 << 2; source -= pow2[21];} else target[2] &= (UINT8_MAX ^ 1 << 2);
		case 21: if (source >= pow2[20]) {target[2] |= 1 << 3; source -= pow2[20];} else target[2] &= (UINT8_MAX ^ 1 << 3);
		case 20: if (source >= pow2[19]) {target[2] |= 1 << 4; source -= pow2[19];} else target[2] &= (UINT8_MAX ^ 1 << 4);
		case 19: if (source >= pow2[18]) {target[2] |= 1 << 5; source -= pow2[18];} else target[2] &= (UINT8_MAX ^ 1 << 5);
		case 18: if (source >= pow2[17]) {target[2] |= 1 << 6; source -= pow2[17];} else target[2] &= (UINT8_MAX ^ 1 << 6);
		case 17: if (source >= pow2[16]) {target[2] |= 1 << 7; source -= pow2[16];} else target[2] &= (UINT8_MAX ^ 1 << 7);

		case 16: if (source >= pow2[15]) {target[1] |= 1 << 0; source -= pow2[15];} else target[1] &= (UINT8_MAX ^ 1 << 0);
		case 15: if (source >= pow2[14]) {target[1] |= 1 << 1; source -= pow2[14];} else target[1] &= (UINT8_MAX ^ 1 << 1);
		case 14: if (source >= pow2[13]) {target[1] |= 1 << 2; source -= pow2[13];} else target[1] &= (UINT8_MAX ^ 1 << 2);
		case 13: if (source >= pow2[12]) {target[1] |= 1 << 3; source -= pow2[12];} else target[1] &= (UINT8_MAX ^ 1 << 3);
		case 12: if (source >= pow2[11]) {target[1] |= 1 << 4; source -= pow2[11];} else target[1] &= (UINT8_MAX ^ 1 << 4);
		case 11: if (source >= pow2[10]) {target[1] |= 1 << 5; source -= pow2[10];} else target[1] &= (UINT8_MAX ^ 1 << 5);
		case 10: if (source >= pow2[9])  {target[1] |= 1 << 6; source -= pow2[9];}  else target[1] &= (UINT8_MAX ^ 1 << 6);
		case  9: if (source >= pow2[8])  {target[1] |= 1 << 7; source -= pow2[8];}  else target[1] &= (UINT8_MAX ^ 1 << 7);

		case  8: if (source >= pow2[7])  {target[0] |= 1 << 0; source -= pow2[7];}  else target[0] &= (UINT8_MAX ^ 1 << 0);
		case  7: if (source >= pow2[6])  {target[0] |= 1 << 1; source -= pow2[6];}  else target[0] &= (UINT8_MAX ^ 1 << 1);
		case  6: if (source >= pow2[5])  {target[0] |= 1 << 2; source -= pow2[5];}  else target[0] &= (UINT8_MAX ^ 1 << 2);
		case  5: if (source >= pow2[4])  {target[0] |= 1 << 3; source -= pow2[4];}  else target[0] &= (UINT8_MAX ^ 1 << 3);
		case  4: if (source >= pow2[3])  {target[0] |= 1 << 4; source -= pow2[3];}  else target[0] &= (UINT8_MAX ^ 1 << 4);
		case  3: if (source >= pow2[2])  {target[0] |= 1 << 5; source -= pow2[2];}  else target[0] &= (UINT8_MAX ^ 1 << 5);
		case  2: if (source >= pow2[1])  {target[0] |= 1 << 6; source -= pow2[1];}  else target[0] &= (UINT8_MAX ^ 1 << 6);
		case  1: if (source >= pow2[0])  {target[0] |= 1 << 7; source -= pow2[0];}  else target[0] &= (UINT8_MAX ^ 1 << 7);
	}
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
static int packrat_read_compact(const int pri, const int bitsPos, const char *pathPrd, const int id, char ** const data) {
	if (pri < 0) return PACKRAT_ERROR_MISC;

	const off_t endPri = lseek(pri, 0, SEEK_END);
	const int infoBytes = bytesInBits(bitsPos, bytesInBits_UP);

	uint64_t len;
	int prd;

	const uint64_t pos = getPos(pri, infoBytes, id, bitsPos);
	if (pos == UINT64_MAX) return PACKRAT_ERROR_MISC;

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
		return PACKRAT_ERROR_READ;
	}

	return len;
}

// Read: Pack Rat Zero (Pr0)
static int packrat_read_zero(const int pri, const int bitsPos, const int bitsLen, const char * const pathPrd, const int id, char ** const data) {
	if (pri < 0) return PACKRAT_ERROR_MISC;

	const int infoBytes = bytesInBits(bitsPos + bitsLen, bytesInBits_UP);

	// Pack Rat Index: Position and Length
	char info[infoBytes];
	ssize_t bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	if (bytesRead != infoBytes) {
		close(pri);
		return PACKRAT_ERROR_READ;
	}

	const uint64_t pos = pruint_fetch(info, 0,       bitsPos);
	const uint64_t len = pruint_fetch(info, bitsPos, bitsLen);
	close(pri);

	if (len == 0) return PACKRAT_ERROR_EMPTY;

	// Pack Rat Data: File contents
	const int prd = open(pathPrd, O_RDONLY);
	if (prd == -1) return PACKRAT_ERROR_OPEN;

	const off_t prdSize = lseek(prd, 0, SEEK_END);
	if (prdSize < 1) {
		close(prd);
		return PACKRAT_ERROR_READ;
	} else if (pos + len > (unsigned long)prdSize) {
		close(prd);
		return PACKRAT_ERROR_ID;
	}

	*data = malloc(len + 1);
	if (*data == NULL) return PACKRAT_ERROR_ALLOC;

	bytesRead = pread(prd, *data, len, pos);
	close(prd);

	if (bytesRead != (ssize_t)len) {
		free(*data);
		return PACKRAT_ERROR_READ;
	}

	return len;
}

// Read: Main function - chooses format automatically.
// Returns length of file on success, or a negative error code on failure
int packrat_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data) {
	if (pathPri == NULL || pathPrd == NULL || id < 0 || data == NULL) return -1;

	const int pri = open(pathPri, O_RDONLY);
	if (pri == -1) return PACKRAT_ERROR_OPEN;

	char header[5];
	const ssize_t bytesRead = read(pri, header, 5);
	if (bytesRead != 5) return PACKRAT_ERROR_READ;

	if (header[0] != 'P' || header[1] != 'r') return PACKRAT_ERROR_FILESIG;

	const int bitsPos = header[3];
	const int bitsLen = header[4];

	int ret = -1;
	if (header[2] == '0') ret = packrat_read_zero(pri, bitsPos, bitsLen, pathPrd, id, data);
	if (header[2] == 'C') ret = packrat_read_compact(pri, bitsPos, pathPrd, id, data);

	close(pri);
	return ret;
}

// Append data to the .prd file
static off_t packrat_addFile(const int prd, const size_t len, const char * const data) {
	if (prd < 0 || len < 1 || data == NULL) return 0;

	const off_t pos = lseek(prd, 0, SEEK_END);
	if (pos < 0) return pos;

	ssize_t bytesWritten = write(prd, data, len);
	return (bytesWritten == (ssize_t)len) ? pos : -1;
}

// Write: Pack Rat Compact (PrC)
static int packrat_write_compact(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len, const int bitsPos) {
	const int pri = open(pathPri, O_WRONLY | O_APPEND);
	if (pri < 0) return -1;

	const int prd = open(pathPrd, O_WRONLY | O_APPEND);
	if (prd < 0) {close(pri); return -1;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -1;}
	if (flock(prd, LOCK_EX) != 0) {close(pri); close(prd); return -1;}

	const int infoBytes = bytesInBits(bitsPos, bytesInBits_UP);

	const int id = (lseek(pri, 0, SEEK_END) - 5) / infoBytes; // Ignore first five bytes (the file header)

	const off_t pos = packrat_addFile(prd, len, data);
	if (pos < 0) {
		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);
		return pos;
	}

	char cpr_pos[infoBytes];
	bzero(cpr_pos, infoBytes);
	pruint_store(cpr_pos, pos, bitsPos);

	const int ret = write(pri, cpr_pos, infoBytes);

	flock(pri, LOCK_UN);
	flock(prd, LOCK_UN);
	close(pri);
	close(prd);

	if (ret == infoBytes) return id;

	return -1;
}

// Write: Pack Rat Zero (Pr0)
static int packrat_write_zero(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len, const int bitsPos, const int bitsLen) {
	const int pri = open(pathPri, O_WRONLY | O_APPEND);
	if (pri < 0) return -1;

	const int prd = open(pathPrd, O_WRONLY | O_APPEND);
	if (prd < 0) {close(pri); return -1;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -1;}
	if (flock(prd, LOCK_EX) != 0) {close(pri); close(prd); return -1;}

	const int infoBytes = bytesInBits(bitsPos + bitsLen, bytesInBits_UP);
	const int posBytes = bytesInBits(bitsPos, bytesInBits_UP);
	const int lenBytes = bytesInBits(bitsLen, bytesInBits_UP);

	const int id = (lseek(pri, 0, SEEK_END) - 5) / infoBytes; // Ignore first five bytes (the file header)

	const off_t pos = packrat_addFile(prd, len, data);
	if (pos < 0) {
		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);
		return pos;
	}

	char cpr_pos[posBytes]; pruint_store(cpr_pos, pos, bitsPos);
	char cpr_len[lenBytes]; pruint_store(cpr_len, len, bitsLen);

	char cpr_full[infoBytes];
	bzero(cpr_full, infoBytes);
	bitcpy(cpr_full, cpr_pos, 0, 0, bitsPos);
	bitcpy(cpr_full, cpr_len, bitsPos, 0, bitsLen);

	const int ret = write(pri, cpr_full, infoBytes);

	flock(pri, LOCK_UN);
	flock(prd, LOCK_UN);
	close(pri);
	close(prd);

	if (ret == infoBytes) return id;

	return -1;
}

// Append placeholder entry to a Pack Rat Zero archive
static int packrat_write_zero_placeholder(const char * const pathPri, const int bitsPos, const int bitsLen) {
	const int pri = open(pathPri, O_WRONLY | O_APPEND);
	if (pri < 0) return -1;

	if (flock(pri, LOCK_EX) != 0) {close(pri); return -1;}

	const int infoBytes = bytesInBits(bitsPos + bitsLen, bytesInBits_UP);

	const int id = (lseek(pri, 0, SEEK_END) - 5) / infoBytes; // Ignore first five bytes (the file header)

	char zeroes[infoBytes];
	bzero(zeroes, infoBytes);
	const int ret = write(pri, zeroes, infoBytes);

	flock(pri, LOCK_UN);
	close(pri);

	return (ret == infoBytes) ? id : -1;
}

static char packrat_write_getBits(const char * const pathPri, int * const bitsPos, int * const bitsLen) {
	const int pri = open(pathPri, O_RDONLY);
	if (pri == -1) return 0;

	char header[5];
	const ssize_t bytesRead = read(pri, header, 5);
	if (bytesRead != 5) return 0;

	if (header[0] != 'P' || header[1] != 'r') return 0;

	const char type = header[2];
	if (type != '0' && type != 'C') return 0;

	*bitsPos = header[3];
	*bitsLen = header[4];

	const int infoBytes = (type == 'C') ? bytesInBits(*bitsPos, bytesInBits_UP) : bytesInBits(*bitsPos + *bitsLen, bytesInBits_UP);

	const int totalSize = lseek(pri, 0, SEEK_END) - 5;
	if (totalSize % infoBytes != 0) {
		// Index file is corrupted
		close(pri);
		return 0;
	}

	close(pri);
	return type;
}

int packrat_write(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len) {
	if (pathPri == NULL) return -1;

	int bitsPos;
	int bitsLen;

	const char type = packrat_write_getBits(pathPri, &bitsPos, &bitsLen);
	if (bitsPos < 1 || bitsPos > 99 || (type == '0' && (bitsLen < 1 || bitsLen > 99))) return -1;
	if (type == '0' && (uint64_t)len > (UINT64_MAX >> (64 - bitsLen))) return -1;

	if (type == '0' && len == 0) return packrat_write_zero_placeholder(pathPri, bitsPos, bitsLen);

	if (pathPrd == NULL || data == NULL || len < 1) return -1;

	if (type == '0') return packrat_write_zero(pathPri, pathPrd, data, len, bitsPos, bitsLen);
	if (type == 'C') return packrat_write_compact(pathPri, pathPrd, data, len, bitsPos);

	return -100;
}

static int packrat_update_zero(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len, const int bitsPos, const int bitsLen) {
	const int pri = open(pathPri, O_RDWR);
	if (pri < 0) return -1;

	const int prd = open(pathPrd, O_WRONLY);
	if (prd < 0) {close(pri); return -1;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -1;}
	if (flock(prd, LOCK_EX) != 0) {flock(pri, LOCK_UN); close(pri); close(prd); return -1;}

	const int infoBytes = bytesInBits(bitsPos + bitsLen, bytesInBits_UP);
	const int posBytes = bytesInBits(bitsPos, bytesInBits_UP);
	const int lenBytes = bytesInBits(bitsLen, bytesInBits_UP);

	// Pack Rat Index: Position and Length
	char info[infoBytes];
	ssize_t bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	if (bytesRead != infoBytes) {
		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);
		return -1;
	}

	const uint64_t oldPos = pruint_fetch(info, 0,       bitsPos);
	const uint64_t oldLen = pruint_fetch(info, bitsPos, bitsLen);

	if (oldLen == (uint64_t)len) {
		const ssize_t bytesWritten = pwrite(prd, data, len, oldPos);

		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);

		if (bytesWritten != len) return -1;
	} else if ((uint64_t)len < oldLen) {
		ssize_t bytesWritten = pwrite(prd, data, len, oldPos);

		flock(prd, LOCK_UN);
		close(prd);

		if (bytesWritten != len) {
			flock(pri, LOCK_UN);
			close(pri);
			return -1;
		}

		char cpr_pos[posBytes];
		char cpr_len[lenBytes];
		pruint_store(cpr_pos, oldPos, bitsPos);
		pruint_store(cpr_len, len, bitsLen);

		char cpr_full[infoBytes];
		bzero(cpr_full, infoBytes);
		bitcpy(cpr_full, cpr_pos, 0, 0, bitsPos);
		bitcpy(cpr_full, cpr_len, bitsPos, 0, bitsLen);

		bytesWritten = pwrite(pri, cpr_full, infoBytes, 5 + id * infoBytes);

		flock(pri, LOCK_UN);
		close(pri);

		if (bytesWritten != infoBytes) return -1;
	} else /* len > oldLen */{
		const off_t newPos = lseek(prd, 0, SEEK_END);
		ssize_t bytesWritten = write(prd, data, len);

		flock(prd, LOCK_UN);
		close(prd);

		if (bytesWritten != len) {
			flock(pri, LOCK_UN);
			close(pri);
			return -1;
		}

		char cpr_pos[posBytes];
		char cpr_len[lenBytes];
		pruint_store(cpr_pos, newPos, bitsPos);
		pruint_store(cpr_len, len, bitsLen);

		char cpr_full[infoBytes];
		bzero(cpr_full, infoBytes);
		bitcpy(cpr_full, cpr_pos, 0, 0, bitsPos);
		bitcpy(cpr_full, cpr_len, bitsPos, 0, bitsLen);

		bytesWritten = pwrite(pri, cpr_full, infoBytes, 5 + id * infoBytes);

		flock(pri, LOCK_UN);
		close(pri);

		if (bytesWritten != infoBytes) return -1;
	}

	return 0;
}

int packrat_update(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len) {
	if (pathPri == NULL || pathPrd == NULL || id < 0 || data == NULL || len < 1) return -1;

	int bitsPos;
	int bitsLen;

	const char type = packrat_write_getBits(pathPri, &bitsPos, &bitsLen);
	if (bitsPos < 1 || bitsPos > 99 || bitsLen < 1 || bitsLen > 99) return -1;

	if (type == '0' && (uint64_t)len > (UINT64_MAX >> (64 - bitsLen))) return -90;

// TODO check if requested ID is too high

	if (type == '0') return packrat_update_zero(pathPri, pathPrd, id, data, len, bitsPos, bitsLen);
//	if (type == 'C') return packrat_update_compact(pathPri, pathPrd, data, len, bitsPos);

	return -100;
}

int packrat_create(const char * const pathPri, const char * const pathPrd, const uint8_t bitsPos, const uint8_t bitsLen, const char type) {
	if (pathPri == NULL || pathPrd == NULL || bitsPos < 1 || bitsPos > 99 || (type == '0' && (bitsLen < 1 || bitsLen > 99))) return -1;

	const int prd = open(pathPrd, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (prd < 0) return -1;
	close(prd);

	const int pri = open(pathPri, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (pri < 0) return -1;
	if (flock(pri, LOCK_EX) != 0) {close(pri); return -1;}

	char header[5];
	header[0] = 'P';
	header[1] = 'r';
	header[2] = type;
	header[3] = bitsPos;
	header[4] = (type == 'C') ? 0 : bitsLen;

	const ssize_t bytesWritten = write(pri, header, 5);

	flock(pri, LOCK_UN);
	close(pri);
	return (bytesWritten == 5) ? 0 : -1;
}
