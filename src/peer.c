#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include "hash.h"
#include "util.h"
#include "log.h"
#include "peer.h"
#include "socket.h"

#define BUF_SIZE 4096

extern int EPFD;

char *NAME;
hashtable *peers;
const int tick_ms = 500;

void (*handle_peer_msg)(int, peer_msg);

void (*handle_new_peers)(void);

epoll_cb *init_peer(int fd);

static PD *get_pd(int fd, epoll_cb *cb);

static void clear_pd(PD *pd);

void exec_cmd(peer_msg msg, epoll_cb *cb) {
  if (!strcmp(msg.cmd, "debug")) {
    if (!strcmp(msg.args[0], "conn")) {
      log_debug("connecting...");
      int fd = connect1(msg.args[1]);
      epoll_cb *peer_cb = init_peer(fd);
      if (peer_cb != NULL) {
        char *peer_name = strdup(msg.args[1]); // just a debug cmd, so it's fine to alloc.
        peer_cb->data = peer_name;
        PD *pd = get_pd(fd, peer_cb);
        hash_set(peers, peer_name, pd);
      }
    } else if (!strcmp(msg.args[0], "close")) {
      PD *pd = hash_getv(peers, msg.args[1]);
      log_debug("disconnecting fd [%d] ...", pd->fd);
      close1(pd->cb);
      clear_pd(pd);
    } else if (!strcmp(msg.args[0], "fail")) {
      int *x = malloc(sizeof(int));
      free(x);
      free(x);
    }
  } else if (!strcmp(msg.cmd, "peers")) {
    if (strcmp(msg.args[0], "nc") != 0 && strcmp(msg.args[0], NAME) != 0) {
      char *peer_name = hash_getk(peers, msg.args[0]);
      if (peer_name == NULL) {
        peer_name = strdup(msg.args[0]);
      }
      if (cb->data == NULL) {
        cb->data = peer_name;
      }
      PD *pd = hash_getv(peers, peer_name);
      if (pd == NULL) {
        pd = get_pd(cb->fd, cb);
      } else if (pd->fd <= 0) {
        pd->fd = cb->fd;
        pd->cb = cb;
      } else if (pd->fd != cb->fd) {
        if (strcmp(NAME, peer_name) < 0) {
          log_debug("double connection to %s, closing my end...", peer_name);
          close1(pd->cb);
          clear_pd(pd);
          pd->fd = cb->fd;
          pd->cb = cb;
        }
      }
      hash_set(peers, peer_name, pd);
    }
    for (int i = 1; i < msg.argc; i++) {
      if (!strcmp(NAME, msg.args[i]) || !strcmp(msg.args[0], msg.args[i])) {
        continue;
      }
      char *existing = (char *)hash_getk(peers, msg.args[i]);
      if (existing == NULL) {
        existing = strdup(msg.args[i]);
        hash_set(peers, existing, NULL);
      }
    }
  } else if (handle_peer_msg != NULL) {
    handle_peer_msg(cb->fd, msg);
  }
}

void peer_EPOLLIN(epoll_cb *cb) {
  char *toks[1024];
  char *cmds[128];
  const char *delim_tok = ",";
  const char *delim_cmd = "\0";
  char buf[BUF_SIZE];
  ssize_t bytes = read2(cb, buf);
  if (bytes < 0) {
    // cb disconnected
    return;
  }
  buf[bytes] = 0;
  trim_end(buf + bytes - 1);
  size_t cmd_c = strsplit(buf, delim_cmd, cmds);
  for (int c = 0; c < cmd_c; c++) {
    size_t tok_c = strsplit(cmds[c], delim_tok, toks);
    peer_msg msg = {toks[0], toks + 1, (int)tok_c - 1};
    exec_cmd(msg, cb);
  }
}

void disconnect_peer(epoll_cb *cb) {
  log_debug("disconnect_peer [%s]", (char *)cb->data);
  if (cb->data != NULL) {
    PD *pd = hash_getv(peers, cb->data);
    cb->data = NULL;
    if (pd->fd == cb->fd) {
      clear_pd(pd); // leave it in, key is a known peer name. are we clearing it twice?
    }
  }
}

void log_stats(char **keys, size_t len) {
  int total = 0, conn = 0;
  char conn_buf[BUF_SIZE], total_buf[BUF_SIZE];
  char *conn_ptr = conn_buf, *total_ptr = total_buf;
  memset(conn_ptr, 0, MIN(32, BUF_SIZE));
  memset(total_ptr, 0, MIN(32, BUF_SIZE));
  for (int i = 0; i < len; i++) {
    PD *pd = hash_getv(peers, keys[i]);
    if (pd != NULL && pd->fd > 0) {
      conn++;
      conn_ptr += snprintf(conn_ptr, 10, ",%s", keys[i]);
    }
    total++;
    total_ptr += snprintf(total_ptr, 10, ",%s", keys[i]);
  }
  log_info("(%d/%d) [conn:%s] [total:%s]", conn, total, conn_buf + 1, total_buf + 1);
}

void peer_reconnect(epoll_cb *cb) {
  if (peers->len <= 0) {
    return;
  }
  char **keys = (char **)hash_keys(peers);
  size_t count = (size_t)(log((double)peers->len) / 2.0 + 1);
  count = MIN(count, peers->len);
  char **new = (char **)rand_select((void **)keys, peers->len, count);
  for (int i = 0; i < peers->len; i++) {
    PD *pd = hash_getv(peers, keys[i]);
    int connected = pd != NULL && pd->fd > 0;
    int j;
    for (j = 0; j < count; j++) {
      if (new[j] == keys[i]) {
        if (connected) {
          // keep connection
          break;
        }
        // new connection
        log_debug("connecting to %s", new[j]);
        int fd = connect1(new[j]);
        epoll_cb *cb = init_peer(fd);
        if (cb != NULL) {
          if (pd == NULL) {
            pd = get_pd(fd, cb);
            hash_set(peers, new[j], pd);
          }
          pd->fd = fd;
          pd->cb = cb;
          cb->data = new[j];
        }
      }
    }
    if (j == count && connected) {
      // drop connection
      close1(pd->cb);
      clear_pd(pd);
    }
  }
  free(new);
  free(keys);
  if (handle_new_peers != NULL) {
    handle_new_peers();
  }
}

void peer_tick(epoll_cb *cb) {
  if (peers->len == 0) {
    return;
  }
  char **keys = (char **)hash_keys(peers);
  size_t per_min = peers->len * 3;
  size_t per_tick = (double)per_min / (60000.0 / tick_ms) + 1; // TODO divide by connection count
  char **gossip = (char **)rand_select((void **)keys, peers->len, per_tick);
  char peers_buf[BUF_SIZE];
  char *ptr = peers_buf + snprintf(peers_buf, 20, "%s,%s", "peers", NAME);
  for (int i = 0; i < per_tick; i++) {
    ptr += snprintf(ptr, 10, ",%s", gossip[i]);
  }
  for (int i = 0; i < peers->len; i++) {
    PD *pd = hash_getv(peers, keys[i]);
    if (pd != NULL && pd->fd > 0) {
      write(pd->fd, peers_buf, strlen(peers_buf) + 1);
    }
  }
  log_stats(keys, peers->len);
  free(gossip);
  free(keys);
}

epoll_cb *accept_peer(epoll_cb *listener) {
  int peer_fd = accept(listener->fd, NULL, NULL);
  log_debug("accepting peer [%d]", peer_fd);
  return init_peer(peer_fd);
}

epoll_cb *init_peer(int peer_fd) {
  if (peer_fd <= 0) {
    ERROR_INFO("init_peer negative fd");
    return NULL;
  }
  epoll_cb *peer_cb = alloc_cb(peer_fd);
  peer_cb->event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
  peer_cb->on_EPOLLIN = peer_EPOLLIN;
  peer_cb->on_close = disconnect_peer;
  epoll_ctl(EPFD, EPOLL_CTL_ADD, peer_fd, &peer_cb->event);
  return peer_cb;
}

void init(char *port) {
  port = port ? port : "8080";
  NAME = strdup(port);
  int fd = listen1(port);
  log_info("LISTENER port [%s], fd [%d]", port, fd);
  epoll_cb *cb = alloc_cb(fd);
  cb->event.events = EPOLLIN;
  cb->on_EPOLLIN = (void (*)(struct epoll_cb *))accept_peer;
  epoll_ctl(EPFD, EPOLL_CTL_ADD, fd, &cb->event);
  peers = hash_new(128, hash_str, (int (*)(void *, void *))strcmp);
  timer(tick_ms, peer_tick, NULL);
  timer(2 * tick_ms + 113, peer_reconnect, NULL);
}

static PD *get_pd(int fd, epoll_cb *cb) {
  PD *pd = (PD *)malloc(sizeof(PD));
  clear_pd(pd);
  pd->fd = fd;
  pd->cb = cb;
  return pd;
}

static void clear_pd(PD *pd) {
  memset(pd, 0, sizeof(PD));
}

void set_handlers(void (*handle_msg)(int, peer_msg)) {
  handle_peer_msg = handle_msg;
}

size_t pack_msg(peer_msg msg, char *buf) {
  char *ptr = buf;
  ptr += snprintf(ptr, 16, "%s", msg.cmd);
  for (int i = 0; i < msg.argc; i++) {
    ptr += snprintf(ptr, BUF_SIZE / 2, ",%s", msg.args[i]);
  }
  return ptr - buf;
}

void reply(int fd, peer_msg msg) {
  char buf[BUF_SIZE];
  size_t size = pack_msg(msg, buf);
  write(fd, buf, size);
}

void broadcast(peer_msg msg) {
  char buf[BUF_SIZE];
  size_t size = pack_msg(msg, buf);
  char **keys = (char **)hash_keys(peers);
  for (int i = 0; i < peers->len; i++) {
    PD *pd = hash_getv(peers, keys[i]);
    if (pd != NULL && pd->fd > 0) {
      write(pd->fd, buf, size);
    }
  }
  free(keys);
}