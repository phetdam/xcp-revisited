/**
 * @file fruit3.cc
 * @author Derek Huang
 * @brief C++ program demonstrating fruit addition and polymorphism
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include "pdxcp/fruit.hh"

int main()
{
  // default-construct and print weights
  pdxcp::apple a;
  pdxcp::fruit b;
  // print weights
  std::cout << "a.weight()   = " << a.weight() << std::endl;
  std::cout << "b.weight()   = " << b.weight() << std::endl;
  // print sum of the weights
  std::cout << "a + b weight = " << a + b << std::endl;
  // ~fruit() virtual so when apple is destroyed fruit message also shown. as
  // expected b is destroyed before a, i.e. destroy in reverse order
  return EXIT_SUCCESS;
}
