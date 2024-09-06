#include "test.h"
#include "fakes.h"
#include "cb.h"

TEST(close1_calls_close) {
  epoll_cb *cb = alloc_cb(1);
  close1(cb);
  ASSERT_EQ(1, close_fake.call_count);
}

TEST(free_cb_calls_free_twice) {
  epoll_cb *cb = alloc_cb(1);
  cb->data = "test";
  free_cb(cb);
  ASSERT_EQ(2, free_fake.call_count);
}

TEST(free_cb_calls_free_once_for_null_data) {
  epoll_cb *cb = alloc_cb(1);
  free_cb(cb);
  ASSERT_EQ(1, free_fake.call_count);
}

void cb_test_run(void) {
  RUN_TEST(close1_calls_close);
  RUN_TEST(free_cb_calls_free_twice);
  RUN_TEST(free_cb_calls_free_once_for_null_data);
}