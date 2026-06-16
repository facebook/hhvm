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

# oniguruma/ is special: it is set up from HPHPFindLibs as it must be included
# *first* to take precedence over libc regexp functions
add_subdirectory(oniguruma)

##### --- header --- #####
set(EXTRA_INCLUDE_PATHS)
set(THIRD_PARTY_MODULES)
set(THIRD_PARTY_HEADERS)
include(GNUInstallDirs)

function(TP_INSTALL_HEADERS TARGET SRCDIR DEST)
  file(GLOB_RECURSE files "${SRCDIR}/*.h")
  install(
    CODE "INCLUDE(\"${HPHP_HOME}/CMake/HPHPFunctions.cmake\")
      HHVM_INSTALL_HEADERS(${TARGET}
        \"${CMAKE_CURRENT_BINARY_DIR}/${SRCDIR}\"
      \"\${CMAKE_INSTALL_FULL_INCLUDEDIR}/hphp/third-party/${DEST}\"
      ${files})"
    COMPONENT dev)
endfunction()


##### easy stuff #####

list(APPEND THIRD_PARTY_MODULES forks/libmbfl)
list(APPEND EXTRA_INCLUDE_PATHS
  "${TP_DIR}/forks/libmbfl"
  "${TP_DIR}/forks/libmbfl/mbfl"
  "${TP_DIR}/forks/libmbfl/filters"
)

list(APPEND THIRD_PARTY_MODULES timelib)

# Fatal is header-only. Prefer a getdeps/system install so the devserver flow
# does not need to clone the source tree just for headers.
find_path(
  FATAL_INCLUDE_DIR
  NAMES fatal/portability.h
  PATHS ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES "" include
)
if(FATAL_INCLUDE_DIR)
  message(STATUS "Using fatal headers from: ${FATAL_INCLUDE_DIR}")
  list(APPEND EXTRA_INCLUDE_PATHS "${FATAL_INCLUDE_DIR}")
elseif(EXISTS "${TP_DIR}/fatal/fatal/portability.h")
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fatal")
else()
  message(FATAL_ERROR
    "Could not find fatal headers. Build/install fatal via getdeps or "
    "populate third-party/fatal in the assembled source tree."
  )
endif()

add_subdirectory(libafdt)
get_target_property(AFDT_INCLUDE_DIRS afdt INTERFACE_INCLUDE_DIRECTORIES)
if(AFDT_INCLUDE_DIRS AND NOT "${AFDT_INCLUDE_DIRS}" STREQUAL "AFDT_INCLUDE_DIRS-NOTFOUND")
  list(APPEND EXTRA_INCLUDE_PATHS ${AFDT_INCLUDE_DIRS})
endif()

add_subdirectory(fb-mysql)
add_subdirectory(squangle)

# Add bundled fastlz if the system one will not be used
if(NOT FASTLZ_LIBRARY)
  list(APPEND THIRD_PARTY_MODULES fastlz)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fastlz/src")
endif()

# Add bundled PCRE if the system one will not be used
find_package(PCRE)
if(NOT PCRE_LIBRARY)
  list(APPEND THIRD_PARTY_MODULES forks/pcre)
  # Disable building a few parts of PCRE by default that we don't
  # actually use.
  set(PCRE_BUILD_PCRECPP OFF CACHE BOOL "Build the PCRE C++ library (pcrecpp).")
  set(PCRE_BUILD_PCREGREP OFF CACHE BOOL "Build pcregrep")
  set(PCRE_BUILD_TESTS OFF CACHE BOOL "Build the tests")
endif()

list(APPEND THIRD_PARTY_MODULES opam)

# Add bundled XED library if XED is enabled and the specified XED library
# is either not found or incompatible.
if(HHVM_REQUIRE_XED AND NOT LibXed_FOUND)
  list(APPEND THIRD_PARTY_MODULES xed)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/xed/xed/build/include/xed")
endif()

# TBB (prefers system package, falls back to bundled)
add_subdirectory(tbb)

##### --- footer --- #####

foreach(MODULE ${THIRD_PARTY_MODULES})
  add_subdirectory(${MODULE})
  TP_INSTALL_HEADERS(${MODULE} ${MODULE} ${MODULE})
endforeach()
set(THIRD_PARTY_INCLUDE_PATHS ${EXTRA_INCLUDE_PATHS} CACHE INTERNAL "" FORCE)

if(NOT PCRE_LIBRARY)
  target_include_directories(pcre PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/forks/pcre")
  target_compile_definitions(pcre PUBLIC "PCRE_STATIC=1")
endif()

##### --- new style, but only depends on old style --- #####
add_subdirectory(watchman)

