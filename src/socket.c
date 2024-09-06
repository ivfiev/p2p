#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "log.h"

int listen1(const char *port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *bind_addr;
  if (getaddrinfo(0, port, &hints, &bind_addr) < 0) {
    ERROR_FATAL("getaddrinfo");
  }
  int fd = socket(hints.ai_family, hints.ai_socktype, 0);
  if (fd < 0) {
    ERROR_FATAL("peer_listen");
  }
  int reuse = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    ERROR_FATAL("setsockopt");
  }
  if (bind(fd, bind_addr->ai_addr, bind_addr->ai_addrlen) < 0) {
    ERROR_FATAL("bind");
  }
  if (listen(fd, 128) < 0) {
    ERROR_FATAL("listen");
  }
  freeaddrinfo(bind_addr);
  return fd;
}

int connect1(const char *port) {
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *peer_addr;
  if (getaddrinfo(0, port, &hints, &peer_addr) < 0) {
    ERROR_FATAL("getaddrinfo");
  }
  int fd = socket(hints.ai_family, hints.ai_socktype, 0);

  // TODO - use non-blocking connects & handle connection errors in evt loop
  // TODO - handle partial writes or increase the size of tcp buffers
  // TODO - handle partial reads (treat whitespace as terminator)
  if (connect(fd, peer_addr->ai_addr, peer_addr->ai_addrlen)) {
    ERROR_INFO("connect1.connect");
    freeaddrinfo(peer_addr);
    close(fd);
    return -1;
  }
  freeaddrinfo(peer_addr);
  return fd;
}

char *getname(int socket_fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(struct sockaddr_in);
  char *name = calloc(8, sizeof(char));
  if (getpeername(socket_fd, (struct sockaddr *)&addr, &len) < 0) {
    ERROR_FATAL("getpeername");
  }
  snprintf(name, 8, "%d", ntohs(addr.sin_port));
  return name;
}
