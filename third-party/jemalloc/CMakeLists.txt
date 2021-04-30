include(ExternalProject)

ExternalProject_add(
  bundled_jemalloc
  URL "https://github.com/jemalloc/jemalloc/releases/download/5.2.1/jemalloc-5.2.1.tar.bz2"
  URL_HASH SHA512=0bbb77564d767cef0c6fe1b97b705d368ddb360d55596945aea8c3ba5889fbce10479d85ad492c91d987caacdbbdccc706aa3688e321460069f00c05814fae02
  PREFIX "${JEMALLOC_PREFIX}"
  PATCH_COMMAND
    patch -p1 < "${CMAKE_CURRENT_SOURCE_DIR}/mac-sys-nothrow.patch"
  CONFIGURE_COMMAND
    "<SOURCE_DIR>/configure"
      --prefix=<INSTALL_DIR>
      --includedir=<INSTALL_DIR>/include
      --libdir=<INSTALL_DIR>/lib
      --disable-shared
      --enable-static
      --with-jemalloc-prefix=
  EXCLUDE_FROM_ALL
)

add_library(jemalloc INTERFACE)

if (NOT FORCE_TP_JEMALLOC)
  find_library(JEMALLOC_LIB NAMES jemalloc)
  find_path(JEMALLOC_INCLUDE_DIR NAMES jemalloc/jemalloc.h)
  set(CMAKE_REQUIRED_INCLUDES "${JEMALLOC_INCLUDE_DIR}")
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("
#include <jemalloc/jemalloc.h>

#if JEMALLOC_VERSION_MAJOR < 5
# error jemalloc version >= 5.0 required
#endif

  int main(void) { return 0; }" JEMALLOC_VERSION_MINIMUM)
  set(CMAKE_REQUIRED_INCLUDES)
endif()

if (JEMALLOC_VERSION_MINIMUM)
  message(STATUS "Found jemalloc: ${JEMALLOC_LIB} ${JEMALLOC_INCLUDE_DIR}")
  target_include_directories(jemalloc INTERFACE "${JEMALLOC_INCLUDE_DIR}")
else()
  message(STATUS "Using third-party bundled jemalloc")

  ExternalProject_Get_Property(bundled_jemalloc INSTALL_DIR)
  target_include_directories(jemalloc INTERFACE "${INSTALL_DIR}/include")
  set(JEMALLOC_LIB "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jemalloc_pic${CMAKE_STATIC_LIBRARY_SUFFIX}")
  add_dependencies(jemalloc bundled_jemalloc)
endif()

# the --no-as-needed and -force_load make the linker include all symbols,
# including ones that appear unused - this is essential when using jemalloc
# as a static library: it uses `__attribute__((constructor))` to do very early
# process-level initialization, and some linkers will omit those functions
# when linking a static library as there are no explicit callsites.
#
# This can lead to fun stuff like `malloc()` segfaulting on MacOS.
if (UNIX AND NOT APPLE)
  target_link_libraries(jemalloc INTERFACE "-Wl,--no-as-needed" "${JEMALLOC_LIB}" "-Wl,--as-needed")
  target_link_libraries(jemalloc INTERFACE dl pthread)
elseif (APPLE)
  target_link_libraries(jemalloc INTERFACE "-Wl,-force_load" "${JEMALLOC_LIB}")
endif()
