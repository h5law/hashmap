// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assert.h"
#include "map.h"
#include "vector.h"
#include "xxhash.h"

uint64_t _default_hasher(hashmap_t *map, const char *key, const int len)
{
    if (strlen(key) != len)
        return ~(0);

    XXH64_state_t *xhstate;
    XXH_errorcode xherr;
    XXH64_hash_t xhhash;
    int reduced;

    xhstate = XXH64_createState();
    xherr = XXH64_update(xhstate, key, len);
    if (xherr == XXH_ERROR)
        return ~(0);
    xhhash = XXH64_digest(xhstate);

    // printf("Hash: %llu %s\n", xhhash, key);

    bucket_t empty = {0};
    reduced = (xhhash % map->buckets.capacity + map->buckets.capacity) %
              map->buckets.capacity;
    // printf("Start: %d %s\n", reduced, key);
    bucket_t curr = vector_gpos_type(&map->buckets, bucket_t, reduced);
    while (memcmp(&curr, &empty, sizeof(bucket_t)) != 0) {
        if (memcmp(curr.key, key, len) == 0)
            break;
        // printf("Probing...\n");
        reduced = (reduced + 1) % map->buckets.capacity;
        curr = vector_gpos_type(&map->buckets, bucket_t, reduced);
    }
    // printf("End: %d %s\n", reduced, key);
    XXH64_freeState(xhstate);
    return reduced;
}

hashmap_t hashmap_init(int capacity, double resize_pct, HM_KEY_HASHER hasher_fn)
{
    vector_bucket_t buckets, empty = {0};
    buckets = vector_init_type(bucket_t, capacity, resize_pct);

    hashmap_t map = {
            .buckets = buckets,
            // hasher_fn **must** handle the full process going from a
            // input `const char *key` -> `int bucket_index`, currently
            // the bucket array operates in an open-addressing format
            // with linear (next quadratic) probing. As such the hasher_fn
            // is expected to handle any initial collisions and probing
            // and only then return the final index the bucket entry
            // will remain in - until the bucket array grows in size
            // and the key-set is rehashed and "re-housed" in
            // accordance with the new bucket array length.
            .hasher_fn = hasher_fn};

    return map;
}

void hashmap_free(hashmap_t *map) { vector_free_type(&map->buckets, bucket_t); }

bool hashmap_rehash(hashmap_t *map)
{
    bucket_t curr, empty = {0};
    vector_bucket_t clone = vector_clone_type(&map->buckets, bucket_t);
    vector_empty_type(&map->buckets, bucket_t);
    bool success = true;
    for (int i = 0; i < clone.capacity; ++i) {
        curr = vector_gpos_type(&clone, bucket_t, i);
        if (memcmp(&curr, &empty, sizeof(bucket_t)) == 0)
            continue;

        int idx = map->hasher_fn(  // hasher_fn **should** handle collisions
                map, curr.key,     // (linear/quadratic probing) in the open
                strlen(curr.key)); // addressing key addressing system

        // printf("Remapping:\t%d -> %d\n", i, idx);
        if (!vector_spos_type(&map->buckets, bucket_t, curr, idx)) {
            success = false;
            break;
        }
    }
    return success;
}

bool hashmap_add(hashmap_t *map, const char *key, value_t value)
{
    bucket_t bucket = {
            .key = key,
            .value = value,
    };

    if ((double)(map->buckets.size + 1) >
        (double)(map->buckets.capacity) * map->buckets.load_factor_pct) {
        if (!vector_resize_type(&map->buckets, bucket_t,
                                2 * map->buckets.capacity))
            return false;
        hashmap_rehash(map);
    }

    int idx = map->hasher_fn( // hasher_fn **should** handle collisions
            map, key,         // (linear/quadratic probing) in the open
            strlen(key));     // addressing key addressing system

    return vector_spos_type(&map->buckets, bucket_t, bucket, idx);
}

value_t hashmap_get(hashmap_t *map, const char *key)
{
    bucket_t curr, empty = {0};
    int idx;
    idx = map->hasher_fn(map, key, strlen(key));
    curr = vector_gpos_type(&map->buckets, bucket_t, idx);
    if (memcmp(&curr, &empty, sizeof(bucket_t)) == 0)
        return NIL_VAL;
    if (memcmp(curr.key, key, strlen(key)) != 0)
        return NIL_VAL;
    return curr.value;
}
