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
   * Ctor.
   *
   * @param weight Fruit weight in oz
   * @param kcal_per_oz kcal per oz
   */
  fruit(double weight, double kcal_per_oz);

  /**
   * Dtor.
   *
   * Prints out a message saying the fruit at the given address is destroyed.
   */
  virtual ~fruit();

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

  /**
   * Addition operator member returning the total fruit weight in ounces.
   *
   * This is based on the discussion starting on p313 in Expert C Programming
   * but the design has been improved to take `const` ref and be `noexcept`.
   *
   * @param op Operand fruit
   */
  double operator+(const fruit& op) noexcept;

protected:
  /**
   * Update the fruit's weight in ounces and return the old weight.
   */
  double set_weight(double weight) noexcept;

private:
  double weight_;
  double kcal_per_oz_;
};

/**
 * A candy apple class.
 *
 * Since a candy apple should not be considered a normal fruit is does not
 * inherit fruit and is used only to support the `make_candy_apple` member.
 */
class candy_apple {
public:
  /**
   * Ctor.
   *
   * @param weight Candy apple weight in ounces
   */
  candy_apple(double weight);

  /**
   * Return the weight of the candy apple in ounces.
   */
  double weight() const noexcept;

private:
  double weight_;
};

/**
 * A more sensible class for an apple.
 *
 * This is based on the discussion on p309 in Expert C Programming but the
 * design has been tweaked to make it more semantically correct. E.g. bobbing
 * for a specific apple is a bit strange and candy apples require whole apples.
 */
class apple : public fruit {
public:
  /**
   * Default ctor.
   *
   * The apple's weight is 5.28 oz as 0.33 lbs is average apple weight.
   */
  apple();

  /**
   * Dtor.
   *
   * Prints a message saying the apple at the given address is destroyed. Since
   * `fruit` has a virtual dtor the `fruit` message will be printed after.
   */
  ~apple();

  /**
   * Ctor.
   *
   * The kcal per oz of an apple is 15 according to Google.
   *
   * @param weight Apple weight in oz
   */
  apple(double weight);

  /**
   * Make a candy apple out of this apple.
   *
   * The apple's weight will go to zero and a candy apple is returned.
   */
  candy_apple make_candy_apple();
};

}  // namespace pdxcp

#endif  // PDXCP_FRUIT_HH_
