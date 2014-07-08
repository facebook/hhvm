# Finds libzip.
#
# This module defines:
# LIBZIP_INCLUDE_DIR_ZIP
# LIBZIP_INCLUDE_DIR_ZIPCONF
# LIBZIP_LIBRARY
#

find_package(PkgConfig)
pkg_check_modules(PC_LIBZIP QUIET libzip)

find_path(LIBZIP_INCLUDE_DIR_ZIP
    NAMES zip.h
    HINTS ${PC_LIBZIP_INCLUDE_DIRS})

find_path(LIBZIP_INCLUDE_DIR_ZIPCONF
    NAMES zipconf.h
    HINTS ${PC_LIBZIP_INCLUDE_DIRS})

find_library(LIBZIP_LIBRARY
    NAMES zip)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    LIBZIP DEFAULT_MSG
    LIBZIP_LIBRARY LIBZIP_INCLUDE_DIR_ZIP LIBZIP_INCLUDE_DIR_ZIPCONF)

if (NOT LIBZIP_FOUND)
    message(STATUS "Using third-party bundled libzip")
else()
    message(STATUS "Found libzip: ${LIBZIP_LIBRARY}")
endif (NOT LIBZIP_FOUND)
