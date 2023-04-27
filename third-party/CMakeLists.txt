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

# Required by folly
add_subdirectory(double-conversion)
add_subdirectory(boost)
add_subdirectory(fmt)
add_subdirectory(jemalloc)
add_subdirectory(libsodium)
add_subdirectory(zstd)

add_subdirectory(folly)

# Fatal is a header-only library, so there's nothing to build.
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fatal")

add_subdirectory(brotli) # Required by fizz
add_subdirectory(fizz)
add_subdirectory(wangle)
add_subdirectory(proxygen)
add_subdirectory(thrift)

if (NOT MSVC)
  list(APPEND THIRD_PARTY_MODULES libafdt)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/libafdt/src/src")
endif()

add_subdirectory(lz4)

# fb-mysql must go after lz4 and zstd because it needs to know its <INSTALL_DIR>
add_subdirectory(re2)
add_subdirectory(fb-mysql)
add_subdirectory(squangle)

if (ENABLE_MCROUTER)
  # Build dependency of mcrouter
  add_subdirectory(forks/ragel)

  add_subdirectory(mcrouter)
endif()

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

# Add bundled libzip if the system one will not be used
add_subdirectory(libzip)

list(APPEND THIRD_PARTY_MODULES opam)

# Add bundled XED library if XED is enabled and the specified XED library
# is either not found or incompatible.
if(ENABLE_XED AND NOT LibXed_FOUND)
  list(APPEND THIRD_PARTY_MODULES xed)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/xed/xed/build/include/xed")
endif()

# Add bundled TBB
list(APPEND THIRD_PARTY_MODULES tbb)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/tbb/src/include")

##### rustc #####

add_subdirectory("rustc")

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
