/**
 * @file arrptrbind.cc
 * @author Derek Huang
 * @brief C++ program demonstrating array/pointer function argument binding
 * @copyright MIT License
 *
 * This program is a C++ analogue to `arrptrbind.c` that shows the stronger
 * typing available in C++, in particular using templates to deduce array size
 * and overloading to distinguish between differing type templates.
 */

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>

#include "pdxcp/demangle.hh"

namespace {

/**
 * Take a 1D array and print its type and dimensions.
 *
 * @note Must use reference to avoid decaying first dimension into pointer.
 *
 * @param arr 1D array
 */
template <typename T, std::size_t N>
inline void mdarray_info(T (&arr)[N])
{
  std::cout << "received: " << PDXCP_DEMANGLED_NAME(T) << "[" << N << "]" <<
    std::endl;
}

/**
 * Take a 2D array and print its type and dimensions.
 *
 * @note Must use reference to avoid decaying first dimension into pointer.
 *
 * @param arr 2D array
 */
template <typename T, std::size_t N1, std::size_t N2>
inline void mdarray_info(T (&arr)[N1][N2])
{
  std::cout << "received: " << PDXCP_DEMANGLED_NAME(T) << "[" << N1 << "][" <<
    N2 << "]" << std::endl;
}

/**
 * Take a 3D array and print its type and dimensions.
 *
 * @note Must use reference to avoid decaying first dimension into pointer.
 *
 * @param arr 3D array
 */
template <typename T, std::size_t N1, std::size_t N2, std::size_t N3>
inline void mdarray_info(T (&arr)[N1][N2][N3])
{
  std::cout << "received: " << PDXCP_DEMANGLED_NAME(T) << "[" << N1 << "][" <<
    N2 << "][" << N3 << "]" << std::endl;
}

}  // namespace

int main()
{
  // 3D array, pointer to 2D array, and pointer to 3D array defined on p272
  int apricot[2][3][5];
  int (*p)[3][5] = apricot;
  int (*q)[2][3][5] = &apricot;        // &apricot and apricot are equal
  // some more "interesting" types
  std::string strarr[3][3][3];
  auto strarr_1 = strarr + 1;          // std::string (*strarr_1)[3][3]
  const volatile void* varr[4][5][7];
  auto varr_1 = varr + 1;              // const volatile void* (*varr_1)[5][7]
  // print information on the original input types
  mdarray_info(apricot);
  mdarray_info(*p);
  mdarray_info(*q);
  // print information on the more "interesting" types
  mdarray_info(strarr);
  mdarray_info(strarr[1]);
  mdarray_info(*strarr_1);             // std::string[3][3]
  mdarray_info(strarr[0][2]);
  mdarray_info(varr);
  mdarray_info(varr[1]);
  mdarray_info(*varr_1);               // const volatile void*[5][7]
  mdarray_info(varr[3][3]);
  return EXIT_SUCCESS;
}
