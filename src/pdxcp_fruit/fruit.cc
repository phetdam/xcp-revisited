/**
 * @file fruit.cc
 * @author Derek Huang
 * @brief C++ source for the Expert C Programming C++ introduction via fruit
 * @copyright MIT License
 */

#include "pdxcp/fruit.hh"

#include <cstdio>

namespace pdxcp {

fruit::fruit() noexcept : weight_{2}, kcal_per_oz_{15} {}

// ~fruit() written for the p307 Expert C Programming exercise

fruit::~fruit()
{
  // using printf for exception safety
  std::printf("fruit at %p destroyed\n", this);
}

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

// for the p304 Expert C Programming exercise, slice() and juice() bodies were
// implemented. members can be accessed via this-> but this is unnecessary.

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
