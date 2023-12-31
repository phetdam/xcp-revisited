cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# add pdxcp directory for support library
add_subdirectory(pdxcp)

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
set_target_properties(segsize5d PROPERTIES COMPILE_OPTIONS "-Wall;-g")
# segsize5r: program to get segment sizes (5, 4b compiled with release flags)
add_executable(segsize5r segsize.c)
target_compile_definitions(segsize5r PRIVATE SEGSIZE_STEPS=5)
set_target_properties(
    segsize5r PROPERTIES COMPILE_OPTIONS "-Wall;-O3;-mtune=native"
)
