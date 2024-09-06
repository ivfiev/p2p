#ifndef P2P_UTIL_H
#define P2P_UTIL_H

#include <stddef.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define TIMESTAMP(name) char name[32]; timestamp(name)

void trim_end(char *);

void **rand_select(void **elems, size_t len, size_t k);

void timestamp(char *);

#endif //P2P_UTIL_H
