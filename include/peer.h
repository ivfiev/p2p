#ifndef P2P_PEER_H
#define P2P_PEER_H

#include "cb.h"

typedef struct peer_message {
  char *cmd;
  char **args;
  int argc;
} peer_msg;

typedef struct peer_descriptor {
  int fd;
  epoll_cb *cb;
} PD;

void peer_reconnect(epoll_cb *cb); // timer cb

void set_handlers(void (*handle_msg)(char *, peer_msg));

void reply(char *name, peer_msg);

void broadcast(peer_msg);

#endif //P2P_PEER_H
