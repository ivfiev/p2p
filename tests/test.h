#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

#ifndef FAILS
#define FAILS
extern int FAIL_COUNT;
#endif

void reset_fakes();

#define TEST(name) void name(void)
#define RUN_TEST(test) do { \
  reset_fakes(); \
  int curr = FAIL_COUNT;   \
  printf("Running '%s'... ", #test); \
  test();                 \
  if (FAIL_COUNT > curr) {\
    printf("'%s' failed!\n", #test);                        \
  } else {                \
  printf("'%s' passed!\n", #test); \
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
