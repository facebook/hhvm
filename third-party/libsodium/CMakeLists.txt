cmake_minimum_required(VERSION 2.8.0)
include(ExternalProject)
set(LIBSODIUM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libsodium")
ExternalProject_Add(
  libsodiumBuild
  SOURCE_DIR "${LIBSODIUM_DIR}"
  CONFIGURE_COMMAND
  ./configure "--prefix=${CMAKE_CURRENT_BINARY_DIR}/install"
    --disable-debug
    --disable-dependency-tracking
    --disable-shared
    --enable-static
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install"
  BUILD_IN_SOURCE true
)
set(LIBSODIUM_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/install/include" PARENT_SCOPE)
set(LIBSODIUM_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/install/include" PARENT_SCOPE)
set(LIBSODIUM_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}sodium${CMAKE_STATIC_LIBRARY_SUFFIX}" PARENT_SCOPE)
