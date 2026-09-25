/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 datenbar
 */

/**
 * @file allocator.h
 * @author datenbar
 * @date 26-09-2026
 * @brief Allocator interface every structure in this library uses instead of calling malloc/realloc/free directly.
 * This is what lets the same structure code run in a normal hosted program
 * or in a freestanding environment (embedded, kernel) without being
 * rewritten, only the Allocator implementation changes.
 */

#ifndef LIBSTRUCTS_ALLOCATOR_H
#define LIBSTRUCTS_ALLOCATOR_H

#include <stddef.h>

/**
 * @brief A set of function pointers a structure uses to request and
 *        release memory.
 *
 * realloc takes old_size explicitly because not every allocator tracks
 * allocation sizes internally the way the standard library's malloc does.
 * A bump allocator or a fixed-block pool allocator, for instance, may need
 * to know the old size to do the right thing on grow or shrink.
 */
typedef struct {
  void *(*alloc)(size_t size); /**< Allocate size bytes, returning NULL on failure. */
  void *(*realloc)(void *ptr, size_t old_size, size_t new_size); /**< Resize an existing allocation */
  void (*free)(void *ptr, size_t size); /**< Release a previously allocated block. */
} Allocator;

/**
 * @brief Standard-library-backed allocator for normal hosted programs.
 */
extern Allocator libstructs_default_allocator;

#endif
