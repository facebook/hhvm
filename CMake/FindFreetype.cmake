
find_package(PkgConfig)
pkg_check_modules(PC_FREETYPE QUIET freetype2)

find_path(FREETYPE_INCLUDE_DIR_FT2BUILD
          NAMES ft2build.h
          HINTS ${PC_FREETYPE_INCLUDEDIR} ${PC_FREETYPE_INCLUDE_DIRS}
          PATH_SUFFIXES freetype2)
if(FREETYPE_INCLUDE_DIR_FT2BUILD AND
   NOT EXISTS "${FREETYPE_INCLUDE_DIR_FT2BUILD}/freetype2/freetype/config/ftheader.h")
  add_definitions(-DHAVE_FT2BUILD)
  set(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIR_FT2BUILD})
else()
  find_path(FREETYPE_INCLUDE_DIRS
            NAMES freetype/config/ftheader.h freetype2/config/ftheader.h
            HINTS ${PC_FREETYPE_INCLUDEDIR} ${PC_FREETYPE_INCLUDE_DIRS}
            PATH_SUFFIXES freetype2)
  if(FREETYPE_INCLUDE_DIRS AND
     NOT EXISTS "${FREETYPE_INCLUDE_DIRS}/freetype/config/ftheader.h")
    add_definitions(-DFREETYPE_PATH_FREETYPE2)
  endif()
endif()

find_library(FREETYPE_LIBRARIES NAMES freetype)

include (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freetype DEFAULT_MSG
  FREETYPE_LIBRARIES
  FREETYPE_INCLUDE_DIRS)

mark_as_advanced(FREETYPE_INCLUDE_DIRS FREETYPE_LIBRARIES)
