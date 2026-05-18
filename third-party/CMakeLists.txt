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
add_subdirectory(blake3)
add_subdirectory(fmt)
add_subdirectory(jemalloc)
add_subdirectory(libsodium)
add_subdirectory(zstd)
add_subdirectory(gflags)
add_subdirectory(glog)

# Meta first-party deps: use pre-built libraries via find_package().
# These are built by getdeps.py and found via CMAKE_PREFIX_PATH.
find_package(folly CONFIG REQUIRED)
if(NOT TARGET folly)
  add_library(folly INTERFACE)
  target_link_libraries(folly INTERFACE Folly::folly)
endif()

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

add_subdirectory(brotli) # Required by fizz

find_package(fizz CONFIG REQUIRED)
if(NOT TARGET fizz)
  add_library(fizz INTERFACE)
  target_link_libraries(fizz INTERFACE fizz::fizz)
endif()

find_package(wangle CONFIG REQUIRED)
if(NOT TARGET wangle)
  add_library(wangle INTERFACE)
  target_link_libraries(wangle INTERFACE wangle::wangle)
endif()

find_package(mvfst CONFIG REQUIRED)
if(NOT TARGET mvfst)
  add_library(mvfst INTERFACE)
  target_link_libraries(mvfst INTERFACE mvfst::mvfst_transport mvfst::mvfst_client mvfst::mvfst_server mvfst::mvfst_codec_types)
endif()

# proxygen headers are needed even when FASTCGI is OFF (transport.h includes them)
find_package(proxygen CONFIG REQUIRED)
if(NOT TARGET proxygen)
  add_library(proxygen INTERFACE)
  target_link_libraries(proxygen INTERFACE proxygen::proxygen)
endif()

find_package(FLEX REQUIRED)
find_package(BISON 3.0 REQUIRED)
find_package(FBThrift CONFIG REQUIRED)
add_library(thrift INTERFACE)
target_link_libraries(thrift INTERFACE FBThrift::thriftcpp2 FBThrift::thriftprotocol FBThrift::transport FBThrift::async FBThrift::concurrency FBThrift::rpcmetadata FBThrift::thriftmetadata FBThrift::thriftannotation FBThrift::thrift-core FBThrift::thriftfrozen2)
if(TARGET FBThrift::thrifttyperep)
  target_link_libraries(thrift INTERFACE FBThrift::thrifttyperep)
endif()
if(TARGET FBThrift::thrifttype)
  target_link_libraries(thrift INTERFACE FBThrift::thrifttype)
endif()
if(TARGET FBThrift::thriftanyrep)
  target_link_libraries(thrift INTERFACE FBThrift::thriftanyrep)
endif()
add_executable(thrift1 IMPORTED GLOBAL)
set_target_properties(thrift1 PROPERTIES IMPORTED_LOCATION "${FBTHRIFT_COMPILER}")

if (NOT MSVC)
  add_subdirectory(libafdt)
  get_target_property(AFDT_INCLUDE_DIRS afdt INTERFACE_INCLUDE_DIRECTORIES)
  if(AFDT_INCLUDE_DIRS AND NOT "${AFDT_INCLUDE_DIRS}" STREQUAL "AFDT_INCLUDE_DIRS-NOTFOUND")
    list(APPEND EXTRA_INCLUDE_PATHS ${AFDT_INCLUDE_DIRS})
  endif()
endif()

add_subdirectory(lz4)

# fb-mysql must go after lz4 and zstd because it needs to know its <INSTALL_DIR>
add_subdirectory(re2)
add_subdirectory(fb-mysql)
add_subdirectory(squangle)

if (ENABLE_MCROUTER)
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

# TBB (prefers system package, falls back to bundled)
add_subdirectory(tbb)

##### rustc #####

add_subdirectory("rustc")

# Use the real xplat/usdt implementation for the Linux-only OSS build so
# verification doesn't silently compile away probes.
if(EXISTS "${CMAKE_SOURCE_DIR}/xplat/usdt/usdt.h")
  list(APPEND EXTRA_INCLUDE_PATHS "${CMAKE_SOURCE_DIR}/xplat")
else()
  message(FATAL_ERROR
    "Could not find xplat/usdt/usdt.h for this Linux OSS build. "
    "Populate xplat/usdt in the assembled source tree instead of using the "
    "no-op fallback header."
  )
endif()

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

add_subdirectory(magic_enum)
