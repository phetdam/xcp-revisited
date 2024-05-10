# File: gmake/recipes.mk
# Author: Derek Huang
# Brief: Makefile recipes for C/C++ static/shared library or executable linking
# Copyright: MIT License
#
# Must include config.mk before use to define variables used.
#

# message indicating C static library linking step
define c-link-static-msg
@printf "$(TFIGREEN)Linking C static library $@$(TNORMAL)\n"
endef

# message indicating C++ static library linking step
define cxx-link-static-msg
@printf "$(TFIGREEN)Linking C++ static library $@$(TNORMAL)\n"
endef

# message indicating C shared library linking step
define c-link-shared-msg
@printf "$(TFIGREEN)Linking C shared library $@$(TNORMAL)\n"
endef

# message indicating C++ shared library linking step
define cxx-link-shared-msg
@printf "$(TFIGREEN)Linking C++ shared library $@$(TNORMAL)\n"
endef

# message indicating C executable linking step
define c-link-exec-msg
@printf "$(TFIGREEN)Linking C executable $@$(TNORMAL)\n"
endef

# message indicating C++ executable linking step
define cxx-link-exec-msg
@printf "$(TFIGREEN)Linking C++ executable $@$(TNORMAL)\n"
endef

# message indicating C executable build step
define c-build-exec-msg
@printf "$(TFCYAN)Building C executable $@$(TNORMAL)\n"
endef

# message indicating target has been built
define target-done
@echo "Built target $@"
endef

# link a standalone C static library
define c-link-static
@$(c-link-static-msg)
@$(AR) crs $@ $^
endef

# link a standalone C++ static library
define cxx-link-static
@$(cxx-link-static-msg)
@$(AR) crs $@ $^
endef

# link a standalone C shared library
define c-link-shared
@$(c-link-shared-msg)
@$(CC) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
@$(target-done)
endef

# link a standalone C++ shared library
define cxx-link-shared
@$(cxx-link-shared-msg)
@$(CXX) $(SOFLAGS) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
@$(target-done)
endef

# link a standalone C executable
define c-link-exec
@$(c-link-exec-msg)
@$(CC) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
@$(target-done)
endef

# link a standalone C++ executable
define cxx-link-exec
@$(cxx-link-exec-msg)
@$(CXX) $(BASE_LDFLAGS) $(LDFLAGS) -o $@ $^
@$(target-done)
endef
