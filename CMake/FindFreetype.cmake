
find_package(PkgConfig)
pkg_check_modules(PC_FREETYPE QUIET freetype2)

find_path(FREETYPE_INCLUDE_DIRS NAMES freetype/config/ftheader.h
          HINTS ${PC_FREETYPE_INCLUDEDIR} ${PC_FREETYPE_INCLUDE_DIRS}
          PATH_SUFFIXES freetype2)

find_library(FREETYPE_LIBRARIES NAMES freetype)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype DEFAULT_MSG
    FREETYPE_LIBRARIES
    FREETYPE_INCLUDE_DIRS)

mark_as_advanced(FREETYPE_INCLUDE_DIRS FREETYPE_LIBRARIES)
