#ifndef INCLUDES_PACKRAT_H
#define INCLUDES_PACKRAT_H

int packrat_create(const char *pathPri, const char *pathPrd, const int bitsPos, const int bitsLen, const char type);
int packrat_read(const char *pathPri, const char *pathPrd, const int id, char **data);
int packrat_write(const char *pathPri, const char *pathPrd, const char *data, const off_t len);

#endif
