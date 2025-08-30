/* $hashmap: map.h */

#ifndef HASHMAP_H
#define HASHMAP_H

/* Structure declarations */
#define VECTOR_RESIZE_FACTOR    2
#define VECTOR_LOAD_FACTOR      0.75F
#define VECTOR_DEFAULT_CAPACITY 64
#define VECTOR_MAX_CAPACITY     32768

struct vector;
struct bucket;
struct hashmap;

#include <stdint.h>
#include <stdlib.h>

/* Vector functions */
struct vector *vector_init();
int            vector_set(struct vector *vec, uintptr_t value, size_t index);
uintptr_t      vector_get(struct vector *vec, size_t index);
void           vector_deinit(struct vector *vec, int dealloc);

/* Hasher functions */
uint64_t fnv1a_block_hash(const unsigned char *data, size_t len);
#define HASH_REDUCE(hash, vec)                                                 \
    ((( uint64_t )hash) & ( uint64_t )((( struct vector * )vec->capacity - 1)))

struct hashmap *hashmap_init();
int  hashmap_set(struct hashmap *map, const unsigned char *key, size_t key_len,
                 uintptr_t value);
void add_bucket(struct bucket *b1, struct bucket *b2);
int  find_bucket(struct bucket *b, const unsigned char *key, size_t key_len);
uintptr_t hashmap_get(struct hashmap *map, const unsigned char *key,
                      size_t key_len);
void      hashmap_deinit(struct hashmap *map, int dealloc);

#endif /* HASHMAP_H */

/* vim: ft=c ts=4 sts=4 sw=4 et ai cin */
