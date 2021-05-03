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
get_filename_component(BOOST_LIBRARY_DIR "${FIRST_LIB}" DIRECTORY)

if("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.12.0")
  set(PARALLEL_FLAG "--parallel")
endif()

set(FOLLY_CXXFLAGS "-I${JEMALLOC_INCLUDE_DIR} -I${CMAKE_CURRENT_LIST_DIR}/include")
message(STATUS "FOLLY_CXXFLAGS: ${FOLLY_CXXFLAGS}")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
  if(IS_X64)
    message(STATUS "Enabling SSE 4.2 for bundled folly")
    set(FOLLY_CXXFLAGS "${FOLLY_CXXFLAGS} -msse4.2")
  endif()
endif()

ExternalProject_add(
  bundled_folly
  ${FOLLY_SOURCE_ARGS}
  PATCH_COMMAND
    "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}/patches"
  LIST_SEPARATOR "!!"
  CMAKE_ARGS
    "-DCMAKE_CXX_FLAGS=${FOLLY_CXXFLAGS}"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    -DCMAKE_INSTALL_INCLUDEDIR=include
    -DCMAKE_INSTALL_LIBDIR=lib
    # Options
    -DWITHOUT_GFLAGS=ON
    -DWITHOUT_LIBLZMA=ON
    -DWITHOUT_LIBSNAPPY=ON
    -DWITHOUT_LIBUNWIND=ON
    -DFOLLY_USE_SYMBOLIZER=OFF
    # Dependencies
    -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
    -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
    -DGLOG_INCLUDE_DIR=${LIBGLOG_INCLUDE_DIR}
    -DGLOG_LIBRARY=${LIBGLOG_LIBRARY}
    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_NO_BOOST_CMAKE=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    "-DBOOST_LIBRARYDIR=${BOOST_LIBRARY_DIR}"
    -DBoost_DEBUG=1
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
# Prefered, but needs CMake 3.15 or above. Revist when Ubuntu 18.04 support is removed
#    ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE} --component dev &&
#    ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE} --component lib
# Legacy way:
    ${CMAKE_COMMAND} -DCOMPONENT=dev -P cmake_install.cmake --config ${CMAKE_BUILD_TYPE} &&
    ${CMAKE_COMMAND} -DCOMPONENT=lib -P cmake_install.cmake --config ${CMAKE_BUILD_TYPE}
)
ExternalProject_Get_Property(bundled_folly INSTALL_DIR)
set(
  FOLLY_DEPS
  boost
  double-conversion
  fmt
  libsodium
  jemalloc
  zstd
)
add_dependencies(bundled_folly ${FOLLY_DEPS})
add_library(folly_deps INTERFACE)
target_link_libraries(folly_deps INTERFACE ${FOLLY_DEPS})

add_library(folly INTERFACE)
add_dependencies(folly bundled_folly)
target_include_directories(
  folly
  INTERFACE
  "${INSTALL_DIR}/include"
  "${CMAKE_CURRENT_LIST_DIR}/include"
)
target_link_libraries(folly
  INTERFACE
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}folly${CMAKE_STATIC_LIBRARY_SUFFIX}"
  folly_deps
)

if(UNIX AND NOT APPLE)
  # symbolizer implementation details
  find_package(Libiberty)
  find_package(LibUnwind)
  target_link_libraries(folly INTERFACE ${LIBIBERTY_LIBRARIES} ${LIBELF_LIBRARIES} ${LIBDWARF_LIBRARIES} ${LIBUNWIND_LIBRARIES})
  target_include_directories(folly INTERFACE ${LIBIBERTY_INCLUDE_DIRS} ${LIBUNWIND_INCLUDE_DIRS})
endif()
