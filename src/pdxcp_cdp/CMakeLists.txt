cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

add_library(pdxcp_cdp cdcl_lexer.c cdcl_parser.c)
set_target_properties(pdxcp_cdp PROPERTIES DEFINE_SYMBOL PDXCP_CDP_BUILD_DLL)
