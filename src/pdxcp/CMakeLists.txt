cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_library(pdxcp bvector.c lockable.c)
set_target_properties(pdxcp PROPERTIES DEFINE_SYMBOL PDXCP_BUILD_DLL)
