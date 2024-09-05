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
FAKE_VALUE_FUNC(ssize_t, write, int, const void *, size_t);
FAKE_VALUE_FUNC(ssize_t, read, int, void *, size_t);
FAKE_VALUE_FUNC(ssize_t, send, int, const void *, size_t, int);
FAKE_VALUE_FUNC(int, accept, int, struct sockaddr *, socklen_t *);
FAKE_VALUE_FUNC(int, epoll_ctl, int, int, int, struct epoll_event *);
FAKE_VALUE_FUNC(int, setsockopt, int, int, int, const void *, socklen_t);
FAKE_VALUE_FUNC(int, bind, int, const struct sockaddr *, socklen_t);
FAKE_VALUE_FUNC(int, listen, int, int);
FAKE_VALUE_FUNC(int, timerfd_settime, int, int, const struct itimerspec *, struct itimerspec *);
FAKE_VALUE_FUNC(void **, rand_select, void **, size_t, size_t);

void reset_fakes(void) {
  RESET_FAKE(getaddrinfo);
  RESET_FAKE(freeaddrinfo);
  RESET_FAKE(connect);
  RESET_FAKE(close);
  RESET_FAKE(free);
  RESET_FAKE(write);
  RESET_FAKE(read);
  RESET_FAKE(send);
  RESET_FAKE(accept);
  RESET_FAKE(epoll_ctl);
  RESET_FAKE(timerfd_settime);

  getaddrinfo_fake.custom_fake = getaddrinfo_custom_fake;
}

#pragma clang diagnostic pop