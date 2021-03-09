find_library(ZSTD_LIB NAMES zstd)
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)
set(CMAKE_REQUIRED_INCLUDES "${ZSTD_INCLUDE_DIR}")

option(FORCE_BUNDLED_ZSTD "Always build zstd, instead of using the system version" OFF)
if (FORCE_BUNDLED_ZSTD)
  set(CAN_USE_SYSTEM_ZSTD OFF)
else ()
  check_cxx_source_compiles("
#include <zstd.h>


  int main() {
  static_assert(ZSTD_VERSION_MAJOR == 1);
  static_assert(ZSTD_VERSION_MINOR >= 4);
    return 0;
  }
  " CAN_USE_SYSTEM_ZSTD)
  set(CMAKE_REQUIRED_INCLUDES)
endif ()

add_library(zstd INTERFACE)
if (CAN_USE_SYSTEM_ZSTD)
  message(STATUS "Using system zstd")
  target_include_directories(zstd INTERFACE "${ZSTD_INCLUDE_DIR}")
  target_link_libraries(zstd INTERFACE "${ZSTD_LIB}")
else ()
  message(STATUS "Building zstd")
  include(ExternalProject)
  include(HPHPFunctions)
  SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
    ZSTD_DOWNLOAD_ARGS
    SOURCE_URL
    "https://github.com/facebook/zstd/releases/download/v1.4.9/zstd-1.4.9.tar.gz"
    SOURCE_HASH
    "SHA512=10d325c844be43f801c798158c6f1d1ab57401abf1e783e04f6b9e4ac0ba53cf487204fa3244370b1ade239d5f3a784bf1829e206c4ba61fdd9c2f4e9502b238"
  )
  ExternalProject_Add(
    bundled_zstd
    ${ZSTD_DOWNLOAD_ARGS}
    CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
    <SOURCE_DIR>/build/cmake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    -DZSTD_BUILD_STATIC=ON
    -DZSTD_BUILD_SHARED=OFF
    -DZSTD_BUILD_PROGRAMS=OFF
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )
  add_dependencies(zstd bundled_zstd)
  ExternalProject_Get_Property(bundled_zstd INSTALL_DIR)
  target_include_directories(zstd INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(zstd INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()
