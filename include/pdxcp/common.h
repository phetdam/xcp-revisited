/**
 * @file common.h
 * @author Derek Huang
 * @brief C/C++ header for commmon macros
 * @copyright MIT License
 */

#ifndef PDXCP_COMMON_H_
#define PDXCP_COMMON_H_

// stringification macros
#define PDXCP_STRINGIFY_I(x) #x
#define PDXCP_STRINGIFY(x) PDXCP_STRINGIFY_I(x)

// extern "C" guards
#if defined(__cplusplus)
#define PDXCP_EXTERN_C_BEGIN extern "C" {
#define PDXCP_EXTERN_C_END }
#else
#define PDXCP_EXTERN_C_BEGIN
#define PDXCP_EXTERN_C_END
#endif  // !defined(__cplusplus)

// use C++ inline if possible
#if defined(__cplusplus)
#define PDXCP_INLINE inline
#else
#define PDXCP_INLINE static inline
#endif  // !defined(__cplusplus)

#endif  // PDXCP_COMMON_H_
