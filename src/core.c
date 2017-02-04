#include "core.h"

void mem_swap(void* a, void* b, void* t, size_t n) {
    memcpy(t, a, n);
    memcpy(a, b, n);
    memcpy(b, t, n);
}

void assert_alloc(void* ptr) {
    if (!ptr) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }
}

bool is_power_of_two(size_t n) {
    // bits.stephan-brumme.com
    return ((n & (n - 1)) == 0) && (n != 0);
}

size_t next_power_of_two(size_t n) {
    // bits.stephan-brumme.com
    n--;
    n |= (n >> 1);  //  2 bits
    n |= (n >> 2);  //  4 bits
    n |= (n >> 4);  //  8 bits
    n |= (n >> 8);  // 16 bits
    n |= (n >> 16); // 32 bits
    n |= (n >> 32); // 64 bits
    n++;
    return n;
}

bool checked_next_power_of_two(size_t n, size_t* p) {
    *p = next_power_of_two(n);
    return (n <= *p);
}
