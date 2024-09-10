#ifndef P2P_HASH_H
#define P2P_HASH_H

#include <stddef.h>

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

hashtable *hash_new(size_t cap, size_t (*hash)(void *ptr, size_t N), int (*cmp)(void *k1, void *k2));

void *hash_set(hashtable *ht, void *k, void *v);

void *hash_getk(hashtable *ht, void *k);

void *hash_getv(hashtable *ht, void *k);

struct node *hash_del(hashtable *ht, void *k);

void **hash_keys(hashtable *ht);

void hash_free(hashtable *ht);

#endif //P2P_HASH_H
