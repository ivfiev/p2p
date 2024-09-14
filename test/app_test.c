#include <stdlib.h>
#include <string.h>
#include "peer.h"
#include "test.h"
#include "util.h"
#include "fakes.h"
#include "hash.h"

extern int HASH;
extern hashtable *TEXT;
extern char **CACHED_LINES;

void handle_msg(int, peer_msg msg);

void on_tick(epoll_cb *);

void reset(void);

void setup(char *lines, int hash);

peer_msg get_msg(char *, int, char *);

TEST(add_new_text) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("text", 326757776, test);
  handle_msg(11, msg);
  ASSERT_EQ(326757776, HASH);
  ASSERT_EQ(2, (int)TEXT->len);
  ASSERT_STR_EQ("line1", CACHED_LINES[0]);
  ASSERT_STR_EQ("line2", CACHED_LINES[1]);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, free_fake.call_count);
}

TEST(equal_hashes) {
  reset();
  char test[] = "line1\nline2\n";
  peer_msg msg = get_msg("text", 8, test);
  HASH = 8;
  handle_msg(11, msg);
  ASSERT_EQ(0, (int)TEXT->len);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, free_fake.call_count);
}

TEST(merge_input) {
  reset();
  setup("line1|line3", 0);
  char test[] = "line2\nline4\n";
  peer_msg msg = get_msg("text", 1, test);
  handle_msg(11, msg);
  ASSERT(HASH > 1);
  ASSERT_EQ(4, (int)TEXT->len);
  ASSERT_STR_EQ("line1", CACHED_LINES[0]);
  ASSERT_STR_EQ("line2", CACHED_LINES[1]);
  ASSERT_STR_EQ("line3", CACHED_LINES[2]);
  ASSERT_STR_EQ("line4", CACHED_LINES[3]);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(4, free_fake.call_count);
}

TEST(overlapping_lists) {
  reset();
  setup("line1|line2", 0);
  char test[] = "line0\nline2\n";
  peer_msg msg = get_msg("text", 1, test);
  handle_msg(11, msg);
  ASSERT(HASH > 1);
  ASSERT_EQ(3, (int)TEXT->len);
  ASSERT_STR_EQ("line0", CACHED_LINES[0]);
  ASSERT_STR_EQ("line1", CACHED_LINES[1]);
  ASSERT_STR_EQ("line2", CACHED_LINES[2]);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(4, free_fake.call_count);
}

TEST(hash_msg) {
  reset();
  setup("line1", 7);
  peer_msg msg = get_msg("hash", 1, NULL);
  handle_msg(11, msg);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_STR_EQ("text,7,line1\n", (char *)write_fake.arg1_val);
  ASSERT_EQ(11, write_fake.arg0_val);
  ASSERT_EQ(3, free_fake.call_count);
}

TEST(hash_broadcast) {
  reset();
  setup("line1", 7);
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

TEST(numeric_random_case) {
  reset();
  setup("04|06", 19131875);
  char input[] = "05\n08\n06\n06\n06\n";
  peer_msg msg = get_msg("text", 1, input);
  handle_msg(11, msg);
  ASSERT_STR_EQ("text,706047296,04\n05\n06\n08\n", (char *)write_fake.arg1_val);
}

void app_test_run(void) {
  RUN_TEST(add_new_text);
  RUN_TEST(equal_hashes);
  RUN_TEST(merge_input);
  RUN_TEST(overlapping_lists);
  RUN_TEST(hash_msg);
  RUN_TEST(hash_broadcast);
  RUN_TEST(empty_text);
  RUN_TEST(numeric_random_case);
}

void reset(void) {
  TEXT = hash_new(128, hash_str, (int (*)(void *, void *))strcmp);
  HASH = 0;
  CACHED_LINES = NULL;
}

void setup(char *lines, int hash) {
  HASH = hash;
  char **ls = calloc(16, sizeof(char *));
  char tmp[1024];
  strcpy(tmp, lines);
  size_t count = strsplit(tmp, "|", ls);
  strsort(ls, count);
  for (int i = 0; i < count; i++) {
    char *s = strdup(ls[i]);
    hash_set(TEXT, s, NULL);
    ls[i] = s;
  }
  CACHED_LINES = ls;
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