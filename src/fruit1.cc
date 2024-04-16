/**
 * @file fruit1.cc
 * @author Derek Huang
 * @brief Expert C Programming (p302): compiling and running a C++ program
 * @copyright MIT License
 */

#include <cstdlib>

#include "pdxcp/fruit.hh"
#include "pdxcp/warnings.h"

int main()
{
  // exercise simply requests that we declare a couple of fruit objects. note
  // that this is not declaration but in fact default construction; this is a
  // semantic difference between C and C++. we also silence -Wunused-variable.
PDXCP_GNU_WARNING_PUSH()
PDXCP_GNU_WARNING_DISABLE(-Wunused-variable)
  pdxcp::fruit a;
  pdxcp::fruit b;
PDXCP_GNU_WARNING_POP()
  return EXIT_SUCCESS;
}
