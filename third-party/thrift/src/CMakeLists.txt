# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)

# Package information.
if (NOT DEFINED PACKAGE_VERSION)
  set(PACKAGE_VERSION 1.0.0)
endif ()
project(THRIFT VERSION ${PACKAGE_VERSION} LANGUAGES CXX)

if (NOT DEFINED CPACK_GENERATOR)
  set(CPACK_GENERATOR RPM)
endif ()
set(CPACK_RPM_PACKAGE_PROVIDES thrift)
include(CPack)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BIN_INSTALL_DIR bin CACHE STRING
    "The subdirectory where the compiler binary should be installed")
set(INCLUDE_INSTALL_DIR include CACHE STRING
    "The subdirectory where include files should be installed")
set(LIB_INSTALL_DIR lib CACHE STRING
    "The subdirectory where libraries should be installed")
set(CMAKE_INSTALL_DIR lib/cmake/fbthrift CACHE STRING
    "The subdirectory where CMake package config files should be installed")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Add root dir so qualified includes work, e.g. #include "thrift/compiler/*.h".
include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Set directory of the Find$x.cmake files to properly include dependencies.
set(CMAKE_STD_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/thrift/cmake
  # For in-fbsource builds:
  ${CMAKE_CURRENT_SOURCE_DIR}/../opensource/fbcode_builder/CMake
  # For shipit-transformed builds:
  ${CMAKE_CURRENT_SOURCE_DIR}/build/fbcode_builder/CMake
  ${CMAKE_MODULE_PATH})

# Find required dependencies.
find_package(Boost REQUIRED filesystem)
find_package(fmt CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  message(STATUS "setting C++ standard to C++${CMAKE_CXX_STANDARD}")
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable modular builds.
option(THRIFT_COMPILER_ONLY "Build the Thrift compiler only" OFF)
option(THRIFT_LIB_ONLY "Build the Thrift libraries only" OFF)
if (THRIFT_COMPILER_ONLY OR THRIFT_LIB_ONLY)
  set(build_all OFF)
else ()
  set(build_all ON)
endif ()

set(
  thriftpy AUTO
  CACHE BOOL
  "Install the thrift/lib/py library as an FB Python archive manifest"
)
set_property(CACHE thriftpy PROPERTY STRINGS ON OFF AUTO)
option(thriftpy3
  "Include thrift-py3 library and extensions in the build, requires Cython"
  OFF
)

# Find required dependencies for the Thrift compiler.
if (THRIFT_COMPILER_ONLY OR build_all)
  include_directories(${OPENSSL_INCLUDE_DIR})
  set(THRIFT1 thrift1)
  set(THRIFTCPP2 thriftcpp2)
  include(ThriftLibrary.cmake)
  install(FILES ThriftLibrary.cmake DESTINATION ${INCLUDE_INSTALL_DIR}/thrift)
endif ()

# Find required dependencies for thrift/lib
if (THRIFT_LIB_ONLY OR build_all)
  find_package(Gflags REQUIRED)
  find_package(Glog REQUIRED)
  find_package(folly CONFIG REQUIRED)
  find_package(fizz CONFIG REQUIRED)
  find_package(wangle CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(Zstd REQUIRED)
  find_package(mvfst CONFIG REQUIRED)
  # https://cmake.org/cmake/help/v3.9/module/FindThreads.html
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
  include_directories(
    ${LIBGFLAGS_INCLUDE_DIR}
    ${GLOG_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${ZSTD_INCLUDE_DIRS}
  )
  add_definitions("-DTHRIFT_HAVE_LIBSNAPPY=0")
  if (THRIFT_LIB_ONLY)
    find_program(THRIFT1 thrift1)
    include(${THRIFT_COMPILER_INCLUDE}/thrift/ThriftLibrary.cmake)
  endif ()

  find_package(python-six)
  if("${thriftpy}" STREQUAL "AUTO")
    if(python-six_FOUND)
      message(STATUS
        "Python dependencies found, enabling build of thrift/lib/py"
      )
    else()
      message(STATUS
        "Python dependencies not found, will not build thrift/lib/py"
      )
    endif()
    set(thriftpy "${python-six_FOUND}")
  elseif(thriftpy AND NOT python-six_FOUND)
    message(FATAL_ERROR
        "Unable to find python-six, but building thrift/lib/py was requested"
    )
  endif()
endif ()

if(thriftpy3)
  find_package(PythonInterp 3.6 REQUIRED)
  find_package(PythonLibs 3 REQUIRED)
  find_package(Cython 0.28 REQUIRED)
endif()

# Add the test dependencies
# To run tests: `make test`
if(enable_tests)
  find_package(PythonInterp REQUIRED)
  find_package(GTest REQUIRED)
  find_package(GMock REQUIRED)
  include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
  )
  enable_testing()
endif(enable_tests)

# Create a generalized function for tests
function(thrift_gtest tname srcfile)
  add_executable("${tname}-t" ${srcfile})
  target_link_libraries(
    "${tname}-t"

    ${ARGN}
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_BOTH_LIBRARIES}
    pthread
  )
  gtest_add_tests("${tname}-t" "" ${srcfile})
endfunction(thrift_gtest)

add_subdirectory(thrift)
