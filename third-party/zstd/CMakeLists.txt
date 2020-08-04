find_library(ZSTD_LIB NAMES zstd)
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)
set(CMAKE_REQUIRED_INCLUDES "${ZSTD_INCLUDE_DIR}")
check_cxx_source_compiles("
#include <zstd.h>


int main() {
static_assert(ZSTD_VERSION_MAJOR == 1);
static_assert(ZSTD_VERSION_MINOR >= 4);
  return 0;
}
" CAN_USE_SYSTEM_ZSTD)
set(CMAKE_REQUIRED_INCLUDES)

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
    "https://github.com/facebook/zstd/releases/download/v1.4.5/zstd-1.4.5.tar.gz"
    SOURCE_HASH
    "SHA512=417b813ade6c8436690bd9d6da34a9f87f50e7378752b72e63066befbee496392a4b72896fa56688d814f461871ab31d3c6637497ff2ed7a282d58513631a38b"
  )
  ExternalProject_Add(
    zstdBuild
    ${ZSTD_DOWNLOAD_ARGS}
    CMAKE_ARGS
    <SOURCE_DIR>/build/cmake
    -DZSTD_BUILD_STATIC=ON
    -DZSTD_BUILD_SHARED=OFF
    -DZSTD_BUILD_PROGRAMS=OFF
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  )
  add_dependencies(zstd zstdBuild)
  ExternalProject_Get_Property(zstdBuild INSTALL_DIR)
  target_include_directories(zstd INTERFACE "${INSTALL_DIR}/include")
  target_link_libraries(zstd INTERFACE
    "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zstd${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif ()
