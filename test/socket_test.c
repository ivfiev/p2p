#include "p2p.h"
#include <malloc.h>
#include "test.h"
#include "fakes.h"

TEST(connect1_happy_case) {
  connect1("80808080");
  ASSERT_EQ(1, getaddrinfo_fake.call_count);
  ASSERT_EQ(1, freeaddrinfo_fake.call_count);
  ASSERT_EQ(0, close_fake.call_count);
}

TEST(connect1_fail_case) {
  connect_fake.return_val = -1;
  int ret = connect1("testport");
  ASSERT_EQ(1, getaddrinfo_fake.call_count);
  ASSERT_EQ(1, freeaddrinfo_fake.call_count);
  ASSERT_EQ(1, close_fake.call_count);
  ASSERT_EQ(-1, ret);
}

void socket_test_run(void) {
  RUN_TEST(connect1_happy_case);
  RUN_TEST(connect1_fail_case);
}