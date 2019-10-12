#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdint.h>
#include <math.h> // for ceil, floor

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) // '!!' to make sure this returns 0 or 1

#define INT_2P63 9223372036854775808ULL
#define INT_2P62 4611686018427387904
#define INT_2P61 2305843009213693952
#define INT_2P60 1152921504606846976
#define INT_2P59 576460752303423488
#define INT_2P58 288230376151711744
#define INT_2P57 144115188075855872
#define INT_2P56 72057594037927936
#define INT_2P55 36028797018963968
#define INT_2P54 18014398509481984
#define INT_2P53 9007199254740992
#define INT_2P52 4503599627370496
#define INT_2P51 2251799813685248
#define INT_2P50 1125899906842624
#define INT_2P49 562949953421312
#define INT_2P48 281474976710656
#define INT_2P47 140737488355328
#define INT_2P46 70368744177664
#define INT_2P45 35184372088832
#define INT_2P44 17592186044416
#define INT_2P43 8796093022208
#define INT_2P42 4398046511104
#define INT_2P41 2199023255552
#define INT_2P40 1099511627776
#define INT_2P39 549755813888
#define INT_2P38 274877906944
#define INT_2P37 137438953472
#define INT_2P36 68719476736
#define INT_2P35 34359738368
#define INT_2P34 17179869184
#define INT_2P33 8589934592
#define INT_2P32 4294967296
#define INT_2P31 2147483648
#define INT_2P30 1073741824
#define INT_2P29 536870912
#define INT_2P28 268435456
#define INT_2P27 134217728
#define INT_2P26 67108864
#define INT_2P25 33554432
#define INT_2P24 16777216
#define INT_2P23 8388608
#define INT_2P22 4194304
#define INT_2P21 2097152
#define INT_2P20 1048576
#define INT_2P19 524288
#define INT_2P18 262144
#define INT_2P17 131072
#define INT_2P16 65536
#define INT_2P15 32768
#define INT_2P14 16384
#define INT_2P13 8192
#define INT_2P12 4096
#define INT_2P11 2048
#define INT_2P10 1024
#define INT_2P9 512
#define INT_2P8 256
#define INT_2P7 128
#define INT_2P6 64
#define INT_2P5 32
#define INT_2P4 16
#define INT_2P3 8
#define INT_2P2 4
#define INT_2P1 2
#define INT_2P0 1

// Get bit value, skipping first (skipBits) bits
static int bitCheck(const char * const source, int skipBits) {
	int byteBegin = 0;
	int bitBegin = 0;

	for (;skipBits > 0; skipBits--) {
		bitBegin++;

		if (bitBegin > 7) {
			bitBegin = 0;
			byteBegin++;
		}
	}

	return BIT_CHECK(source[byteBegin], bitBegin);
}

// Store unsigned integer of 1-64 bits to a string
static void simpleUint_toChar(char * const target, uint64_t source, const int bitCount) {
	switch(bitCount) { // no breaks
		case 64: if (source >= INT_2P63) {BIT_SET(target[7], 7); source -= INT_2P63;} else BIT_CLEAR(target[7], 7);
		case 63: if (source >= INT_2P62) {BIT_SET(target[7], 6); source -= INT_2P62;} else BIT_CLEAR(target[7], 6);
		case 62: if (source >= INT_2P61) {BIT_SET(target[7], 5); source -= INT_2P61;} else BIT_CLEAR(target[7], 5);
		case 61: if (source >= INT_2P60) {BIT_SET(target[7], 4); source -= INT_2P60;} else BIT_CLEAR(target[7], 4);
		case 60: if (source >= INT_2P59) {BIT_SET(target[7], 3); source -= INT_2P59;} else BIT_CLEAR(target[7], 3);
		case 59: if (source >= INT_2P58) {BIT_SET(target[7], 2); source -= INT_2P58;} else BIT_CLEAR(target[7], 2);
		case 58: if (source >= INT_2P57) {BIT_SET(target[7], 1); source -= INT_2P57;} else BIT_CLEAR(target[7], 1);
		case 57: if (source >= INT_2P56) {BIT_SET(target[7], 0); source -= INT_2P56;} else BIT_CLEAR(target[7], 0);

		case 56: if (source >= INT_2P55) {BIT_SET(target[6], 7); source -= INT_2P55;} else BIT_CLEAR(target[6], 7);
		case 55: if (source >= INT_2P54) {BIT_SET(target[6], 6); source -= INT_2P54;} else BIT_CLEAR(target[6], 6);
		case 54: if (source >= INT_2P53) {BIT_SET(target[6], 5); source -= INT_2P53;} else BIT_CLEAR(target[6], 5);
		case 53: if (source >= INT_2P52) {BIT_SET(target[6], 4); source -= INT_2P52;} else BIT_CLEAR(target[6], 4);
		case 52: if (source >= INT_2P51) {BIT_SET(target[6], 3); source -= INT_2P51;} else BIT_CLEAR(target[6], 3);
		case 51: if (source >= INT_2P50) {BIT_SET(target[6], 2); source -= INT_2P50;} else BIT_CLEAR(target[6], 2);
		case 50: if (source >= INT_2P49) {BIT_SET(target[6], 1); source -= INT_2P49;} else BIT_CLEAR(target[6], 1);
		case 49: if (source >= INT_2P48) {BIT_SET(target[6], 0); source -= INT_2P48;} else BIT_CLEAR(target[6], 0);

		case 48: if (source >= INT_2P47) {BIT_SET(target[5], 7); source -= INT_2P47;} else BIT_CLEAR(target[5], 7);
		case 47: if (source >= INT_2P46) {BIT_SET(target[5], 6); source -= INT_2P46;} else BIT_CLEAR(target[5], 6);
		case 46: if (source >= INT_2P45) {BIT_SET(target[5], 5); source -= INT_2P45;} else BIT_CLEAR(target[5], 5);
		case 45: if (source >= INT_2P44) {BIT_SET(target[5], 4); source -= INT_2P44;} else BIT_CLEAR(target[5], 4);
		case 44: if (source >= INT_2P43) {BIT_SET(target[5], 3); source -= INT_2P43;} else BIT_CLEAR(target[5], 3);
		case 43: if (source >= INT_2P42) {BIT_SET(target[5], 2); source -= INT_2P42;} else BIT_CLEAR(target[5], 2);
		case 42: if (source >= INT_2P41) {BIT_SET(target[5], 1); source -= INT_2P41;} else BIT_CLEAR(target[5], 1);
		case 41: if (source >= INT_2P40) {BIT_SET(target[5], 0); source -= INT_2P40;} else BIT_CLEAR(target[5], 0);

		case 40: if (source >= INT_2P39) {BIT_SET(target[4], 7); source -= INT_2P39;} else BIT_CLEAR(target[4], 7);
		case 39: if (source >= INT_2P38) {BIT_SET(target[4], 6); source -= INT_2P38;} else BIT_CLEAR(target[4], 6);
		case 38: if (source >= INT_2P37) {BIT_SET(target[4], 5); source -= INT_2P37;} else BIT_CLEAR(target[4], 5);
		case 37: if (source >= INT_2P36) {BIT_SET(target[4], 4); source -= INT_2P36;} else BIT_CLEAR(target[4], 4);
		case 36: if (source >= INT_2P35) {BIT_SET(target[4], 3); source -= INT_2P35;} else BIT_CLEAR(target[4], 3);
		case 35: if (source >= INT_2P34) {BIT_SET(target[4], 2); source -= INT_2P34;} else BIT_CLEAR(target[4], 2);
		case 34: if (source >= INT_2P33) {BIT_SET(target[4], 1); source -= INT_2P33;} else BIT_CLEAR(target[4], 1);
		case 33: if (source >= INT_2P32) {BIT_SET(target[4], 0); source -= INT_2P32;} else BIT_CLEAR(target[4], 0);

		case 32: if (source >= INT_2P31) {BIT_SET(target[3], 7); source -= INT_2P31;} else BIT_CLEAR(target[3], 7);
		case 31: if (source >= INT_2P30) {BIT_SET(target[3], 6); source -= INT_2P30;} else BIT_CLEAR(target[3], 6);
		case 30: if (source >= INT_2P29) {BIT_SET(target[3], 5); source -= INT_2P29;} else BIT_CLEAR(target[3], 5);
		case 29: if (source >= INT_2P28) {BIT_SET(target[3], 4); source -= INT_2P28;} else BIT_CLEAR(target[3], 4);
		case 28: if (source >= INT_2P27) {BIT_SET(target[3], 3); source -= INT_2P27;} else BIT_CLEAR(target[3], 3);
		case 27: if (source >= INT_2P26) {BIT_SET(target[3], 2); source -= INT_2P26;} else BIT_CLEAR(target[3], 2);
		case 26: if (source >= INT_2P25) {BIT_SET(target[3], 1); source -= INT_2P25;} else BIT_CLEAR(target[3], 1);
		case 25: if (source >= INT_2P24) {BIT_SET(target[3], 0); source -= INT_2P24;} else BIT_CLEAR(target[3], 0);

		case 24: if (source >= INT_2P23) {BIT_SET(target[2], 7); source -= INT_2P23;} else BIT_CLEAR(target[2], 7);
		case 23: if (source >= INT_2P22) {BIT_SET(target[2], 6); source -= INT_2P22;} else BIT_CLEAR(target[2], 6);
		case 22: if (source >= INT_2P21) {BIT_SET(target[2], 5); source -= INT_2P21;} else BIT_CLEAR(target[2], 5);
		case 21: if (source >= INT_2P20) {BIT_SET(target[2], 4); source -= INT_2P20;} else BIT_CLEAR(target[2], 4);
		case 20: if (source >= INT_2P19) {BIT_SET(target[2], 3); source -= INT_2P19;} else BIT_CLEAR(target[2], 3);
		case 19: if (source >= INT_2P18) {BIT_SET(target[2], 2); source -= INT_2P18;} else BIT_CLEAR(target[2], 2);
		case 18: if (source >= INT_2P17) {BIT_SET(target[2], 1); source -= INT_2P17;} else BIT_CLEAR(target[2], 1);
		case 17: if (source >= INT_2P16) {BIT_SET(target[2], 0); source -= INT_2P16;} else BIT_CLEAR(target[2], 0);

		case 16: if (source >= INT_2P15) {BIT_SET(target[1], 7); source -= INT_2P15;} else BIT_CLEAR(target[1], 7);
		case 15: if (source >= INT_2P14) {BIT_SET(target[1], 6); source -= INT_2P14;} else BIT_CLEAR(target[1], 6);
		case 14: if (source >= INT_2P13) {BIT_SET(target[1], 5); source -= INT_2P13;} else BIT_CLEAR(target[1], 5);
		case 13: if (source >= INT_2P12) {BIT_SET(target[1], 4); source -= INT_2P12;} else BIT_CLEAR(target[1], 4);
		case 12: if (source >= INT_2P11) {BIT_SET(target[1], 3); source -= INT_2P11;} else BIT_CLEAR(target[1], 3);
		case 11: if (source >= INT_2P10) {BIT_SET(target[1], 2); source -= INT_2P10;} else BIT_CLEAR(target[1], 2);
		case 10: if (source >= INT_2P9)  {BIT_SET(target[1], 1); source -= INT_2P9;}  else BIT_CLEAR(target[1], 1);
		case  9: if (source >= INT_2P8)  {BIT_SET(target[1], 0); source -= INT_2P8;}  else BIT_CLEAR(target[1], 0);

		case  8: if (source >= INT_2P7)  {BIT_SET(target[0], 7); source -= INT_2P7;} else BIT_CLEAR(target[0], 7);
		case  7: if (source >= INT_2P6)  {BIT_SET(target[0], 6); source -= INT_2P6;} else BIT_CLEAR(target[0], 6);
		case  6: if (source >= INT_2P5)  {BIT_SET(target[0], 5); source -= INT_2P5;} else BIT_CLEAR(target[0], 5);
		case  5: if (source >= INT_2P4)  {BIT_SET(target[0], 4); source -= INT_2P4;} else BIT_CLEAR(target[0], 4);
		case  4: if (source >= INT_2P3)  {BIT_SET(target[0], 3); source -= INT_2P3;} else BIT_CLEAR(target[0], 3);
		case  3: if (source >= INT_2P2)  {BIT_SET(target[0], 2); source -= INT_2P2;} else BIT_CLEAR(target[0], 2);
		case  2: if (source >= INT_2P1)  {BIT_SET(target[0], 1); source -= INT_2P1;} else BIT_CLEAR(target[0], 1);
		case  1: if (source >= INT_2P0)  {BIT_SET(target[0], 0); source -= INT_2P0;} else BIT_CLEAR(target[0], 0);
	}
}

// Get unsigned integer of 1-64 bits from a char array of 1-8 bytes
static uint64_t simpleUint_toInt(const char * const c, const int skipBits, const int bitCount) {
	if (bitCount < 1 || bitCount > 64) return 0;

	uint64_t result = 0;

	switch(bitCount) {
		case 64: if (bitCheck(c, skipBits + 63)) result += INT_2P63;
		case 63: if (bitCheck(c, skipBits + 62)) result += INT_2P62;
		case 62: if (bitCheck(c, skipBits + 61)) result += INT_2P61;
		case 61: if (bitCheck(c, skipBits + 60)) result += INT_2P60;
		case 60: if (bitCheck(c, skipBits + 59)) result += INT_2P59;
		case 59: if (bitCheck(c, skipBits + 58)) result += INT_2P58;
		case 58: if (bitCheck(c, skipBits + 57)) result += INT_2P57;
		case 57: if (bitCheck(c, skipBits + 56)) result += INT_2P56;
		case 56: if (bitCheck(c, skipBits + 55)) result += INT_2P55;
		case 55: if (bitCheck(c, skipBits + 54)) result += INT_2P54;
		case 54: if (bitCheck(c, skipBits + 53)) result += INT_2P53;
		case 53: if (bitCheck(c, skipBits + 52)) result += INT_2P52;
		case 52: if (bitCheck(c, skipBits + 51)) result += INT_2P51;
		case 51: if (bitCheck(c, skipBits + 50)) result += INT_2P50;
		case 50: if (bitCheck(c, skipBits + 49)) result += INT_2P49;
		case 49: if (bitCheck(c, skipBits + 48)) result += INT_2P48;
		case 48: if (bitCheck(c, skipBits + 47)) result += INT_2P47;
		case 47: if (bitCheck(c, skipBits + 46)) result += INT_2P46;
		case 46: if (bitCheck(c, skipBits + 45)) result += INT_2P45;
		case 45: if (bitCheck(c, skipBits + 44)) result += INT_2P44;
		case 44: if (bitCheck(c, skipBits + 43)) result += INT_2P43;
		case 43: if (bitCheck(c, skipBits + 42)) result += INT_2P42;
		case 42: if (bitCheck(c, skipBits + 41)) result += INT_2P41;
		case 41: if (bitCheck(c, skipBits + 40)) result += INT_2P40;
		case 40: if (bitCheck(c, skipBits + 39)) result += INT_2P39;
		case 39: if (bitCheck(c, skipBits + 38)) result += INT_2P38;
		case 38: if (bitCheck(c, skipBits + 37)) result += INT_2P37;
		case 37: if (bitCheck(c, skipBits + 36)) result += INT_2P36;
		case 36: if (bitCheck(c, skipBits + 35)) result += INT_2P35;
		case 35: if (bitCheck(c, skipBits + 34)) result += INT_2P34;
		case 34: if (bitCheck(c, skipBits + 33)) result += INT_2P33;
		case 33: if (bitCheck(c, skipBits + 32)) result += INT_2P32;
		case 32: if (bitCheck(c, skipBits + 31)) result += INT_2P31;
		case 31: if (bitCheck(c, skipBits + 30)) result += INT_2P30;
		case 30: if (bitCheck(c, skipBits + 29)) result += INT_2P29;
		case 29: if (bitCheck(c, skipBits + 28)) result += INT_2P28;
		case 28: if (bitCheck(c, skipBits + 27)) result += INT_2P27;
		case 27: if (bitCheck(c, skipBits + 26)) result += INT_2P26;
		case 26: if (bitCheck(c, skipBits + 25)) result += INT_2P25;
		case 25: if (bitCheck(c, skipBits + 24)) result += INT_2P24;
		case 24: if (bitCheck(c, skipBits + 23)) result += INT_2P23;
		case 23: if (bitCheck(c, skipBits + 22)) result += INT_2P22;
		case 22: if (bitCheck(c, skipBits + 21)) result += INT_2P21;
		case 21: if (bitCheck(c, skipBits + 20)) result += INT_2P20;
		case 20: if (bitCheck(c, skipBits + 19)) result += INT_2P19;
		case 19: if (bitCheck(c, skipBits + 18)) result += INT_2P18;
		case 18: if (bitCheck(c, skipBits + 17)) result += INT_2P17;
		case 17: if (bitCheck(c, skipBits + 16)) result += INT_2P16;
		case 16: if (bitCheck(c, skipBits + 15)) result += INT_2P15;
		case 15: if (bitCheck(c, skipBits + 14)) result += INT_2P14;
		case 14: if (bitCheck(c, skipBits + 13)) result += INT_2P13;
		case 13: if (bitCheck(c, skipBits + 12)) result += INT_2P12;
		case 12: if (bitCheck(c, skipBits + 11)) result += INT_2P11;
		case 11: if (bitCheck(c, skipBits + 10)) result += INT_2P10;
		case 10: if (bitCheck(c, skipBits +  9)) result += INT_2P9;
		case  9: if (bitCheck(c, skipBits +  8)) result += INT_2P8;
		case  8: if (bitCheck(c, skipBits +  7)) result += INT_2P7;
		case  7: if (bitCheck(c, skipBits +  6)) result += INT_2P6;
		case  6: if (bitCheck(c, skipBits +  5)) result += INT_2P5;
		case  5: if (bitCheck(c, skipBits +  4)) result += INT_2P4;
		case  4: if (bitCheck(c, skipBits +  3)) result += INT_2P3;
		case  3: if (bitCheck(c, skipBits +  2)) result += INT_2P2;
		case  2: if (bitCheck(c, skipBits +  1)) result += INT_2P1;
		case  1: if (bitCheck(c, skipBits +  0)) result += INT_2P0;
	}

	return result;
}

// Read: Pack Rat Zero (PR0)
static int packrat_read_zero(const int pri, const int bitsPos, const int bitsLen, const char * const pathPrd, const int id, char ** const data) {
	if (pri < 0) return -1;

	const int infoBytes = ceil((bitsPos + bitsLen) / (double)8);

	// Pack Rat Index: Position and Length
	char info[infoBytes];
	int bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	if (bytesRead != infoBytes) {
		close(pri);
		return -2;
	}

	const uint64_t pos = simpleUint_toInt(info, 0,       bitsPos);
	const uint64_t len = simpleUint_toInt(info, bitsPos, bitsLen);
	close(pri);

	if (len < 1) return -2;

	// Pack Rat Data: File contents
	const int prd = open(pathPrd, O_RDONLY);
	if (prd == -1) return -3;

	*data = malloc(len + 1);
	if (*data == NULL) return -4;

	bytesRead = pread(prd, *data, len, pos);
	close(prd);

	if (bytesRead != len) {
		free(*data);
		return -5;
	}

	return len;
}

static uint64_t getPos(const int pri, const int infoBytes, const int id, const int bitsPos) {
	char info[infoBytes];
	const int bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	return (bytesRead != infoBytes) ? -1 : simpleUint_toInt(info, 0, bitsPos);
}

// Read: Pack Rat Compact (PRC)
static int packrat_read_compact(const int pri, const int bitsPos, const char *pathPrd, const int id, char ** const data) {
	if (pri < 0) return -1;

	const off_t endPri = lseek(pri, 0, SEEK_END);
	const int infoBytes = ceil(bitsPos / (double)8);

	uint64_t len;
	int prd;

	const uint64_t pos = getPos(pri, infoBytes, id, bitsPos);
	if (pos < 0) return -2;

	// Pack Rat Data: File contents
	if ((5 + (id + 1) * infoBytes) >= endPri) {
		// Request is for the last file stored
		close(pri);

		prd = open(pathPrd, O_RDONLY);
		if (prd == -1) return -3;

		// Length is eof - start
		const uint64_t endPrd = lseek(prd, 0, SEEK_END);
		len = endPrd - pos;
	} else {
		const uint64_t end = getPos(pri, infoBytes, id + 1, bitsPos);
		len = end - pos;
		close(pri);

		if (len < 1) return -2;

		prd = open(pathPrd, O_RDONLY);
		if (prd == -1) return -3;
	}

	*data = malloc(len + 1);
	if (*data == NULL) return -4;

	const int bytesRead = pread(prd, *data, len, pos);
	close(prd);

	if (bytesRead != len) {
		free(*data);
		return -5;
	}

	return len;
}

// Read: Main function - chooses format automatically.
// Returns length of file on success, or a negative error code on failure
int packrat_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data) {
	if (pathPri == NULL || pathPrd == NULL || id < 0 || data == NULL) return -1;

	const int pri = open(pathPri, O_RDONLY);
	if (pri == -1) return -1;

	char header[5];
	const ssize_t bytesRead = read(pri, header, 5);
	if (bytesRead != 5) return -2;

	if (header[0] != 'P' || header[1] != 'R') return -3;

	const int bitsPos = header[3];
	const int bitsLen = header[4];

	int ret = -4;
	if (header[2] == 'C') ret = packrat_read_compact(pri, bitsPos, pathPrd, id, data);
	if (header[2] == '0') ret = packrat_read_zero(pri, bitsPos, bitsLen, pathPrd, id, data);

	close(pri);
	return ret;
}

// Append data to the .prd file
static off_t packrat_addFile(const int prd, const size_t len, const char * const data) {
	if (len < 1 || data == NULL) return 0;

	const off_t pos = lseek(prd, 0, SEEK_END);
	if (pos < 0) return pos;

	write(prd, data, len);

	return pos;
}

// Write: Pack Rat Compact (PRC)
static int packrat_write_compact(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len, const int bitsPos) {
	const int pri = open(pathPri, O_WRONLY | O_APPEND);
	if (pri < 0) return -2;

	const int prd = open(pathPrd, O_WRONLY | O_APPEND);
	if (prd < 0) {close(pri); return -3;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -4;}
	if (flock(prd, LOCK_EX) != 0) {close(pri); close(prd); return -5;}

	const int infoBytes = ceil(bitsPos / (double)8);

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
	simpleUint_toChar(cpr_pos, pos, bitsPos);

	const int ret = write(pri, cpr_pos, infoBytes);

	flock(pri, LOCK_UN);
	flock(prd, LOCK_UN);
	close(pri);
	close(prd);

	if (ret == infoBytes) return id;

	return -6;
}

static void bitcopy(char * const target, const int targetBitBegin, const char * const source, int len) {
	int sourceByte = 0;
	int sourceBit = 0;

	int targetByte = 0;
	int targetBit = targetBitBegin;

	for (;len > 0; len--) {
		if (BIT_CHECK(source[sourceByte], sourceBit)) {
			BIT_SET(target[targetByte], targetBit);
		} else {
			BIT_CLEAR(target[targetByte], targetBit);
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

// Write: Pack Rat Zero (PR0)
static int packrat_write_zero(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len, const int bitsPos, const int bitsLen) {
	const int pri = open(pathPri, O_WRONLY | O_APPEND);
	if (pri < 0) return -2;

	const int prd = open(pathPrd, O_WRONLY | O_APPEND);
	if (prd < 0) {close(pri); return -3;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -4;}
	if (flock(prd, LOCK_EX) != 0) {close(pri); close(prd); return -5;}

	const int infoBytes = ceil((bitsPos + bitsLen) / (double)8);
	const int posBytes = ceil(bitsPos / (double)8);
	const int lenBytes = ceil(bitsLen / (double)8);

	const int id = (lseek(pri, 0, SEEK_END) - 5) / infoBytes; // Ignore first five bytes (the file header)

	const off_t pos = packrat_addFile(prd, len, data);
	if (pos < 0) {
		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);
		return pos;
	}

	char cpr_pos[posBytes]; simpleUint_toChar(cpr_pos, pos, bitsPos);
	char cpr_len[lenBytes]; simpleUint_toChar(cpr_len, len, bitsLen);

	const int skipBytes = floor(bitsPos / (double)8);
	const int skipBits = bitsPos % 8;

	char cpr_full[infoBytes];
	bitcopy(cpr_full, 0, cpr_pos, bitsPos);
	bitcopy(cpr_full + skipBytes, skipBits, cpr_len, bitsLen);

	const int ret = write(pri, cpr_full, infoBytes);

	flock(pri, LOCK_UN);
	flock(prd, LOCK_UN);
	close(pri);
	close(prd);

	if (ret == infoBytes) return id;

	return -6;
}

static char packrat_write_getBits(const char * const pathPri, int * const bitsPos, int * const bitsLen) {
	const int pri = open(pathPri, O_RDONLY);
	if (pri == -1) return -10;

	char header[5];
	const ssize_t bytesRead = read(pri, header, 5);
	if (bytesRead != 5) return -11;

	if (header[0] != 'P' || header[1] != 'R') return -12;

	const char type = header[2];
	if (type != '0' && type != 'C') return -13;

	*bitsPos = header[3];
	*bitsLen = header[4];

	const int infoBytes = (type == 'C') ? ceil(*bitsPos / (double)8) : ceil((*bitsPos + *bitsLen) / (double)8);

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
	if (pathPri == NULL || pathPrd == NULL || data == NULL || len < 1) return -1;

	int bitsPos;
	int bitsLen;

	const char type = packrat_write_getBits(pathPri, &bitsPos, &bitsLen);
	if (bitsPos < 1 || bitsPos > 99 || (type == '0' && (bitsLen < 1 || bitsLen > 99))) return -1;

	if (type == '0' && len > pow(2, bitsLen)) return -1;

	if (type == '0') return packrat_write_zero(pathPri, pathPrd, data, len, bitsPos, bitsLen);
	if (type == 'C') return packrat_write_compact(pathPri, pathPrd, data, len, bitsPos);

	return -100;
}

int packrat_create(const char * const pathPri, const char * const pathPrd, const int bitsPos, const int bitsLen, const char type) {
	if (pathPri == NULL || pathPrd == NULL || bitsPos < 1 || bitsPos > 99 || (type == '0' && (bitsLen < 1 || bitsLen > 99))) return -1;

	const int prd = open(pathPrd, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (prd < 0) return -2;
	close(prd);

	const int pri = open(pathPri, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (pri < 0) return -3;
	if (flock(pri, LOCK_EX) != 0) {close(pri); return -4;}

	char header[5];
	header[0] = 'P';
	header[1] = 'R';
	header[2] = type;
	header[3] = bitsPos;
	header[4] = (type == 'C') ? 0 : bitsLen;

	write(pri, header, 5);

	flock(pri, LOCK_UN);
	close(pri);
	return 0;
}

static int packrat_update_zero(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len, const int bitsPos, const int bitsLen) {
	if (pathPri == NULL || pathPrd == NULL || id < 0 || data == NULL || len < 1 || bitsPos < 1 || bitsPos > 99 || bitsLen < 1 || bitsLen > 99) return -1;

	const int pri = open(pathPri, O_RDWR);
	if (pri < 0) return -2;

	const int prd = open(pathPrd, O_WRONLY);
	if (prd < 0) {close(pri); return -3;}

	// Lock both files
	if (flock(pri, LOCK_EX) != 0) {close(pri); close(prd); return -4;}
	if (flock(prd, LOCK_EX) != 0) {flock(pri, LOCK_UN); close(pri); close(prd); return -5;}

	const int infoBytes = ceil((bitsPos + bitsLen) / (double)8);
	const int posBytes = ceil(bitsPos / (double)8);
	const int lenBytes = ceil(bitsLen / (double)8);

	// Pack Rat Index: Position and Length
	char info[infoBytes];
	int bytesRead = pread(pri, info, infoBytes, 5 + id * infoBytes);
	if (bytesRead != infoBytes) {
		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);
		return -6;
	}

	const uint64_t oldPos = simpleUint_toInt(info, 0,       bitsPos);
	const uint64_t oldLen = simpleUint_toInt(info, bitsPos, bitsLen);

	if (oldLen == len) {
		const ssize_t bytesWritten = pwrite(prd, data, len, oldPos);

		flock(pri, LOCK_UN);
		flock(prd, LOCK_UN);
		close(pri);
		close(prd);

		if (bytesWritten != len) return -8;
	} else if (len < oldLen) {
		ssize_t bytesWritten = pwrite(prd, data, len, oldPos);

		flock(prd, LOCK_UN);
		close(prd);

		if (bytesWritten != len) {
			flock(pri, LOCK_UN);
			close(pri);
			return -7;
		}

		char cpr_pos[posBytes];
		char cpr_len[lenBytes];
		simpleUint_toChar(cpr_pos, oldPos, bitsPos);
		simpleUint_toChar(cpr_len, len, bitsLen);

		const int skipBytes = floor(bitsPos / (double)8);
		const int skipBits = bitsPos % 8;

		char cpr_full[infoBytes];
		bitcopy(cpr_full, 0, cpr_pos, bitsPos);
		bitcopy(cpr_full + skipBytes, skipBits, cpr_len, bitsLen);

		bytesWritten = pwrite(pri, cpr_full, infoBytes, 5 + id * infoBytes);

		flock(pri, LOCK_UN);
		close(pri);

		if (bytesWritten != infoBytes) return -8;
	} else /* len > oldLen */{
		const off_t newPos = lseek(prd, 0, SEEK_END);
		ssize_t bytesWritten = write(prd, data, len);

		flock(prd, LOCK_UN);
		close(prd);

		if (bytesWritten != len) {
			flock(pri, LOCK_UN);
			close(pri);
			return -7;
		}

		char cpr_pos[posBytes];
		char cpr_len[lenBytes];
		simpleUint_toChar(cpr_pos, newPos, bitsPos);
		simpleUint_toChar(cpr_len, len, bitsLen);

		const int skipBytes = floor(bitsPos / (double)8);
		const int skipBits = bitsPos % 8;

		char cpr_full[infoBytes];
		bitcopy(cpr_full, 0, cpr_pos, bitsPos);
		bitcopy(cpr_full + skipBytes, skipBits, cpr_len, bitsLen);

		bytesWritten = pwrite(pri, cpr_full, infoBytes, 5 + id * infoBytes);

		flock(pri, LOCK_UN);
		close(pri);

		if (bytesWritten != infoBytes) return -8;
	}

	return 0;
}

int packrat_update(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len) {
	if (pathPri == NULL || pathPrd == NULL || id < 0 || data == NULL || len < 1) return -1;

	int bitsPos;
	int bitsLen;

	const char type = packrat_write_getBits(pathPri, &bitsPos, &bitsLen);

	if (type == '0' && len > pow(2, bitsLen)) return -90;

// TODO check if requested ID is too high

	if (type == '0') return packrat_update_zero(pathPri, pathPrd, id, data, len, bitsPos, bitsLen);
//	if (type == 'C') return packrat_update_compact(pathPri, pathPrd, data, len, bitsPos);

	return -100;
}
