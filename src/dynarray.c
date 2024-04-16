/**
 * @file dynarray.c
 * @author Derek Huang
 * @brief Expert C Programming (p286): dynamic array expansion
 * @copyright MIT License
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdxcp/warnings.h"

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
} byte_vector;

/**
 * Initialize a `byte_vector` structure.
 *
 * This is equivalent to performing `byte_vector v = {0}` to zero the struct.
 *
 * @param vec Byte vector to initialize
 */
static inline void
byte_vector_init(byte_vector *vec)
{
  vec->data = NULL;
  vec->size = 0;
  vec->capacity = 0;
}

/**
 * Destroy a `byte_vector` structure.
 *
 * If the struct is to be reused, `byte_vector_init` must first be called.
 *
 * @param vec Byte vector to destroy
 */
static inline void
byte_vector_destroy(byte_vector *vec)
{
  free(vec->data);
}

/**
 * Expand the `byte_vector` data buffer.
 *
 * If the `byte_vector` is newly initialized with `byte_vector_init`, the size
 * is zero, and the expansion is to 16 bytes. Otherwise, capacity is doubled.
 *
 * @param vec Byte vector to expand
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
static bool
byte_vector_expand(byte_vector *vec)
{
  // expand, either to 16 bytes if empty or doubling the capacity
  size_t new_capacity = (!vec->capacity) ? 16 : 2 * vec->capacity;
  unsigned char *new_data = realloc(vec->data, new_capacity);
  // error, this is ENOMEM
  if (!new_data)
    return false;
  // success, update data and capacity
  vec->data = new_data;
  vec->capacity = new_capacity;
  return true;
}

/**
 * Add a single byte to the `byte_vector.`
 *
 * @param vec Byte vector
 * @param c Byte to add
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
static inline bool
byte_vector_add_1(byte_vector *vec, unsigned char c)
{
  // expand if necessary and report error
  if (vec->size == vec->capacity && !byte_vector_expand(vec))
    return false;
  // add to buffer + increment size
  vec->data[vec->size++] = c;
  return true;
}

/**
 * Add a block of bytes to the `byte_vector`.
 *
 * @param vec Byte vector
 * @param buf Buffer of bytes to add
 * @param buf_size Number of bytes in the buffer
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
static bool
byte_vector_add_n(byte_vector *vec, unsigned char *buf, size_t buf_size)
{
  // repeatedly expand as necessary
  while (vec->capacity <= vec->size + buf_size)
    if (!byte_vector_expand(vec))
      return false;
  // copy buf and update size
  memcpy(vec->data + vec->size, buf, buf_size);
  vec->size += buf_size;
  return true;
}

/**
 * Print the size and capacity of the `byte_vector` with fixed field width.
 *
 * @param vec Byte vector whose size and capacity are to be reported
 * @param u_width Width of the print field
 */
static inline void
report_sizes(byte_vector *vec, unsigned int u_width)
{
  printf(
    "vec->size: %*zu, vec->capacity: %*zu\n",
    u_width,
    vec->size,
    u_width,
    vec->capacity
  );
}

/**
 * Add a fragmented message to the `byte_vector`.
 *
 * The concatentation of `c1, `s1, `c2`, `s2` will be added, and the change in
 * the `byte_vector` size and capacity will be printed to `stdout`.
 *
 * @note This function has been purposefully made to trigger reallocations.
 *
 * @param vec Byte vector
 * @param u_width Field width to use when printing size and capacity
 * @param c1 First character to add
 * @param s1 First null-terminated string to add
 * @param c2 Second character to add
 * @param s2 Second null-terminated string to add
 * @param include_null `true` to include a final null terminator
 * @returns `true` on success, `false` on error (`errno` is ENOMEM)
 */
static bool
add_fragment(
  byte_vector *vec,
  size_t u_width,
  char c1,
  const char *s1,
  char c2,
  const char *s2,
  bool include_null)
{
  // add c1 + print sizes
  if (!byte_vector_add_1(vec, c1)) {
    fprintf(stderr, "Error: Failed to reallocate when adding '%c'\n", c1);
    return false;
  }
  report_sizes(vec, u_width);
  // add s1 + print sizes
  if (!byte_vector_add_n(vec, (unsigned char *) s1, strlen(s1))) {
    fprintf(stderr, "Error: Failed to reallocate when adding \"%s\"\n", s1);
    return EXIT_FAILURE;
  }
  report_sizes(vec, u_width);
  // add c2 + print sizes
  if (!byte_vector_add_1(vec, c2)) {
    fprintf(stderr, "Error: Failed to reallocate when adding '%c'\n", c2);
    return false;
  }
  report_sizes(vec, u_width);
  // add s2 + print sizes
  if (!byte_vector_add_n(vec, (unsigned char *) s2, strlen(s2))) {
    fprintf(stderr, "Error: Failed to reallocate when adding \"%s\"\n", s2);
    return false;
  }
  // add final null terminator if requested
  if (include_null && !byte_vector_add_1(vec, '\0')) {
    fprintf(stderr, "Error: Failed to reallocate when adding \\0\n");
    return false;
  }
  report_sizes(vec, u_width);
  return true;
}

int
main(void)
{
  // printf field with for size/capacity
  unsigned int u_width = 4u;
  // new initialized byte_vector + print initial (zero) size and capacity
  byte_vector vec;
  byte_vector_init(&vec);
  report_sizes(&vec, u_width);
  // exit status
  int status = EXIT_SUCCESS;
  // add message fragments. this is part of the Special Task Force A-01 creed.
  // we also disable -Wparentheses since the assignment is clear here
PDXCP_GNU_WARNING_PUSH()
PDXCP_GNU_WARNING_DISABLE(-Wparentheses)
  if (
    status = !add_fragment(
      &vec,
      u_width,
      'A',
      "chieve your mission wi",
      't',
      "h all your might\n",
      false
    )
  )
    goto cleanup;
  if (
    status = !add_fragment(
      &vec,
      u_width,
      'D',
      "espair not till y",
      'o',
      "ur last breath\n",
      false
    )
  )
    goto cleanup;
  if (
    status = !add_fragment(
      &vec,
      u_width,
      'M',
      "ake you",
      'r',
      " death count",
      true
    )
  )
    goto cleanup;
PDXCP_GNU_WARNING_POP()
  // print the message, destroy, exit
  printf("%s\n", (const char *) vec.data);
cleanup:
  byte_vector_destroy(&vec);
  return status;
}
