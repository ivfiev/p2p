#include "p2p.h"
#include "test.h"

int EPFD;
int FAIL_COUNT;

void cb_test_run(void);

void socket_test_run(void);

void log_test_run(void);

int main(void) {
  cb_test_run();
  socket_test_run();
  log_test_run();
  if (FAIL_COUNT > 0) {
    printf("\n\nTests failed! %d\n", FAIL_COUNT);
  }
  return FAIL_COUNT;
}