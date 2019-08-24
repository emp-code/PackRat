#ifndef INCLUDES_PACKRAT_H
#define INCLUDES_PACKRAT_H

int packrat_create(const char * const pathPri, const char * const pathPrd, const int bitsPos, const int bitsLen, const char type);
int packrat_read(const char * const pathPri, const char * const pathPrd, const int id, char ** const data);
int packrat_write(const char * const pathPri, const char * const pathPrd, const char * const data, const off_t len);
int packrat_update(const char * const pathPri, const char * const pathPrd, const int id, const char * const data, const off_t len);

#endif
