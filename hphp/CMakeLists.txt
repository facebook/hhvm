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

add_definitions("-DHHVM")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/facebook")
  # I'm sorry, but facebook's internal repo has the top level dir stored inside
  # of hphp/, so we need to pull that one in first if it exists
  CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
  get_filename_component(HPHP_HOME "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
  set(TP_DIR "${HPHP_HOME}/hphp/public_tld/third-party/")
  set(TP_BUILD_DIR "${TP_DIR}")
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/facebook/CMake"
                        "${CMAKE_CURRENT_SOURCE_DIR}/public_tld/CMake"
                        ${CMAKE_MODULE_PATH})
  include(HPHPCompiler)
endif()

include(HHVMProject)

include(HHVMExtensionConfig)
add_custom_target(generated_systemlib)
# Before we do anything else, we'll configure the extensions,
# so that the defines take effect globally.
file(GLOB subdirs ${CMAKE_CURRENT_SOURCE_DIR}/runtime/ext/*)
foreach (dir ${subdirs})
  if (IS_DIRECTORY ${dir})
    if (EXISTS "${dir}/config.cmake")
      set(HRE_CURRENT_EXT_PATH "${dir}")
      include("${dir}/config.cmake")
    endif()
  endif()
endforeach()
HHVM_EXTENSION_RESOLVE_DEPENDENCIES()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/facebook")
  include(FBTLD)
endif()

include(HPHPSetup)
include(HPHPFindLibs)
include(HHVMGenerateConfig)
HHVM_GENERATE_CONFIG("${CMAKE_CURRENT_BINARY_DIR}/util/hphp-config.h")

# Link against this target if you `#include "hphp/runtime/base/foo.h"`; it
# brings in all the transitive dependencies (e.g. rust FFI, folly, boost)
#
# It's currently really the wrong way around: we should declare all the
# dependencies directly against this target without using `hphp_link()`, then
# `hphp_link()` should be changed to link against this target. Done like this
# for now to minimize risk while unblocking adding new dependencies.
#
# Even that's wrong: given improvements in CMake over the years, we don't need
# and should remove many of the HPHP/hphp/HHVM CMake macros, including
# hphp_link, and just use target_link_libraries throughout.
add_library(hhvm_base_headers INTERFACE)
hphp_link(hhvm_base_headers INTERFACE)
target_include_directories(hhvm_base_headers INTERFACE ${CMAKE_SOURCE_DIR})

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
      "${LIBC_INCLUDE_PATH}/resolv.h"
      "${ARCH_INCLUDE_PATH}"
      "${CCLIENT_INCLUDE_PATH}"
      "${LIBPNG_INCLUDE_DIRS}/png.h"
      "${LDAP_INCLUDE_DIR}/ldap.h"
      "${LIBSQLITE3_INCLUDE_DIR}/sqlite3ext.h"
      "${CMAKE_SOURCE_DIR}"
      "${CMAKE_BINARY_DIR}")

  # XED headers need to be wrapped in extern "C"
  if (ENABLE_XED)
    if (LibXed_INCLUDE_DIR)
      set_property(DIRECTORY
          APPEND PROPERTY COTIRE_PREFIX_HEADER_IGNORE_PATH
            "${LibXed_INCLUDE_DIR}")
    else()
      set_property(DIRECTORY
          APPEND PROPERTY COTIRE_PREFIX_HEADER_IGNORE_PATH
            "${TP_DIR}/xed/xed/build/include/xed")
    endif()
  endif()
endif()

# Only thing to do directly in tools is install this one script. Tools also has
# its own Makefile, so just do it here instead of dealing with moving that all
# around.
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/tools/oss-repo-mode
  RENAME hhvm-repo-mode
  DESTINATION bin
  COMPONENT dev)

add_definitions("-DUSE_CMAKE")

enable_language(ASM)

if (NOT PCRE_LIBRARY)
  link_libraries(pcre)
endif()

add_subdirectory(tools/hfsort)
add_subdirectory(tools/version)
add_subdirectory(tools/tc-print)

add_subdirectory(compiler)
add_subdirectory(hack)
add_subdirectory(hhbbc)
add_subdirectory(neo)

add_subdirectory(runtime)
add_subdirectory(runtime/ext)

# The runtime/test binary require GTest and GMock to be installed globally
option(RUNTIME_TEST_BIN "Create the HHVM runtime/test binary" OFF)
if (RUNTIME_TEST_BIN)
  add_subdirectory(runtime/test)
endif ()

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

include(test/dso_test/dso_test.cmake)

# Keep this last
add_subdirectory(tools/hphpize)
