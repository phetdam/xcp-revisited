/**
 * @file cdcl_lexer_test.cc
 * @author Derek Huang
 * @brief cdcl_lexer.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/cdcl_lexer.h"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "pdxcp/features.h"
#include "pdxcp/memory.hh"
#include "pdxcp/string.hh"

// helper macro for checking if we have fmemopen
#if defined(PDXCP_GNU) || defined(PDXCP_POSIX_C_2008)
#define PDXCP_HAS_FMEMOPEN
#endif  // !defined(PDXCP_GNU) && !defined(PDXCP_POSIX_C_2008)

// fmemopen requires _GNU_SOURCE or _POSIX_C_SOURCE >= 200809L
#ifdef PDXCP_HAS_FMEMOPEN
#include <stdio.h>
#endif  // PDXCP_HAS_FMEMOPEN

/**
 * Equality operator for `pdxcp_cdcl_token`.
 *
 * Uses `string_equal` instead of `strcmp` to avoid buffer overruns.
 *
 * @note Must be in top-level namespace for ADL to work correctly.
 *
 * @param first First token
 * @param second Second token
 */
bool operator==(const pdxcp_cdcl_token& first, const pdxcp_cdcl_token& second)
{
  return first.type == second.type && pdxcp::string_equal(first.text, second.text);
}

/**
 * Google Test value printer for `pdxcp_cdcl_token`.
 *
 * @note Must be in top-level namespace for ADL to work correctly.
 */
void PrintTo(const pdxcp_cdcl_token& token, std::ostream* out)
{
  // operator<< overload exists for const CharT*; rely on pointer decay here
  *out << "token(" << pdxcp_cdcl_token_type_string(token.type) << ", \"" <<
    // prevent buffer overflow if text array is not null-terminated
    pdxcp::safe_stream_wrapper{token.text} << "\")";
}

namespace {

/**
 * Helper function for creating a `pdxcp_cdcl_token` from a string view.
 *
 * The string view's contents are copied into the token's text array.
 *
 * @param type Token type
 * @param text Token text, must not exceed `PDXCP_CDCL_MAX_TOKEN_LEN`
 */
auto create_cdcl_token(pdxcp_cdcl_token_type type, const std::string_view& text)
{
  // throw if text is too large
  if (text.size() > PDXCP_CDCL_MAX_TOKEN_LEN)
    throw std::runtime_error{"Token text exceeds PDXCP_CDCL_MAX_TOKEN_LEN"};
  // populate token contents
  pdxcp_cdcl_token token;
  token.type = type;
  std::strcpy(token.text, text.data());
  // token text must be null-terminated
  token.text[PDXCP_CDCL_MAX_TOKEN_LEN] = '\0';
  return token;
}

#ifdef PDXCP_HAS_FMEMOPEN
/**
 * Return a `unique_file_stream` backed by an STL string.
 *
 * Wraps a `fmemopen` call with read-only access.
 *
 * @param str Backing string. Must be in scope during file stream usage.
 */
inline auto memopen_string(const std::string& str)
{
  auto stream = fmemopen((void*) str.c_str(), str.size(), "r");
  if (!stream)
    throw std::runtime_error{
      "fmemopen error: " + std::string{std::strerror(errno)}
    };
  return pdxcp::unique_file_stream{stream};
}
#endif  // PDXCP_HAS_FMEMOPEN

/**
 * C declaration lexer test fixture base.
 */
class LexerTest : public ::testing::Test {};

/**
 * Struct holding the input and output for a `LexerParamTest`.
 *
 * @param input Input string fragment
 * @param tokens Expected tokens yielded from the input string
 */
struct LexerParamTestInput {
  const std::string input;
  std::vector<pdxcp_cdcl_token> tokens;
};

/**
 * Google Test value printer for `LexerParamTestInput`.
 *
 * @note Must be in top-level namespace for ADL to work correctly.
 */
void PrintTo(const LexerParamTestInput& input, std::ostream* out)
{
  // print string input
  *out << "{\"" << input.input << "\", ";
  // print tokens separated by commas
  for (auto it = input.tokens.begin(); it != input.tokens.end(); it++) {
    if (std::distance(input.tokens.begin(), it))
      *out << ", ";
    PrintTo(*it, out);
  }
}

/**
 * C declaration lexer parametrized test fixture.
 */
class LexerParamTest
  : public LexerTest,
    public ::testing::WithParamInterface<LexerParamTestInput> {};

/**
 * Check that parsing a single token works as expected.
 */
TEST_P(LexerParamTest, SingleTokenTest)
{
  // must contain single token
  ASSERT_EQ(1u, GetParam().tokens.size()) << "Only one token allowed";
  // open memory-backed stream
  auto stream = memopen_string(GetParam().input);
  // get single token + check for success
  pdxcp_cdcl_token token;
  auto status = pdxcp_cdcl_get_token(stream, &token);
  // input will be printed but we also print lexer status for additional info
  ASSERT_EQ(pdxcp_cdcl_lexer_status_ok, status) << "Lexer status: " <<
    pdxcp_cdcl_lexer_status_message(status);
  // check for token equality
  EXPECT_EQ(GetParam().tokens[0], token);
}

INSTANTIATE_TEST_SUITE_P(
  Base,
  LexerParamTest,
  ::testing::Values(
    LexerParamTestInput{
      "iden_1",
      {create_cdcl_token(pdxcp_cdcl_token_type_iden, "iden_1")}
    },
    LexerParamTestInput{
      "another_iden",
      {create_cdcl_token(pdxcp_cdcl_token_type_iden, "another_iden")}
    },
    LexerParamTestInput{
      "_underscore_1_iden",
      {create_cdcl_token(pdxcp_cdcl_token_type_iden, "_underscore_1_iden")}
    }
  )
);

}  // namespace
