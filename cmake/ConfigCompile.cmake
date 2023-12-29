cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# not ready for Windows yet
if(WIN32)
    message(FATAL_ERROR "Cannot build project on Windows (yet)")
endif()

# whenever MSVC is supported...
if(MSVC)
    add_compile_options(/Wall)
    # note: /fsanitize=address does not seem to work well with Google Test.
    # not sure if this has any relation to whether debug/release CRT is used
    if(ENABLE_ASAN)
        message(WARNING "ENABLE_ASAN ignored on Windows")
    endif()
else()
    add_compile_options(-Wall)
    # enable AddressSanitizer
    if(ENABLE_ASAN)
        # need to specify for both compile + link
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
        message(STATUS "AddressSanitizer (-fsanitize=address) enabled")
    endif()
endif()
