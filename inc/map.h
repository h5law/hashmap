// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HASHMAP_H_SHARED
#define HASHMAP_H_SHARED

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "vector.h"

////////////////////////////////////////////////////////////////////////////////
//                                HashMap Typing                              //
////////////////////////////////////////////////////////////////////////////////

typedef struct bucket_t {
    const char *key;
    value_t value;
} bucket_t;

typedef struct vector_bucket_t vector_bucket_t;

#ifndef _DYN_VEC_BUCKET_T
#define _DYN_VEC_BUCKET_T

VECTOR_DEFINE(bucket_t)

#endif /* ifndef _DYN_VEC_BUCKET_T */

typedef struct hashmap_t hashmap_t;

typedef uint64_t (*HM_KEY_HASHER)(hashmap_t *, const char *key, const int len);

typedef struct hashmap_t {
    vector_bucket_t buckets;
    HM_KEY_HASHER hasher_fn;
} hashmap_t;

////////////////////////////////////////////////////////////////////////////////
//                             HashMap Life Cycle                             //
////////////////////////////////////////////////////////////////////////////////

hashmap_t hashmap_init(int capacity, double load_factor_pct,
                       HM_KEY_HASHER hasher_fn);
void hashmap_free(hashmap_t *map);
bool hashmap_rehash(hashmap_t *map);

////////////////////////////////////////////////////////////////////////////////
//                             HashMap Modifiers                              //
////////////////////////////////////////////////////////////////////////////////

uint64_t _default_hasher(hashmap_t *map, const char *key, const int len);

bool hashmap_add(hashmap_t *map, const char *key, value_t value);
bool hashmap_delete(hashmap_t *map, const char *key);
value_t hashmap_get(hashmap_t *map, const char *key);
bool hashmap_clear(hashmap_t *map);

#endif // !HASHMAP_H_SHARED

#ifdef __cplusplus
} /* extern "C" */
#endif
