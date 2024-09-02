#ifndef P2P
#define P2P

#include <stdio.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define EPOLL_MAX_EVENTS 128
#define BUF_SIZE 4096
#define TIMESTAMP(name) char name[32]; timestamp(name)
#define ERROR_FATAL(msg) err_fatal("%s at %s:%d", msg, __FILE_NAME__, __LINE__)
#define ERROR_INFO(msg) err_info("%s at %s:%d", msg, __FILE_NAME__, __LINE__)

// cb
typedef struct epoll_cb {
  int fd;
  struct epoll_event event;
  void *data;

  void (*on_EPOLLIN)(struct epoll_cb *cb);

  void (*on_close)(struct epoll_cb *cb);

} epoll_cb;

epoll_cb *alloc_cb(int fd);

void free_cb(epoll_cb *cb);

ssize_t read2(epoll_cb *cb, char *buf);

void close1(epoll_cb *cb);

// socket
int listen1(const char *port);

int connect1(const char *port);

char *getname(int socket_fd);

// timer
typedef struct timer_data {
  struct itimerspec *its;
  void *data;
} timer_data;

int timer(long ms, void (*on_tick)(epoll_cb *cb), void *data);

// log
struct log_data {
  int stdout_fd;
  int stderr_fd;
  int socket_fd;
  char *port;
  bool reconnect;
};

void err_fatal(const char *format, ...) __attribute__((format(printf, 1, 2)));

void err_info(const char *format, ...) __attribute__((format(printf, 1, 2)));

void log_debug(const char *format, ...) __attribute__((format(printf, 1, 2)));

void log_info(const char *format, ...) __attribute__((format(printf, 1, 2)));

// hashtable
struct node {
  void *key;
  void *val;
  struct node *next;
};

typedef struct hashtable {
  struct node **nodes;
  size_t cap;
  size_t len;

  int (*cmp)(void *k1, void *k2);

  size_t (*hash)(void *ptr, size_t N);
} hashtable;

size_t hash_int(void *ptr, size_t N);

size_t hash_str(void *ptr, size_t N);

int intcmp(int, int);

hashtable *hash_new(size_t cap, size_t (*hash)(void *ptr, size_t N), int (*cmp)(void *k1, void *k2));

void *hash_set(hashtable *ht, void *k, void *v);

void *hash_getk(hashtable *ht, void *k);

void *hash_getv(hashtable *ht, void *k);

struct node *hash_del(hashtable *ht, void *k);

void **hash_keys(hashtable *ht);

void hash_free(hashtable *ht);

// utils
void trim_end(char *);

void **rand_select(void **elems, size_t len, size_t k);

void timestamp(char *);

#endif