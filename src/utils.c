#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void trim_end(char *str) {
  while (isspace(*str)) {
    *str-- = '\0';
  }
}

size_t hash_int(void *ptr, size_t N) {
  long long ll = (long long)ptr;
  size_t i = ((ll >> 32) ^ ll);
  return i % N;
}

int intcmp(int i, int j) {
  return i - j;
}

size_t hash_str(void *ptr, size_t N) {
  size_t i = 0;
  char *str = ptr;
  char c;
  while ((c = *str++)) {
    i += i * 89 + (int)c;
    i %= N;
  }
  return i;
}

size_t hash_strs(void **ptr, size_t count, size_t N) {
  size_t hash = 0;
  for (int i = 0; i < count; i++) {
    size_t h = hash_str(ptr[i], N);
    hash = h + hash + h * hash;
    hash %= N;
  }
  return hash;
}

double randf() {
  return (double)rand() / RAND_MAX;
}

__attribute__((weak))
void **rand_select(void **elems, size_t len, size_t k) {
  void **result = calloc(k, sizeof(void *));
  int selected = 0;
  for (int i = 0; selected < k; i++) {
    double prob = (double)(k - selected) / (len - i);
    if (randf() < prob) {
      result[selected++] = elems[i];
    }
  }
  return result;
}

void timestamp(char *buf) {
  struct timeval tv;
  struct tm *tm_info;
  gettimeofday(&tv, NULL);
  tm_info = localtime(&tv.tv_sec);
  size_t c = strftime(buf, 12, "%H:%M:%S", tm_info);
  snprintf(buf + c, 19, ".%03ld", tv.tv_usec / 1000);
}

size_t strsplit(char *str, const char *sep, char **buf) {
  size_t i = 0;
  char *save_ptr;
  char *tmp = strtok_r(str, sep, &save_ptr);
  for (buf[i++] = tmp; tmp != NULL;) {
    tmp = strtok_r(NULL, sep, &save_ptr);
    if (tmp != NULL) {
      buf[i++] = tmp;
    }
  }
  return i;
}