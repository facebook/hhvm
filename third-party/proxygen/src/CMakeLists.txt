# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.10)
include(CheckCXXCompilerFlag)

project(
    proxygen
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  # for in-fbsource builds
  "${CMAKE_CURRENT_SOURCE_DIR}/../opensource/fbcode_builder/CMake"
  # For shipit-transformed builds
  "${CMAKE_CURRENT_SOURCE_DIR}/build/fbcode_builder/CMake"
  ${CMAKE_MODULE_PATH})

option(BUILD_SHARED_LIBS
  "If enabled, build proxygen as a shared library.  \
  This is generally discouraged, since proxygen does not commit to having \
  a stable ABI."
  OFF
)
option(BUILD_SAMPLES
  "If enabled, proxygen will build various examples/samples"
  ON
)
# Mark BUILD_SHARED_LIBS as an "advanced" option, since enabling it
# is generally discouraged.
mark_as_advanced(BUILD_SHARED_LIBS)

include(FBBuildOptions)
fb_activate_static_library_option()

# PROXYGEN_FBCODE_ROOT is where the top level proxygen/ directory resides, so
# an #include <proxygen/path/to/file> will resolve to
# $PROXYGEN_FBCODE_ROOT/proxygen/path/to/file on disk
set(PROXYGEN_FBCODE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Similarly, PROXYGEN_GENERATED_ROOT is where the top level proxygen/ directory
# resides for generated files, so a #include <proxygen/path/to/generated/file>
# will be at $PROXYGEN_GENERATED_ROOT/proxygen/path/to/generated/file
set(PROXYGEN_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROXYGEN_GENERATED_ROOT})

# Build-time program requirements.
find_program(PROXYGEN_PYTHON python3)
if(NOT PROXYGEN_PYTHON)
    message(FATAL_ERROR "python is required for the proxygen build")
endif()

find_program(PROXYGEN_GPERF gperf)
if(NOT PROXYGEN_GPERF)
    message(FATAL_ERROR "gperf is required for the proxygen build")
endif()

# Dependencies
#
# IMPORTANT: If you change this, make the analogous update in:
#   cmake/proxygen-config.cmake.in
find_package(fmt REQUIRED)
find_package(folly REQUIRED)
find_package(wangle REQUIRED)
find_package(mvfst REQUIRED)
find_package(Zstd REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads)
find_package(Boost 1.58 REQUIRED
  COMPONENTS
    iostreams
    context
    filesystem
    program_options
    regex
    system
    thread
)

list(APPEND
    _PROXYGEN_COMMON_COMPILE_OPTIONS
    -Wall
    -Wextra
)

CHECK_CXX_COMPILER_FLAG(-Wnoexcept-type COMPILER_HAS_W_NOEXCEPT_TYPE)
if (COMPILER_HAS_W_NOEXCEPT_TYPE)
  list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-noexcept-type)
endif()
CHECK_CXX_COMPILER_FLAG(-Wunused-parameter COMPILER_HAS_W_UNUSED_PARAMETER)
if (COMPILER_HAS_W_UNUSED_PARAMETER)
  list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-unused-parameter)
endif()
CHECK_CXX_COMPILER_FLAG(-Wmissing-field-initializers COMPILER_HAS_W_MISSING_FIELD_INITIALIZERS)
if (COMPILER_HAS_W_MISSING_FIELD_INITIALIZERS)
  list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-missing-field-initializers)
endif()
CHECK_CXX_COMPILER_FLAG(-Wnullability-completeness COMPILER_HAS_W_NULLABILITY_COMPLETENESS)
if (COMPILER_HAS_W_NULLABILITY_COMPLETENESS)
    list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-nullability-completeness)
endif()
CHECK_CXX_COMPILER_FLAG(-Wdeprecated-register COMPILER_HAS_W_DEPRECATED_REGISTER)
if (COMPILER_HAS_W_DEPRECATED_REGISTER)
    list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-deprecated-register)
endif()
CHECK_CXX_COMPILER_FLAG(-Wregister COMPILER_HAS_W_REGISTER)
if (COMPILER_HAS_W_REGISTER)
    list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-register)
endif()
CHECK_CXX_COMPILER_FLAG(-Wunused-value COMPILER_HAS_W_UNUSED_VALUE)
if (COMPILER_HAS_W_UNUSED_VALUE)
    list(APPEND _PROXYGEN_COMMON_COMPILE_OPTIONS -Wno-unused-value)
endif()

SET(GFLAG_DEPENDENCIES "")
SET(PROXYGEN_EXTRA_LINK_LIBRARIES "")
SET(PROXYGEN_EXTRA_INCLUDE_DIRECTORIES "")

find_package(gflags CONFIG QUIET)
if (gflags_FOUND)
  message("module path: ${CMAKE_MODULE_PATH}")
  message(STATUS "Found gflags from package config")
  if (TARGET gflags-shared)
    list(APPEND GFLAG_DEPENDENCIES gflags-shared)
  elseif (TARGET gflags)
    list(APPEND GFLAG_DEPENDENCIES gflags)
  else()
    message(FATAL_ERROR
            "Unable to determine the target name for the GFlags package.")
  endif()
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${GFLAGS_LIBRARIES})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${GFLAGS_INCLUDE_DIR})
else()
  find_package(Gflags REQUIRED MODULE)
  list(APPEND PROXYGEN_EXTRA_LINK_LIBRARIES ${LIBGFLAGS_LIBRARY})
  list(APPEND PROXYGEN_EXTRA_INCLUDE_DIRECTORIES ${LIBGFLAGS_INCLUDE_DIR})
  list(APPEND CMAKE_REQUIRED_LIBRARIES ${LIBGFLAGS_LIBRARY})
  list(APPEND CMAKE_REQUIRED_INCLUDES ${LIBGFLAGS_INCLUDE_DIR})
endif()

include(ProxygenTest)

add_subdirectory(proxygen)

if (NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "lib")
endif()

if (NOT DEFINED CMAKE_INSTALL_DIR)
    set(CMAKE_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/proxygen/")
endif()

install(
    EXPORT proxygen-exports
    FILE proxygen-targets.cmake
    NAMESPACE proxygen::
    DESTINATION ${CMAKE_INSTALL_DIR}
)
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/proxygen-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/proxygen-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DIR}
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/proxygen-config.cmake
    DESTINATION ${CMAKE_INSTALL_DIR}
)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
