/**
 * @file lockable.g
 * @author Derek Huang
 * @brief C/C++ header for lockable types
 * @copyright MIT License
 */

#ifndef PDXCP_LOCKABLE_H_
#define PDXCP_LOCKABLE_H_

#include <errno.h>
#include <pthread.h>

#include "pdxcp/common.h"

PDXCP_EXTERN_C_BEGIN

/**
 * Macro for a lockable type's type name.
 *
 * A corresponding `PDXCP_LKABLE_DEF` is required to define the lockable type.
 *
 * @param type Target type
 */
#define PDXCP_LKABLE(type) PDXCP_CONCAT(lockable_, type)

/**
 * Define a type encapsulating a type member with a `pthread_mutex_t`.
 *
 * @param type Value member type
 */
#define PDXCP_LKABLE_DEF(type) \
  typedef struct { \
    type value; \
    pthread_mutex_t mutex; \
  } PDXCP_LKABLE(type);

/**
 * Macro for invoking a lockable type's getter.
 *
 * A corresponding `PDXCP_LKABLE_GET_DEF` is required to define the getter.
 *
 * @param type Target type
 */
#define PDXCP_LKABLE_GET(type) PDXCP_CONCAT(PDXCP_LKABLE(type), _get)

/**
 * Define the getter function for a lockable type.
 *
 * All getters are invoked using `PDXCP_LKABLE_GET(type)` and take two
 * arguments. The first is a `PDXCP_LKABLE(type) *lockable` input and the
 * second is a `type *out` output. Zero is returned on success, `-EINVAL` if
 * any arg is `NULL`, and other negative values for additional errors.
 *
 * @param type Type to define getter for
 */
#define PDXCP_LKABLE_GET_DEF(type) \
  int \
  PDXCP_LKABLE_GET(type)(PDXCP_LKABLE(type) *lockable, type *out) \
  { \
    int status; \
    /* args must be non-NULL */ \
    if (!lockable || !out) \
      return -EINVAL; \
    /* attempt lock */ \
    if ((status = pthread_mutex_lock(&lockable->mutex))) \
      return -status; \
    /* write and attempt unlock */ \
    *out = lockable->value; \
    return -pthread_mutex_unlock(&lockable->mutex); \
  }

/**
 * Macro for invoking a lockable type's by-value setter.
 *
 * A corresponding `PDXCP_LKABLE_SET_V_DEF` is required to define the setter.
 *
 * @param type Target type
 */
#define PDXCP_LKABLE_SET_V(type) PDXCP_CONCAT(PDXCP_LKABLE(type), _set_v)

/**
 * Define the by-value setter function for a lockable type.
 *
 * All setters are invoked using `PDXCP_LKABLE_SET_V(type)` and take two
 * arguments. The first is a `PDXCP_LKABLE(type) *lockable` input and the
 * second is a `type in` input. Zero is returned on success, `-EINVAL` if any
 * arg is `NULL`, with other negative values for any additional errors.
 */
#define PDXCP_LKABLE_SET_V_DEF(type) \
  int \
  PDXCP_LKABLE_SET_V(type)(PDXCP_LKABLE(type) *lockable, type in) \
  { \
    int status; \
    /* lockable must be non-NULL */ \
    if (!lockable) \
      return -EINVAL; \
    /* attempt lock */ \
    if ((status = pthread_mutex_lock(&lockable->mutex))) \
      return -status; \
    /* update and attempt unlock */ \
    lockable->value = in; \
    return -pthread_mutex_unlock(&lockable->mutex); \
  }

/**
 * Macro for invoking a lockable type's by-pointer setter.
 *
 * A corresponding `PDXCP_LKABLE_SET_P_DEF` is required to define the setter.
 */
#define PDXCP_LKABLE_SET_P(type) PDXCP_CONCAT(PDXCP_LKABLE(type), _set_p)

/**
 * Define the by-pointer setter function for a lockable type.
 *
 * All setters are invoked using `PDXCP_LKABLE_SET_P(type)` and take two
 * arguments. The first is a `PDXCP_LKABLE(type) *lockable` input and the
 * second is a `const type *in` input. Zero is returned on success, `-EINVAL`
 * if any arg is `NULL`, with other negative values for any additional errors.
 */
#define PDXCP_LKABLE_SET_P_DEF(type) \
  int \
  PDXCP_LKABLE_SET_P(type)(PDXCP_LKABLE(type) *lockable, const type *in) \
  { \
    int status; \
    /* args must be non-NULL */ \
    if (!lockable || !in) \
      return -EINVAL; \
    /* attempt lock */ \
    if ((status = pthread_mutex_lock(&lockable->mutex))) \
      return -status; \
    /* update and attempt unlock */ \
    lockable->value = *in; \
    return -pthread_mutex_unlock(&lockable->mutex); \
  }

PDXCP_EXTERN_C_END

#endif  // PDXCP_LOCKABLE_H_
