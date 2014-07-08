# - Try to find libdl
# Once done this will define
#
#  LIBDL_FOUND - system has libdl
#  LIBDL_INCLUDE_DIRS - the libdl include directory
#  LIBDL_LIBRARIES - Link these to use libdl
#  LIBDL_NEEDS_UNDERSCORE - If extern "C" symbols are prefixed (BSD/Apple)
#

find_path (LIBDL_INCLUDE_DIRS NAMES dlfcn.h)
find_library (LIBDL_LIBRARIES NAMES dl)
include (FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibDL DEFAULT_MSG
  LIBDL_LIBRARIES
  LIBDL_INCLUDE_DIRS)

SET(CMAKE_REQUIRED_LIBRARIES dl)
INCLUDE(CheckCSourceRuns)
CHECK_C_SOURCE_RUNS("#include <dlfcn.h>
#include <stdlib.h>
void testfunc() {}
int main() {
  testfunc();
  if (dlsym(0, \"_testfunc\") != (void*)0) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}" LIBDL_NEEDS_UNDERSCORE)

mark_as_advanced(LIBDL_INCLUDE_DIRS LIBDL_LIBRARIES LIBDL_NEEDS_UNDERSCORE)
