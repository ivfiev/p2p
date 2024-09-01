#include "p2p.h"
#include <malloc.h>
#include "fff.h"
#include "test.h"
#include "fakes.h"

int getaddrinfo_custom_fake(const char *ptr1, const char *ptr2, const struct addrinfo *ptr3, struct addrinfo **ptr4) {
  *ptr4 = malloc(100);
  return 0;
}

static void setup(void) {
  RESET_FAKE(getaddrinfo);
  RESET_FAKE(freeaddrinfo);
  RESET_FAKE(close);
  RESET_FAKE(connect);
  getaddrinfo_fake.custom_fake = getaddrinfo_custom_fake;
}

TEST(connect1_happy_case) {
  connect1("8080");
  ASSERT_EQ(1, getaddrinfo_fake.call_count);
  ASSERT_EQ(1, freeaddrinfo_fake.call_count);
  ASSERT_EQ(0, close_fake.call_count);
}

TEST(connect1_fail_case) {
  connect_fake.return_val = -1;
  connect1("8080");
  ASSERT_EQ(1, getaddrinfo_fake.call_count);
  ASSERT_EQ(1, freeaddrinfo_fake.call_count);
  ASSERT_EQ(1, close_fake.call_count);
}

void socket_test_run(void) {
  RUN_TEST(connect1_happy_case);
  RUN_TEST(connect1_fail_case);
}