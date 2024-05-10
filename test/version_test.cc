/**
 * @file version_test.cc
 * @author Derek Huang
 * @brief version.h unit tests
 * @copyright MIT License
 */

#include "pdxcp/version.h"

#include <gtest/gtest.h>

#include "pdxcp/common.h"

namespace {

/**
 * Base version macro testing fixture.
 */
class VersionTest : public ::testing::Test {};

/**
 * Get major version from `PDXCP_VERSION`.
 */
#define GET_MAJOR() PDXCP_VERSION / PDXCP_VERSION_MAJOR_MULTIPLIER

/**
 * Get minor version from `PDXCP_VERSION`.
 */
#define GET_MINOR() (PDXCP_VERSION % PDXCP_VERSION_MAJOR_MULTIPLIER) / \
    PDXCP_VERSION_MINOR_MULTIPLIER

/**
 * Get patch version from `PDXCP_VERSION`.
 */
#define GET_PATCH() ( \
  PDXCP_VERSION % PDXCP_VERSION_MAJOR_MULTIPLIER % \
  PDXCP_VERSION_MINOR_MULTIPLIER) / PDXCP_VERSION_PATCH_MULTIPLIER

/**
 * Create version string from major, minor, patch versions.
 */
#define MAKE_VERSION_STRING() \
  PDXCP_STRINGIFY( \
    PDXCP_CONCAT( \
      PDXCP_CONCAT(PDXCP_VERSION_MAJOR, .), \
      PDXCP_CONCAT(PDXCP_CONCAT(PDXCP_VERSION_MINOR, .), PDXCP_VERSION_PATCH) \
    ) \
  )

/**
 * Test that `PDXCP_VERSION` major version matches.
 */
TEST_F(VersionTest, MajorVersionTest)
{
  EXPECT_EQ(GET_MAJOR(), PDXCP_VERSION_MAJOR);
}

/**
 * Test that `PDXCP_VERSION` minor version matches.
 */
TEST_F(VersionTest, MinorVersionTest)
{
  EXPECT_EQ(GET_MINOR(), PDXCP_VERSION_MINOR);
}

/**
 * Test that `PDXCP_VERSION` patch version matches.
 */
TEST_F(VersionTest, PatchVersionTest)
{
  EXPECT_EQ(GET_PATCH(), PDXCP_VERSION_PATCH);
}

/**
 * Test that stringified version matches.
 */
TEST_F(VersionTest, StringVersionTest)
{
  EXPECT_STREQ(MAKE_VERSION_STRING(), PDXCP_STRING_VERSION);
}

}  // namespace
