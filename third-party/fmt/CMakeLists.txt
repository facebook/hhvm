include(ExternalProject)

set(FMT_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/fmt-prefix")
ExternalProject_add(
  fmtBuild
  URL "https://github.com/fmtlib/fmt/releases/download/6.1.2/fmt-6.1.2.zip"
  URL_HASH SHA512=d21085a2010786ff18c47acb033d9e4d51a3d58f9707cd9adf0f44642c1e4d80fd8cddafe58d95bb4f3e4a84ac5799caafead4a9feb12cc549b03d4d389fcc93
  PREFIX "${FMT_PREFIX}"
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${FMT_PREFIX}
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
)
cmake_minimum_required(VERSION 2.8.0)

add_library(fmt INTERFACE)
add_dependencies(fmt fmtBuild)
target_include_directories(fmt INTERFACE "${FMT_PREFIX}/include")
target_link_libraries(fmt INTERFACE
  "${FMT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmt${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
