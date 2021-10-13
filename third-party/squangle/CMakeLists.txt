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
get_target_property(fbmysqlclient_INCLUDE_DIR fbmysqlclient INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(re2_INCLUDE_DIR re2 INTERFACE_INCLUDE_DIRECTORIES)

SET_HHVM_THIRD_PARTY_SOURCE_ARGS(
  SQUANGLE_SOURCE_ARGS
  SOURCE_URL
  "https://github.com/facebook/squangle/archive/refs/tags/v2021.06.28.00.tar.gz"
  SOURCE_HASH
  "SHA256=edac06983f92b072980bf5cf19ac9f1807127bf10fc2478545992e91e6a8514b"
  FILENAME_PREFIX
  "squangle-"
)

ExternalProject_Add(
  bundled_squangle
  ${SQUANGLE_SOURCE_ARGS}
  PATCH_COMMAND
    "${CMAKE_COMMAND}" -E make_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    &&
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_SOURCE_DIR}/CMake/FindGlog.cmake"
    <SOURCE_DIR>/squangle/cmake/FindGlog.cmake
    &&
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_SOURCE_DIR}/CMake/FindBoost.cmake"
    <SOURCE_DIR>/squangle/cmake/FindBoost.cmake
    &&
    "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/squangle-CMakeLists.txt"
    <SOURCE_DIR>/CMakeLists.txt
    &&
    "${CMAKE_CURRENT_SOURCE_DIR}/../apply-quilt-patches.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}/patches"
  CMAKE_ARGS
    "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}"
    -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMake
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>

    -DBoost_NO_SYSTEM_PATHS=ON
    -DBoost_NO_BOOST_CMAKE=ON
    -DBoost_USE_STATIC_LIBS=${Boost_USE_STATIC_LIBS}
    -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
    "-DBOOST_LIBRARYDIR=${BOOST_LIBRARY_DIR}"

    "-Dfmt_DIR=${FMT_INSTALL_DIR}/lib/cmake/fmt"
    "-Dfolly_DIR=${FOLLY_INSTALL_DIR}/lib/cmake/folly"

    "-Dfbmysqlclient_INCLUDE_DIR=${fbmysqlclient_INCLUDE_DIR}"
    "-Dre2_INCLUDE_DIR=${re2_INCLUDE_DIR}"

    "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -I${JEMALLOC_INCLUDE_DIR}"
)
ExternalProject_Get_property(bundled_squangle INSTALL_DIR)

set(
  SQUANGLE_DEPS
  boost
  fbmysqlclient
  folly
  re2
)
add_dependencies(bundled_squangle ${SQUANGLE_DEPS})
add_library(squangle INTERFACE)
add_dependencies(squangle bundled_squangle)
target_include_directories(squangle INTERFACE "${INSTALL_DIR}/include")
target_link_libraries(
  squangle
  INTERFACE
  ${SQUANGLE_DEPS} ${OPENSSL_LIBRARIES}
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}squangle${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
set(SQUANGLE_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)
