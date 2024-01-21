/**
 * @file string.h
 * @author Derek Huang
 * @brief C/C++ header for string helpers
 * @copyright MIT License
 */

#ifndef PDXCP_STRING_H_
#define PDXCP_STRING_H_

#include <stdbool.h>
#include <string.h>

#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Check that two strings are equal.
 *
 * @param a First null-terminated string
 * @param b Second null-terminated string
 */
PDXCP_INLINE bool
pdxcp_streq(const char *a, const char *b) PDXCP_NOEXCEPT
{
  return !strcmp(a, b);
}

PDXCP_EXTERN_C_END

#endif  // PDXCP_STRING_H_
