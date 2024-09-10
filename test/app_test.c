#include <stdlib.h>
#include <string.h>
#include "peer.h"
#include "test.h"
#include "util.h"

extern size_t COUNT;
extern char *TEXT[];
extern int HASH;

void handle_msg(peer_msg msg);

void reset(void);

peer_msg get_msg(char *, int, char *, epoll_cb *);

TEST(add_new_text) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("lines", 1, test, NULL);
  handle_msg(msg);
  ASSERT(HASH > 0);
  ASSERT_EQ(2, (int)COUNT);
  ASSERT_STR_EQ("line1", TEXT[0]);
  ASSERT_STR_EQ("line2", TEXT[1]);
}

TEST(equal_hashes) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("lines", 8, test, NULL);
  HASH = 8;
  handle_msg(msg);
  ASSERT_EQ(0, (int)COUNT);
}

TEST(merge_input) {
  reset();
  COUNT = 2;
  TEXT[0] = "line1";
  TEXT[1] = "line3";
  char test[] = "line2\nline4\n";
  peer_msg msg = get_msg("lines", 1, test, NULL);
  handle_msg(msg);
  ASSERT(HASH > 1);
  ASSERT_EQ(4, (int)COUNT);
  ASSERT_STR_EQ("line1", TEXT[0]);
  ASSERT_STR_EQ("line2", TEXT[1]);
  ASSERT_STR_EQ("line3", TEXT[2]);
  ASSERT_STR_EQ("line4", TEXT[3]);
}

void app_test_run(void) {
  RUN_TEST(add_new_text);
  RUN_TEST(equal_hashes);
  RUN_TEST(merge_input);
}

void reset(void) {
  for (int i = 0; i < COUNT; i++) {
    TEXT[i] = NULL;
  }
  COUNT = HASH = 0;
}

peer_msg get_msg(char *cmd, int h, char *test_input, epoll_cb *cb) {
  char *h_str = malloc(16);
  snprintf(h_str, 15, "%d", h);
  char **args = calloc(2, sizeof(char *));
  args[0] = h_str;
  args[1] = test_input;
  peer_msg msg = {cmd, args, 2, cb};
  return msg;
}