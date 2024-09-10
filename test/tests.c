#include "test.h"

int EPFD;
int FAIL_COUNT;
int TEST_COUNT;

void cb_test_run(void);

void socket_test_run(void);

void log_test_run(void);

void peer_test_run(void);

void util_test_run(void);

void app_test_run(void);

int main(void) {
  cb_test_run();
  socket_test_run();
  log_test_run();
  peer_test_run();
  util_test_run();
  app_test_run();
  printf("\n\nTests run:\t\t%d\n", TEST_COUNT);
  printf("Tests failed:\t%d\n", FAIL_COUNT);
  return FAIL_COUNT;
}