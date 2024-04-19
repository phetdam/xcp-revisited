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
  pdxcp::banana b;
  pdxcp::fruit f;
  // print weights
  std::cout << "a.weight()       = " << a.weight() << std::endl;
  std::cout << "b.weight()       = " << b.weight() << std::endl;
  std::cout << "f.weight()       = " << f.weight() << std::endl;
  // print sum of the weights
  std::cout << "a + b + f weight = " << a + b + f << std::endl;
  // print juice yields and new weights
  std::cout << "a.juice()        = " << a.juice() << std::endl;
  std::cout << "b.juice()        = " << b.juice() << std::endl;
  std::cout << "f.juice()        = " << f.juice() << std::endl;
  std::cout << "a.weight()       = " << a.weight() << std::endl;
  std::cout << "b.weight()       = " << b.weight() << std::endl;
  std::cout << "f.weight()       = " << f.weight() << std::endl;
  // ~fruit() virtual so when subclasses are destroyed fruit messages are also
  // shown. as expected destruction order is reversed, i.e. f, b, then a
  return EXIT_SUCCESS;
}
