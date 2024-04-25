# File: Makefile
# Author: Derek Huang
# Brief: Makefile for the Expert C Programming exercises
# Copyright: MIT License
#
# Debug build:
# make CC=gcc -j
#
# Release build:
# make CC=gcc CONFIG=Release -j
#

# include C/C++ compile/link and testing configuration
include gmake/config.mk

# include C/C++ object and executable pattern rules
include gmake/patterns.mk

# phony targets
.PHONY: clean

# build all targets. segsizes depends on all targets in NON_SEGSIZE_TGTS
all: segsizes
	@echo "All targets built"

# cleanup
clean:
	@$(RM) -rv $(BUILDDIR)

###############################################################################

# libpdxcp: support library with some shared utility code
LIB_OBJS = \
$(BUILDDIR)/src/pdxcp/bvector.$(LIBOBJSUFFIX) \
$(BUILDDIR)/src/pdxcp/lockable.$(LIBOBJSUFFIX)
-include $(LIB_OBJS:%=%.d)
$(BUILDDIR)/$(LIBFILE): $(LIB_OBJS)
ifneq ($(BUILD_SHARED),)
	@printf "$(TFIGREEN)Linking C shared library $@$(TNORMAL)\n"
	@$(CC) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
else
	@printf "$(TFIGREEN)Linking C static library $@$(TNORMAL)\n"
	@$(AR) crs $@ $^
endif
	@echo "Built target $@"

# libpdxcp_cdcl: cdcl C declaration parser support library
CDCL_LIB_OBJS = \
$(BUILDDIR)/src/pdxcp_cdp/cdcl_lexer.$(LIBOBJSUFFIX) \
$(BUILDDIR)/src/pdxcp_cdp/cdcl_parser.$(LIBOBJSUFFIX)
-include $(CDCL_LIB_OBJS:%=%.d)
$(BUILDDIR)/$(CDCL_LIBFILE): $(CDCL_LIB_OBJS)
ifneq ($(BUILD_SHARED),)
	@printf "$(TFIGREEN)Linking C shared library $@$(TNORMAL)\n"
	@$(CC) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
else
	@printf "$(TFIGREEN)Linking C static library $@$(TNORMAL)\n"
	@$(AR) crs $@ $^
endif
	@echo "Built target $@"

# libpdxcp_fruit: C++ fruit library to support book's C++ exercises
# if no C++ compiler available, object list is empty to prevent compilation
ifneq ($(CXX_PATH),)
FRUIT_LIB_OBJS = $(BUILDDIR)/src/pdxcp_fruit/fruit.$(LIBOBJCXXSUFFIX)
-include $(FRUIT_LIB_OBJS:%=%.d)
else
FRUIT_LIB_OBJS =
endif
$(BUILDDIR)/$(FRUIT_LIBFILE): $(FRUIT_LIB_OBJS)
ifneq ($(CXX_PATH),)
ifneq ($(BUILD_SHARED),)
	@printf "$(TFIGREEN)Linking C++ shared library $@$(TNORMAL)\n"
	@$(CXX) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
else
	@printf "$(TFIGREEN)Linking C++ static library $@$(TNORMAL)\n"
	@$(AR) crs $@ $^
endif
	@echo "Built target $@"
endif

# pdxcp_test: C++ Google Test executable
# if not building tests, object list is empty to prevent compilation
ifneq ($(BUILD_TESTS),)
TEST_OBJS = \
$(BUILDDIR)/test/cdcl_lexer_test.cc.o \
$(BUILDDIR)/test/cdcl_parser_test.cc.o \
$(BUILDDIR)/test/lockable_test.cc.o \
$(BUILDDIR)/test/string_test.cc.o
TEST_LIBS = $(GTEST_MAIN_LIBS) -l$(LIBNAME) -l$(CDCL_LIBNAME)
TEST_LDFLAGS = $(BASE_LDFLAGS) $(RPATH_FLAGS) $(LDFLAGS)
-include $(TEST_OBJS:%=%.d)
else
TEST_OBJS =
TEST_LIBS =
TEST_LDFLAGS =
endif
# link only if we are building tests, otherwise do nothing
$(BUILDDIR)/pdxcp_test: $(BUILDDIR)/$(CDCL_LIBFILE) $(TEST_OBJS)
ifneq ($(BUILD_TESTS),)
	@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
	@$(CXX) $(TEST_LDFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIBS)
	@echo "Built target $@"
endif

# rejmp: uses setjmp/longjmp to restart itself
REJMP_OBJS = $(BUILDDIR)/src/rejmp.o
-include $(REJMP_OBJS:%=%.d)
$(BUILDDIR)/rejmp: $(REJMP_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# sigcatch: catches and handles SIGBREAK and SIGINT
SIGCATCH_OBJS = $(BUILDDIR)/src/sigcatch.o
-include $(SIGCATCH_OBJS:%=%.d)
$(BUILDDIR)/sigcatch: $(SIGCATCH_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# locapprox: finds approximate stack, data, text, heap locations
LOCAPPROX_OBJS = $(BUILDDIR)/src/locapprox.o
-include $(LOCAPPROX_OBJS:%=%.d)
$(BUILDDIR)/locapprox: $(LOCAPPROX_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# sigbus: creates and catches a bus error caused by misaligned address use
SIGBUS_OBJS = $(BUILDDIR)/src/sigbus.o
-include $(SIGBUS_OBJS:%=%.d)
$(BUILDDIR)/sigbus: $(SIGBUS_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# sigsegv: creates and catches segmentation fault caused by null pointer use
SIGSEGV_OBJS = $(BUILDDIR)/src/sigsegv.o
-include $(SIGSEGV_OBJS:%=%.d)
$(BUILDDIR)/sigsegv: $(SIGSEGV_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# kbsig: signal-driven input handling program
KBSIG_OBJS = $(BUILDDIR)/src/kbsig.o
-include $(KBSIG_OBJS:%=%.d)
$(BUILDDIR)/kbsig: $(KBSIG_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# kbpoll: event-driven input handling program using pthreads. this is a more
# realistic implementation of what kbsig is trying to do using poll(). although
# -pthread should technically be used for both compiling and linking, only the
# oldest C libraries require passing -pthread when compiling. see the man page
# for feature_test_macros(7) and look for the text on _REENTRANT
KBPOLL_OBJS = $(BUILDDIR)/src/kbpoll.o
-include $(KBPOLL_OBJS:%=%.d)
KBPOLL_LDFLAGS = $(BASE_LDFLAGS) $(RPATH_FLAGS) $(LDFLAGS)
$(BUILDDIR)/kbpoll: $(KBPOLL_OBJS) $(BUILDDIR)/$(LIBFILE)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(KBPOLL_LDFLAGS) -o $@ $(KBPOLL_OBJS) -lpthread -l$(LIBNAME)
	@echo "Built target $@"

# all targets that are not related to segsizes
NON_SEGSIZE_TGTS = \
$(BUILDDIR)/$(LIBFILE) \
$(BUILDDIR)/$(CDCL_LIBFILE) \
$(BUILDDIR)/$(FRUIT_LIBFILE) \
$(BUILDDIR)/pdxcp_test \
$(BUILDDIR)/rejmp \
$(BUILDDIR)/sigcatch \
$(BUILDDIR)/locapprox \
$(BUILDDIR)/sigbus \
$(BUILDDIR)/sigsegv \
$(BUILDDIR)/kbsig \
$(BUILDDIR)/kbpoll \
$(BUILDDIR)/filehash \
$(BUILDDIR)/zerobits \
$(BUILDDIR)/arrptrcmp \
$(BUILDDIR)/mdarrinc \
$(BUILDDIR)/arrptrbind \
$(BUILDDIR)/arrptrbind++ \
$(BUILDDIR)/dynarray \
$(BUILDDIR)/fruit1 \
$(BUILDDIR)/fruit2 \
$(BUILDDIR)/fruit3

# final ls + size call for showing the segment sizes for segsize[N]. note that
# on Windows (MinGW), we need to also add the .exe suffix. awk is used to
# filter the ls output so that we only print file name + sizes. we depend on
# the NON_SEGSIZE_TGTS with all the non-segsize targets so this rule is last
SEGSIZE_TGTS = $(BUILDDIR)/segsize1 \
$(BUILDDIR)/segsize2 \
$(BUILDDIR)/segsize3 \
$(BUILDDIR)/segsize4a \
$(BUILDDIR)/segsize4b \
$(BUILDDIR)/segsize5d \
$(BUILDDIR)/segsize5r
segsizes: $(NON_SEGSIZE_TGTS) $(SEGSIZE_TGTS)
	@echo
	@echo "segsize[N] disk sizes"
	@ls -l build/segsize* | awk '{print "  " $$5 "  " $$9}'
	@echo
	@echo "segsize[N] image sizes"
	@size $(SEGSIZE_TGTS:%=%$(EXESUFFIX))
	@echo

# segsize1: program to get segment sizes (1)
$(BUILDDIR)/segsize1: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -o $@ $<
	@echo "Built target $@"

# segsize2: program to get segment sizes (2)
$(BUILDDIR)/segsize2: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=2 -o $@ $<
	@echo "Built target $@"

# segsize3: program to get segment sizes (3)
$(BUILDDIR)/segsize3: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=3 -o $@ $<
	@echo "Built target $@"

# segsize4a: program to get segment sizes (4a, uninitialized auto array)
$(BUILDDIR)/segsize4a: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -o $@ $<
	@echo "Built target $@"

# segsize4b: program to get segment sizes (4b, initialized auto array)
$(BUILDDIR)/segsize4b: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -DSEGSIZE_STEPS_4B -o $@ $<
	@echo "Built target $@"

# segsize5d: program to get segment sizes (5, 4b compiled with debug flags)
$(BUILDDIR)/segsize5d: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) -Wall -g -DSEGSIZE_STEPS=5 -o $@ $<
	@echo "Built target $@"

# segsize5r: program to get segment sizes (5, 4b compiled with release flags)
$(BUILDDIR)/segsize5r: src/segsize.c
	@mkdir -p $(@D)
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) -Wall -O3 -mtune=native -DSEGSIZE_STEPS=5 -o $@ $<
	@echo "Build target $@"

# filehash: hash-table based file info struct lookup program. this does not
# actually allocate any file descriptors and is just to demonstrate hash table
# lookup. define FILE_HASH to 1 for a trivial hash that returns zero
FILEHASH_OBJS = $(BUILDDIR)/src/filehash.o
-include $(FILEHASH_OBJS:%=%.d)
$(BUILDDIR)/filehash: $(FILEHASH_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# zerobits: check that 0.0 and 0 have the same bits. true on most machines
ZEROBITS_OBJS = $(BUILDDIR)/src/zerobits.o
-include $(ZEROBITS_OBJS:%=%.d)
$(BUILDDIR)/zerobits: $(ZEROBITS_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# arrptrcmp: compare addressing semantics between arrays and pointers
ARRPTRCMP_OBJS = $(BUILDDIR)/src/arrptrcmp.o
-include $(ARRPTRCMP_OBJS:%=%.d)
$(BUILDDIR)/arrptrcmp: $(ARRPTRCMP_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# mdarrinc: multidimensional array address increment
MDARRINC_OBJS = $(BUILDDIR)/src/mdarrinc.o
-include $(MDARRINC_OBJS:%=%.d)
$(BUILDDIR)/mdarrinc: $(MDARRINC_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# arrptrbind: array/pointer function argument binding
ARRPTRBIND_OBJS = $(BUILDDIR)/src/arrptrbind.o
-include $(ARRPTRBIND_OBJS:%=%.d)
$(BUILDDIR)/arrptrbind: $(ARRPTRBIND_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"

# arrptrbind++: C++ array/pointer function argument binding
ifneq ($(CXX_PATH),)
ARRPTRBINDXX_OBJS = $(BUILDDIR)/src/arrptrbind.cc.o
-include $(ARRPTRBINDXX_OBJS:%=%.d)
else
ARRPTRBINDXX_OBJS =
endif
$(BUILDDIR)/arrptrbind++: $(ARRPTRBINDXX_OBJS)
ifneq ($(CXX_PATH),)
	@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
	@$(CXX) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built target $@"
endif

# dynarray: dynamic array expansion
DYNARRAY_OBJS = $(BUILDDIR)/src/dynarray.o
-include $(DYNARRAY_OBJS:%=%.d)
$(BUILDDIR)/dynarray: $(BUILDDIR)/$(LIBFILE) $(DYNARRAY_OBJS)
	@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
	@$(CC) $(RPATH_LDFLAGS) $(LDFLAGS) -o $@ $(DYNARRAY_OBJS) -l$(LIBNAME)
	@echo "Built target $@"

# fruit1: compiling and running a C++ program
ifneq ($(CXX_PATH),)
FRUIT1_OBJS = $(BUILDDIR)/src/fruit1.cc.o
-include $(FRUIT1_OBJS:%=%.d)
else
FRUIT1_OBJS =
endif
$(BUILDDIR)/fruit1: $(BUILDDIR)/$(FRUIT_LIBFILE) $(FRUIT1_OBJS)
ifneq ($(CXX_PATH),)
	@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
	@$(CXX) $(RPATH_LDFLAGS) $(LDFLAGS) -o $@ $(FRUIT1_OBJS) -l$(FRUIT_LIBNAME)
	@echo "Built target $@"
endif

# fruit2: calling C++ fruit member functions
ifneq ($(CXX_PATH),)
FRUIT2_OBJS = $(BUILDDIR)/src/fruit2.cc.o
-include $(FRUIT2_OBJS:%=%.d)
else
FRUIT2_OBJS =
endif
$(BUILDDIR)/fruit2: $(BUILDDIR)/$(FRUIT_LIBFILE) $(FRUIT2_OBJS)
ifneq ($(CXX_PATH),)
	@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
	@$(CXX) $(RPATH_LDFLAGS) $(LDFLAGS) -o $@ $(FRUIT2_OBJS) -l$(FRUIT_LIBNAME)
	@echo "Built target $@"
endif

# fruit3: C++ program demonstrating fruit addition and polymorphism
ifneq ($(CXX_PATH),)
FRUIT3_OBJS = $(BUILDDIR)/src/fruit3.cc.o
-include $(FRUIT2_OBJS:%=%.d)
else
FRUIT3_OBJS =
endif
$(BUILDDIR)/fruit3: $(BUILDDIR)/$(FRUIT_LIBFILE) $(FRUIT3_OBJS)
ifneq ($(CXX_PATH),)
	@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
	@$(CXX) $(RPATH_LDFLAGS) $(LDFLAGS) -o $@ $(FRUIT3_OBJS) -l$(FRUIT_LIBNAME)
	@echo "Built target $@"
endif
