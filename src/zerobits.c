/**
 * @file zerobits.c
 * @author Derek Huang
 * @brief Expert C Programming (p258): check that 0.0 and 0 have the same bits
 * @copyright MIT License
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * 64-bit unsigned int `printf` length specifier.
 *
 * This is used to prevent `printf` complaining when `long` is mixed with
 * `long long` in terms of type formatting specification
 */
#if ULONG_MAX == 0xFFFFFFFFFFFFFFFF
#define U64_LENSPEC "l"
#elif ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
#define U64_LENSPEC "ll"
#else
#error "zerobits.c: neither unsigned long nor unsigned long long are 64 bits"
#endif  // ULONG_MAX != (1 << 64) - 1 && ULLONG_MAX != (1 << 64) - 1

int
main(void)
{
  printf(
    "0.0 bits are 0x%" U64_LENSPEC "x, 0 bits are 0x%" U64_LENSPEC "x, "
    "bits are %s\n",
    (uint64_t) 0.,
    (uint64_t) 0u,
    ((uint64_t) 0. ^ (uint64_t) 0u) ? "different" : "same"
  );
  return EXIT_SUCCESS;
}
