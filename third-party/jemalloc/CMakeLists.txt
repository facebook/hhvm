include(ExternalProject)

set(JEMALLOC_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/jemallocBuild-prefix")
ExternalProject_add(
  jemallocBuild
  URL "https://github.com/jemalloc/jemalloc/releases/download/5.2.1/jemalloc-5.2.1.tar.bz2"
  URL_HASH SHA512=0bbb77564d767cef0c6fe1b97b705d368ddb360d55596945aea8c3ba5889fbce10479d85ad492c91d987caacdbbdccc706aa3688e321460069f00c05814fae02
  PREFIX "${JEMALLOC_PREFIX}"
  CONFIGURE_COMMAND
    "${JEMALLOC_PREFIX}/src/jemallocBuild/configure" "--prefix=${JEMALLOC_PREFIX}"
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
  message(STATUS "Found jemalloc: ${JEMALLOC_LIB}")
  target_link_libraries(jemalloc INTERFACE "${JEMALLOC_LIB}")
  target_include_directories(jemalloc INTERFACE "${JEMALLOC_INCLUDE_DIR}")
else()
  message(STATUS "Using third-party bundled jemalloc")

  set(JEMALLOC_INCLUDE_DIR "${JEMALLOC_PREFIX}/include")
  set(JEMALLOC_LIB "${JEMALLOC_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jemalloc_pic${CMAKE_STATIC_LIBRARY_SUFFIX}")
  add_dependencies(jemalloc jemallocBuild)
  target_link_libraries(jemalloc INTERFACE "${JEMALLOC_LIB}")
  if(UNIX AND NOT APPLE)
    target_link_libraries(jemalloc INTERFACE dl pthread)
  endif()
  target_include_directories(jemalloc INTERFACE "${JEMALLOC_PREFIX}/include")
endif()
