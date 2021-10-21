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

get_target_property(ZSTD_INCLUDE_DIR zstd INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(ZSTD_LIBRARY zstd INTERFACE_LINK_LIBRARIES)
get_target_property(JEMALLOC_INCLUDE_DIR jemalloc INTERFACE_INCLUDE_DIRECTORIES)


SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  PROXYGEN_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/facebook/proxygen/releases/download/v2021.10.18.00/proxygen-v2021.10.18.00.tar.gz"
  SOURCE_HASH
  "SHA256=7de4002ab5aa0b59343533cc9c26efbafed3401b1579b948cd2678f10fc9382e"
)

ExternalProject_Add(
  bundled_proxygen
  ${PROXYGEN_SOURCE_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_SOURCE_DIR}/CMake/FindBoost.cmake"
    <SOURCE_DIR>/wangle/cmake/FindBoost.cmake
  CMAKE_ARGS
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

    -DBUILD_TESTS=OFF
    -DBUILD_SAMPLES=OFF

    -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
    -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_NO_BOOST_CMAKE=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    "-DBOOST_LIBRARYDIR=${BOOST_LIBRARY_DIR}"
    -DZSTD_INCLUDE_DIR=${ZSTD_INCLUDE_DIR}
    -DZSTD_LIBRARY_RELEASE=${ZSTD_LIBRARY}

    "-DFizz_DIR=${FIZZ_INSTALL_DIR}/lib/cmake/fizz"
    "-Dfmt_DIR=${FMT_INSTALL_DIR}/lib/cmake/fmt"
    "-Dfolly_DIR=${FOLLY_INSTALL_DIR}/lib/cmake/folly"
    "-Dwangle_DIR=${WANGLE_INSTALL_DIR}/lib/cmake/wangle"

    "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -I${JEMALLOC_INCLUDE_DIR}"
)
ExternalProject_Get_property(bundled_proxygen INSTALL_DIR)

set(
  PROXYGEN_DEPS
  fizz
  fmt
  folly
  wangle
)
add_dependencies(bundled_proxygen ${PROXYGEN_DEPS})
add_library(proxygen INTERFACE)
add_dependencies(proxygen bundled_proxygen)
target_include_directories(proxygen INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  proxygen
  INTERFACE
  ${PROXYGEN_DEPS} ${OPENSSL_LIBRARIES}
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}proxygen${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
set(PROXYGEN_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
