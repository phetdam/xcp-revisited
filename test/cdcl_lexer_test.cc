/**
 * @file cdcl_lexer_test.cc
 * @author Derek Huang
 * @brief cdcl_lexer.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/cdcl_lexer.h"

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
bool operator==(
  const pdxcp_cdcl_token& first, const pdxcp_cdcl_token& second) noexcept
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
  *out << "{" << pdxcp_cdcl_token_type_string(token.type) << ", \"" <<
    // prevent buffer overflow if text array is not null-terminated
    pdxcp::safe_stream_wrapper{token.text} << "\"}";
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
  // create zeroed token + populate token contents
  pdxcp_cdcl_token token{};
  token.type = type;
  std::strcpy(token.text, text.data());
  return token;
}

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
  const std::vector<pdxcp_cdcl_token> tokens;
};

/**
 * Google Test value printer for `LexerParamTestInput`.
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
 * C declaration lexer parametrized test fixture for single-token tests.
 */
class LexerSingleTokenTest : public LexerParamTest {};

/**
 * Check that parsing a single token works as expected.
 */
TEST_P(LexerSingleTokenTest, Test)
{
#if defined(PDXCP_HAS_FMEMOPEN)
  // must contain single token
  ASSERT_EQ(1, GetParam().tokens.size()) << "Only one input token allowed";
  // open memory-backed stream
  auto stream = pdxcp::memopen_string(GetParam().input);
  // get single token + check for success
  pdxcp_cdcl_token token;
  auto status = pdxcp_cdcl_get_token(stream, &token);
  // input will be printed but we also print lexer status for additional info
  ASSERT_EQ(pdxcp_cdcl_lexer_status_ok, status) << "Lexer status: " <<
    pdxcp_cdcl_lexer_status_message(status);
  // check for token equality
  EXPECT_EQ(GetParam().tokens[0], token);
  // get another token; this should result in EOF
  status = pdxcp_cdcl_get_token(stream, &token);
  ASSERT_EQ(pdxcp_cdcl_lexer_status_fgetc_eof, status) << "Lexer status: " <<
    pdxcp_cdcl_lexer_status_message(status);
#else
  GTEST_SKIP();
#endif  // !defined(PDXCP_HAS_FMEMOPEN)
}

// identifiers
INSTANTIATE_TEST_SUITE_P(
  IdenTokens,
  LexerSingleTokenTest,
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

// single-char tokens
INSTANTIATE_TEST_SUITE_P(
  CharTokens,
  LexerSingleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "[",
      {create_cdcl_token(pdxcp_cdcl_token_type_langle, "")}
    },
    LexerParamTestInput{
      ")",
      {create_cdcl_token(pdxcp_cdcl_token_type_rparen, "")}
    },
    LexerParamTestInput{
      "/",
      {create_cdcl_token(pdxcp_cdcl_token_type_slash, "")}
    },
    LexerParamTestInput{
      "*",
      {create_cdcl_token(pdxcp_cdcl_token_type_star, "")}
    }
  )
);

// structs
INSTANTIATE_TEST_SUITE_P(
  StructTokens,
  LexerSingleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "struct my_struct_1",
      {create_cdcl_token(pdxcp_cdcl_token_type_struct, "my_struct_1")}
    },
    LexerParamTestInput{
      "struct       _my_struct_2",
      {create_cdcl_token(pdxcp_cdcl_token_type_struct, "_my_struct_2")}
    }
  )
);

// enums
INSTANTIATE_TEST_SUITE_P(
  EnumTokens,
  LexerSingleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "enum    my_enum_1",
      {create_cdcl_token(pdxcp_cdcl_token_type_enum, "my_enum_1")}
    },
    LexerParamTestInput{
      "enum    _my_enum_2",
      {create_cdcl_token(pdxcp_cdcl_token_type_enum, "_my_enum_2")}
    }
  )
);

// type qualifiers
INSTANTIATE_TEST_SUITE_P(
  QualTokens,
  LexerSingleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "const",
      {create_cdcl_token(pdxcp_cdcl_token_type_q_const, "")}
    },
    LexerParamTestInput{
      "volatile",
      {create_cdcl_token(pdxcp_cdcl_token_type_q_volatile, "")}
    },
    LexerParamTestInput{
      "signed",
      {create_cdcl_token(pdxcp_cdcl_token_type_q_signed, "")}
    },
    LexerParamTestInput{
      "unsigned",
      {create_cdcl_token(pdxcp_cdcl_token_type_q_unsigned, "")}
    }
  )
);

// built-in types
INSTANTIATE_TEST_SUITE_P(
  BuiltinTypes,
  LexerSingleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "void",
      {create_cdcl_token(pdxcp_cdcl_token_type_t_void, "")}
    },
    LexerParamTestInput{
      "char      ",
      {create_cdcl_token(pdxcp_cdcl_token_type_t_char, "")}
    },
    LexerParamTestInput{
      "   int   ",
      {create_cdcl_token(pdxcp_cdcl_token_type_t_int, "")}
    },
    LexerParamTestInput{
      "  double     ",
      {create_cdcl_token(pdxcp_cdcl_token_type_t_double, "")}
    }
  )
);

/**
 * C declaration lexer parametrized test fixture for multi-token tests.
 */
class LexerMultipleTokenTest : public LexerParamTest {};

/**
 * Check that parsing a string of tokens works as expected.
 */
TEST_P(LexerMultipleTokenTest, Test)
{
#if defined(PDXCP_HAS_FMEMOPEN)
  // number of tokens. we allow zero tokens to be read
  auto n_tokens = GetParam().tokens.size();
  // open memory-backed stream
  auto stream = pdxcp::memopen_string(GetParam().input);
  // lexer status + reserved vector of tokens
  pdxcp_cdcl_lexer_status status;
  std::vector<pdxcp_cdcl_token> tokens;
  tokens.reserve(n_tokens);
  // push tokens into vector until non-ok status detected
  do {
    pdxcp_cdcl_token token;
    if (PDXCP_CDCL_LEXER_OK(status = pdxcp_cdcl_get_token(stream, &token)))
      tokens.push_back(token);
  }
  while (PDXCP_CDCL_LEXER_OK(status));
  // must have read n_tokens, otherwise error
  ASSERT_EQ(n_tokens, tokens.size()) << "Read only " << tokens.size() <<
    " of " << n_tokens << " tokens. Lexer status: " <<
    pdxcp_cdcl_lexer_status_message(status);
  // only EOF is expected once we break the loop
  ASSERT_EQ(pdxcp_cdcl_lexer_status_fgetc_eof, status) << "Lexer status: " <<
    pdxcp_cdcl_lexer_status_message(status);
  // check for token equality
  EXPECT_EQ(GetParam().tokens, tokens);
#else
  GTEST_SKIP();
#endif  // !defined(PDXCP_HAS_FMEMOPEN)
}

// simple declarations
INSTANTIATE_TEST_SUITE_P(
  SimpleDecls,
  LexerMultipleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "int hello;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_t_int, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "hello"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    },
    LexerParamTestInput{
      "char *str;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_t_char, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "str"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    },
    LexerParamTestInput{
      "struct    my_struct  **b;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_struct, "my_struct"),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "b"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    }
  )
);

// declarations with type qualifiers
INSTANTIATE_TEST_SUITE_P(
  QualDecls,
  LexerMultipleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "unsigned long const *x;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_q_unsigned, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_long, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "x"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    },
    LexerParamTestInput{
      "unsigned char *const *const y;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_q_unsigned, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_char, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "y"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    }
  )
);

// comment blocks
INSTANTIATE_TEST_SUITE_P(
  CommentBlocks,
  LexerMultipleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "// this is a line comment with no terminating newline",
      {}
    },
    LexerParamTestInput{
      "// this is a line comment with a terminating newline\n",
      {}
    },
    LexerParamTestInput{
      "/* this is a single-line comment block, no terminating newline */",
      {}
    },
    LexerParamTestInput{
      "/**\n"
      " * This is a stylized Doxygen-style comment block.\n"
      " *\n"
      " * There is a lot more content in this block compared to the other\n"
      " * inputs that have been used to test the lexer's comment-skipping.\n"
      " */\n",
      {}
    },
    LexerParamTestInput{
      "/**\n"
      " * This is a comment block with an extra * and / at the end. Although\n"
      " * this would be a parse error, the lexer understands the tokens.\n"
      " */*/",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_slash, "")
      }
    }
  )
);

// declarations with comment blocks
INSTANTIATE_TEST_SUITE_P(
  CommentedDecls,
  LexerMultipleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "const double **x;\n"
      "// random line comment without terminating newline",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_double, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "x"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    },
    LexerParamTestInput{
      "/**\n"
      " * y is a pointer to volatile void.\n"
      " *\n"
      " * It's some memory that may change asynchronously.\n"
      " */\n"
      "volatile void *y;",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_q_volatile, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_void, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "y"),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    }
  )
);

// function pointer declarations
// note: these may not be understood by parser but are good for testing lexer
INSTANTIATE_TEST_SUITE_P(
  FuncPtrDecls,
  LexerMultipleTokenTest,
  ::testing::Values(
    LexerParamTestInput{
      "void **(*my_func)(double x, volatile int *);",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_t_void, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_lparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "my_func"),
        create_cdcl_token(pdxcp_cdcl_token_type_rparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_lparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_double, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "x"),
        create_cdcl_token(pdxcp_cdcl_token_type_comma, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_volatile, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_int, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_rparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    },
    LexerParamTestInput{
      "volatile double *(*func1)(int x, const volatile *y[], long const);",
      {
        create_cdcl_token(pdxcp_cdcl_token_type_q_volatile, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_double, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_lparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "func1"),
        create_cdcl_token(pdxcp_cdcl_token_type_rparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_lparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_int, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "x"),
        create_cdcl_token(pdxcp_cdcl_token_type_comma, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_volatile, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_star, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_iden, "y"),
        create_cdcl_token(pdxcp_cdcl_token_type_langle, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_rangle, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_comma, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_t_long, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_q_const, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_rparen, ""),
        create_cdcl_token(pdxcp_cdcl_token_type_semicolon, "")
      }
    }
  )
);

}  // namespace
