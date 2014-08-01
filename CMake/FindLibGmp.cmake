
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARY NAMES gmp)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    GMP DEFAULT_MSG
    GMP_LIBRARY GMP_INCLUDE_DIR)

if (NOT GMP_FOUND)
  message(STATUS "Did not find libgmp")
else()
  message(STATUS "Found GMP: ${GMP_LIBRARY}; functions will be included")
endif (NOT GMP_FOUND)

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY)
