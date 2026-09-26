#ifndef LIBSTRUCTS_PANIC_H
#define LIBSTRUCTS_PANIC_H

/**
 * @file panic.h
 * @brief LS_ASSERT is what structure code uses instead of a raw assert().
 *
 * In a normal hosted build it's just assert() from the standard library.
 * In a kernel/freestanding build (compiled with -DLIBSTRUCTS_KERNEL_BUILD),
 * it calls kernel_panic() instead, which the kernel itself must provide.
 */

#ifdef LIBSTRUCTS_KERNEL_BUILD
void kernel_panic(const char* msg);
#define LS_ASSERT(cond)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    do {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
        if (!(cond))                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   \
            kernel_panic(#cond);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    } while (0)
#else
#include <assert.h>
#define LS_ASSERT(cond) assert(cond)
#endif

#endif
