#include "test.h"
#include "fakes.h"
#include "hash.h"
#include "util.h"

TEST(str_hash_test) {
  size_t h1 = hash_str("Test1", 117);
  size_t h2 = hash_str("Test2", 117);
  ASSERT(h1 > 0 && h2 > 0 && h1 != h2);
}

TEST(str_split_test_commas) {
  char str[] = "hello,world";
  char *buf[16];
  size_t i = strsplit(str, ",", buf);
  ASSERT_EQ(2, (int)i);
  ASSERT_STR_EQ("hello", buf[0]);
  ASSERT_STR_EQ("world", buf[1]);
}

TEST(str_split_test_newlines) {
  char str[] = "hello\nworld\n\n";
  char *buf[16];
  size_t i = strsplit(str, "\n", buf);
  ASSERT_EQ(2, (int)i);
  ASSERT_STR_EQ("hello", buf[0]);
  ASSERT_STR_EQ("world", buf[1]);
}

TEST(str_split_empty_string) {
  char str[] = "text,0,";
  char *buf[16];
  size_t i = strsplit(str, ",", buf);
  ASSERT_EQ(2, (int)i);
}

void util_test_run(void) {
  RUN_TEST(str_hash_test);
  RUN_TEST(str_split_test_commas);
  RUN_TEST(str_split_test_newlines);
  RUN_TEST(str_split_empty_string);
}