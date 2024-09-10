#include <stdlib.h>
#include <string.h>
#include "peer.h"
#include "hash.h"
#include "util.h"

#define MAX_LINES 4096

size_t COUNT;
char *TEXT[MAX_LINES];
int HASH;

void handle_hash_msg(peer_msg msg) {

}

void handle_lines_msg(peer_msg msg) {
  int h = atoi(msg.args[0]);
  char *tmp[MAX_LINES];
  char *msg_lines[MAX_LINES];
  int i, j, k;
  if (h == HASH) {
    return;
  }
  size_t lines_len = strsplit(msg.args[1], "\n", msg_lines);
  qsort(msg_lines, lines_len, sizeof(char *), (__compar_fn_t)strcmp);
  for (i = j = k = 0; k < MAX_LINES && (i < COUNT || j < lines_len); k++) {
    if (j == lines_len) {
      tmp[k] = TEXT[i++];
    } else if (i == COUNT) {
      tmp[k] = strdup(msg_lines[j++]);
    } else {
      int cmp = strcmp(TEXT[i], msg_lines[j]);
      if (cmp <= 0) {
        tmp[k] = TEXT[i++];
      } else {
        tmp[k] = strdup(msg_lines[j++]);
      }
    }
  }
  COUNT = k;
  memcpy(TEXT, tmp, COUNT * sizeof(char *));
  HASH = (int)hash_strs((void **)TEXT, COUNT, 1000000007);
}

void handle_msg(peer_msg msg) {
  if (!strcmp("hash", msg.cmd)) {
    handle_hash_msg(msg);
  } else if (!strcmp("lines", msg.cmd)) {
    handle_lines_msg(msg);
  }
}

void handle_peers(void) {
  peer_msg msg = {};
  broadcast(msg);
}

int init_app(void) {
  char *app = getenv("P2P_APP");
  if (app == NULL || strcmp(app, "1") != 0) {
    return -1;
  }
  set_handlers(handle_msg, handle_peers);
  return 0;
}