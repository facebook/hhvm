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

ExternalProject_Add(
  bundled_squangle
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/
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

    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}

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

# This is a bit messy because we need to support old cmake.
# - if we directly declare the library as INTERFACE, the dependencies are not resolved
#   in-order
# - if we make `squangle` the imported target, the dependency on `bundled_squangle` is ignored
# - so, we need 3 targets:
#   - bundled_squangle: actually build the thing
#   - imported_squangle: specify how to link the thing
#   - squangle: specifies that we want to use imported_squangle, but we need to build
#     bundled_squangle first
#
# This can go back to the normal squangle INTERFACE -> bundled_squangle pattern once we're
# universally using CMake 3.11 or above.

add_library(imported_squangle STATIC IMPORTED)
add_dependencies(imported_squangle bundled_squangle)
set_target_properties(
  imported_squangle
  PROPERTIES
  IMPORTED_LOCATION
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}squangle${CMAKE_STATIC_LIBRARY_SUFFIX}"
  INTERFACE_INCLUDE_DIRECTORIES
  "${INSTALL_DIR}/include"
  INTERFACE_LINK_LIBRARIES
  "${SQUANGLE_DEPS};${OPENSSL_LIBRARIES}"
)
set(SQUANGLE_INSTALL_DIR "${INSTALL_DIR}" PARENT_SCOPE)

add_library(squangle INTERFACE)
add_dependencies(squangle bundled_squangle)
target_link_libraries(squangle INTERFACE imported_squangle)
