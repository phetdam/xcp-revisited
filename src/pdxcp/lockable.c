/**
 * @file lockable.c
 * @author Derek Huang
 * @brief C source for lockable types
 * @copyright MIT License
 */

#include "pdxcp/lockable.h"

#include <stdbool.h>
#include <stddef.h>

// TODO: maybe move *_DEF macros here to make them non-public

/**
 * Predefined lockable type getter definitions.
 */
PDXCP_LKABLE_GET_DEF(pdxcp_bool)
PDXCP_LKABLE_GET_DEF(int)
PDXCP_LKABLE_GET_DEF(size_t)

/**
 * Predefined lockable type by-value setter defintions.
 */
PDXCP_LKABLE_SET_V_DEF(pdxcp_bool)
PDXCP_LKABLE_SET_V_DEF(int)
PDXCP_LKABLE_SET_V_DEF(size_t)
