// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#ifndef GENERIC_VALUES_H

#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
//                            Generic/Dynamic Typing                          //
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t value_t;

// Leading bit ahead of the exponent, when QNaN boxing/tagging values into
// doubles the sign bit can be used as another information bit (50->51 free
// bits). However, using (QNAN | SIGN_BIT | value) as a mask to set the leading
// 12 bits plus the quiet QNaN bit and Intel's FP Indefinite bit gives enough
// room for 48 bit pointers to be stored in the double without any ambiguity as
// to whether they are a number or not.
#define SIGN_BIT                                                               \
    ((uint64_t)0x8000000000000000) // 1 << 63
                                   // 1000 0000 0000 0000 0000 ...

// QNaN is the 1-filled exponent for a double with the quiet (51) and Intel's
// FP Indefinite (50) bits enabled signifying a quiet NaN - leaving 1 leading
// and 50 trailing bits for type definition inside a valid double value.
#define QNAN                                                                   \
    ((uint64_t)0x7ffc000000000000) // ((1 << 13) - 1) << 50
                                   // 0111 1111 1111 1100 0000 ...

#define TAG_NIL 1   // 01.
#define TAG_FALSE 2 // 10.
#define TAG_TRUE 3  // 11.

#define FALSE_VAL ((value_t)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((value_t)(uint64_t)(QNAN | TAG_TRUE))
#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define NIL_VAL ((value_t)(uint64_t)(QNAN | TAG_NIL))
#define OBJ_VAL(obj) (value_t)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define AS_BOOL(val) (val == TRUE_VAL)
#define AS_NUMBER(val) _value_to_number(val)
#define AS_OBJ(val) ((void *)(uintptr_t)((val) & ~(SIGN_BIT | QNAN)))

#define IS_NIL(val) (val == NIL_VAL)
#define IS_BOOL(val)                                                           \
    ((val) | 1) == TRUE_VAL)
#define IS_NUMBER(val) ((val & QNAN) != QNAN)
#define IS_OBJ(val) ((val & (SIGN_BIT | QNAN)) == (SIGN_BIT | QNAN))

static inline double _value_to_number(value_t *val)
{
    double number;
    memcpy(&number, val, sizeof(value_t));
    return number;
}

static inline value_t _number_to_value(double num)
{
    value_t val;
    memcpy(&val, &num, sizeof(double));
    return val;
}

#endif /* ifndef GENERIC_VALUES_H */

#ifdef __cplusplus
};
#endif /* ifdef __cplusplus */
