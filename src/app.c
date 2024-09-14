#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "peer.h"
#include "hash.h"
#include "util.h"
#include "log.h"

#define MAX_LINES 256
#define BUF_SIZE 4096

extern int LOG_LEVEL;

hashtable *TEXT;
int HASH;
char **CACHED_LINES;

static peer_msg pack_hash_msg(void);

static peer_msg pack_text_msg(void);

static void free_msg(peer_msg);

static void set_cached_lines(void) {
  if (CACHED_LINES != NULL) {
    free(CACHED_LINES);
  }
  char **keys = (char **)hash_keys(TEXT);
  strsort(keys, TEXT->len);
  CACHED_LINES = keys;
}

static void set_hash(void) {
  HASH = (int)hash_strs((void **)CACHED_LINES, TEXT->len, 1000000007);
}

void handle_hash_msg(int fd, peer_msg msg) {
  int msg_hash = atoi(msg.args[0]);
  if (msg_hash == HASH) {
    return;
  }
  peer_msg reply_msg = pack_text_msg();
  reply(fd, reply_msg);
  free_msg(reply_msg);
}

void handle_text_msg(int fd, peer_msg msg) {
  int msg_hash = atoi(msg.args[0]);
  if (msg_hash == HASH) {
    return;
  }
  if (msg.argc < 2 && TEXT->len > 0) { // no-text ""-case
    goto REPLY;
  }
  char *msg_text[MAX_LINES];
  size_t msg_len = strsplit(msg.args[1], "\n", msg_text);
  for (int i = 0; i < msg_len; i++) {
    char *k = hash_getk(TEXT, msg_text[i]);
    if (k == NULL) {
      k = strdup(msg_text[i]);
      hash_set(TEXT, k, NULL);
    }
  }
  set_cached_lines();
  set_hash();
  if (HASH != msg_hash) {
    REPLY:
    peer_msg reply_msg = pack_text_msg();
    reply(fd, reply_msg);
    free_msg(reply_msg);
  }
}

void handle_msg(int fd, peer_msg msg) {
  if (!strcmp("hash", msg.cmd)) {
    handle_hash_msg(fd, msg);
  } else if (!strcmp("text", msg.cmd)) {
    handle_text_msg(fd, msg);
  }
}

static void log_stats(void) {
  log_info("TEXT:");
  for (int i = 0; i < TEXT->len; i++) {
    log_info("%s", CACHED_LINES[i]);
  }
}

void on_tick(epoll_cb *cb) {
  log_stats();
  peer_msg msg = pack_hash_msg();
  broadcast(msg);
  free_msg(msg);
}

int init_app(void) {
  char *app = getenv("P2P_APP");
  if (app == NULL || strcmp(app, "1") != 0) {
    return -1;
  }
  TEXT = hash_new(MAX_LINES, hash_str, (int (*)(void *, void *))strcmp);
  set_handlers(handle_msg);
  timer(1000, on_tick, NULL);
  return 0;
}

static peer_msg pack_hash_msg(void) {
  char **args = (char **)calloc(1, sizeof(char *));
  char *hash = malloc(16);
  snprintf(hash, 15, "%d", HASH);
  args[0] = hash;
  peer_msg msg = {"hash", args, 1};
  return msg;
}

static peer_msg pack_text_msg(void) {
  char **args = (char **)calloc(1, sizeof(char *));
  char *hash = malloc(16);
  char *text = calloc(BUF_SIZE, sizeof(char));
  char *ptr = text;
  snprintf(hash, 15, "%d", HASH);
  for (int i = 0; i < TEXT->len; i++) {
    ptr += snprintf(ptr, BUF_SIZE, "%s\n", CACHED_LINES[i]);
  }
  args[0] = hash;
  args[1] = text;
  peer_msg msg = {"text", args, 2};
  return msg;
}

static void free_msg(peer_msg msg) {
  for (int i = 0; i < msg.argc; i++) {
    free(msg.args[i]);
  }
  free(msg.args);
}