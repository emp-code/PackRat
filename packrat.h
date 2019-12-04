#ifndef INCLUDES_PACKRAT_H
#define INCLUDES_PACKRAT_H

#include <stdint.h>

#define PACKRAT_ERROR_MISC -1
#define PACKRAT_ERROR_ALLOC -2

#define PACKRAT_ERROR_NODATA -10
#define PACKRAT_ERROR_TOOBIG -11

#define PACKRAT_ERROR_OPEN -20
#define PACKRAT_ERROR_LOCK -21
#define PACKRAT_ERROR_READWRITE -22

#define PACKRAT_ERROR_FILESIG -30
#define PACKRAT_ERROR_CORRUPT -31
#define PACKRAT_ERROR_INDEX -32

#define PACKRAT_ERROR_EMPTY -40

#define PACKRAT_ERROR_ID -50

int packrat_create(const char * const pathPri, const char * const pathPrd, const uint8_t bitsPos, const uint8_t bitsLen, const char type);
int packrat_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data);
int packrat_write(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len);
int packrat_update(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len);

#endif
