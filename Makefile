# File: Makefile
# Author: Derek Huang
# Brief: Makefile for the Expert C Programming exercises
# Copyright: MIT License
#
# Debug build:
# make CC=gcc
#
# Release build:
# make CC=gcc BUILD_TYPE=Release
#

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
$(info Build Directory: $(BUILDDIR))

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

# include directories
INCLUDE_FLAGS = -Iinclude

# base compile flags
BASE_CFLAGS = $(INCLUDE_FLAGS) -Wall
ifeq ($(CONFIG),Release)
BASE_CFLAGS += -O3 -mtune=native
else
BASE_CFLAGS += -g
endif

# base linker flags
BASE_LDFLAGS =

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

# phony targets
.PHONY: clean

# build all targets. segsizes should go last so the print statements are done
# after all the other targets are built as the final build step.
all: \
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
$(BUILDDIR)/kbpoll: src/kbpoll.c $(HEADERS)
	@echo "Building $@..."
	$(CC) $(BASE_CFLAGS) -pthread $(CFLAGS) -o $@ $<

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
