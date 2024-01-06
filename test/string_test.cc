/**
 * @file string_test.cc
 * @author Derek Huang
 * @brief string.hh unit tests
 * @copyright MIT License
 */

#include "pdxcp/string.hh"

#include <array>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

namespace {

/**
 * String unit tests base test fixture.
 */
class StringTest : public ::testing::Test {};

/**
 * Test that the `length` function works as expected.
 */
TEST_F(StringTest, LengthTest)
{
#define INPUT_STRING "input string"
  EXPECT_EQ(
    std::extent_v<std::remove_reference_t<decltype(INPUT_STRING)>>,
    pdxcp::length(INPUT_STRING)
  );
#undef INPUT_STRING
}

/**
 * Test that the `length` function works in a constant expression context.
 *
 * Here we specifically choose use inside a template.
 */
TEST_F(StringTest, LengthTemplateTest)
{
#define INPUT_STRING "hello"
  // can't use templates containing commas in EXPECT_EQ
  using array_type = std::array<char, pdxcp::length(INPUT_STRING)>;
  EXPECT_EQ(
    std::extent_v<std::remove_reference_t<decltype(INPUT_STRING)>>,
    array_type{}.size()
  );
#undef INPUT_STRING
}

/**
 * Traits class for the `string_equal` test.
 */
template <std::size_t N1, std::size_t N2>
struct StringEqualTestTraits {
  using first_type = char[N1];
  using second_type = char[N2];
  static inline constexpr std::size_t first_size = N1;
  static inline constexpr std::size_t second_size = N2;
};

/**
 * Testing fixture template for `string_equal`.
 *
 * The use of a template allows us to pass different sized array types for the
 * same input. We do this to ensure that `string_equal` behaves as advertised.
 */
template <typename InputType>
class StringEqualTest : public StringTest {};

using StringEqualTestTypes = ::testing::Types<
  StringEqualTestTraits<14, 40>,
  StringEqualTestTraits<10, 10>,
  StringEqualTestTraits<22, 15>
>;
TYPED_TEST_SUITE(StringEqualTest, StringEqualTestTypes);

/**
 * Test that null-terminated string equality holds across sizes.
 */
TYPED_TEST(StringEqualTest, StringEqTest)
{
#define INPUT_STRING "hello"
  typename TypeParam::first_type first = {INPUT_STRING};
  typename TypeParam::second_type second = {INPUT_STRING};
  EXPECT_TRUE(pdxcp::string_equal(first, second));
#undef INPUT_STRING
}

/**
 * Test that null-terminated string inequality holds across sizes.
 */
TYPED_TEST(StringEqualTest, StringNeqTest)
{
  typename TypeParam::first_type first = {"hello"};
  typename TypeParam::second_type second = {"bye"};
  EXPECT_FALSE(pdxcp::string_equal(first, second));
}

/**
 * Test that inequality holds when a null terminator is missing.
 */
TYPED_TEST(StringEqualTest, MissingNullTest)
{
#define INPUT_STRING "hello"
  typename TypeParam::first_type first = {INPUT_STRING};
  typename TypeParam::second_type second = {INPUT_STRING};
  // fill second with non-null values. we use extent_v to get INPUT_STRING size
  // and note that this quantity - 1 gives the index of the null terminator
  for (auto i = pdxcp::length(INPUT_STRING) - 1; i < TypeParam::second_size; i++)
    second[i] = 'a';
  EXPECT_FALSE(pdxcp::string_equal(first, second));
#undef INPUT_STRING
}

/**
 * Test that equality holds even if chars after null terminator differ.
 */
TYPED_TEST(StringEqualTest, NeqTrailingTest)
{
#define INPUT_STRING "hello"
  // input size and convenience type aliases
  constexpr auto input_size = pdxcp::length(INPUT_STRING);
  using first_type = typename TypeParam::first_type;
  using second_type = typename TypeParam::second_type;
  // since we are writing past the null terminator, need to ensure that the
  // array types are large enough using a static_assert for meaningful test.
  // better to catch this at compile time instead of runtime
  static_assert(input_size < std::extent_v<first_type>, "first_type too small");
  static_assert(input_size < std::extent_v<second_type>, "second_type too small");
  first_type first = {INPUT_STRING};
  second_type second = {INPUT_STRING};
  // fill first and second with differing non-null values after null
  for (auto i = input_size; i < TypeParam::first_size; i++)
    first[i] = 'a';
  for (auto i = input_size; i < TypeParam::second_size; i++)
    second[i] = 'b';
  EXPECT_TRUE(pdxcp::string_equal(first, second));
#undef INPUT_STRING
}

/**
 * Test that arrays compare equal or different based on size.
 *
 * Both arrays are not null-terminated.
 *
 * @note Skips inputs where the arrays are different sizes.
 */
TYPED_TEST(StringEqualTest, ArraySizeEqTest)
{
  typename TypeParam::first_type first;
  typename TypeParam::second_type second;
  std::memset(first, 'a', TypeParam::first_size);
  std::memset(second, 'a', TypeParam::second_size);
  // equal if same size, not equal otherwise
  if constexpr (TypeParam::first_size == TypeParam::second_size)
    EXPECT_TRUE(pdxcp::string_equal(first, second));
  else
    EXPECT_FALSE(pdxcp::string_equal(first, second));
}

}  // namespace
