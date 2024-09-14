#ifndef P2P_UTIL_H
#define P2P_UTIL_H

#include <stddef.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define TIMESTAMP(name) char name[32]; timestamp(name)

void trim_end(char *);

void **rand_select(void **elems, size_t len, size_t k);

void timestamp(char *);

size_t strsplit(char *str, const char *sep, char **buf);

size_t hash_int(void *ptr, size_t N);

size_t hash_str(void *ptr, size_t N);

size_t hash_strs(void **ptr, size_t count, size_t N);

int intcmp(int i, int j);

void strsort(char **, size_t);

#endif //P2P_UTIL_H
