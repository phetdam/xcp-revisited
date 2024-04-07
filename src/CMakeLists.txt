cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# add support library and C declaration parser library
add_subdirectory(pdxcp)
add_subdirectory(pdxcp_cdp)

# rejmp: uses setjmp/longjmp to restart itself
add_executable(rejmp rejmp.c)
# sigcatch: catches and handles SIGBREAK and SIGINT
add_executable(sigcatch sigcatch.c)
# locapprox: finds approximate stack, data, text, heap locations
add_executable(locapprox locapprox.c)
# sigbus: creates and catches a bus error caused by misaligned address use
add_executable(sigbus sigbus.c)
# sigsegv: creates and catches segmentation fault caused by null pointer use
add_executable(sigsegv sigsegv.c)
# kbsig: signal-driven input handling program
add_executable(kbsig kbsig.c)
# kbpoll: event-driven input handling program using pthreads. this is a more
# realistic implementation of what kbsig is trying to do using poll()
add_executable(kbpoll kbpoll.c)
target_compile_options(kbpoll PRIVATE -pthread)
target_link_options(kbpoll PRIVATE -pthread)
target_link_libraries(kbpoll PRIVATE pdxcp)
# filehash: hash-table based file info struct lookup program. this does not
# actually allocate any file descriptors and is just to demonstrate hash table
# lookup. define FILE_HASH to 1 for a trivial hash that returns zero
add_executable(filehash filehash.c)

# segsize1: program to get segment sizes (1)
add_executable(segsize1 segsize.c)
# segsize2: program to get segment sizes (2)
add_executable(segsize2 segsize.c)
target_compile_definitions(segsize2 PRIVATE SEGSIZE_STEPS=2)
# segsize3: program to get segment sizes (3)
add_executable(segsize3 segsize.c)
target_compile_definitions(segsize3 PRIVATE SEGSIZE_STEPS=3)
# segsize4a: program to get segment sizes (4a, uninitialized auto array)
add_executable(segsize4a segsize.c)
target_compile_definitions(segsize4a PRIVATE SEGSIZE_STEPS=4)
# segsize4b: program to get segment sizes (4b, initialized auto array)
add_executable(segsize4b segsize.c)
target_compile_definitions(segsize4b PRIVATE SEGSIZE_STEPS=4 SEGSIZE_STEPS_4B)
# segsize5d: program to get segment sizes (5, 4b compiled with debug flags)
add_executable(segsize5d segsize.c)
target_compile_definitions(segsize5d PRIVATE SEGSIZE_STEPS=5)
target_compile_options(segsize5d PRIVATE -Wall -g -O0)
# segsize5r: program to get segment sizes (5, 4b compiled with release flags)
add_executable(segsize5r segsize.c)
target_compile_definitions(segsize5r PRIVATE SEGSIZE_STEPS=5)
target_compile_options(segsize5r PRIVATE -Wall -O3 -mtune=native)

# convenience variable for all the segsize.c outputs
set(
    PDXCP_SEGSIZE_TARGETS
    segsize1 segsize2 segsize3 segsize4a segsize4b segsize5d segsize5r
)
# convenience variable to hold targets we manually pin segsizes to ensure that
# it ends up being built last. pdxcp_test not added if BUILD_TESTS is false
set(PDXCP_SEGSIZES_PINS kbpoll)
# note: when pdxcp_test also ends up depending on the support libraries, it
# will be the only dependency pin that is needed by segsizes
if(BUILD_TESTS)
    set(PDXCP_SEGSIZES_PINS ${PDXCP_SEGSIZES_PINS} pdxcp_test)
endif()
# final post-build command for printing the disk sizes
add_custom_target(
    segsizes ALL
            echo
    COMMAND echo "segsize[N] disk sizes"
    # can't use glob so we run ls -l output through grep. CMake automatically
    # double quotes the argument so for awk we double quote ourselves so that
    # the parser knows that there is but a single string argument sent to awk
    COMMAND ls -l | grep segsize | awk "{print \"  \" $5 \"  \" $9}"
    COMMAND echo
    COMMAND echo "segsize[N] image sizes"
    COMMAND size ${PDXCP_SEGSIZE_TARGETS}
    COMMAND echo
    # by depending on kbpoll + pdxcp_test, we guarantee segsizes is built last.
    # kbpoll also depends on the pdxcp target so by the time everything is
    # linked there should be no other worker processes running
    DEPENDS ${PDXCP_SEGSIZES_PINS} ${PDXCP_SEGSIZE_TARGETS}
    # this way we don't have to worry about prepending paths to the targets
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run comparison of segsize[N] disk and image sizes"
    VERBATIM
    COMMAND_EXPAND_LISTS
)
