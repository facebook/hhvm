find_package(PkgConfig)
pkg_check_modules(FRIBIDI QUIET fribidi)

find_path(FRIBIDI_INCLUDE_DIR
          NAMES fribidi/fribidi.h)

find_library(FRIBIDI_LIBRARY NAMES fribidi)

if (NOT FRIBIDI_FOUND)
  message(STATUS "GNU FriBidi not found")
else()
  message(STATUS "Found GNU FriBidi: ${FRIBIDI_LIBRARY}")
endif (NOT FRIBIDI_FOUND)
