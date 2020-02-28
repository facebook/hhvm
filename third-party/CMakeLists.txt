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

##### --- header --- #####
set(EXTRA_INCLUDE_PATHS)
set(THIRD_PARTY_MODULES)
set(THIRD_PARTY_HEADERS)

function(TP_INSTALL_HEADERS TARGET SRCDIR DEST)
  file(GLOB_RECURSE files "${SRCDIR}/*.h")
  install(
    CODE "INCLUDE(\"${HPHP_HOME}/CMake/HPHPFunctions.cmake\")
      HHVM_INSTALL_HEADERS(${TARGET}
        \"${CMAKE_CURRENT_BINARY_DIR}/${SRCDIR}\"
      \"\${CMAKE_INSTALL_PREFIX}/include/hphp/third-party/${DEST}\"
      ${files})"
    COMPONENT dev)
endfunction()


##### easy stuff #####

if(JEMALLOC_ENABLED)
  list(APPEND THIRD_PARTY_MODULES jemalloc)
endif()

list(APPEND THIRD_PARTY_MODULES forks/libmbfl)
list(APPEND EXTRA_INCLUDE_PATHS
  "${TP_DIR}/forks/libmbfl"
  "${TP_DIR}/forks/libmbfl/mbfl"
  "${TP_DIR}/forks/libmbfl/filters"
)

list(APPEND THIRD_PARTY_MODULES forks/timelib)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/forks/timelib")

list(APPEND THIRD_PARTY_MODULES folly)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/forks/folly/src")

# Fatal is a header-only library, so there's nothing to build.
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fatal")

list(APPEND THIRD_PARTY_MODULES fizz)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fizz/src")

list(APPEND THIRD_PARTY_MODULES wangle)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/wangle/src")

list(APPEND THIRD_PARTY_MODULES brotli)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/brotli/src/c/include")

list(APPEND THIRD_PARTY_MODULES proxygen)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/proxygen/src")

list(APPEND THIRD_PARTY_MODULES fmt)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fmt/fmt/include")

list(APPEND THIRD_PARTY_MODULES rsocket-cpp)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/rsocket-cpp/src")

list(APPEND THIRD_PARTY_MODULES thrift)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/thrift/src")
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/thrift/gen")

list(APPEND THIRD_PARTY_MODULES zstd)
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/zstd/src/lib")
list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/zstd/src/lib/dictBuilder")

if (NOT MSVC)
  list(APPEND THIRD_PARTY_MODULES libafdt)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/libafdt/src/src")
endif()

if(ENABLE_ASYNC_MYSQL)
  find_package(RE2)
  if(NOT RE2_LIBRARY)
    message(STATUS "Building RE2 from third-party")
    list(APPEND THIRD_PARTY_MODULES re2)
  endif()
  list(APPEND THIRD_PARTY_MODULES squangle)
  list(APPEND THIRD_PARTY_MODULES fb-mysql)
endif()

if (ENABLE_MCROUTER)
  # Ragel is only used at build time to generate some files for MCRouter.
  list(APPEND THIRD_PARTY_MODULES forks/ragel)

  list(APPEND THIRD_PARTY_MODULES mcrouter)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/mcrouter/src")
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/mcrouter/gen")
endif()

# Add bundled fastlz if the system one will not be used
if(NOT FASTLZ_LIBRARY)
  list(APPEND THIRD_PARTY_MODULES fastlz)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/fastlz/src")
endif()

# Add bundled libsqlite3 if the system one will not be used
if(NOT LIBSQLITE3_LIBRARY)
  list(APPEND THIRD_PARTY_MODULES forks/libsqlite3)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/forks/libsqlite3")
endif()

# Add bundled lz4 if the system one will not be used
if(NOT LZ4_FOUND)
  list(APPEND THIRD_PARTY_MODULES lz4)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/lz4/src/lib")
endif()

# Add bundled double-conversion if the system one will not be used
if(NOT DOUBLE_CONVERSION_FOUND)
  list(APPEND THIRD_PARTY_MODULES double-conversion)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/double-conversion")
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

list(APPEND THIRD_PARTY_MODULES ocaml)

# Add bundled XED library if XED is enabled and the specified XED library
# is either not found or incompatible.
if(ENABLE_XED AND NOT LibXed_FOUND)
  list(APPEND THIRD_PARTY_MODULES xed)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/xed/xed/build/include/xed")
endif()

##### boost #####

# boost checks
find_package(Boost 1.62.0 COMPONENTS context fiber filesystem iostreams program_options regex system thread)
if ("${Boost_VERSION}" EQUAL "107000")
  # https://github.com/boostorg/variant/issues/69
  message(WARNING "System boost is blacklisted version")

  set(Boost_FOUND false)
endif()

add_library(boost INTERFACE)
add_custom_target(boostMaybeBuild)
if(NOT Boost_FOUND)
  # The boost library can't have a dependency on boostBuild because of:
  # https://public.kitware.com/Bug/view.php?id=15414
  # Instead we add the dependency to the target being linked with hphp_link
  add_dependencies(boostMaybeBuild boostBuild)
  message(STATUS "Using third-party bundled boost")
  add_subdirectory(boost)
  # These are the archive names prefixed with lib so that cmake does not
  # find these to be system archive instead of these third party ones.
  set(Boost_LIBRARIES
      "libboost_fiber"
      "libboost_context"
      "libboost_filesystem"
      "libboost_iostreams"
      "libboost_program_options"
      "libboost_regex"
      "libboost_system"
      "libboost_thread"
      CACHE STRING "" FORCE
  )
  foreach(lib ${Boost_LIBRARIES})
    add_library(${lib} STATIC IMPORTED)
    set_property(TARGET ${lib} PROPERTY IMPORTED_LOCATION
                 "${CMAKE_CURRENT_SOURCE_DIR}/boost/build/lib/${lib}.a")
  endforeach()
  set(Boost_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/boost/build/include" CACHE STRING "" FORCE)
  set(Boost_LIBRARY_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/boost/build/lib" CACHE STRING "" FORCE)
  TP_INSTALL_HEADERS(boost boost/build/include/boost boost)
  list(APPEND EXTRA_INCLUDE_PATHS "${TP_DIR}/boost/build/include")
endif()
target_include_directories(boost BEFORE INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(boost INTERFACE ${Boost_LIBRARIES})

##### libsodium #####

add_library(libsodium INTERFACE)
add_custom_target(libsodiumMaybeBuild)
find_package(LibSodium 1.0.9)
if (LIBSODIUM_INCLUDE_DIRS AND LIBSODIUM_LIBRARIES)
  message(STATUS "Using system libsodium ${LIBSODIUM_VERSION}")
else()
  message(STATUS "Using third-party bundled libsodium")
  add_dependencies(libsodiumMaybeBuild libsodiumBuild)
  add_subdirectory(libsodium EXCLUDE_FROM_ALL)
endif()
target_include_directories(libsodium INTERFACE ${LIBSODIUM_INCLUDE_DIRS})
target_link_libraries(libsodium INTERFACE ${LIBSODIUM_LIBRARIES})
add_definitions("-DHAVE_LIBSODIUM")
list(APPEND EXTRA_INCLUDE_PATHS "${LIBSODIUM_INCLUDE_DIRS}")
list(APPEND THIRD_PARTY_DEFINITIONS "-DHAVE_LIBSODIUM")

##### rustc #####

add_subdirectory("rustc")

##### --- footer --- #####

foreach(MODULE ${THIRD_PARTY_MODULES})
  add_subdirectory(${MODULE})
  # Install headers here, rather than in the individual submodules, so that we
  # can easily control the destination. Put them in include/hphp/third-party.
  if(MODULE STREQUAL folly)
    TP_INSTALL_HEADERS(folly folly/src/folly folly)
  else()
    TP_INSTALL_HEADERS(${MODULE} ${MODULE} ${MODULE})
  endif()
endforeach()
set(THIRD_PARTY_INCLUDE_PATHS ${EXTRA_INCLUDE_PATHS} CACHE INTERNAL "" FORCE)

if(NOT PCRE_LIBRARY)
  target_include_directories(pcre PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/forks/pcre")
  target_compile_definitions(pcre PUBLIC "PCRE_STATIC=1")
endif()

