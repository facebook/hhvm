#
# $Id$
#
# - Find libunwind
# Find libunwind
#
#  LIBUNWIND_INCLUDE_DIR - where to find unwind.h and libunwind.h
#  LIBUNWIND_LIBRARY     - List of libraries when using libunwind
#  LIBUNWIND_FOUND       - True if libunwind found.


if(LIBUNWIND_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBUNWIND_FIND_QUIETLY TRUE)
endif()

FIND_PATH(LIBUNWIND_INCLUDE_DIR libunwind.h)
if(NOT EXISTS "${LIBUNWIND_INCLUDE_DIR}/unwind.h")
  message(FATAL_ERROR "libunwind.h found without matching unwind.h")
  SET(LIBUNWIND_INCLUDE_DIR "")
endif()

FIND_LIBRARY(LIBUNWIND_LIBRARY unwind)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUNWIND DEFAULT_MSG LIBUNWIND_LIBRARY LIBUNWIND_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBUNWIND_LIBRARY LIBUNWIND_INCLUDE_DIR)
