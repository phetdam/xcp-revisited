/**
 * @file features.h
 * @author Derek Huang
 * @brief C/C++ header for detecting platform-specific features
 * @copyright MIT License
 */

#ifndef PDXCP_FEATURES_H_
#define PDXCP_FEATURES_H_

// GNU/Linux features
#ifdef _GNU_SOURCE
#define PDXCP_GNU
#endif  // _GNU_SOURCE
// BSD-derived features
#ifdef _BSD_SOURCE
#define PDXCP_BSD
#endif  // _BSD_SOURCE
// "default" definitions
#ifdef _DEFAULT_SOURCE
#define PDXCP_DEFAULT
#endif  // _DEFAULT_SOURCE
// various POSIX features
#ifdef _POSIX_C_SOURCE
// POSIX.1-1990 + ISO C features
#if _POSIX_C_SOURCE >= 1
#define PDXCP_POSIX_C_1
#define PDXCP_POSIX_C_1990
#endif  // _POSIX_C_SOURCE < 1
// POSIX.2-1992 features
#if _POSIX_C_SOURCE >= 2
#define PDXCP_POSIX_C_2
#define PDXCP_POSIX_C_1992
#endif  // _POSIX_C_SOURCE < 2
// POSIX.1b real-time extension features
#if _POSIX_C_SOURCE >= 199309L
#define PDXCP_POSIX_C_1B
#define PDXCP_POSIX_C_1993
#endif  // _POSIX_C_SOURCE < 199309L
// POSIX.1c threads extension features
#if _POSIX_C_SOURCE >= 199506L
#define PDXCP_POSIX_C_1C
#define PDXCP_POSIX_C_1995
#endif  // _POSIX_C_SOURCE < 199506L
// POSIX.1-2001 features
#if _POSIX_C_SOURCE >= 200112L
#define PDXCP_POSIX_C_2001
#endif  // _POSIX_C_SOURCE < 200112L
// POSIX.1-2008 features
#if _POSIX_C_SOURCE >= 200809L
#define PDXCP_POSIX_C_2008
#endif  // _POSIX_C_SOURCE < 200809L
#endif  // _POSIX_C_SOURCE

#endif  // PDXCP_FEATURES_H_
