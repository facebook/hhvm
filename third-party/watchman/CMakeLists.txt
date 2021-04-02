add_library(libwatchmanclient INTERFACE)

include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  WATCHMAN_DOWNLOAD_ARGS
  SOURCE_URL "https://github.com/facebook/watchman/archive/refs/tags/v2021.03.22.00.tar.gz"
  SOURCE_HASH "SHA256=fa846bbdbd3d4c16a950a09179e28ec4eac3e6d65cec68f841370f14226fbcb5"
  # The tarball name is just the tag name, which can conflict in the cache
  FILENAME_PREFIX "watchman-"
)

get_target_property(BOOST_INCLUDE_DIR boost INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOUBLE_CONVERSION_INCLUDE_DIR double-conversion INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(FMT_INCLUDE_DIR fmt INTERFACE_INCLUDE_DIRECTORIES)

ExternalProject_Add(
  bundled_watchman
  ${WATCHMAN_DOWNLOAD_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/cppclient-CMakeLists.txt"
    "cppclient/CMakeLists.txt"
  SOURCE_SUBDIR cppclient
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    "-DBOOST_INCLUDE_DIR=${BOOST_INCLUDE_DIR}"
    "-DDOUBLE_CONVERSION_INCLUDE_DIR=${DOUBLE_CONVERSION_INCLUDE_DIR}"
    "-DFMT_INCLUDE_DIR=${FMT_INCLUDE_DIR}"
    "-DFOLLY_INCLUDE_DIR=${CMAKE_CURRENT_LIST_DIR}/../folly"
    "-DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}"
)
add_dependencies(bundled_watchman folly boost fmt double-conversion)

add_dependencies(libwatchmanclient bundled_watchman)
