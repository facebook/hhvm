# Finds liblz4.
#
# This module defines:
# LZ4_INCLUDE_DIR
# LZ4_LIBRARY
#

find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
find_library(LZ4_LIBRARY NAMES lz4)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    LZ4 DEFAULT_MSG
    LZ4_LIBRARY LZ4_INCLUDE_DIR)

if (NOT LZ4_FOUND)
  message(STATUS "Using third-party bundled LZ4")
else()
  message(STATUS "Found LZ4: ${LZ4_LIBRARY}")
endif (NOT LZ4_FOUND)

mark_as_advanced(LZ4_INCLUDE_DIR LZ4_LIBRARY)
