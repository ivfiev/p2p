#include "fff.h"
#include <sys/socket.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, close, int);
FAKE_VOID_FUNC(free, void*);
FAKE_VALUE_FUNC(int, getaddrinfo, void*, void*, void*, void**);
FAKE_VOID_FUNC(freeaddrinfo, void*);
FAKE_VALUE_FUNC(int, socket, int, int, int);
FAKE_VALUE_FUNC(int, connect, int, const struct sockaddr *, socklen_t); // NOLINT(*-sizeof-expression)