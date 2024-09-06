#ifndef P2P_CB_H
#define P2P_CB_H

#include <sys/types.h>
#include <sys/epoll.h>

typedef struct epoll_cb {
  int fd;
  struct epoll_event event;
  void *data;

  void (*on_EPOLLIN)(struct epoll_cb *cb);

  void (*on_close)(struct epoll_cb *cb);

} epoll_cb;

epoll_cb *alloc_cb(int fd);

void free_cb(epoll_cb *cb);

ssize_t read2(epoll_cb *cb, char *buf);

void close1(epoll_cb *cb);

typedef struct timer_data {
  struct itimerspec *its;
  void *data;
} timer_data;

int timer(long ms, void (*on_tick)(epoll_cb *cb), void *data);

#endif //P2P_CB_H
