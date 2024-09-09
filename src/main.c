#include <time.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "log.h"
#include "cb.h"

#define EPOLL_MAX_EVENTS 128

int EPFD;

void init(char *port);

int init_log();

int is_timer(int fd);

void on_error(epoll_cb *cb);

static void init_logs_flush(void);

static int cmp_events(struct epoll_event *e1, struct epoll_event *e2) {
  epoll_cb *cb1 = e1->data.ptr;
  epoll_cb *cb2 = e2->data.ptr;
  int t1 = is_timer(cb1->fd);
  int t2 = is_timer(cb2->fd);
  return t1 - t2;
}

int main(int argc, char **argv) {
  srand(time(NULL));
  struct epoll_event events[EPOLL_MAX_EVENTS];
  memset(events, 0, sizeof(events));

  EPFD = epoll_create1(EPOLL_CLOEXEC);

  if (!init_log()) {
    init_logs_flush();
  }

  init(argv[1]);

  for (;;) {
    int ready = epoll_wait(EPFD, events, EPOLL_MAX_EVENTS, -1);

    qsort(events, ready, sizeof(struct epoll_event), (__compar_fn_t)cmp_events);

    for (int i = 0; i < ready; i++) {
      epoll_cb *cb = events[i].data.ptr;

      if (events[i].events & EPOLLIN) {
        cb->on_EPOLLIN(cb);
      }

      if (events[i].events & (EPOLLHUP | EPOLLERR)) {
        log_debug("EPOLLHUP | EPOLLERR - closing fd [%d]", cb->fd);
        close1(cb);
      }
    }
  }
}

static void flush_logs(int sig) {
  // not reentrant-safe...
  log_info("Signal [%s:%d] received, errno [%s], flushing & aborting...", strsignal(sig), sig, strerror(errno));
  on_error(NULL);
  exit(1);
}

static void init_logs_flush(void) {
  int sigs[] = {SIGABRT, SIGINT, SIGTERM, SIGFPE, SIGSEGV, SIGPIPE, 0};
  struct sigaction sig;
  memset(&sig, 0, sizeof(sig));
  sig.sa_handler = flush_logs;
  for (int i = 0; sigs[i]; i++) {
    if (sigaction(sigs[i], &sig, NULL) < 0) {
      log_info("Signal %d handler failed", sigs[i]);
    }
  }
}