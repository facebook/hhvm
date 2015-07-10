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

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/facebook")
  # I'm sorry, but facebook's internal repo has the top level dir stored inside
  # of hphp/, so we need to pull that one in first if it exists
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
  set(HPHP_HOME ${CMAKE_CURRENT_SOURCE_DIR}/..)
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/facebook/CMake"
                        "${CMAKE_CURRENT_SOURCE_DIR}/public_tld/CMake"
                        ${CMAKE_MODULE_PATH})
  include(FBTLD)
endif()

include(HPHPSetup)
include(FollySetup)
if (ENABLE_COTIRE)
  include(cotire)
  FIND_PATH(LIBC_INCLUDE_PATH stdlib.h)

  # Detect the architecture-specific include directory
  IF("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "")
    # For CentOS/Red Hat where they store it directly in /usr/include
    SET(ARCH_INCLUDE_PATH "${LIBC_INCLUDE_PATH}/bits")
  ELSE()
    FIND_PATH(ARCH_INCLUDE_PATH ${CMAKE_LIBRARY_ARCHITECTURE})
    SET(ARCH_INCLUDE_PATH "${ARCH_INCLUDE_PATH}/${CMAKE_LIBRARY_ARCHITECTURE}")
  ENDIF()

  set_property(DIRECTORY
    PROPERTY COTIRE_PREFIX_HEADER_IGNORE_PATH
      "${LIBC_INCLUDE_PATH}/stdlib.h"
      "${LIBC_INCLUDE_PATH}/string.h"
      "${LIBC_INCLUDE_PATH}/ansidecl.h"
      "${LIBC_INCLUDE_PATH}/bfd.h"
      "${LIBC_INCLUDE_PATH}/libelf.h"
      "${LIBC_INCLUDE_PATH}/elf.h"
      "${LIBC_INCLUDE_PATH}/gelf.h"
      "${ARCH_INCLUDE_PATH}"
      "${CCLIENT_INCLUDE_PATH}"
      "${JEMALLOC_INCLUDE_DIR}/jemalloc"
      "${ONIGURUMA_INCLUDE_DIR}/onigposix.h"
      "${ONIGURUMA_INCLUDE_DIR}/oniguruma.h"
      "${LIBPNG_INCLUDE_DIRS}/png.h"
      "${LDAP_INCLUDE_DIR}/ldap.h"
      "${CMAKE_SOURCE_DIR}"
      "${CMAKE_BINARY_DIR}")
endif()

# Only thing to do directly in tools is install this one script. Tools also has
# its own Makefile, so just do it here instead of dealing with moving that all
# around.
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/tools/oss-repo-mode
  RENAME hhvm-repo-mode
  DESTINATION bin
  COMPONENT dev)

add_definitions("-DHHVM")
add_definitions("-DUSE_CMAKE")

enable_language(ASM)

add_subdirectory(tools/bootstrap)
add_subdirectory(tools/hfsort)
add_subdirectory(tools/version)
add_subdirectory(tools/tc-print)

add_subdirectory(compiler)
add_subdirectory(hack)
add_subdirectory(hhbbc)
add_subdirectory(neo)
add_subdirectory(parser)
add_subdirectory(scan-methods)

add_subdirectory(runtime)
add_subdirectory(runtime/ext)
add_subdirectory(runtime/ext_hhvm)
if (ENABLE_ZEND_COMPAT)
  add_subdirectory(runtime/ext_zend_compat)
endif()
add_subdirectory(system)
add_subdirectory(util)
add_subdirectory(vixl)
add_subdirectory(zend)

add_subdirectory(hhvm)

option(TEST_BIN "Create the HHVM test binary" OFF)
if (TEST_BIN)
  add_subdirectory(test)
endif ()

add_subdirectory(tools/gdb)

# Keep this last
add_subdirectory(tools/hphpize)
