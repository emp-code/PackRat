#ifndef PACKRAT_H
#define PACKRAT_H

enum packrat_status {
	PACKRAT_OK,

	// General
	PACKRAT_ERROR_PARAM = -100,
	PACKRAT_ERROR_ALLOC,

	// I/O
	PACKRAT_ERROR_OPEN,
	PACKRAT_ERROR_LOCK,
	PACKRAT_ERROR_READ_PRI,
	PACKRAT_ERROR_READ_PRD,
	PACKRAT_ERROR_WRITE,

	// Data
	PACKRAT_ERROR_TOOBIG,
	PACKRAT_ERROR_FORMAT,
	PACKRAT_ERROR_HEADER,
	PACKRAT_ERROR_MISMATCH
};

int packrat_create(const char * const pathPri, const char * const pathPrd, const int bitsPos, const int bitsLen); // bitsPos=0 for Compact
int packrat_write(const char * const pathPri, const char * const pathPrd, const unsigned char * const data, const int lenData);
int packrat_read(const char * const pathPri, const char * const pathPrd, const unsigned long fileNum, unsigned char ** const data);
//int packrat_replace(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len);

#endif
