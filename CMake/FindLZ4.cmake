# Finds liblz4.
#
# This module defines:
# LZ4_FOUND
# LZ4_INCLUDE_DIR
# LZ4_LIBRARY
#

find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
find_library(LZ4_LIBRARY NAMES lz4)

# fb-mysql requires LZ4F_resetDecompressionContext() which was added in v1.8.0
if (LZ4_LIBRARY)
  include(CheckCSourceRuns)
  set(CMAKE_REQUIRED_INCLUDES ${LZ4_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${LZ4_LIBRARY})
  check_c_source_runs("
#include <lz4.h>
int main() {
  int good = (LZ4_VERSION_MAJOR > 1) ||
    ((LZ4_VERSION_MAJOR == 1) && (LZ4_VERSION_MINOR >= 8));
return !good;
}" LZ4_GOOD_VERSION)
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    LZ4 DEFAULT_MSG
    LZ4_LIBRARY LZ4_INCLUDE_DIR LZ4_GOOD_VERSION)

if (NOT LZ4_FOUND)
  message(STATUS "Using third-party bundled LZ4")
else()
  message(STATUS "Found LZ4: ${LZ4_LIBRARY}")
endif (NOT LZ4_FOUND)

mark_as_advanced(LZ4_INCLUDE_DIR LZ4_LIBRARY)
