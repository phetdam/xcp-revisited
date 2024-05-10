# File: gmake/version.mk
# Author: Derek Huang
# Brief: Makefile version extraction file
# Copyright: MIT License
#
# This file must be included before building any Makefile rules in the project.
#

# major, minor, patch versions from CMakeLists.txt (same directory as Makefile)
VERSION_MAJOR = $(shell \
cat CMakeLists.txt | grep 'PDXCP_VERSION_MAJOR' | head -1 | \
sed -E 's/[a-zA-Z_() ]+//g')
VERSION_MINOR = $(shell \
cat CMakeLists.txt | grep 'PDXCP_VERSION_MINOR' | head -1 | \
sed -E 's/[a-zA-Z_() ]+//g')
VERSION_PATCH = $(shell \
cat CMakeLists.txt | grep 'PDXCP_VERSION_PATCH' | head -1 | \
sed -E 's/[a-zA-Z_() ]+//g')
# version string constructed from major, minor, patch versions
VERSION_STRING = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
