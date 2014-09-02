#
# $Id$
#
# - Find pcre
# Find the native PCRE includes and library
#
#  PCRE_INCLUDE_DIR  - where to find pcre.h, etc.
#  PCRE_LIBRARY      - Path to the pcre library.
#  PCRE_FOUND        - True if pcre found.


IF (PCRE_INCLUDE_DIR)
  # Already in cache, be silent
  SET(PCRE_FIND_QUIETLY TRUE)
ENDIF (PCRE_INCLUDE_DIR)

FIND_PATH(SYSTEM_PCRE_INCLUDE_DIR pcre.h)

SET(PCRE_NAMES pcre)
FIND_LIBRARY(SYSTEM_PCRE_LIBRARY NAMES ${PCRE_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set PCRE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  PCRE DEFAULT_MSG SYSTEM_PCRE_LIBRARY SYSTEM_PCRE_INCLUDE_DIR
)

# Make sure it's got the jit enabled. If not, don't use it.
IF (PCRE_FOUND)
  INCLUDE(CheckCSourceRuns)
  SET(CMAKE_REQUIRED_LIBRARIES pcre)
  CHECK_C_SOURCE_RUNS("#include <pcre.h>
int main() {
  int has_jit = 0;
  pcre_config(PCRE_CONFIG_JIT, &has_jit);
  return has_jit ? 0 : 1;
}
" SYSTEM_PCRE_HAS_JIT)
ENDIF (PCRE_FOUND)

IF(NOT SYSTEM_PCRE_HAS_JIT)
  MESSAGE(STATUS
    "System PCRE does not have JIT enabled - will use hhvm-third-party/pcre")
  UNSET(PCRE_INCLUDE_DIR CACHE)
  UNSET(PCRE_LIBRARY CACHE)

  # This is used to configure bundled pcre
  SET(PCRE_SUPPORT_JIT ON CACHE BOOL "")
  SET(PCRE_SUPPORT_UTF ON CACHE BOOL "")
  SET(PCRE_SUPPORT_UNICODE_PROPERTIES ON CACHE BOOL "")
ELSE()
  SET(PCRE_INCLUDE_DIR ${SYSTEM_PCRE_INCLUDE_DIR}
    CACHE PATH "PCRE include directory")
  SET(PCRE_LIBRARY ${SYSTEM_PCRE_LIBRARY} CACHE FILEPATH "PCRE library")
ENDIF()

MARK_AS_ADVANCED( PCRE_LIBRARY PCRE_INCLUDE_DIR )
