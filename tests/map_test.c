// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "map.h"
#include "vector.h"
#include "xxhash.h"

typedef struct map_value {
    char scratch[8];
    uint64_t bytes, hash;
} map_value;

void _print_buckets(hashmap_t *map)
{
    bucket_t curr, empty = {0};
    for (int i = 0; i < map->buckets.capacity; ++i) {
        curr = vector_gpos_type(&map->buckets, bucket_t, i);
        if (memcmp(&curr, &empty, sizeof(bucket_t)) == 0)
            printf("%d: nomás yo\n", i);
        else
            printf("%d:\t(%s\t%f)\n", i, curr.key,
                   _value_to_number(&curr.value));
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    hashmap_t map, empty = {0};
    bool ok;
    value_t val;

    map = hashmap_init(10, 0.6, _default_hasher);
    ASSERT(memcmp(&map, &empty, sizeof(hashmap_t)) != 0,
           "validate new map is not empty",
           "memcmp(&map, &empty, sizeof(hashmap_t)) != 0");

    static char *buf;
    // printf("INSERTING 0-9:\n");
    for (int i = 0; i < 10; ++i) {
        buf = (char *)calloc(16, sizeof(char));
        sprintf(buf, "key%d", i);

        // printf("INTERNAL MAP INSERT LOGS: %d\n", i);
        ok = hashmap_add(&map, buf, _number_to_value((double)i));
        ASSERT(ok == true, "add value to mapping", "ok == true");

        // printf("INTERNAL MAP LAYOUT: %d\n", i);
        // _print_buckets(&map);
    }

    // printf("VALIDATING 0-9:\n");
    for (int i = 0; i < 10; ++i) {
        buf = (char *)calloc(16, sizeof(char));
        sprintf(buf, "key%d", i);

        value_t val = hashmap_get(&map, buf);
        ASSERT(_value_to_number(&val) == (double)i, "validate value in mapping",
               "_value_to_number(&val) == (double)i");
    }

    // _print_buckets(&map);

    hashmap_free(&map);

    map = hashmap_init(10, 0.75, _default_hasher);
    ASSERT(memcmp(&map, &empty, sizeof(hashmap_t)) != 0,
           "validate new map is not empty",
           "memcmp(&map, &empty, sizeof(hashmap_t)) != 0");

    map_value *val1;
    val1 = (map_value *)calloc(1, sizeof(map_value));
    ASSERT(val1 != NULL, "check malloc succeeded", "val1 != NULL");

    memcpy(val1->scratch, "one", strlen("one"));
    memcpy(&val1->bytes, val1->scratch, 8);
    XXH64_state_t *xstate = XXH64_createState();
    XXH64_update(xstate, &val1, sizeof(map_value));
    uint64_t hash = XXH64_digest(xstate);
    // printf("%llu\n", hash);
    val1->hash = hash;
    ASSERT(hashmap_add(&map, "shell idea 1", OBJ_VAL(val1)),
           "add malloc'ed value to mapping",
           "hashmap_add(&map, \"shell idea 1 \", OBJ_VAL(val1))");
    value_t got = hashmap_get(&map, "shell idea 1");
    // printf("val:\t%s\t%llu\t%s\t%llu\n", val1->scratch, val1->bytes,
    //        "shell idea 1", val1->hash);
    // printf("got:\t%s\t%llu\t%s\t%llu\n", ((map_value *)AS_OBJ(got))->scratch,
    //        ((map_value *)AS_OBJ(got))->bytes, "shell idea 1",
    //        ((map_value *)AS_OBJ(got))->hash);
    ASSERT(memcmp((map_value *)AS_OBJ(got), val1, sizeof(map_value)) == 0,
           "validate malloc'ed value is stored correctly as reference",
           "memcmp((map_value *)AS_OBJ(got), val1, sizeof(map_value)) == 0");

    free(val1);
    got = hashmap_get(&map, "shell idea 1");
    // printf("val:\t%s\t%llu\t%s\t%llu\n", val1->scratch, val1->bytes,
    //        "shell idea 1", val1->hash);
    // printf("got:\t%s\t%llu\t%s\t%llu\n", ((map_value *)AS_OBJ(got))->scratch,
    //        ((map_value *)AS_OBJ(got))->bytes, "shell idea 1",
    //        ((map_value *)AS_OBJ(got))->hash);
    ASSERT(memcmp((map_value *)AS_OBJ(got), val1, sizeof(map_value)) == 0,
           "validate mapping's reference is also pointing to the free'd value",
           "memcmp((map_value *)AS_OBJ(got), val1, sizeof(map_value)) == 0");
}
