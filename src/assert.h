// SPDX-License-Identifier: (BSD-3-Clause)
// Copyright 2024 (c) Harry Law <h5law>
// https://github.com/h5law/hashmap

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ASSERT_H_MACRO

#define ASSERT(cond, name, str)                                                \
    ({                                                                         \
        char *icon[2] = {"❌", "✅"};                                          \
        char *statuses[2] = {"FAILED", "PASSED"};                              \
        int status = cond == true ? 1 : 0;                                     \
        printf("(%s)[L%d] %s (%s)\n\tTEST :: %s\n", __FILE__, __LINE__,        \
               statuses[status], icon[status], name);                          \
        if (status == 0)                                                       \
            printf("\tCOND :: %s\n", str);                                     \
        printf("\n");                                                          \
        status;                                                                \
    })

#endif /* ifndef ASSERT_H_MACRO */

#ifdef __cplusplus
} /* extern "C" */
#endif
