include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  FMT_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/fmtlib/fmt/releases/download/6.1.2/fmt-6.1.2.zip"
  SOURCE_HASH
  "SHA512=d21085a2010786ff18c47acb033d9e4d51a3d58f9707cd9adf0f44642c1e4d80fd8cddafe58d95bb4f3e4a84ac5799caafead4a9feb12cc549b03d4d389fcc93"
)

set(INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/fmt-prefix")
ExternalProject_add(
  bundled_fmt
  ${FMT_SOURCE_ARGS}
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)
cmake_minimum_required(VERSION 2.8.0)
ExternalProject_get_property(bundled_fmt INSTALL_DIR)

add_library(fmt INTERFACE)
add_dependencies(fmt bundled_fmt)
target_include_directories(fmt INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(fmt INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
set(FMT_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
