/**
 * @file fruit.cc
 * @author Derek Huang
 * @brief C++ source for the Expert C Programming C++ introduction via fruit
 * @copyright MIT License
 */

#include "pdxcp/fruit.hh"

#include <cstdio>
#include <stdexcept>

namespace pdxcp {

///////////////////////////////////////////////////////////////////////////////
// fruit
///////////////////////////////////////////////////////////////////////////////

fruit::fruit() noexcept : weight_{2}, kcal_per_oz_{15} {}

fruit::fruit(double weight, double kcal_per_oz)
  : weight_{weight}, kcal_per_oz_{kcal_per_oz}
{
  // sanity check; can't have negative values
  if (weight_ < 0)
    throw std::invalid_argument{"weight cannot be negative"};
  if (kcal_per_oz_ < 0)
    throw std::invalid_argument{"kcal per oz cannot be negative"};
}

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

double fruit::set_weight(double weight) noexcept
{
  auto old_weight = weight_;
  weight_ = weight;
  return old_weight;
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

double fruit::operator+(const fruit& op) noexcept
{
  return weight_ + op.weight();
}

///////////////////////////////////////////////////////////////////////////////
// candy_apple
///////////////////////////////////////////////////////////////////////////////

candy_apple::candy_apple(double weight) : weight_{weight}
{
  if (weight_ < 0)
    throw std::invalid_argument{"weight cannot be negative"};
}

double candy_apple::weight() const noexcept
{
  return weight_;
}

///////////////////////////////////////////////////////////////////////////////
// apple
///////////////////////////////////////////////////////////////////////////////

apple::apple() : apple{5.28} {}

apple::apple(double weight) : fruit{weight, 15} {}

apple::~apple()
{
  std::printf("apple at %p destroyed\n", this);
}

candy_apple apple::make_candy_apple()
{
  return set_weight(0);
}

}  // namespace pdxcp
