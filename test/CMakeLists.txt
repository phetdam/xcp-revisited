cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_executable(pdxcp_test lockable_test.cc string_test.cc)
target_link_libraries(pdxcp_test PRIVATE GTest::gtest_main pdxcp)
