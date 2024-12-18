// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "vector.h"

typedef struct pair_t {
    const char *key, *value;
} pair_t;

pair_t new_pair(const char *key, const char *value)
{
    pair_t pair = {
            .key = key,
            .value = value,
    };
    return pair;
}

char *print_pair(pair_t *pair)
{
    static char buf[128];
    sprintf(buf, "(K:\t%s\t:::\tV:\t%s)", pair->key, pair->value);
    return buf;
}

int main(int argc, char *argv[])
{
    VECTOR_DEFINE(pair_t);
    vector_pair_t pairs, pairs2, pairs3, pairs4;
    pair_t *test_pair;
    const char *key, *value;
    key = "test key";
    value = "test value";

    test_pair = (pair_t *)calloc(sizeof(pair_t), 1);
    *test_pair = new_pair(key, value);

    pairs = vector_init_type(pair_t, 10, (double)0.7);
    pairs2 = vector_init_type(pair_t, 5, (double)0.55);

    char *tmp_k, *tmp_v;

    // printf("SETTING: \n");
    vector_spos_type(&pairs2, pair_t, *test_pair, 1);
    // for (int j = 0; j < pairs2.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs2, pair_t, j);
    //     // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");
    vector_spos_type(&pairs2, pair_t, *test_pair, 3);
    // for (int j = 0; j < pairs2.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs2, pair_t, j);
    //     // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");
    vector_spos_type(&pairs2, pair_t, *test_pair, 4);
    // for (int j = 0; j < pairs2.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs2, pair_t, j);
    //     // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");

    pair_t got = vector_gpos_type(&pairs2, pair_t, 1);
    ASSERT(memcmp(&got, test_pair, sizeof(pair_t)) == 0,
           "vector spos value set correctly (index 1)",
           "memcmp(&(vector_gpos_type(&pairs2, pair_t, 1)), test_pair, \
sizeof(pair_t)) == 0 ");
    got = vector_gpos_type(&pairs2, pair_t, 3);
    ASSERT(memcmp(&got, test_pair, sizeof(pair_t)) == 0,
           "vector spos value set correctly (index 3)",
           "memcmp(&(vector_gpos_type(&pairs2, pair_t, 3)), test_pair, \
sizeof(pair_t)) == 0");
    got = vector_gpos_type(&pairs2, pair_t, 4);
    ASSERT(memcmp(&got, test_pair, sizeof(pair_t)) == 0,
           "vector spos value set correctly (index 4)",
           "memcmp(&(vector_gpos_type(&pairs2, pair_t, 4)), test_pair, \
sizeof(pair_t)) == 0");

    // printf("PUSHING: \n");
    test_pair = (pair_t *)calloc(sizeof(pair_t), 1);
    for (int i = 0; i < 10; ++i) {
        tmp_k = (char *)calloc(16, sizeof(char));
        tmp_v = (char *)calloc(16, sizeof(char));
        sprintf(tmp_k, "test key %d", i);
        sprintf(tmp_v, "test value %d", i);
        // print_pair(&pairs.array[i]);
        *test_pair = new_pair(tmp_k, tmp_v);
        vector_push_type(&pairs, pair_t, *test_pair);

        // for (int j = 0; j < pairs.capacity; ++j) {
        //     pair_t p = vector_gpos_type(&pairs, pair_t, j);
        //     printf("%d: %s\n", j, print_pair(&p));
        // }
        // printf("\n");

        ASSERT(pairs.size == i + 1, "vector size == no. elems (1 indexed)",
               "pairs.size == i + 1");
        pair_t p = vector_gpos_type(&pairs, pair_t, i);
        ASSERT(memcmp(&p, test_pair, sizeof(pair_t)) == 0,
               "vector push value set correctly (vector->size-1)",
               "memcmp(&pairs->array[pairs.size-1], test_pair, sizeof(pair_t)) \
== 0");
    }

    // printf("CONCAT: \n");
    pairs3 = vector_concat_type(&pairs, &pairs2, pair_t);
    // for (int j = 0; j < pairs3.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs3, pair_t, j);
    // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");
    ASSERT(pairs3.size == pairs.size + pairs2.size &&
                   pairs3.capacity == pairs.capacity + pairs2.capacity,
           "concat size and capacity checks",
           "pairs3.size, pairs3.capacity == pairs.size+pairs2.size, \
pairs.capacity+pairs2.capacity");
    ASSERT(memcmp(pairs3.array, pairs.array, pairs.capacity * sizeof(pair_t)) ==
                   0,
           "concat vector first part equal to vector 1",
           "memcmp(pairs3.array, pairs.array, sizeof(pair_t)*pairs.capacity) \
== 0");
    ASSERT(memcmp(pairs3.array + pairs.capacity, pairs2.array,
                  pairs2.capacity * sizeof(pair_t)) == 0,
           "concat vector second part equal to vector 2",
           "memcmp(pairs3.array + pairs.capacity, pairs2.array, sizeof(pair_t)*pairs2.capacity) \
== 0");

    // printf("CLONE: \n");
    pairs4 = vector_clone_type(&pairs3, pair_t);
    // for (int j = 0; j < pairs4.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs4, pair_t, j);
    // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");
    ASSERT(memcmp(pairs4.array, pairs3.array,
                  pairs4.capacity * sizeof(pair_t)) == 0,
           "cloned vector equals original vector",
           "memcmp(pairs4.array, pairs3.array, sizeof(pair_t)*pairs4.capacity) \
== 0");

    // printf("EMPTY: \n");
    vector_empty_type(&pairs4, pair_t);
    // for (int j = 0; j < pairs4.capacity; ++j) {
    //     pair_t p = vector_gpos_type(&pairs4, pair_t, j);
    // printf("%d: %s\n", j, print_pair(&p));
    // }
    // printf("\n");
    pair_t *empty = (pair_t *)calloc(pairs4.capacity, sizeof(pair_t));
    ASSERT(memcmp(pairs4.array, empty, pairs4.capacity * sizeof(pair_t)) == 0,
           "emptied vector has zeroed array",
           "memcmp(pairs4.array, calloc(1, pairs4.capacity), sizeof(pair_t)*pairs4.capacity) \
== 0");

    vector_free_type(&pairs, pair_t);
    vector_free_type(&pairs2, pair_t);
    vector_free_type(&pairs3, pair_t);
    vector_free_type(&pairs4, pair_t);

    return EXIT_SUCCESS;
}
