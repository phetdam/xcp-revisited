/**
 * @file demangle.hh
 * @author Derek Huang
 * @brief C++ header for demangling type names
 * @copyright MIT License
 */

#ifndef PDXCP_DEMANGLE_HH_
#define PDXCP_DEMANGLE_HH_

#include <memory>
#include <string>
#include <stdexcept>
#include <typeinfo>

#include "pdxcp/features.hh"

#ifdef PDXCP_HAS_ITANIUM_ABI
#include <cxxabi.h>

#include "pdxcp/memory.hh"
#endif  // PDXCP_HAS_ITANIUM_ABI

namespace pdxcp {

/**
 * Return a demangled version of the mangled type name.
 *
 * On platforms where the compiler follows the Itanium ABI this will properly
 * demangle the name. Otherwise, the function returns its argument verbatim.
 *
 * @param mangled_name Mangled type name
 */
inline std::string demangled_name(const char* mangled_name)
{
#if defined(PDXCP_HAS_ITANIUM_ABI)
  // abi::__cxa_demangle return status
  int status;
  // demangle mangled_name, owning memory with unique_ptr. since
  // abi::__cxa_demangle allocates with malloc() we delete with free()
  std::unique_ptr<char[], malloc_deleter> type_name{
    abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status)
  };
  // check status if type_name is nullptr
  if (!type_name) {
    switch (status) {
      case -1:
        throw std::runtime_error{"memory allocation failure during demangling"};
      case -2:
        throw std::runtime_error{
          "mangled_name is not a valid Itanium ABI mangled type name"
        };
      // no -3 case for invalid arguments as both buf, n are nullptr
      default:
        throw std::runtime_error{
          "type name demangling failed with status " + std::to_string(status)
        };
    }
  }
  // return from null-terminated string
  return type_name.get();
#else
  return mangled_name;
#endif  // !defined(PDXCP_HAS_ITANIUM_ABI)
}

/**
 * Return the demangled type name as a `std::string` from a type or expression.
 *
 * @param x Type or expression to get string type name from
 */
#define PDXCP_DEMANGLED_NAME(x) pdxcp::demangled_name(typeid(x).name())

}  // namespace pdxcp

#endif  // PDXCP_DEMANGLE_HH_
