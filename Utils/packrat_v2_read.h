#ifndef PACKRAT_V2_READER
#define PACKRAT_V2_READER

#define PACKRAT_ERROR_MISC -1
#define PACKRAT_ERROR_ALLOC -2
#define PACKRAT_ERROR_NODATA -10
#define PACKRAT_ERROR_OPEN -20
#define PACKRAT_ERROR_READWRITE -22
#define PACKRAT_ERROR_FILESIG -30
#define PACKRAT_ERROR_CORRUPT -31
#define PACKRAT_ERROR_EMPTY -40
#define PACKRAT_ERROR_ID -50

int packrat_v2_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data);

#endif
