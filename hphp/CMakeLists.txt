#
#   +----------------------------------------------------------------------+
#   | HipHop for PHP                                                       |
#   +----------------------------------------------------------------------+
#   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
#   | Copyright (c) 1997-2010 The PHP Group                                |
#   +----------------------------------------------------------------------+
#   | This source file is subject to version 3.01 of the PHP license,      |
#   | that is bundled with this package in the file LICENSE, and is        |
#   | available through the world-wide-web at the following url:           |
#   | http://www.php.net/license/3_01.txt                                  |
#   | If you did not receive a copy of the PHP license and are unable to   |
#   | obtain it through the world-wide-web, please send a note to          |
#   | license@php.net so we can mail you a copy immediately.               |
#   +----------------------------------------------------------------------+
#

if (EXISTS "public_tld")
  # I'm sorry, but facebook's internal repo has the top level dir stored inside
  # of hphp/, so we need to pull that one in first if it exists
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
  PROJECT(hphp C CXX ASM)
  SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/public_tld/CMake"
                        ${CMAKE_MODULE_PATH})
  set(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public_tld/third-party/")
  if (NOT EXISTS ${TP_DIR})
    message(FATAL_ERROR "I can't find ${TP_DIR}. Maybe try\ngit submodule update --init --recursive")
  endif()
else()
  set(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/")
endif()

include_directories("${TP_DIR}/libsqlite3")
include_directories("${TP_DIR}/timelib")
include_directories("${TP_DIR}/libafdt/src")
include_directories("${TP_DIR}/libmbfl")
include_directories("${TP_DIR}/libmbfl/mbfl")
include_directories("${TP_DIR}/libmbfl/filter")
include_directories("${TP_DIR}/lz4")
include_directories("${TP_DIR}/double-conversion/src")
include_directories("${TP_DIR}/folly")
include_directories("${TP_DIR}/libzip")
include_directories("${TP_DIR}")

if (EXISTS "public_tld")
  add_subdirectory(${TP_DIR})
endif()

# Some included files need to use relative paths, but they could be included
# from anywhere, so give them a variable that points to a known place. Only use
# this if your file is not named CMakeLists.txt
set(HPHP_HOME ${CMAKE_CURRENT_SOURCE_DIR}/..)

include(HPHPSetup)
include(FollySetup)

add_definitions("-DHHVM")
add_definitions("-DUSE_CMAKE")

add_subdirectory(tools/bootstrap)

add_subdirectory(compiler)
add_subdirectory(hack)
add_subdirectory(hhbbc)
add_subdirectory(neo)
add_subdirectory(parser)

add_subdirectory(runtime)
add_subdirectory(runtime/ext)
add_subdirectory(runtime/ext_hhvm)
add_subdirectory(runtime/ext_zend_compat)
add_subdirectory(system)
add_subdirectory(util)
add_subdirectory(vixl)
add_subdirectory(zend)

add_subdirectory(hhvm)

option(TEST_BIN "Create the HHVM test binary" OFF)
if (TEST_BIN)
	add_subdirectory(test)
endif ()

# Keep this last
add_subdirectory(tools/hphpize)
