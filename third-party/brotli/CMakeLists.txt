add_library(brotli INTERFACE)
include(ExternalProject)

include(HPHPFunctions)
SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  BROTLI_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/google/brotli/archive/refs/tags/v1.0.9.tar.gz"
  SOURCE_HASH
  "SHA256=f9e8d81d0405ba66d181529af42a3354f838c939095ff99930da6aa9cdf6fe46"
  FILENAME_PREFIX "brotli-"
)

ExternalProject_Add(
  bundled_brotli
  ${BROTLI_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)
add_dependencies(brotli bundled_brotli)

ExternalProject_Get_Property(bundled_brotli INSTALL_DIR)
target_include_directories(brotli INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  brotli
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlidec-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlienc-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}brotlicommon-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
