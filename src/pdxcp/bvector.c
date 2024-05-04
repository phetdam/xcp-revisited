/**
 * @file bvector.c
 * @author Derek Huang
 * @brief C source for a dynamic byte vector
 * @copyright MIT License
 */

#include "pdxcp/bvector.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pdxcp/common.h"

void
pdxcp_bvector_init(pdxcp_bvector *vec) PDXCP_NOEXCEPT
{
  vec->data = NULL;
  vec->size = 0;
  vec->capacity = 0;
}

void
pdxcp_bvector_destroy(pdxcp_bvector *vec) PDXCP_NOEXCEPT
{
  free(vec->data);
}

bool
pdxcp_bvector_expand(pdxcp_bvector *vec) PDXCP_NOEXCEPT
{
  // expand, either to PDXCP_BVECTOR_ZERO_EXPAND_SIZE if empty or 2 * capacity
  size_t new_capacity = (!vec->capacity) ?
    PDXCP_BVECTOR_ZERO_EXPAND_SIZE : 2 * vec->capacity;
  unsigned char *new_data = realloc(vec->data, new_capacity);
  // error, this is ENOMEM
  if (!new_data)
    return false;
  // success, update data and capacity
  vec->data = new_data;
  vec->capacity = new_capacity;
  return true;
}

bool
pdxcp_bvector_add(pdxcp_bvector *vec, unsigned char c) PDXCP_NOEXCEPT
{
  // expand if necessary and report error
  if (vec->size == vec->capacity && !pdxcp_bvector_expand(vec))
    return false;
  // add to buffer + increment size
  vec->data[vec->size++] = c;
  return true;
}

bool
pdxcp_bvector_add_n(
  pdxcp_bvector *vec, unsigned char *buf, size_t buf_size) PDXCP_NOEXCEPT
{
  // repeatedly expand as necessary
  while (vec->capacity <= vec->size + buf_size)
    if (!pdxcp_bvector_expand(vec))
      return false;
  // copy buf and update size
  memcpy(vec->data + vec->size, buf, buf_size);
  vec->size += buf_size;
  return true;
}
