#include "p2p.h"
#include "test.h"
#include "fakes.h"

char *FAKE_BYTES;
extern hashtable *peers;

void init(char *port);

void accept_peer(epoll_cb *cb);

void peer_EPOLLIN(epoll_cb *cb);

ssize_t custom_fake_read(int fd, void *buf, size_t max);

TEST(peer_add_fail_case) {
  init("8080");
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = -1;
  accept_peer(cb);
  ASSERT_EQ(0, (int)peers->len);
}

TEST(peer_add_success_case) {
  init("8080");
  read_fake.custom_fake = custom_fake_read;
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = 11;
  accept_peer(cb);
  ASSERT_EQ(0, (int)peers->len);
  ASSERT_EQ(4, epoll_ctl_fake.call_count);
  ASSERT_EQ(1, epoll_ctl_fake.arg1_history[3]);
  ASSERT_EQ(11, epoll_ctl_fake.arg2_history[3]);
  cb = alloc_cb(11);
  FAKE_BYTES = "peers,testport";
  peer_EPOLLIN(cb);
  ASSERT_EQ(1, (int)peers->len);
  ASSERT_EQ(4, epoll_ctl_fake.call_count);
}

ssize_t custom_fake_read(int fd, void *buf, size_t max) {
  size_t len = strlen(FAKE_BYTES);
  strncpy(buf, FAKE_BYTES, len);
  return (ssize_t)len;
}

void peer_test_run(void) {
  RUN_TEST(peer_add_fail_case);
  RUN_TEST(peer_add_success_case);
}
