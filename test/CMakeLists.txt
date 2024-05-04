cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_executable(
    pdxcp_test
        bvector_test.cc
        cdcl_lexer_test.cc
        cdcl_parser_test.cc
        lockable_test.cc
        string_test.cc
)
target_link_libraries(pdxcp_test PRIVATE GTest::gtest_main pdxcp pdxcp_cdp)
