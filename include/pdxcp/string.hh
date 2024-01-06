/**
 * @file string.hh
 * @author Derek Huang
 * @brief C++ header for string handling extensions
 * @copyright MIT License
 */

#ifndef PDXCP_STRING_HH_
#define PDXCP_STRING_HH_

#include <cstddef>
#include <ostream>
#include <sstream>

namespace pdxcp {

/**
 * Return the length of an array.
 *
 * Works on string literals as expected. This is more concise than using the
 * typical `std::extent_v<std::remove_reference_t<decltype("my literal")>>`.
 * Removing the reference is necessary since the literal is an lvalue.
 */
template <std::size_t N>
inline constexpr auto length(const char (&first)[N])
{
  return N;
}

/**
 * Check that two char arrays containing null-terminated strings are equal.
 *
 * Characters after the null terminator are ignored. This function ensures that
 * comparing two char arrays does not result in a buffer overrun if one is
 * missing a null terminator and that comparing two char arrays without null
 * terminators will still work as expected.
 *
 * @note Two arrays of different sizes can compare as equal if they start with
 *  the same null-terminated string. The characters after the null are ignored.
 *
 * @tparam N1 First array size
 * @tparam N2 Second array size
 *
 * @param first First character array
 * @param second Second character array
 */
template <std::size_t N1, std::size_t N2>
bool string_equal(const char (&first)[N1], const char (&second)[N2]) noexcept
{
  constexpr auto N = std::min(N1, N2);
  // loop through the shared characters
  for (std::size_t i = 0; i < N; i++) {
    // if null characters are reached at same index, done
    if (first[i] == '\0' && second[i] == '\0')
      return true;
    // otherwise, unequal characters means unequal
    if (first[i] != second[i])
      return false;
  }
  // if sizes are different, then arrays are not equal
  return N1 == N2;
}

/**
 * Safe output stream wrapper for a char array.
 *
 * Using this ensures that no buffer overrun occurs if the array is not
 * null-terminated as assumed by the `std::ostream::operator<<` overloads.
 *
 * @tparam N Array size
 */
template <std::size_t N>
class safe_stream_wrapper {
public:
  static inline constexpr std::size_t size = N;

  /**
   * Ctor.
   *
   * @param arr Reference to array of `N` chars
   */
  safe_stream_wrapper(const char (&arr)[N]) noexcept : ref_{arr} {}

  /**
   * Return const reference to the array.
   */
  const auto& ref() const noexcept { return ref_; }

  /**
   * Safely write the char array to the output stream.
   *
   * If the array is null-terminated, writes up to the null terminator, and if
   * not, writes all the characters without overrunning the buffer.
   */
  auto& write(std::ostream& out) const
  {
    for (std::size_t i = 0; i < N && ref_[i] != '\0'; i++)
      out.put(ref_[i]);
    return out;
  }

private:
  const char (&ref_)[N];
};

/**
 * `operator<<` overload for `safe_stream_wrapper`.
 *
 * @tparam N Array size
 *
 * @param out Output stream
 * @param value Char array stream wrapper
 */
template <std::size_t N>
inline auto& operator<<(std::ostream& out, const safe_stream_wrapper<N>& value)
{
  return value.write(out);
}

}  // namespace pdxcp

#endif  // PDXCP_STRING_HH_
