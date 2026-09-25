/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 datenbar
 */

/**
 * @file allocator.c 
 * @author datenbar
 * @date 26-09-2026
 * @brief Default allocator interface for allocator.h 
 * This file is only ever pulled into a hosted build (a normal program, not
 * a freestanding/kernel target). It is the one place in the library that
 * is allowed to call malloc/realloc/free directly, because it exists
 * specifically to provide the libc-backed implementation of the Allocator
 * interface. Structure code (vector.c, list.c, and so on) must never call
 * these functions directly; it always goes through the Allocator it was
 * given at creation time.
 */ 

#include "allocator.h"
#include <stdlib.h>

static void* default_alloc(size_t size) {
    return malloc(size);
}

static void* default_realloc(void* ptr, size_t old_size, size_t new_size) {
    (void)old_size; /* the standard library's realloc already tracks this internally */
    return realloc(ptr, new_size);
}

static void default_free(void* ptr, size_t size) {
    (void)size; /* likewise, unused here */
    free(ptr);
}

Allocator libstructs_default_allocator = {
    .alloc = default_alloc,
    .realloc = default_realloc,
    .free = default_free
};
