# File: Makefile
# Author: Derek Huang
# Brief: Makefile for the Expert C Programming exercises
# Copyright: MIT License
#
# Debug build:
# make CC=gcc
#
# Release build:
# make CC=gcc CONFIG=Release
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
# Google Test version (for testing)
ifneq ($(PKG_CONFIG),)
GTEST_VERSION = $(shell $(PKG_CONFIG) --modversion gtest)
else
GTEST_VERSION =
endif
# if empty, could not find using pkg-config
ifneq ($(GTEST_VERSION),)
$(info Google Test: $(GTEST_VERSION))
else
$(info Google Test: None)
endif
# build tests or not. automatically on if Google Test found using pkg-config,
# but if not found/no C++ compiler, overrides user option specified
ifeq ($(GTEST_VERSION),)
BUILD_TESTS =
else ifeq ($(CXX_PATH),)
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

# base compile flags
BASE_CFLAGS = $(INCLUDE_FLAGS) $(LIBRARY_FLAGS) -Wall
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

# Google Test compile/link flags
ifneq ($(GTEST_VERSION),)
GTEST_CXXFLAGS = $(shell $(PKG_CONFIG) --cflags)
GTEST_LDFLAGS = $(shell $(PKG_CONFIG) --libs)
else
GTEST_CXXFLAGS =
GTEST_LDFLAGS =
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

# include directory with namespace
INCLUDENS = include/pdxcp

# couldn't get automatic header dependency generation using -MMD or other GCC
# -MM options to work so we track project headers manually here
HEADERS = $(shell ls $(INCLUDENS))
# need simple expansion or expansion will be recursive
HEADERS := $(HEADERS:%.h=$(INCLUDENS)/%.h)
HEADERS := $(HEADERS:%.hh=$(INCLUDENS)/%.hh)

# C object compile rule
$(BUILDDIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -o $@ -c $<

# C -fPIC object compile rule (shared library)
$(BUILDDIR)/%.PIC.o: %.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) -fPIC $(BASE_CFLAGS) $(CFLAGS) -o $@ -c $<

# C++ object compile rule
$(BUILDDIR)/%.cc.o: %.cc $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(BASE_CFLAGS) $(CXXFLAGS) -o $@ -c $<

# C++ -fPIC object compile rule (shared library)
$(BUILDDIR)/%.PIC.cc.o: %.cc $(HEADERS)
	@mkdir -p ($@D)
	$(CXX) -fPIC $(BASE_CFLAGS) $(CXXFLAGS) -o $@ -c $<

# phony targets
.PHONY: clean

# build all targets. segsizes should go last so the print statements are done
# after all the other targets are built as the final build step.
all: \
$(BUILDDIR)/$(LIBFILE) \
$(BUILDDIR)/$(CDCL_LIBFILE) \
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
$(BUILDDIR)/$(LIBFILE): $(BUILDDIR)/src/pdxcp/lockable.$(LIBOBJSUFFIX)
	@echo "Linking $@..."
ifneq ($(BUILD_SHARED),)
	$(CC) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
else
	$(AR) crs $@ $^
endif

# libpdxcp_cdcl: cdcl C declaration parser support library
$(BUILDDIR)/$(CDCL_LIBFILE): $(BUILDDIR)/src/pdxcp_cdp/cdcl_lexer.$(LIBOBJSUFFIX)
	@echo "Linking $@..."
ifneq ($(BUILD_SHARED),)
	$(CC) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
else
	$(AR) crs $@ $^
endif

# rejmp: uses setjmp/longjmp to restart itself
$(BUILDDIR)/rejmp: $(BUILDDIR)/src/rejmp.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# sigcatch: catches and handles SIGBREAK and SIGINT
$(BUILDDIR)/sigcatch: $(BUILDDIR)/src/sigcatch.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# locapprox: finds approximate stack, data, text, heap locations
$(BUILDDIR)/locapprox: $(BUILDDIR)/src/locapprox.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# sigbus: creates and catches a bus error caused by misaligned address use
$(BUILDDIR)/sigbus: $(BUILDDIR)/src/sigbus.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# sigsegv: creates and catches segmentation fault caused by null pointer use
$(BUILDDIR)/sigsegv: $(BUILDDIR)/src/sigsegv.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# kbsig: signal-driven input handling program
$(BUILDDIR)/kbsig: $(BUILDDIR)/src/kbsig.o
	@echo "Linking $@..."
	$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^

# kbpoll: event-driven input handling program using pthreads. this is a more
# realistic implementation of what kbsig is trying to do using poll()
$(BUILDDIR)/kbpoll: src/kbpoll.c $(HEADERS) $(BUILDDIR)/$(LIBFILE)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) -pthread $(CFLAGS) $(RPATH_FLAGS) -o $@ $< -l$(LIBNAME)

# final ls + size call for showing the segment sizes for segsize[N]. note that
# on Windows (MinGW), we need to also add the .exe suffix. awk is used to
# filter the ls output so that we only print file name + sizes
segsizes: \
$(BUILDDIR)/segsize1 \
$(BUILDDIR)/segsize2 \
$(BUILDDIR)/segsize3 \
$(BUILDDIR)/segsize4a \
$(BUILDDIR)/segsize4b \
$(BUILDDIR)/segsize5d \
$(BUILDDIR)/segsize5r
	@echo
	@echo "segsize[N] disk sizes"
	@ls -l build/segsize* | awk '{print "  " $$5 "  " $$9}'
	@echo
	@echo "segsize[N] image sizes"
	@size $(^:%=%$(EXESUFFIX))
	@echo

# segsize1: program to get segment sizes (1)
$(BUILDDIR)/segsize1: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -o $@ $<

# segsize2: program to get segment sizes (2)
$(BUILDDIR)/segsize2: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=2 -o $@ $<

# segsize3: program to get segment sizes (3)
$(BUILDDIR)/segsize3: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=3 -o $@ $<

# segsize4a: program to get segment sizes (4a, uninitialized auto array)
$(BUILDDIR)/segsize4a: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -o $@ $<

# segsize4b: program to get segment sizes (4b, initialized auto array)
$(BUILDDIR)/segsize4b: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) $(CFLAGS) -DSEGSIZE_STEPS=4 -DSEGSIZE_STEPS_4B -o $@ $<

# segsize5d: program to get segment sizes (5, 4b compiled with debug flags)
$(BUILDDIR)/segsize5d: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) -Wall -g -DSEGSIZE_STEPS=5 -o $@ $<

# segsize5r: program to get segment sizes (5, 4b compiled with release flags)
$(BUILDDIR)/segsize5r: src/segsize.c $(HEADERS)
	@echo "Building $@..."
	$(CC) -Wall -O3 -mtune=native -DSEGSIZE_STEPS=5 -o $@ $<
