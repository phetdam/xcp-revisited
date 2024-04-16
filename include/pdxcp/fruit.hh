/**
 * @file fruit.hh
 * @author Derek Huang
 * @brief C++ header for the Expert C Programming C++ introduction via fruit
 * @copyright MIT License
 */

#ifndef PDXCP_FRUIT_HH_
#define PDXCP_FRUIT_HH_

namespace pdxcp {

/**
 * A fruit class.
 *
 * This is to support the Expert C Programming introduction to C++ and has been
 * improved from the original presented in the book.
 */
class fruit {
public:
  /**
   * Peel the fruit.
   */
  void peel();

  /**
   * Slice the fruit.
   */
  void slice();

  /**
   * Juice the fruit.
   */
  void juice();

private:
  unsigned int weight_;
  unsigned int cals_per_oz_;
};

}  // namespace pdxcp

#endif  // PDXCP_FRUIT_HH_
