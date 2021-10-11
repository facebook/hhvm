include(ExternalProject)
include(HPHPFunctions)

get_target_property(BOOST_INCLUDE_DIR boost INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(BOOST_LIBRARIES boost INTERFACE_LINK_LIBRARIES)
list(GET BOOST_LIBRARIES 0 FIRST_LIB)
if("${FIRST_LIB}" MATCHES ".+/${CMAKE_STATIC_LIBRARY_PREFIX}boost_.+${CMAKE_STATIC_LIBRARY_SUFFIX}$")
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(Boost_USE_STATIC_LIBS OFF)
endif()
get_filename_component(BOOST_LIBRARY_DIR "${FIRST_LIB}" DIRECTORY)

get_target_property(JEMALLOC_INCLUDE_DIR jemalloc INTERFACE_INCLUDE_DIRECTORIES)


SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  THRIFT_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/facebook/fbthrift/archive/refs/tags/v2021.06.28.00.tar.gz"
  SOURCE_HASH
  "SHA256=75283c2cb71cbd510938f3536a3eac6bd1415918a37dd4ec54536041deb1e44d"
  FILENAME_PREFIX
  "fbthrift-"
)

ExternalProject_Add(
  bundled_thrift
  ${THRIFT_SOURCE_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_SOURCE_DIR}/CMake/FindGlog.cmake"
    <SOURCE_DIR>/thrift/cmake/FindGlog.cmake
    &&
    "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}/patches"
  CMAKE_ARGS
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    
    -DWITHOUT_GFLAGS=ON

    -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
    -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_NO_BOOST_CMAKE=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    "-DBOOST_LIBRARYDIR=${BOOST_LIBRARY_DIR}"
    "-DLIBGLOG_INCLUDE_DIR=${LIBGLOG_INCLUDE_DIR}"
    "-DLIBGLOG_LIBRARY=${LIBGLOG_LIBRARY}"

    "-Dfizz_DIR=${FIZZ_INSTALL_DIR}/lib/cmake/fizz"
    "-Dfmt_DIR=${FMT_INSTALL_DIR}/lib/cmake/fmt"
    "-Dfolly_DIR=${FOLLY_INSTALL_DIR}/lib/cmake/folly"
    "-Dwangle_DIR=${WANGLE_INSTALL_DIR}/lib/cmake/wangle"

    "-DCMAKE_CXX_FLAGS=-I${JEMALLOC_INCLUDE_DIR}"
)
ExternalProject_Get_property(bundled_thrift INSTALL_DIR)

set(
  THRIFT_DEPS
  fizz
  fmt
  folly
  proxygen
  wangle
)
add_dependencies(bundled_thrift ${THRIFT_DEPS})
add_library(thrift INTERFACE)
add_dependencies(thrift bundled_thrift)
target_include_directories(thrift INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  thrift
  INTERFACE
  ${THRIFT_DEPS} ${OPENSSL_LIBRARIES}
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}thrift${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
set(THRIFT_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
