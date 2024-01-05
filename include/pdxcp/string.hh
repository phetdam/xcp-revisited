/**
 * @file string.hh
 * @author Derek Huang
 * @brief C++ header for string handling extensions
 * @copyright MIT License
 */

#ifndef PDXCP_STRING_HH_
#define PDXCP_STRING_HH_

#include <cstddef>
#include <sstream>

namespace pdxcp {

/**
 * Check that two char arrays containing null-terminated strings are equal.
 *
 * Characters after the null terminator are ignored. This function ensures that
 * comparing two char arrays does not result in a buffer overrun if one is
 * missing a null terminator and that comparing two char arrays without null
 * terminators will still work as expected.
 *
 * @tparam N Array size
 *
 * @param first First character array
 * @param second Second character array
 */
template <std::size_t N>
bool string_equal(const char (&first)[N], const char (&second)[N]) noexcept
{
  for (std::size_t i = 0; i < N; i++) {
    // if null characters are reached at same index, break
    if (first[i] == '\0' && second[i] == '\0')
      break;
    // otherwise, unequal characters means unequal
    if (first[i] != second[i])
      return false;
  }
  return true;
}

}  // namespace pdxcp

#endif  // PDXCP_STRING_HH_
