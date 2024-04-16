/**
 * @file fruit.cc
 * @author Derek Huang
 * @brief C++ source for the Expert C Programming C++ introduction via fruit
 * @copyright MIT License
 */

#include "pdxcp/fruit.hh"

namespace pdxcp {

fruit::fruit() noexcept : weight_{2}, kcal_per_oz_{15} {}

double fruit::weight() const noexcept
{
  return weight_;
}

double fruit::kcal_per_oz() const noexcept
{
  return kcal_per_oz_;
}

void fruit::peel() noexcept
{
  weight_ *= 0.98;
}

void fruit::slice(unsigned int n_slices) noexcept
{
  for (decltype(n_slices) i = 0; i < n_slices; i++)
    weight_ *= 0.9;
}

double fruit::juice() noexcept
{
  auto juice_weight = 0.6 * weight_;
  weight_ -= juice_weight;
  return juice_weight;
}

}  // namespace pdxcp
