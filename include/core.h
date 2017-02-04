#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <limits.h>

#include <stdio.h>
#include <string.h>

/// Swaps the given values using `t` as temporary variable.
void mem_swap(void* a, void* b, void* t, size_t n);

/// Fires an error if `ptr` is `NULL`.
void assert_alloc(void* ptr);

/// Is `n` a power of two ?
bool is_power_of_two(size_t n);

/// Returns the power of two following `n`.
size_t next_power_of_two(size_t n);

/// Returns `false` if an overflow occured,
/// `p` is filled with the power of two following `n`.
bool checked_next_power_of_two(size_t n, size_t* p);

#endif // CORE_H
