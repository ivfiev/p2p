#ifndef P2P_PEER_H
#define P2P_PEER_H

#include "cb.h"

typedef struct peer_descriptor {
  int fd;
  epoll_cb *cb;
} PD;

void peer_reconnect(epoll_cb *cb); // timer cb

#endif //P2P_PEER_H
