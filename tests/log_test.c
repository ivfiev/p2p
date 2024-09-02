#include "p2p.h"
#include "test.h"
#include "fakes.h"

void on_error(epoll_cb *cb);

static epoll_cb *setup();

TEST(on_error_happy_path) {
  epoll_cb *cb = setup();
  on_error(cb);
  ASSERT_EQ(0, read_fake.call_count);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, send_fake.call_count);
  on_error(nullptr);
  ASSERT_EQ(1, read_fake.call_count);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(1, send_fake.call_count);
}

TEST(on_error_no_socket) {
  epoll_cb *cb = setup();
  struct log_data *data = cb->data;
  data->socket_fd = 0;
  on_error(cb);
  on_error(nullptr);
  ASSERT_EQ(1, read_fake.call_count);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ(0, send_fake.call_count);
}

TEST(on_error_no_pipe) {
  epoll_cb *cb = setup();
  cb->fd = 0;
  on_error(cb);
  on_error(nullptr);
  ASSERT_EQ(0, read_fake.call_count);
  ASSERT_EQ(0, write_fake.call_count);
  ASSERT_EQ(0, send_fake.call_count);
}

void log_test_run(void) {
  RUN_TEST(on_error_happy_path);
  RUN_TEST(on_error_no_socket);
  RUN_TEST(on_error_no_pipe);
}

static epoll_cb *setup() {
  epoll_cb *cb = alloc_cb(10);
  struct log_data *data = malloc(sizeof(struct log_data));
  cb->data = data;
  data->socket_fd = 11;
  data->stdout_fd = 12;
  data->stderr_fd = 13;
  return cb;
}