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

// concatenation macros
#define PDXCP_CONCAT_I(x, y) x ## y
#define PDXCP_CONCAT(x, y) PDXCP_CONCAT_I(x, y)

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

// C++ standard macro that works correctly with MSVC
#if defined(_MSVC_LANG)
#define PDXCP_CPLUSPLUS _MSVC_LANG
#else
#define PDXCP_CPLUSPLUS __cplusplus
#endif  // !defined(_MSVC_LANG)

// C++11 noexcept
#if defined(PDXCP_CPLUSPLUS)
#if PDXCP_CPLUSPLUS >= 201103L
#define PDXCP_NOEXCEPT noexcept
#else
#define PDXCP_NOEXCEPT
#endif  // PDXCP_CPLUSPLUS < 201103L
#else
#define PDXCP_NOEXCEPT
#endif  // !defined(PDXCP_CPLUSPLUS)

#endif  // PDXCP_COMMON_H_
