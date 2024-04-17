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
   * Default ctor.
   *
   * We assume that the fruit weighs 2 oz and has 15 kcal per ounce. Based on a
   * Google search apparently apples have around 15 kcal per ounce.
   *
   * This ctor is mostly used to allow `fruit1.cc` to compile.
   */
  fruit() noexcept;

  /**
   * Dtor.
   *
   * Prints out a message saying the fruit at the given address is destroyed.
   */
  ~fruit();

  /**
   * Return the weight of the fruit in oz.
   */
  double weight() const noexcept;

  /**
   * Return the number of kcals per oz for the fruit.
   */
  double kcal_per_oz() const noexcept;

  /**
   * Peel a layer off the fruit.
   *
   * This reduces the fruit's weight by 2%.
   */
  void peel() noexcept;

  /**
   * Slice small layers off the fruit.
   *
   * Each slice results in a 10% reduction in the fruit's weight.
   *
   * @param n_slices Number to layers to slice off
   */
  void slice(unsigned int n_slices) noexcept;

  /**
   * Juice the fruit.
   *
   * This reduces the weight of the fruit by 60% and returns the number of
   * fluid ounces of juice that the fruit yielded. For simplicity, the density
   * of the fruit is assumed to be such that an ounce is converted to a single
   * fluid ounce. For density K given in g/mL, the formula is as follows:
   *
   * N fluid ounces = M ounces / (1.0432 * K g/mL)
   */
  double juice() noexcept;

private:
  double weight_;       // weight in oz
  double kcal_per_oz_;  // kcals per oz
};

}  // namespace pdxcp

#endif  // PDXCP_FRUIT_HH_
