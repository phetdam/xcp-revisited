/**
 * @file cdcl_common.h
 * @author Derek Huang
 * @brief C/C++ header for C declaration parser common macros
 * @copyright MIT License
 */

#ifndef PDXCP_CDCL_COMMON_H_
#define PDXCP_CDCL_COMMON_H_

#include "pdxcp/common.h"

/**
 * Macro defining case statement returning the stringified value.
 *
 * @param value Enum, integral, etc. value
 */
#define PDXCP_STRING_CASE(value) \
  case value: \
    return PDXCP_STRINGIFY(value)

#endif  // PDXCP_CDCL_COMMON_H_
