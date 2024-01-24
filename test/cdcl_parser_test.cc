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
  // create input stream + expect successful parse
  auto stream = pdxcp::memopen_string(GetParam().input);
  EXPECT_EQ(
    pdxcp_cdcl_parser_status_ok,
    pdxcp_cdcl_stream_parse(stream, stdout, nullptr)
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
    ParserParamTestInput{"double *y;"}
  )
);

// cv-qualified declarations
INSTANTIATE_TEST_SUITE_P(
  QualDecls,
  ParserParamTest,
  ::testing::Values(
    ParserParamTestInput{"const int *const a;"},
    ParserParamTestInput{"volatile void *volatile *const b;"}
  )
);

}  // namespace
