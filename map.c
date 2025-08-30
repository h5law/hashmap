/* $hashmap: map.c */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "map.h"

#ifndef NO_DEBUG
#include <stdio.h>
#define DEBUG(...)                                                             \
    do {                                                                       \
        fprintf(stderr, "%d: %s    ", __LINE__, __FILE__);                     \
        fprintf(stderr, __VA_ARGS__);                                          \
    } while (0);
#else
#define NO_DEBUG
#define DEBUG(...)
#endif

struct vector {
    uintptr_t *buffer;
    size_t     capacity;
    size_t     count;
};

struct vector *vector_init()
{
    struct vector *vec = {0};
    vec->buffer        = malloc(VECTOR_DEFAULT_CAPACITY * sizeof(uintptr_t));
    return vec;
}

int vector_set(struct vector *vec, uintptr_t value, size_t index)
{
    size_t new_count = vec->count + 1;
    if (new_count > VECTOR_MAX_CAPACITY)
        return -1;
    if (index > vec->capacity)
        return -2;
    vec->buffer[index] = value;
    vec->count         = new_count;
    size_t new_cap     = VECTOR_RESIZE_FACTOR * new_count * sizeof(uintptr_t);
    if ((4 * new_count >= 3 * vec->capacity) && new_cap < VECTOR_MAX_CAPACITY) {
        vec->buffer = realloc(vec->buffer, VECTOR_RESIZE_FACTOR * new_count *
                                                   sizeof(uintptr_t));
        memset(vec->buffer + (vec->capacity * sizeof(uintptr_t)), 0,
               new_cap - vec->capacity + 1);
        vec->capacity = new_cap;
        return 1;
    }
    return 0;
}

uintptr_t vector_get(struct vector *vec, size_t index)
{
    if (index >= vec->capacity)
        return 0;
    return vec->buffer[index];
}

void vector_deinit(struct vector *vec, int dealloc)
{
    if (dealloc)
        for (size_t i = 0; i < vec->capacity; ++i)
            free(( void * )vec->buffer[i]);
    free(vec->buffer);
}

struct bucket {
    struct bucket *next;
    const void    *key;
    size_t         key_len;
    uint64_t       hash;
    uintptr_t      value;
};

struct hashmap {
    struct vector *vec;
    size_t         size;
};

struct hashmap *hashmap_init()
{
    struct hashmap *map = {0};
    struct vector  *vec = vector_init();
    return map;
}

/* FNV-1a standard initial prime offset */
#define HASH_VALUE_OFFSET 0XCBF29CE484222325
/* SplitMix64 highly dispersive odd u64 */
#define HASH_VALUE_PRIME  0XD6E8FEB86659FD93

uint64_t fnv1a_block_hash(const unsigned char *data, size_t len)
{
    size_t   blocks = len / 8;
    uint64_t hash   = HASH_VALUE_OFFSET;
    for (size_t i = 0; i < blocks; ++i) {
        hash ^= *( const uint64_t * )data;
        hash *= HASH_VALUE_PRIME;
        data += 8;
    }

    /* Process any left-overs bytes from the block processor */
    uint64_t last = len & 0xFF;
    switch (len % 8) {
    case 7:
        last |= ( uint64_t )data[6] << 56; /* fallthrough */
    case 6:
        last |= ( uint64_t )data[5] << 48; /* fallthrough */
    case 5:
        last |= ( uint64_t )data[4] << 40; /* fallthrough */
    case 4:
        last |= ( uint64_t )data[3] << 32; /* fallthrough */
    case 3:
        last |= ( uint64_t )data[2] << 24; /* fallthrough */
    case 2:
        last |= ( uint64_t )data[1] << 16; /* fallthrough */
    case 1:
        last |= ( uint64_t )data[0] << 8;
        hash ^= last;
        hash *= HASH_VALUE_PRIME;
    }

    return hash;
}

void add_bucket(struct bucket *b1, struct bucket *b2)
{
    struct bucket *b = b1->next;
    if (!b)
        b1->next = b2;
    while (b->next) {
        b = b->next;
    }
    b->next = b2;
}

int hashmap_set(struct hashmap *map, const unsigned char *key, size_t key_len,
                uintptr_t value)
{
    int            rc    = 0;
    uint64_t       hash  = 0;
    size_t         index = 0;
    struct bucket *b     = NULL;

    hash                 = fnv1a_block_hash(key, key_len);
    index                = HASH_REDUCE(hash, map->vec);
    b                    = calloc(1, sizeof(struct bucket));
    if (!b) {
        DEBUG("error creating bucket object for key %s", key);
        return -3;
    }
    b->key        = key;
    b->key_len    = key_len;
    b->hash       = index;
    b->value      = value;

    uintptr_t ptr = vector_get(map->vec, index);
    if (!ptr)
        rc = vector_set(map->vec, ( uintptr_t )( void * )b, index);
    else
        add_bucket(( struct bucket * )ptr, b);

    if (rc == 0)
        return 0;

    /* Handle error cases */
    if (rc == -1)
        DEBUG("map has reached maximum capacity");
    if (rc == -2)
        DEBUG("index for vector bucket was out of range got %zu max was %zu",
              index, map->vec->capacity);
    if (rc != 1)
        DEBUG("unknown error while setting key %s", key);

    /* rc == 1 -> rehash newly grown vector */
    uintptr_t tmp[map->vec->capacity];
    memset(tmp, 0, map->vec->capacity * sizeof(uintptr_t));

    for (size_t i = 0; i < map->vec->capacity; ++i) {
        if (map->vec->buffer[i] == 0)
            continue;
        struct bucket *bb = ( struct bucket * )map->vec->buffer[i];
        do {
            uint64_t hash  = fnv1a_block_hash(b->key, b->key_len);
            size_t   index = HASH_REDUCE(hash, map->vec);
            b->hash        = index;
            tmp[index]     = ( uintptr_t )bb;
        } while (b->next);
        memmove(map->vec->buffer, tmp, map->vec->capacity * sizeof(uintptr_t));
    }

    return rc;
}

int find_bucket(struct bucket *b, const unsigned char *key, size_t key_len)
{
    do {
        if (memcmp(b->key, key, key_len) == 0 && b->key_len == key_len)
            return 0;
        b = b->next;
    } while (b->next);
    return -1;
}

uintptr_t hashmap_get(struct hashmap *map, const unsigned char *key, size_t key_len)
{
    uint64_t hash  = fnv1a_block_hash(b->key, b->key_len);
    size_t   index = HASH_REDUCE(hash, map->vec);
    uintptr_t val = vector_get(map->vec, index);
    if (val)
        return val;
    return find_bucket((struct bucket *)val, key, key_len);
}

void hashmap_deinit(struct hashmap *map, int dealloc)
{
    if (dealloc) {
        struct bucket *b = NULL;
        for (size_t i = 0; i < map->vec->capacity; ++i) {
            if (map->vec->buffer[i] == 0)
                continue;
            b = ( struct bucket * )map->vec->buffer[i];
            do {
                struct bucket *tmp = b->next;
                free(b);
                b = tmp;
            } while (b);
        }
    }
    vector_deinit(map->vec, 0); /* Don't need to free values again */
}

/* vim: ft=c ts=4 sts=4 sw=4 et ai cin */
