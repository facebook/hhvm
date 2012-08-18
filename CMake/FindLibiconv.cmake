#
# $Id$
#
# - Find libiconv
# Find libiconv
#
#  LIBICONV_INCLUDE_DIR - where to find iconv.h, etc.
#  LIBICONV_LIBRARY     - List of libraries when using libiconv.
#  LIBICONV_FOUND       - True if libiconv found.


IF (LIBICONV_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBICONV_FIND_QUIETLY TRUE)
ENDIF ()

FIND_PATH(LIBICONV_INCLUDE_DIR iconv.h)

FIND_LIBRARY(LIBICONV_LIBRARY iconv)

# handle the QUIETLY and REQUIRED arguments and set Libmemcached_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBICONV DEFAULT_MSG LIBICONV_LIBRARY LIBICONV_INCLUDE_DIR)

SET(LIBICONV_OLD_CMAKE_REQUIRED_LIBRARIES)
SET(CMAKE_REQUIRED_LIBRARIES iconv)
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#include <string.h>
#include <iconv.h>
int main() {
  iconv_t cd = 0;
  const char *in_p = \"testing\";
  size_t in_left = strlen(in_p);
  char out_p[20];
  size_t out_left = sizeof(out_p);
  iconv(cd, (const char **)&in_p, &in_left, (char **)&out_p, &out_left);
  return 0;
}" LIBICONV_CONST)
SET(CMAKE_REQUIRED_LIBRARIES LIBICONV_OLD_CMAKE_REQUIRED_LIBRARIES)

MARK_AS_ADVANCED(LIBICONV_LIBRARY LIBICONV_INCLUDE_DIR LIBICONV_CONST)
