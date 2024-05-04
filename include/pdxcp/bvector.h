/**
 * @file bvector.h
 * @author Derek Huang
 * @brief C/C++ header for a dynamic byte vector
 * @copyright MIT License
 */

#ifndef PDXCP_BVECTOR_H_
#define PDXCP_BVECTOR_H_

#include <stdbool.h>
#include <stddef.h>

#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Struct for a dynamic array managing a buffer of bytes.
 *
 * @note This is a more sophisticated dynamic array implementation than the
 *  simple one given in the book. setjmp/longjmp is also unnecessary here.
 *
 * @param data Pointer to byte data
 * @param size Number of bytes (elements) being managed
 * @param capacity Total number of bytes allocated, greater or equal to `size`
 */
typedef struct {
  unsigned char *data;
  size_t size;
  size_t capacity;
} pdxcp_bvector;

/**
 * Number of bytes in a byte vector on first expansion from zero capacity.
 */
#define PDXCP_BVECTOR_ZERO_EXPAND_SIZE 16

/**
 * Initialize a `pdxcp_bvector` structure.
 *
 * This is equivalent to using `pdxcp_bvector v = {0}` to zero the struct.
 *
 * @param vec Byte vector to initialize
 */
void
pdxcp_bvector_init(pdxcp_bvector *vec) PDXCP_NOEXCEPT;

/**
 * Destroy a `pdxcp_bvector` structure.
 *
 * If the struct is to be reused, `pdxcp_bvector_init` must first be called.
 *
 * @param vec Byte vector to destroy
 */
void
pdxcp_bvector_destroy(pdxcp_bvector *vec) PDXCP_NOEXCEPT;

/**
 * Expand the `pdxcp_bvector` data buffer.
 *
 * If the byte vector is newly initialized with `pdxcp_bvector_init`, the size
 * is zero and the expansion is to `PDXCP_BVECTOR_ZERO_EXPAND_SIZE` bytes.
 * Otherwise, the byte vector capacity is instead doubled.
 *
 * @param vec Byte vector to expand
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
bool
pdxcp_bvector_expand(pdxcp_bvector *vec) PDXCP_NOEXCEPT;

/**
 * Add a single byte to the `pdxcp_bvector.`
 *
 * @param vec Byte vector
 * @param c Byte to add
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
bool
pdxcp_bvector_add(pdxcp_bvector *vec, unsigned char c) PDXCP_NOEXCEPT;

/**
 * Add a block of bytes to the `pdxcp_bvector`.
 *
 * @param vec Byte vector
 * @param buf Buffer of bytes to add
 * @param buf_size Number of bytes in the buffer
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
bool
pdxcp_bvector_add_n(
  pdxcp_bvector *vec, unsigned char *buf, size_t buf_size) PDXCP_NOEXCEPT;

PDXCP_EXTERN_C_END

#endif  // PDXCP_BVECTOR_H_
