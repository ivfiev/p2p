#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "peer.h"
#include "hash.h"
#include "util.h"
#include "log.h"

#define MAX_LINES 256
#define BUF_SIZE 4096

size_t COUNT;
char *TEXT[MAX_LINES];
int HASH;

static peer_msg pack_hash_msg(void);

static peer_msg pack_text_msg(void);

static void free_msg(peer_msg);

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
  char *tmp[MAX_LINES];
  char *msg_text[MAX_LINES];
  int i, j, k;
  if (msg_hash == HASH) {
    return;
  }
  size_t lines_len = strsplit(msg.args[1], "\n", msg_text);
  qsort(msg_text, lines_len, sizeof(char *), (__compar_fn_t)strcmp);
  for (i = j = k = 0; k < MAX_LINES && (i < COUNT || j < lines_len); k++) {
    if (j == lines_len) {
      tmp[k] = TEXT[i++];
    } else if (i == COUNT) {
      tmp[k] = strdup(msg_text[j++]);
    } else {
      int cmp = strcmp(TEXT[i], msg_text[j]);
      if (cmp <= 0) {
        tmp[k] = TEXT[i++];
        j += !cmp;
      } else {
        tmp[k] = strdup(msg_text[j++]);
      }
    }
  }
  COUNT = k;
  memcpy(TEXT, tmp, COUNT * sizeof(char *));
  HASH = (int)hash_strs((void **)TEXT, COUNT, 1000000007);
  if (HASH != msg_hash) {
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
  for (int i = 0; i < COUNT; i++) {
    log_info("%s", TEXT[i]);
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
  char *text = malloc(BUF_SIZE);
  char *ptr = text;
  snprintf(hash, 15, "%d", HASH);
  for (int i = 0; i < COUNT; i++) {
    ptr += snprintf(ptr, BUF_SIZE, "%s\n", TEXT[i]);
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