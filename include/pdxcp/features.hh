/**
 * @file features.hh
 * @author Derek Huang
 * @brief C++ header for detecting compile-time features
 * @copyright MIT License
 */

#ifndef PDXCP_FEATURES_HH_
#define PDXCP_FEATURES_HH_

#include "pdxcp/features.h"

// check for cxxabi.h for Itanium C++ ABI
#ifdef PDXCP_HAS_INCLUDE_AVAILABLE
#if __has_include(<cxxabi.h>)
#define PDXCP_HAS_ITANIUM_ABI
#endif  // !__has_include(<cxxabi.h>)
#endif  // PDXCP_HAS_INCLUDE_AVAILABLE

#endif  // PDXCP_FEATURES_HH_
