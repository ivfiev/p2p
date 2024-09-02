#include "fff.h"

#ifndef P2P_FAKES_H
#define P2P_FAKES_H
DECLARE_FAKE_VALUE_FUNC(int, close, int);
DECLARE_FAKE_VOID_FUNC(free, void*);
DECLARE_FAKE_VALUE_FUNC(int, getaddrinfo, const char *, const char *, const struct addrinfo *, struct addrinfo**);
DECLARE_FAKE_VOID_FUNC(freeaddrinfo, struct addrinfo *);
DECLARE_FAKE_VALUE_FUNC(int, socket, int, int, int);
DECLARE_FAKE_VALUE_FUNC(int, connect, int, const struct sockaddr *, socklen_t);
DECLARE_FAKE_VALUE_FUNC(ssize_t, write, int, const void *, size_t);
DECLARE_FAKE_VALUE_FUNC(ssize_t, read, int, void *, size_t);
DECLARE_FAKE_VALUE_FUNC(ssize_t, send, int, const void *, size_t, int);
#endif //P2P_FAKES_H
