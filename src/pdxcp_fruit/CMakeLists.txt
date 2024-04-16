cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

if(CMAKE_CXX_COMPILER)
    add_library(pdxcp_fruit fruit.cc)
    set_target_properties(
        pdxcp_fruit PROPERTIES
        DEFINE_SYMBOL PDXCP_FRUIT_BUILD_DLL
    )
endif()
