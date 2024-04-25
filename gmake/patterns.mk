# File: make/patterns.mk
# Author: Derek Huang
# Brief: Makefile C/C++ object and executable pattern rules
# Copyright: MIT License
#
# Must include config.mk before use to define BASE_CFLAGS, BASE_CXXFLAGS,
# TFGREEN, TFNORMAL, and BUILDDIR variables used in this file.
#

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
