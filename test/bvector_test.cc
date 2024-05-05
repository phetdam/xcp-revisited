/**
 * @file bvector_test.cc
 * @author Derek Huang
 * @brief bvector.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/bvector.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

/**
 * Byte vector wrapper class that ensures we never forget to free memory.
 */
class byte_vector {
public:
  /**
   * Default ctor.
   */
  byte_vector()
  {
    pdxcp_bvector_init(&vec_);
  }

  /**
   * Deleted copy ctor.
   */
  byte_vector(const byte_vector&) = delete;

  /**
   * Dtor.
   */
  ~byte_vector()
  {
    pdxcp_bvector_destroy(&vec_);
  }

  /**
   * Allow read-write access to the `pdxcp_bvector` members.
   */
  pdxcp_bvector* operator->() noexcept
  {
    return &vec_;
  }

  /**
   * Allow implicit conversion to pointer for working with C functions.
   */
  operator pdxcp_bvector*() noexcept
  {
    return &vec_;
  }

private:
  pdxcp_bvector vec_;
};

/**
 * Base testing fixture for byte vector tests.
 */
class ByteVectorTest : public ::testing::Test {};

/**
 * Input struct for byte vector expansion tests.
 *
 * @param input Input string to put into byte vector
 * @param capacity Expected capacity of the byte vector after adding input
 */
struct ByteVectorExpandInput {
  const std::string input;
  const std::size_t capacity;
};

/**
 * Google Test input type printer overload for byte vector expansion inputs.
 */
void PrintTo(const ByteVectorExpandInput& input, std::ostream* out)
{
  *out << "{\"" << input.input << "\", " << input.capacity << "}";
}

/**
 * Parametrized testing fixture for testing byte vector expansion.
 */
class ByteVectorExpandTest
  : public ByteVectorTest,
    public ::testing::WithParamInterface<ByteVectorExpandInput> {};

/**
 * Test that byte vector expansion works when adding one byte at a time.
 */
TEST_P(ByteVectorExpandTest, SingleAddTest)
{
  // vector + reference to input
  byte_vector vec;
  const auto& input = GetParam().input;
  // add all input chars to byte vector
  for (auto c : input)
    if (!pdxcp_bvector_add(vec, c))
      GTEST_FAIL() << "ENOMEM adding " << c << " to the byte vector";
  // check that size + capacity are as expected
  EXPECT_EQ(input.size(), vec->size);
  EXPECT_EQ(GetParam().capacity, vec->capacity);
}

/**
 * Test that byte vector expansion works when adding multiple bytes at a time.
 */
TEST_P(ByteVectorExpandTest, MultiAddTest)
{
  // vector + reference to input
  byte_vector vec;
  const auto& input = GetParam().input;
  // add all input chars to byte vector
  if (!pdxcp_bvector_add_n(vec, (unsigned char*) input.c_str(), input.size()))
    GTEST_FAIL() << "ENOMEM adding \"" << input << "\" to byte vector";
  // check that size + capacity are as expected
  EXPECT_EQ(input.size(), vec->size);
  EXPECT_EQ(GetParam().capacity, vec->capacity);
}

INSTANTIATE_TEST_SUITE_P(
  Base,
  ByteVectorExpandTest,
  ::testing::ValuesIn(
    std::vector<ByteVectorExpandInput>{
      // 38 chars
      {
        "the quick fox jumps over the brown dog",
        4 * PDXCP_BVECTOR_ZERO_EXPAND_SIZE
      },
      // 12 chars
      {"short string", PDXCP_BVECTOR_ZERO_EXPAND_SIZE},
      // 61 chars
      {
        "once upon a time there was a particular oft-used stock phrase",
        4 * PDXCP_BVECTOR_ZERO_EXPAND_SIZE
      },
      // 98 chars
      // from Demon Hunter's "One Thousand Apologies"
      {
        "The reason gone, the damage stays\n"
        "All the delicate ways that I deepened our grave\n"
        "My apology pales",
        8 * PDXCP_BVECTOR_ZERO_EXPAND_SIZE
      }
    }
  )
);

}  // namespace