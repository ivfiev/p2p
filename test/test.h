#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

#ifndef FAILS
#define FAILS
extern int FAIL_COUNT;
extern int TEST_COUNT;
#endif

void reset_fakes();

#define TEST(name) void name(void)
#define RUN_TEST(test) do { \
  TEST_COUNT++;  \
  reset_fakes(); \
  int curr = FAIL_COUNT;   \
  printf("Running '%s'...\n", #test); \
  test();                 \
  if (FAIL_COUNT > curr) {\
    printf("✗ '%s' failed!\n\n", #test);                        \
  } else {                \
  printf("✓ '%s' passed!\n\n", #test); \
  }                        \
} while (0)

#define ASSERT_EQ(expected, actual) do { \
  if ((expected) != (actual)) {        \
    FAIL_COUNT++;                            \
    printf("Assertion failed: %s == %s\n", #expected, #actual); \
    printf("Expected: %d, Actual: %d\n", (expected), (actual)); \
    return; \
  } \
} while (0)

#endif
