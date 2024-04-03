/**
 * @file cdcl_parser_test.cc
 * @author Derek Huang
 * @brief cdcl_parser.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/cdcl_parser.h"

#include <cstdio>
#include <string>
#include <ostream>

#include <gtest/gtest.h>

#include "pdxcp/string.hh"

namespace {

/**
 * C declaration parser test fixture base.
 */
class ParserTest : public ::testing::Test {};

/**
 * Struct holding the input for a `ParserParamTest`.
 *
 * @note Currently only holds a string; need to spend more time developing
 *  internal data structures for the parser
 */
struct ParserParamTestInput {
  const std::string input;
};

/**
 * Google Test value printer for `ParserParamTestInput`.
 */
void PrintTo(const ParserParamTestInput& input, std::ostream* out)
{
  *out << input.input;
}

/**
 * C declaration parser parametrized test fixture.
 */
class ParserParamTest
  : public ParserTest,
    public ::testing::WithParamInterface<ParserParamTestInput> {};

/**
 * Check that the parser works as expected.
 *
 * This only checks that the parser runs since we don't have any parser data
 * structures to hold parsing state designed just yet.
 */
TEST_P(ParserParamTest, PureTest)
{
#if defined(PDXCP_HAS_FMEMOPEN)
  // create input stream
  auto stream = pdxcp::memopen_string(GetParam().input);
  // parse and write error info
  pdxcp_cdcl_parser_errinfo errinfo;
  auto status = pdxcp_cdcl_stream_parse(stream, stdout, &errinfo);
  // note: only stream parser error text when status is
  // pdxcp_cdcl_parser_status_parse_err, result is undefined otherwise
  EXPECT_EQ(pdxcp_cdcl_parser_status_ok, status) << "Parser status: " <<
    pdxcp_cdcl_parser_status_string(status) << "\nParser error text: " <<
    (
      (status == pdxcp_cdcl_parser_status_parse_err) ?
        errinfo.parser.text : "(none)"
    );
#else
  GTEST_SKIP();
#endif  // !defined(PDXCP_HAS_FMEMOPEN)
}

// simple declarations
INSTANTIATE_TEST_SUITE_P(
  SimpleDecls,
  ParserParamTest,
  ::testing::Values(
    ParserParamTestInput{"int **x;"},
    ParserParamTestInput{"double *y;"},
    ParserParamTestInput{"struct my_struct_1 **z;"}
  )
);

// cv-qualified declarations
INSTANTIATE_TEST_SUITE_P(
  QualDecls,
  ParserParamTest,
  ::testing::Values(
    ParserParamTestInput{"const int *const a;"},
    ParserParamTestInput{"volatile void *volatile *const b;"},
    ParserParamTestInput{"const enum my_enum **const * const volatile c;"}
  )
);

// declarations with extra parentheses
INSTANTIATE_TEST_SUITE_P(
  ExtraParenDecls,
  ParserParamTest,
  ::testing::Values(
    // surprisingly, these are accepted by actual compilers
    ParserParamTestInput{"int (**(*b));"},
    ParserParamTestInput{"const volatile double (**c);"},
    ParserParamTestInput{"volatile struct my_struct **((*(d)));"}  // legal
  )
);

// declarations with array specifiers
INSTANTIATE_TEST_SUITE_P(
  ArrayDecls,
  ParserParamTest,
  ::testing::Values(
    ParserParamTestInput{"double *x[100];"},
    ParserParamTestInput{"unsigned int *arr[10][100];"},
    // technically not accepted by actual compilers, as first dimension size
    // can only be missing if declared as a function parameter
    ParserParamTestInput{"const unsigned char **b[][50];"},
    ParserParamTestInput{"struct my_struct *const *b[100];"},
    // add extra parentheses since this is technically legal
    ParserParamTestInput{"volatile enum new_enum (**const *c)[90];"}
  )
);

/**
 * Struct holding the input for a `ParserErrorParamTest`.
 *
 * @param input Input fragment to parse
 * @param status Expected parser status
 * @param message Expected parser error message
 */
struct ParserErrorParamTestInput {
  const std::string input;
  const pdxcp_cdcl_parser_status status;
  const std::string message;
};

/**
 * Google Test value printer for `ParserErrorParamTestInput`.
 */
void PrintTo(const ParserErrorParamTestInput& input, std::ostream* out)
{
  *out << "{" << input.input << ", " <<
    pdxcp_cdcl_parser_status_string(input.status) << ", " <<
    input.message << "}";
}

/**
 * C declaration parser parametrized test fixture for testing parser errors.
 */
class ParserErrorParamTest
  : public ParserTest,
    public ::testing::WithParamInterface<ParserErrorParamTestInput> {};

/**
 * Test that the correct status and error are emitted on parsing failures.
 */
TEST_P(ParserErrorParamTest, ErrorTest)
{
#if defined(PDXCP_HAS_FMEMOPEN)
  // create input stream
  auto stream = pdxcp::memopen_string(GetParam().input);
  // parse and write error info
  pdxcp_cdcl_parser_errinfo errinfo;
  auto status = pdxcp_cdcl_stream_parse(stream, stdout, &errinfo);
  // check that returned status and errinfo status are the same
  ASSERT_EQ(status, errinfo.parser.status) << "Parser returned " <<
    pdxcp_cdcl_parser_status_string(status) << " while errinfo received " <<
    pdxcp_cdcl_parser_status_string(errinfo.parser.status);
  // check that error status and message are as expected
  EXPECT_EQ(GetParam().status, errinfo.parser.status) << "expected: " <<
    pdxcp_cdcl_parser_status_string(GetParam().status) << ", actual: " <<
    pdxcp_cdcl_parser_status_string(errinfo.parser.status);
  // check error info based on the status
  switch (status) {
    // TODO: consider handling pdxcp_cdcl_parser_status_lexer_err so we can
    // check the lexer error status and possible message if any
    // parser error
    case pdxcp_cdcl_parser_status_parse_err:
      EXPECT_EQ(GetParam().message, errinfo.parser.text);
      break;
    // no-op
    default:
      break;
  }
  // parser error text meaningful only if pdxcp_cdcl_parser_status_parse_err.
  // we use braces because GTEST_AMBIGUOUS_ELSE_BLOCKER_ doesn't actually work
  // with GCC 11.3 as the Google Test writers may have expected
  if (status == pdxcp_cdcl_parser_status_parse_err) {
    EXPECT_EQ(GetParam().message, errinfo.parser.text);
  }
#else
  GTEST_SKIP();
#endif  // !defined(PDXCP_HAS_FMEMOPEN)
}

// simple declaration mishaps
INSTANTIATE_TEST_SUITE_P(
  SimpleDecls,
  ParserErrorParamTest,
  ::testing::Values(
    ParserErrorParamTestInput{"int **;", pdxcp_cdcl_parser_status_lexer_err, ""},
    ParserErrorParamTestInput{"int a", pdxcp_cdcl_parser_status_lexer_err, ""},
    ParserErrorParamTestInput{
      "*y;",
      pdxcp_cdcl_parser_status_parse_err,
      "Unexpectedly ran out of tokens when parsing pointers, missing type"
    },
    ParserErrorParamTestInput{
      "double *yyy * x;",
      pdxcp_cdcl_parser_status_parse_err,
      "Incomplete declaration for identifier yyy"
    },
    // note: if there is a cv-qualifier before the unexpected token, then the
    // error reported is from stream_parse_ptrs, not stream_parse_type
    ParserErrorParamTestInput{
      "enum my_enum * [ const volatile abc;",
      pdxcp_cdcl_parser_status_parse_err,
      "Unexpected token type pdxcp_cdcl_token_type_langle with text \"\" when "
      "parsing pointers"
    },
    ParserErrorParamTestInput{
      "struct my_struct * [ def;",
      pdxcp_cdcl_parser_status_parse_err,
      "Unexpected token type pdxcp_cdcl_token_type_langle with text \"\" when "
      "parsing identifier type"
    }
  )
);

// declarations with mismatched parentheses
INSTANTIATE_TEST_SUITE_P(
  ParenDecls,
  ParserErrorParamTest,
  ::testing::Values(
    ParserErrorParamTestInput{
      "const double ((**(*x));",
      pdxcp_cdcl_parser_status_parse_err,
      "Mismatched parentheses when parsing pointers, read 3 '(' 2 ')'"
    },
    ParserErrorParamTestInput{
      "const volatile int ((**(*x)))));",
      pdxcp_cdcl_parser_status_parse_err,
      "Mismatched parentheses when parsing pointers, read 3 '(' 5 ')'"
    }
  )
);

// invalid array declarations
INSTANTIATE_TEST_SUITE_P(
  ArrayDecls,
  ParserErrorParamTest,
  ::testing::Values(
    ParserErrorParamTestInput{
      "volatile void *x[100[];",
      pdxcp_cdcl_parser_status_parse_err,
      "Array specifier contains duplicate left angle bracket"
    },
    ParserErrorParamTestInput{
      "const double **y[]1000];",
      pdxcp_cdcl_parser_status_parse_err,
      "Array specifier size read without matching left angle bracket"
    },
    ParserErrorParamTestInput{
      "unsigned int z[88]]];",
      pdxcp_cdcl_parser_status_parse_err,
      "Array specifier missing matching left angle bracket"
    },
    ParserErrorParamTestInput{
      "double **a[100][];",
      pdxcp_cdcl_parser_status_parse_err,
      "Multidimensional array specifier must have bounds for all dimensions "
      "except for the first"
    },
    ParserErrorParamTestInput{
      "const double b[100](;",
      pdxcp_cdcl_parser_status_parse_err,
      "Unexpected token type pdxcp_cdcl_token_type_lparen with text \"\" when "
      "parsing array specifiers"
    },
    ParserErrorParamTestInput{
      "const double b[100][50]",
      pdxcp_cdcl_parser_status_lexer_err,
      ""
    }
  )
);

}  // namespace
