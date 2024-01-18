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

# C compiler path (required)
CC_PATH = $(shell which $(CC))
ifeq ($(CC),)
$(error Could not find required C compiler)
endif
$(info CC path: $(CC_PATH))
# C compiler version string
CC_VERSION_STRING = $(shell $(CC) --version | head -1)
$(info CC version: $(CC_VERSION_STRING))
# C standard, default to C11 with GNU extensions
C_STANDARD ?= gnu11
$(info C standard: $(C_STANDARD))
# C++ compiler path (optional)
CXX_PATH = $(shell which $(CXX))
$(info CXX path: $(CXX_PATH))
# C++ compiler version string
ifneq ($(CXX_PATH),)
CXX_VERSION_STRING = $(shell $(CXX) --version | head -1)
$(info CXX version: $(CXX_VERSION_STRING))
else
CXX_VERSION_STRING =
$(info CXX version: None)
endif
# C++ standard, default to C++17 with GNU extensions. if no C++ compiler, empty
ifneq ($(CXX_PATH),)
CXX_STANDARD ?= gnu++17
$(info C++ standard: $(CXX_STANDARD))
else
CXX_STANDARD =
$(info C++ standard: None)
endif

# Debug by default
CONFIG ?= Debug
# either Debug or Release only
ifeq ($(CONFIG),Debug)
$(info Config: $(CONFIG))
else ifeq ($(CONFIG),Release)
$(info Config: $(CONFIG))
else
$(error CONFIG must be either Debug or Release)
endif

# build directory. could use absolute, but relative is shorter
BUILDDIR ?= build
$(info Build directory: $(BUILDDIR))

# shared library is on by default
BUILD_SHARED ?= 1

# determine library config
ifneq ($(BUILD_SHARED),)
LIBCONFIG = Shared
else
LIBCONFIG = Static
endif
$(info Build libraries as: $(LIBCONFIG))

# support library link name, stem, file with suffix
LIBNAME = pdxcp
LIBSTEM = lib$(LIBNAME)
ifneq ($(BUILD_SHARED),)
LIBFILE = $(LIBSTEM).so
else
LIBFILE = $(LIBSTEM).a
endif

# cdcl support library link name, stem, file with suffix
CDCL_LIBNAME = $(LIBNAME)_cdcl
CDCL_LIBSTEM = lib$(CDCL_LIBNAME)
ifneq ($(BUILD_SHARED),)
CDCL_LIBFILE = $(CDCL_LIBSTEM).so
else
CDCL_LIBFILE = $(CDCL_LIBSTEM).a
endif

# message about libraries we're building
$(info Libraries: $(LIBFILE) $(CDCL_LIBFILE))

# check if we are on Windows (MinGW). note extra space in sed pattern
WIN32 = $(shell \
echo | gcc -dM -E - | grep '\#define _WIN32' | sed 's/\#define _WIN32 //g')
ifneq ($(WIN32),)
$(info Win32: Yes)
else
$(info Win32: No)
endif

# executable suffix (.exe on Windows)
ifneq ($(WIN32),)
EXESUFFIX = .exe
else
EXESUFFIX =
endif
$(info Executable suffix: $(EXESUFFIX))

# pkg-config path (for Google Test)
PKG_CONFIG = $(shell which pkg-config)
$(info pkg-config: $(PKG_CONFIG))
# minimum Google Test version, both as version string and integral
GTEST_MIN_VERSION = 1.10.0
GTEST_MIN_VERSION_INT = $(shell echo $(GTEST_MIN_VERSION) | sed 's/\./0/g')
# Google Test version string + integral (for testing)
ifneq ($(PKG_CONFIG),)
GTEST_VERSION = $(shell $(PKG_CONFIG) --modversion gtest)
GTEST_VERSION_INT = $(shell echo $(GTEST_VERSION) | sed 's/\./0/g')
else
GTEST_VERSION =
GTEST_VERSION_INT = 0
endif
# if empty, could not find using pkg-config
ifneq ($(GTEST_VERSION),)
$(info Google Test: $(GTEST_VERSION) (req. >=$(GTEST_MIN_VERSION)))
else
$(info Google Test: None (req. >=$(GTEST_MIN_VERSION)))
endif
# check if Google Test version is greater than the minimum
GTEST_VERSION_OK = \
$(shell \
if [ $(GTEST_VERSION_INT) -ge $(GTEST_MIN_VERSION_INT) ]; \
then echo 1; else echo; fi)
# build tests or not. automatically on if Google Test found using pkg-config,
# but if no C++ compiler, not found, or version too low, forces BUILD_TESTS=
ifeq ($(CXX_PATH),)
BUILD_TESTS =
else ifeq ($(GTEST_VERSION),)
BUILD_TESTS =
else ifeq ($(GTEST_VERSION_OK),)
BUILD_TESTS =
else
BUILD_TESTS ?= 1
endif
# print test build status
ifneq ($(BUILD_TESTS),)
$(info Build tests: Yes)
else
$(info Build tests: No)
endif

# include + link directories
INCLUDE_FLAGS = -Iinclude
LIBRARY_FLAGS = -L$(BUILDDIR)

# base C compile flags
BASE_CFLAGS = $(INCLUDE_FLAGS) -Wall
ifeq ($(CONFIG),Release)
BASE_CFLAGS += -O3 -mtune=native
else
BASE_CFLAGS += -g
endif

# base linker flags
BASE_LDFLAGS = $(LIBRARY_FLAGS)

# enable AddressSanitizer
ifeq ($(ENABLE_ASAN),)
$(info AddressSanitizer: Disabled)
else
$(info AddressSanitizer: Enabled)
BASE_CFLAGS += -fsanitize=address
BASE_LDFLAGS += -fsanitize=address
endif

# enable gprof
ifeq ($(ENABLE_PG),)
$(info Profile generation: Disabled)
else
$(info Profile generation: Enabled)
BASE_CFLAGS += -pg
BASE_LDFLAGS += -pg
endif

# base C++ compile flags. expand simply to avoid picking up further updates
BASE_CXXFLAGS := $(BASE_CFLAGS)

# add C version flags. need to do this here since base C++ flags inherits it
BASE_CFLAGS += -std=$(C_STANDARD)
# add C++ version flags if they exist
ifneq ($(CXX_STANDARD),)
BASE_CXXFLAGS += -std=$(CXX_STANDARD)
endif

# Google Test compile/link flags + add to C++ flags
ifneq ($(GTEST_VERSION),)
GTEST_CFLAGS = $(shell $(PKG_CONFIG) --cflags gtest)
GTEST_LIBS = $(shell $(PKG_CONFIG) --libs gtest)
GTEST_MAIN_LIBS = $(shell $(PKG_CONFIG) --libs gtest_main)
BASE_CXXFLAGS += $(GTEST_CFLAGS)
else
GTEST_CFLAGS =
GTEST_LIBS =
GTEST_MAIN_LIBS =
endif

# extra linker flags for shared libraries
SOFLAGS = -shared -fPIC

# object suffix for PIC objects (to match shared library rule)
ifneq ($(BUILD_SHARED),)
LIBOBJSUFFIX = PIC.o
else
LIBOBJSUFFIX = o
endif

# add current location of object to rpath. needed to find shared libraries in
# the same directory at runtime. we don't set this as a default for now
ifneq ($(BUILD_SHARED),)
RPATH_FLAGS = -Wl,-rpath,'$$ORIGIN'
else
RPATH_FLAGS =
endif

# check that we have color capabilities
TERMCOLORS = $(shell tput colors)
ifneq ($(TERMCOLORS),)
$(info Terminal colors: $(TERMCOLORS))
# set colors and normal. F for foreground, I for high-intensity
TFGREEN = $(shell tput setaf 2)
TFCYAN = $(shell tput setaf 6)
TFIGREEN = $(shell tput setaf 10)
TNORMAL = $(shell tput sgr0)
else
$(info Terminal colors: None)
TFGREEN =
TFCYAN =
TFIGREEN =
TNORMAL =
endif

# C object compile rule
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	@printf "$(TFGREEN)Building C object $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -MMD -MT $@ -MF $@.d -o $@ -c $<

# C -fPIC object compile rule (shared library)
$(BUILDDIR)/%.PIC.o: %.c
	@mkdir -p $(@D)
	@printf "$(TFGREEN)Building C object $@$(TNORMAL)\n"
	@$(CC) -fPIC $(BASE_CFLAGS) $(CFLAGS) -MMD -MT $@ -MF $@.d -o $@ -c $<

# C++ object compile rule
$(BUILDDIR)/%.cc.o: %.cc
	@mkdir -p $(@D)
	@printf "$(TFGREEN)Building C++ object $@$(TNORMAL)\n"
	@$(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) -MMD -MT $@ -MF $@.d -o $@ -c $<

# C++ -fPIC object compile rule (shared library)
$(BUILDDIR)/%.PIC.cc.o: %.cc
	@mkdir -p $(@D)
	@printf "$(TFGREEN)Building C++ object $@$(TNORMAL)\n"
	@$(CXX) -fPIC $(BASE_CXXFLAGS) $(CXXFLAGS) -MMD -MT $@ -MF $@.d -o $@ -c $<

# phony targets
.PHONY: clean

# build all targets. segsizes should go last so the print statements are done
# after all the other targets are built as the final build step.
all: \
$(BUILDDIR)/$(LIBFILE) \
$(BUILDDIR)/$(CDCL_LIBFILE) \
$(BUILDDIR)/pdxcp_test \
$(BUILDDIR)/rejmp \
$(BUILDDIR)/sigcatch \
$(BUILDDIR)/locapprox \
$(BUILDDIR)/sigbus \
$(BUILDDIR)/sigsegv \
$(BUILDDIR)/kbsig \
$(BUILDDIR)/kbpoll \
segsizes
	@echo "All targets built"

# cleanup
clean:
	@$(RM) -rv $(BUILDDIR)

###############################################################################

# libpdxcp: support library with some shared utility code
LIB_OBJS = $(BUILDDIR)/src/pdxcp/lockable.$(LIBOBJSUFFIX)
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
CDCL_LIB_OBJS = $(BUILDDIR)/src/pdxcp_cdp/cdcl_lexer.$(LIBOBJSUFFIX)
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

# pdxcp_test: C++ Google Test executable
# if not building tests, object list should be empty to prevent compilation
ifneq ($(BUILD_TESTS),)
TEST_OBJS = \
$(BUILDDIR)/test/lockable_test.cc.o \
$(BUILDDIR)/test/string_test.cc.o
-include $(TEST_OBJS:%=%.d)
else
TEST_OBJS =
endif
TEST_LIBS = $(GTEST_MAIN_LIBS) -l$(LIBNAME)
TEST_LDFLAGS = $(BASE_LDFLAGS) $(RPATH_FLAGS) $(LDFLAGS)
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

# final ls + size call for showing the segment sizes for segsize[N]. note that
# on Windows (MinGW), we need to also add the .exe suffix. awk is used to
# filter the ls output so that we only print file name + sizes. we also depend
# on the pdxcp_test unit test runner to ensure this is built last
SEGSIZE_TGTS = $(BUILDDIR)/segsize1 \
$(BUILDDIR)/segsize2 \
$(BUILDDIR)/segsize3 \
$(BUILDDIR)/segsize4a \
$(BUILDDIR)/segsize4b \
$(BUILDDIR)/segsize5d \
$(BUILDDIR)/segsize5r
segsizes: $(BUILDDIR)/pdxcp_test $(SEGSIZE_TGTS)
	@echo
	@echo "segsize[N] disk sizes"
	@ls -l build/segsize* | awk '{print "  " $$5 "  " $$9}'
	@echo
	@echo "segsize[N] image sizes"
	@size $(SEGSIZE_TGTS:%=%$(EXESUFFIX))
	@echo

# segsize1: program to get segment sizes (1)
$(BUILDDIR)/segsize1: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -o $@ $<
	@echo "Built target $@"

# segsize2: program to get segment sizes (2)
$(BUILDDIR)/segsize2: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=2 -o $@ $<
	@echo "Built target $@"

# segsize3: program to get segment sizes (3)
$(BUILDDIR)/segsize3: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=3 -o $@ $<
	@echo "Built target $@"

# segsize4a: program to get segment sizes (4a, uninitialized auto array)
$(BUILDDIR)/segsize4a: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -o $@ $<
	@echo "Built target $@"

# segsize4b: program to get segment sizes (4b, initialized auto array)
$(BUILDDIR)/segsize4b: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -DSEGSIZE_STEPS_4B -o $@ $<
	@echo "Built target $@"

# segsize5d: program to get segment sizes (5, 4b compiled with debug flags)
$(BUILDDIR)/segsize5d: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) -Wall -g -DSEGSIZE_STEPS=5 -o $@ $<
	@echo "Built target $@"

# segsize5r: program to get segment sizes (5, 4b compiled with release flags)
$(BUILDDIR)/segsize5r: src/segsize.c
	@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
	@$(CC) -Wall -O3 -mtune=native -DSEGSIZE_STEPS=5 -o $@ $<
	@echo "Build target $@"
