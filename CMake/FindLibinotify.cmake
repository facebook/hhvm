#
# $Id$
#
# - Find libinotify
# Find libinotify
#
#  LIBINOTIFY_INCLUDE_DIR - where to find sys/inotify.h
#  LIBINOTIFY_LIBRARY     - List of libraries when using libinotify
#  LIBINOTIFY_FOUND       - True if libinotify found.


if(LIBINOTIFY_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBINOTIFY_FIND_QUIETLY TRUE)
endif()

FIND_PATH(LIBINOTIFY_INCLUDE_DIR sys/inotify.h)
FIND_LIBRARY(LIBINOTIFY_LIBRARY inotify)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBINOTIFY DEFAULT_MSG LIBINOTIFY_LIBRARY LIBINOTIFY_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBINOTIFY_LIBRARY LIBINOTIFY_INCLUDE_DIR)
