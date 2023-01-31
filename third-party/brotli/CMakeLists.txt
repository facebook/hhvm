add_library(brotli INTERFACE)
include(ExternalProject)

include(HPHPFunctions)
SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  BROTLI_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/google/brotli/archive/refs/tags/v1.0.9.tar.gz"
  SOURCE_HASH
  "SHA256=43f9b6ef43c7471bac9d826118b30e35c46e11a090fc31712a28a3f7ebf405bb"
  FILENAME_PREFIX "brotli-"
)

ExternalProject_Add(
  bundled_brotli
  ${BROTLI_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
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
