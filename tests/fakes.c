#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvisibility"
#pragma ide diagnostic ignored "bugprone-sizeof-expression"

#include "fff.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>

DEFINE_FFF_GLOBALS;

int getaddrinfo_custom_fake(const char *ptr1, const char *ptr2, const struct addrinfo *ptr3, struct addrinfo **ptr4) {
  *ptr4 = malloc(100);
  return 0;
}

FAKE_VALUE_FUNC(int, close, int);
FAKE_VOID_FUNC(free, void*);
FAKE_VALUE_FUNC(int, getaddrinfo, const char *, const char *, const struct addrinfo *, struct addrinfo **);
FAKE_VOID_FUNC(freeaddrinfo, struct addrinfo *);
FAKE_VALUE_FUNC(int, socket, int, int, int);
FAKE_VALUE_FUNC(int, connect, int, const struct sockaddr *, socklen_t);

void reset_fakes(void) {
  RESET_FAKE(getaddrinfo);
  RESET_FAKE(freeaddrinfo);
  RESET_FAKE(close);
  RESET_FAKE(connect);
  RESET_FAKE(close);
  RESET_FAKE(free);

  getaddrinfo_fake.custom_fake = getaddrinfo_custom_fake;
}

#pragma clang diagnostic pop