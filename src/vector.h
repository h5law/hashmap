// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GENERIC_VECTOR_H

#include <checkint.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
//                              Dynamic Array Types                           //
////////////////////////////////////////////////////////////////////////////////

#define VECTOR_DEFINE(T)                                                       \
    typedef struct vector_##T {                                                \
        int size, end_ptr, capacity;                                           \
        double load_factor_pct;                                                \
        T *array;                                                              \
    } vector_##T;

#define vector_init_type(T, cap, limit_pct)                                    \
    ({                                                                         \
        vector_##T vec = {0};                                                  \
        if ((double)limit_pct > 0.0 || (double)limit_pct <= 1.0) {             \
            vec.size = 0;                                                      \
            vec.end_ptr = 0;                                                   \
            vec.capacity = (int)cap;                                           \
            vec.load_factor_pct = (double)limit_pct;                           \
            vec.array = (T *)calloc(cap, sizeof(T));                           \
        }                                                                      \
        vec;                                                                   \
    })

#define vector_gpos_type(vec, T, index)                                        \
    ({                                                                         \
        T val = {0};                                                           \
        if ((int)index >= 0 && (int)index < ((vector_##T *)vec)->capacity)     \
            memcpy(&val, ((vector_##T *)vec)->array + (int)index, sizeof(T));  \
        val;                                                                   \
    })

#define vector_spos_type(vec, T, val, index)                                   \
    ({                                                                         \
        bool success = true;                                                   \
        if ((int)index < 0 || (int)index >= ((vector_##T *)vec)->capacity)     \
            success = false;                                                   \
        else {                                                                 \
            int err = CHECKINT_NO_ERROR;                                       \
            int new_size =                                                     \
                    check_int32_add(((vector_##T *)vec)->size, 1, &err);       \
            if (err != CHECKINT_NO_ERROR)                                      \
                success = false;                                               \
            else {                                                             \
                if ((double)new_size >                                         \
                    (double)(((vector_##T *)vec)->capacity) *                  \
                            ((vector_##T *)vec)->load_factor_pct) {            \
                    int new_cap = check_int32_mul(                             \
                            (int)((vector_##T *)vec)->capacity, 2, &err);      \
                    if (err != CHECKINT_NO_ERROR)                              \
                        success = false;                                       \
                    else                                                       \
                        success = vector_resize_type((vector_##T *)vec, T,     \
                                                     new_cap);                 \
                }                                                              \
                if (success) {                                                 \
                    ((vector_##T *)vec)->end_ptr =                             \
                            (int)index >= ((vector_##T *)vec)->end_ptr         \
                                    ? index + 1                                \
                                    : ((vector_##T *)vec)->end_ptr;            \
                    ((vector_##T *)vec)->size = new_size;                      \
                    memcpy(((vector_##T *)vec)->array + (int)index,            \
                           (T *)(&(val)), sizeof(T));                          \
                };                                                             \
            };                                                                 \
        };                                                                     \
        success;                                                               \
    })

#define vector_push_type(vec, T, val)                                          \
    ({                                                                         \
        bool success = true;                                                   \
        int err = CHECKINT_NO_ERROR;                                           \
        int new_size = check_int32_add(((vector_##T *)vec)->size, 1, &err);    \
        if (err != CHECKINT_NO_ERROR)                                          \
            success = false;                                                   \
        int index = ((vector_##T *)vec)->end_ptr;                              \
        if (success) {                                                         \
            if ((double)new_size >                                             \
                (double)(((vector_##T *)vec)->capacity) *                      \
                        ((vector_##T *)vec)->load_factor_pct) {                \
                int new_cap = check_int32_mul(                                 \
                        (int)((vector_##T *)vec)->capacity, 2, &err);          \
                if (err != CHECKINT_NO_ERROR)                                  \
                    success = false;                                           \
                else                                                           \
                    success =                                                  \
                            vector_resize_type((vector_##T *)vec, T, new_cap); \
            }                                                                  \
            memcpy(((vector_##T *)vec)->array + index, (T *)(&(val)),          \
                   sizeof(T));                                                 \
            ((vector_##T *)vec)->size = new_size;                              \
            ((vector_##T *)vec)->end_ptr =                                     \
                    check_int32_add(((vector_##T *)vec)->end_ptr, 1, &err);    \
            if (err != CHECKINT_NO_ERROR)                                      \
                success = false;                                               \
        }                                                                      \
        success;                                                               \
    })

#define vector_clone_type(vec, T)                                              \
    ({                                                                         \
        vector_##T dup =                                                       \
                vector_init_type(T, ((vector_##T *)vec)->capacity,             \
                                 ((vector_##T *)vec)->load_factor_pct);        \
        dup.size = ((vector_##T *)vec)->size;                                  \
        memcpy(dup.array, ((vector_##T *)vec)->array,                          \
               ((vector_##T *)vec)->capacity * sizeof(T));                     \
        dup;                                                                   \
    })

#define vector_concat_type(vec1, vec2, T)                                      \
    ({                                                                         \
        double x = ((vector_##T *)vec1)->load_factor_pct;                      \
        double y = ((vector_##T *)vec2)->load_factor_pct;                      \
        double lim = fmin(x, y);                                               \
        vector_##T new_vec =                                                   \
                vector_init_type(T,                                            \
                                 ((vector_##T *)vec1)->capacity +              \
                                         ((vector_##T *)vec2)->capacity,       \
                                 lim);                                         \
        new_vec.size =                                                         \
                ((vector_##T *)vec1)->size + ((vector_##T *)vec2)->size;       \
        memcpy((&new_vec)->array, ((vector_##T *)vec1)->array,                 \
               ((vector_##T *)vec1)->capacity * sizeof(T));                    \
        memcpy((&new_vec)->array + ((vector_##T *)vec1)->capacity,             \
               ((vector_##T *)vec2)->array,                                    \
               ((vector_##T *)vec2)->capacity * sizeof(T));                    \
        new_vec;                                                               \
    })

#define vector_resize_type(vec, T, new_cap)                                    \
    ({                                                                         \
        T *new_array = (T *)calloc((int)new_cap, sizeof(T));                   \
        bool success = new_array != NULL;                                      \
        if (success) {                                                         \
            int x = (int)new_cap;                                              \
            int y = ((vector_##T *)vec)->capacity;                             \
            int len = x > y ? y : x;                                           \
            memcpy(new_array, (T *)((vector_##T *)vec)->array,                 \
                   len * sizeof(T));                                           \
            ((vector_##T *)vec)->array = new_array;                            \
            ((vector_##T *)vec)->capacity = new_cap;                           \
        }                                                                      \
        success;                                                               \
    })

#define vector_empty_type(vec, T)                                              \
    ({                                                                         \
        bool success = false;                                                  \
        memset(((vector_##T *)vec)->array, 0,                                  \
               ((vector_##T *)vec)->capacity * sizeof(T));                     \
        ((vector_##T *)vec)->size = 0;                                         \
        success;                                                               \
    })

#define vector_free_type(vec, T)                                               \
    ({                                                                         \
        free((T *)(((vector_##T *)vec)->array));                               \
        (((vector_##T *)vec)->array) = NULL;                                   \
        ((vector_##T *)vec)->size = ((vector_##T *)vec)->capacity = 0;         \
    })

#endif /* ifndef GENERIC_VECTOR_H */

#ifdef __cplusplus
} /* extern "C" */
#endif
