add_library(brotli INTERFACE)
include(ExternalProject)

include(HPHPFunctions)
SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  BROTLI_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/google/brotli/archive/refs/tags/v1.1.0.tar.gz"
  SOURCE_HASH
  "SHA256=e720a6ca29428b803f4ad165371771f5398faba397edf6778837a18599ea13ff"
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
