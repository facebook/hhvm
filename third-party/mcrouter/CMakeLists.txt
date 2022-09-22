add_library(mcrouter INTERFACE)

include(ExternalProject)
include(HPHPFunctions)

set(
  MCROUTER_DEPS
  boost
  fizz
  fmt
  folly
  jemalloc
  thrift
  wangle
)
add_library(mcrouter_deps INTERFACE)
target_link_libraries(mcrouter_deps INTERFACE ${MCROUTER_DEPS})
add_dependencies(mcrouter_deps ragel)

ExternalProject_Add(
  bundled_mcrouter
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E create_symlink
    "${CMAKE_CURRENT_SOURCE_DIR}/mcrouter-CMakeLists.txt"
    "CMakeLists.txt"
    &&
    "${CMAKE_COMMAND}" -E create_symlink
    "${CMAKE_CURRENT_SOURCE_DIR}/mcrouter-thrift-codegen.cmake"
    "mcrouter-thrift-codegen.cmake"
    &&
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h"
    "mcrouter/"
    &&
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/config-impl.h"
    "mcrouter/"
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}

    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    "-DBOOST_INCLUDE_DIR=$<TARGET_PROPERTY:boost,INTERFACE_INCLUDE_DIRECTORIES>"
    "-DGLOG_INCLUDE_DIR=${GLOG_INCLUDE_DIR}"
    "-DJEMALLOC_INCLUDE_DIR=$<TARGET_PROPERTY:jemalloc,INTERFACE_INCLUDE_DIRECTORIES>"
    "-DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}"

    "-Dragel_EXECUTABLE=$<TARGET_FILE:ragel>"

    "-Dfmt_DIR=${fmt_DIR}"
    "-Dfolly_DIR=${FOLLY_INSTALL_DIR}/lib/cmake/folly"
    "-Dfizz_DIR=${FIZZ_INSTALL_DIR}/lib/cmake/fizz"
    "-DFBThrift_DIR=${THRIFT_INSTALL_DIR}/lib/cmake/fbthrift"
    "-Dwangle_DIR=${WANGLE_INSTALL_DIR}/lib/cmake/wangle"
  DEPENDS ${MCROUTER_DEPS} ragel
  EXCLUDE_FROM_ALL
)

add_dependencies(mcrouter bundled_mcrouter)
ExternalProject_Get_Property(bundled_mcrouter INSTALL_DIR)
ExternalProject_Get_Property(bundled_mcrouter SOURCE_DIR)
ExternalProject_Get_Property(bundled_mcrouter BINARY_DIR)
target_include_directories(
  mcrouter
  INTERFACE
  "${SOURCE_DIR}"
  "${BINARY_DIR}"
  "${BINARY_DIR}/thrift-gen/src"
)
target_link_libraries(
  mcrouter
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mcrouter${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mcrouter_thrift_lib${CMAKE_STATIC_LIBRARY_SUFFIX}"
  mcrouter_deps
)
