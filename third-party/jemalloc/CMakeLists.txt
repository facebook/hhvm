include(ExternalProject)

ExternalProject_add(
  bundled_jemalloc
  URL "https://github.com/jemalloc/jemalloc/releases/download/5.3.0/jemalloc-5.3.0.tar.bz2"
  URL_HASH SHA256=2db82d1e7119df3e71b7640219b6dfe84789bc0537983c3b7ac4f7189aecfeaa
  PREFIX "${JEMALLOC_PREFIX}"
  PATCH_COMMAND
    ${CMAKE_COMMAND} -E echo "Skipping mac-sys-nothrow patch (not needed on Linux, incompatible with 5.3.0)"
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

#if JEMALLOC_VERSION_MAJOR < 5 || (JEMALLOC_VERSION_MAJOR == 5 && JEMALLOC_VERSION_MINOR < 3)
# error jemalloc version >= 5.3 required
#endif

  int main(void) { return 0; }" JEMALLOC_VERSION_MINIMUM)
  set(CMAKE_REQUIRED_INCLUDES)
endif()

if (JEMALLOC_VERSION_MINIMUM)
  message(STATUS "Found jemalloc: ${JEMALLOC_LIB} ${JEMALLOC_INCLUDE_DIR}")
  target_include_directories(jemalloc INTERFACE "${JEMALLOC_INCLUDE_DIR}")
  include_directories(BEFORE "${JEMALLOC_INCLUDE_DIR}")
else()
  message(STATUS "Using third-party bundled jemalloc")

  ExternalProject_Get_Property(bundled_jemalloc INSTALL_DIR)
  target_include_directories(jemalloc BEFORE INTERFACE "${INSTALL_DIR}/include")
  include_directories(BEFORE "${INSTALL_DIR}/include")
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
