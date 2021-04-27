include(ExternalProject)
include(HPHPFunctions)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  FOLLY_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/facebook/folly/releases/download/v2021.03.22.00/folly-v2021.03.22.00.tar.gz"
  SOURCE_HASH
  "SHA256=1e1ee0fa1dd61763f4ea87bad76362f39cdbc0ecb599893693b3013e7d43ed87"
)

get_target_property(BOOST_INCLUDE_DIR boost INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOUBLE_CONVERSION_INCLUDE_DIR double-conversion INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(DOUBLE_CONVERSION_LIBRARY double-conversion INTERFACE_LINK_LIBRARIES)
get_target_property(LIBFMT_INCLUDE_DIR fmt INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(LIBFMT_LIBRARY fmt INTERFACE_LINK_LIBRARIES)
get_target_property(LIBSODIUM_INCLUDE_DIR libsodium INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(LIBSODIUM_LIBRARY libsodium INTERFACE_LINK_LIBRARIES)
get_target_property(ZSTD_INCLUDE_DIR zstd INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ZSTD_LIBRARY zstd INTERFACE_LINK_LIBRARIES)
get_target_property(JEMALLOC_INCLUDE_DIR jemalloc INTERFACE_INCLUDE_DIRECTORIES)

get_target_property(BOOST_LIBRARIES boost INTERFACE_LINK_LIBRARIES)
list(GET BOOST_LIBRARIES 0 FIRST_LIB)
if("${FIRST_LIB}" MATCHES ".+/${CMAKE_STATIC_LIBRARY_PREFIX}boost_.+${CMAKE_STATIC_LIBRARY_SUFFIX}$")
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(Boost_USE_STATIC_LIBS OFF)
endif()

if ("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.12.0")
  set(PARALLEL_FLAG "--parallel")
endif()

ExternalProject_add(
  bundled_folly
  ${FOLLY_SOURCE_ARGS}
  PATCH_COMMAND
    patch -p1 --force < "${CMAKE_CURRENT_LIST_DIR}/folly-cmake-fixup.patch" || true &&
    patch -p1 --force < "${CMAKE_CURRENT_LIST_DIR}/folly-install-component.patch" || true &&
    patch -p1 --force < "${CMAKE_CURRENT_LIST_DIR}/disable-gflags.patch" || true
  LIST_SEPARATOR "!!"
  CMAKE_ARGS
    "-DCMAKE_CXX_FLAGS=-I${JEMALLOC_INCLUDE_DIR} -I${CMAKE_CURRENT_LIST_DIR}/include"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DWITHOUT_GFLAGS=TRUE
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
    -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
    -DGLOG_INCLUDE_DIR=${LIBGLOG_INCLUDE_DIR}
    -DGLOG_LIBRARY=${LIBGLOG_LIBRARY}
    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    -DBoost_LIBRARY_DIRS_RELEASE=${BOOST_INCLUDE_DIR}/../lib
    -DBoost_LIBRARY_DIRS_DEBUG=${BOOST_INCLUDE_DIR}/../lib
    -DDOUBLE_CONVERSION_INCLUDE_DIR=${DOUBLE_CONVERSION_INCLUDE_DIR}
    -DDOUBLE_CONVERSION_LIBRARY=${DOUBLE_CONVERSION_LIBRARY}
    -DLIBFMT_INCLUDE_DIR=${LIBFMT_INCLUDE_DIR}
    -DLIBFMT_LIBRARY=${LIBFMT_LIBRARY}
    -DLIBSODIUM_INCLUDE_DIR=${LIBSODIUM_INCLUDE_DIR}
    -DLIBSODIUM_LIBRARY=${LIBSODIUM_LIBRARY}
    -DZSTD_INCLUDE_DIR=${ZSTD_INCLUDE_DIR}
    -DZSTD_LIBRARY_RELEASE=${ZSTD_LIBRARY}
  BUILD_COMMAND
    ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE} --target folly ${PARALLEL_FLAG}
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_LIST_DIR}/include"
      "<INSTALL_DIR>/include" &&
    ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE} --component dev &&
    ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE} --component lib
)
ExternalProject_Get_Property(bundled_folly INSTALL_DIR)
add_dependencies(bundled_folly boost fmt jemalloc libsodium zstd double-conversion)

add_library(folly INTERFACE)
add_dependencies(folly bundled_folly)
target_include_directories(
  folly
  INTERFACE
  "${INSTALL_DIR}/include"
  "${CMAKE_CURRENT_LIST_DIR}/include"
)
target_link_libraries(folly INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}folly${CMAKE_STATIC_LIBRARY_SUFFIX}"
  fmt # referred to by folly's headers, so anything using folly needs to find them
)
