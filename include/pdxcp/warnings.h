/**
 * @file warnings.h
 * @author Derek Huang
 * @brief C/C++ header for compiler warning controls
 * @copyright MIT License
 */

#ifndef PDXCP_WARNINGS_H_
#define PDXCP_WARNINGS_H_

#include "pdxcp/common.h"

// macros for disabling GCC/Clang warnings
#if defined(__GNUC__)
#define PDXCP_GNU_WARNING_PUSH() _Pragma("GCC diagnostic push")
#define PDXCP_GNU_WARNING_DISABLE(w) \
  _Pragma(PDXCP_STRINGIFY(GCC diagnostic ignored PDXCP_STRINGIFY(w)))
#define PDXCP_GNU_WARNING_POP() _Pragma("GCC diagnostic pop")
#else
#define PDXCP_GNU_WARNING_PUSH()
#define PDXCP_GNU_WARNING_DISABLE(w)
#define PDXCP_GNU_WARNING_POP()
#endif  // !defined(__GNUC__)

#endif  // PDXCP_WARNINGS_H_
