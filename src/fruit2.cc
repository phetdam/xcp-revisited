/**
 * @file fruit2.cc
 * @author Derek Huang
 * @brief Expert C Programming (p305): calling C++ fruit member functions
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include "pdxcp/fruit.hh"

int main()
{
  // default-construct fruit and print weight
  pdxcp::fruit a;
  std::cout << "a.weight() = " << a.weight() << std::endl;
  // slice 8 times and report new weight
  a.slice(8);
  std::cout << "a.slice(8);\na.weight() = " << a.weight() << std::endl;
  // juice and report new weight and juice yield
  auto a_juice = a.juice();
  std::cout << "a.juice();\na.weight() = " << a.weight() << std::endl;
  std::cout << "a_juice    = " << a_juice << std::endl;
  return EXIT_SUCCESS;
}
