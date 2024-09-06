#include "p2p.h"
#include "test.h"
#include "fakes.h"

char *FAKE_BYTES;
extern hashtable *peers;

void init(char *port);

epoll_cb *accept_peer(epoll_cb *listener);

void peer_EPOLLIN(epoll_cb *cb);

void peer_tick(epoll_cb *cb);

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
  PD *pd = hash_getv(peers, "testport");
  ASSERT_EQ(11, pd->fd);
  ASSERT_STR_EQ("testport", (char *)pd->cb->data);
}

TEST(peer_multiple_add) {
  init("8080");
  read_fake.custom_fake = custom_fake_read;
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = 11;
  accept_peer(cb);
  cb = alloc_cb(11);
  FAKE_BYTES = "peers,8081,8082,8083,8084\n";
  peer_EPOLLIN(cb);
  PD *pd = hash_getv(peers, "8081");
  ASSERT_EQ(4, (int)peers->len);
  ASSERT_STR_EQ("8081", (char *)pd->cb->data);
  char *keys[] = {"8082", "8083", "8084"};
  for (int i = 0; i < 3; i++) {
    pd = hash_getv(peers, keys[i]);
    ASSERT_PTR_EQ(NULL, pd);
  }
}

TEST(peer_double_connection) {
  init("8080");
  read_fake.custom_fake = custom_fake_read;
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = 11;
  cb = accept_peer(cb);
  FAKE_BYTES = "peers,8081";
  peer_EPOLLIN(cb);
  epoll_cb *dupe = alloc_cb(12);
  peer_EPOLLIN(dupe);
  PD *pd = hash_getv(peers, "8081");
  ASSERT_EQ(1, (int)peers->len);
  ASSERT_EQ(12, pd->fd);
  ASSERT_EQ(12, pd->cb->fd);
  ASSERT_STR_EQ("8081", (char *)pd->cb->data);
  ASSERT_EQ(1, close_fake.call_count);
  ASSERT_EQ(11, close_fake.arg0_val);
  ASSERT_EQ(1, free_fake.call_count);
  ASSERT_PTR_EQ(cb, free_fake.arg0_val);
}

TEST(peer_gossip) {
  init("808");
  read_fake.custom_fake = custom_fake_read;
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = 11;
  cb = accept_peer(cb);
  FAKE_BYTES = "peers,8081,80801,808011,8080111,808081111";
  peer_EPOLLIN(cb);
  const char *keys[] = {"8080111"};
  rand_select_fake.return_val = (void **)keys;
  peer_tick(nullptr);
  ASSERT_EQ(1, write_fake.call_count);
  ASSERT_EQ((int)strlen("peers,808,8080111") + 1, (int)write_fake.arg2_val);
  ASSERT_EQ(2, free_fake.call_count);
}

TEST(peer_reconnections) {
  init("8080");
  read_fake.custom_fake = custom_fake_read;
  epoll_cb *cb = alloc_cb(10);
  accept_fake.return_val = 11;
  cb = accept_peer(cb);
  FAKE_BYTES = "peers,8081,8082,8083,8084,8085,8086,8087,8088\n";
  peer_EPOLLIN(cb);
  ASSERT_STR_EQ("8081", (char *)cb->data);
  char *retval[] = {"8083", "8084"};
  rand_select_fake.return_val = (void **)retval;
  int fds[] = {12, 13};
  socket_fake.return_val_seq = fds;
  socket_fake.return_val_seq_len = 2;
  peer_reconnect(nullptr);
  // cleanup 8081
  ASSERT_PTR_EQ(NULL, cb->data);
  ASSERT_EQ(1, close_fake.call_count);
  ASSERT_EQ(3, free_fake.call_count);
  ASSERT_EQ(EPOLL_CTL_DEL, epoll_ctl_fake.arg1_history[4]);
  // connected peers have fds, others in hash but empty
  ASSERT_EQ(8, (int)peers->len);
  PD *pd = hash_getv(peers, "8081");
  ASSERT_EQ(0, pd->fd);
  ASSERT_PTR_EQ(NULL, pd->cb);
  pd = hash_getv(peers, "8083");
  ASSERT_EQ(12, pd->fd);
  ASSERT_PTR_EQ(retval[0], pd->cb->data);
  pd = hash_getv(peers, "8084");
  ASSERT_EQ(13, pd->fd);
  ASSERT_PTR_EQ(retval[1], pd->cb->data);
  ASSERT_EQ(1, close_fake.call_count);
  ASSERT_PTR_EQ(NULL, hash_getv(peers, "8082"));
  ASSERT_PTR_EQ(NULL, hash_getv(peers, "8085"));
  ASSERT_PTR_EQ(NULL, hash_getv(peers, "8086"));
  ASSERT_PTR_EQ(NULL, hash_getv(peers, "8087"));
  ASSERT_PTR_EQ(NULL, hash_getv(peers, "8088"));
}

ssize_t custom_fake_read(int fd, void *buf, size_t max) {
  size_t len = strlen(FAKE_BYTES);
  strncpy(buf, FAKE_BYTES, len);
  return (ssize_t)len;
}

void peer_test_run(void) {
  RUN_TEST(peer_add_fail_case);
  RUN_TEST(peer_add_success_case);
  RUN_TEST(peer_multiple_add);
  RUN_TEST(peer_double_connection);
  RUN_TEST(peer_reconnections);
  RUN_TEST(peer_gossip);
}
