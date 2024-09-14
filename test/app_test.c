#include <stdlib.h>
#include <string.h>
#include "peer.h"
#include "test.h"
#include "util.h"
#include "fakes.h"

extern size_t COUNT;
extern char *TEXT[];
extern int HASH;

void handle_msg(int, peer_msg msg);

void on_tick(epoll_cb *);

void reset(void);

peer_msg get_msg(char *, int, char *);

TEST(add_new_text) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("text", 326757776, test);
  handle_msg(11, msg);
  ASSERT_EQ(326757776, HASH);
  ASSERT_EQ(2, (int)COUNT);
  ASSERT_STR_EQ("line1", TEXT[0]);
  ASSERT_STR_EQ("line2", TEXT[1]);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, free_fake.call_count);
}

TEST(equal_hashes) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("text", 8, test);
  HASH = 8;
  handle_msg(11, msg);
  ASSERT_EQ(0, (int)COUNT);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, free_fake.call_count);
}

TEST(merge_input) {
  reset();
  COUNT = 2;
  TEXT[0] = "line1";
  TEXT[1] = "line3";
  char test[] = "line2\nline4\n";
  peer_msg msg = get_msg("text", 1, test);
  handle_msg(11, msg);
  ASSERT(HASH > 1);
  ASSERT_EQ(4, (int)COUNT);
  ASSERT_STR_EQ("line1", TEXT[0]);
  ASSERT_STR_EQ("line2", TEXT[1]);
  ASSERT_STR_EQ("line3", TEXT[2]);
  ASSERT_STR_EQ("line4", TEXT[3]);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(3, free_fake.call_count);
}

TEST(overlapping_lists) {
  reset();
  COUNT = 2;
  TEXT[0] = "line1";
  TEXT[1] = "line2";
  char test[] = "line0\nline2\n";
  peer_msg msg = get_msg("text", 1, test);
  handle_msg(11, msg);
  ASSERT(HASH > 1);
  ASSERT_EQ(3, (int)COUNT);
  ASSERT_STR_EQ("line0", TEXT[0]);
  ASSERT_STR_EQ("line1", TEXT[1]);
  ASSERT_STR_EQ("line2", TEXT[2]);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(3, free_fake.call_count);
}

TEST(hash_msg) {
  reset();
  COUNT = 1;
  HASH = 7;
  TEXT[0] = "line1";
  peer_msg msg = get_msg("hash", 1, NULL);
  handle_msg(11, msg);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_STR_EQ("text,7,line1\n", (char *)write_fake.arg1_val);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(3, free_fake.call_count);
}

TEST(hash_broadcast) {
  reset();
  COUNT = 1;
  HASH = 7;
  TEXT[0] = "line1";
  on_tick(NULL);
  ASSERT(write_fake.call_count > 0);
  ASSERT_STR_EQ("hash,7", (char *)write_fake.arg1_val);
  ASSERT_EQ(3, free_fake.call_count);
}

TEST(empty_text) {
  reset();
  peer_msg msg = get_msg("hash", 1, NULL);
  handle_msg(11, msg);
  ASSERT_STR_EQ("text,0,", (char *)write_fake.arg1_val);
}

void app_test_run(void) {
  RUN_TEST(add_new_text);
  RUN_TEST(equal_hashes);
  RUN_TEST(merge_input);
  RUN_TEST(overlapping_lists);
  RUN_TEST(hash_msg);
  RUN_TEST(hash_broadcast);
  RUN_TEST(empty_text);
}

void reset(void) {
  for (int i = 0; i < COUNT; i++) {
    TEXT[i] = NULL;
  }
  COUNT = HASH = 0;
}

peer_msg get_msg(char *cmd, int h, char *test_input) {
  char *h_str = malloc(16);
  snprintf(h_str, 15, "%d", h);
  char **args = calloc(2, sizeof(char *));
  args[0] = h_str;
  args[1] = test_input;
  peer_msg msg = {cmd, args, 1 + (test_input != NULL)};
  return msg;
}