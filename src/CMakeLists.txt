cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# add C support library, C declaration parser library, and C++ fruit library
add_subdirectory(pdxcp)
add_subdirectory(pdxcp_cdp)
add_subdirectory(pdxcp_fruit)

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
    # this way we don't have to worry about prepending paths to the targets
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run comparison of segsize[N] disk and image sizes"
    VERBATIM
    COMMAND_EXPAND_LISTS
)
# segsizes depends on all other targets including libraries
add_dependencies(
    segsizes
        pdxcp
        pdxcp_cdp
        rejmp
        sigcatch
        locapprox
        sigbus
        sigsegv
        kbsig
        kbpoll
        ${PDXCP_SEGSIZE_TARGETS}
        filehash
        zerobits
        arrptrcmp
        mdarrinc
        arrptrbind
        dynarray
)
# only add pdxcp_test if tests are being built
if(BUILD_TESTS)
    add_dependencies(segsizes pdxcp_test)
endif()
# only add pdxcp_fruit and other C++ programs if C++ compiler is available
if(CMAKE_CXX_COMPILER)
    add_dependencies(
        segsizes
            pdxcp_fruit
            arrptrbind++
            fruit1
            fruit2
    )
endif()

# filehash: hash-table based file info struct lookup program. this does not
# actually allocate any file descriptors and is just to demonstrate hash table
# lookup. define FILE_HASH to 1 for a trivial hash that returns zero
add_executable(filehash filehash.c)
# zerobits: check that 0.0 and 0 have the same bits. true on most machines
add_executable(zerobits zerobits.c)
# arrptrcmp: compare addressing semantics between arrays and pointers
add_executable(arrptrcmp arrptrcmp.c)
# mdarrinc: multidimensional array address increment
add_executable(mdarrinc mdarrinc.c)
# arrptrbind: array/pointer function argument binding
add_executable(arrptrbind arrptrbind.c)
# dynarray: dynamic array expansion
add_executable(dynarray dynarray.c)
# C++ programs only compiled if compiler is available
if(CMAKE_CXX_COMPILER)
    # arrptrbind++: C++ array/pointer function argument binding
    add_executable(arrptrbind++ arrptrbind.cc)
    # fruit1: compiling and running a C++ program
    add_executable(fruit1 fruit1.cc)
    target_link_libraries(fruit1 PRIVATE pdxcp_fruit)
    # fruit2: calling C++ fruit member functions
    add_executable(fruit2 fruit2.cc)
    target_link_libraries(fruit2 PRIVATE pdxcp_fruit)
    # fruit3: C++ program demonstrating fruit addition and polymorphism
    add_executable(fruit3 fruit3.cc)
    target_link_libraries(fruit3 PRIVATE pdxcp_fruit)
endif()
