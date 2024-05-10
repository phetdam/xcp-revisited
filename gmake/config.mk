# File: gmake/config.mk
# Author: Derek Huang
# Brief: Makefile C/C++ compile/link and testing configuration file
# Copyright: MIT License
#
# This file must be included before building any Makefile rules in the project.
#

# TODO: break this file into smaller pieces

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

# fruit C++ library link name, stem, file with suffix
FRUIT_LIBNAME = $(LIBNAME)_fruit
FRUIT_LIBSTEM = lib$(FRUIT_LIBNAME)
ifneq ($(BUILD_SHARED),)
FRUIT_LIBFILE = $(FRUIT_LIBSTEM).so
else
FRUIT_LIBFILE = $(FRUIT_LIBSTEM).a
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

# object suffix for C PIC objects (to match shared library rule)
ifneq ($(BUILD_SHARED),)
LIBOBJSUFFIX = PIC.o
else
LIBOBJSUFFIX = o
endif

# object suffix for C++ PIC objects (to match shared library rule)
ifneq ($(BUILD_SHARED),)
LIBOBJCXXSUFFIX = PIC.cc.o
else
LIBOBJCXXSUFFIX = cc.o
endif

# add current location of object to rpath. needed to find shared libraries in
# the same directory at runtime. we don't set this as a default for now
ifneq ($(BUILD_SHARED),)
RPATH_FLAGS = -Wl,-rpath,'$$ORIGIN'
else
RPATH_FLAGS =
endif

# base linker flags with rpath flags included. whenever we link against our own
# libraries BASE_LDFLAGS and RPATH_FLAGS are almost always used together
RPATH_LDFLAGS = $(BASE_LDFLAGS) $(RPATH_FLAGS)

# check that we have color capabilities
TERMCOLORS = $(shell tput colors)
ifneq ($(TERMCOLORS),)
$(info Terminal colors: $(TERMCOLORS))
# set colors and normal. F for foreground, I for high-intensity
TFGREEN = $(shell tput setaf 2)
TFCYAN = $(shell tput setaf 6)
TFIGREEN = $(shell tput setaf 10)
TFICYAN = $(shell tput setaf 14)
TNORMAL = $(shell tput sgr0)
else
$(info Terminal colors: None)
TFGREEN =
TFCYAN =
TFIGREEN =
TFICYAN =
TNORMAL =
endif
