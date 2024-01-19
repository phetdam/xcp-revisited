/**
 * @file error.h
 * @author Derek Huang
 * @brief C/C++ header for error helpers
 * @copyright MIT License
 */

#ifndef PDXCP_ERROR_H_
#define PDXCP_ERROR_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MinGW provides unistd.h
#if !defined(_WIN32) || defined(__MINGW32__)
#include <unistd.h>
#endif  // defined(_WIN32) && !defined(__MINGW32__)

#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Print message for the error code and exit with `exit(EXIT_FAILURE)`.
 *
 * @param err Error code, e.g. from `errno`
 */
PDXCP_INLINE void
pdxcp_error_exit(int err)
{
  fprintf(stderr, "Error: %s\n", strerror(err));
  exit(EXIT_FAILURE);
}

/**
 * Print message with custom message format for the error code and exit.
 *
 * @param err Error code, e.g. from `errno`
 * @param fmt `printf` style message format
 * @param ... Variadic args to pass to `fprintf`
 */
#define PDXCP_ERROR_EXIT_EX(err, fmt, ...) \
  do { \
    fprintf(stderr, "Error: " fmt ": %s\n", __VA_ARGS__, strerror(err)); \
    exit(EXIT_FAILURE); \
  } \
  while (0)

/**
 * Print message for the error code and exit with `_exit(EXIT_FAILURE)`.
 *
 * @param err Error code, e.g. from `errno`
 */
PDXCP_INLINE void
pdxcp_error_exit_now(int err)
{
  fprintf(stderr, "Error: %s\n", strerror(err));
  _exit(EXIT_FAILURE);
}

/**
 * Exit using `pdxcp_error_exit(errno)` if `expr` is `false`.
 *
 * @param expr Expression to evaluate
 */
#define PDXCP_ERRNO_EXIT_IF(expr) \
  if (expr) \
    pdxcp_error_exit(errno)

/**
 * Exit using `PDXCP_ERROR_EXIT_EX(errno, fmt, ...)` if `expr` is `false`.
 *
 * @param expr Expression to evaluate
 * @param fmt `printf` style message format
 * @param ... Variadic args to pass to `fprintf`
 */
#define PDXCP_ERRNO_EXIT_EX_IF(expr, fmt, ...) \
  if (expr) \
    PDXCP_ERROR_EXIT_EX(errno, fmt, __VA_ARGS__)

/**
 * Exit using `pdxcp_error_exit_now(errno)` if `expr` is `false`.
 *
 * @param expr Expression to evaluate
 */
#define PDXCP_ERRNO_EXIT_NOW_IF(expr) \
  if (expr) \
    pdxcp_error_exit_now(errno)

PDXCP_EXTERN_C_END

#endif  // PDXCP_ERROR_H_
