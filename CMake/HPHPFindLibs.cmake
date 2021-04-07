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

include(CheckFunctionExists)

# libdl
find_package(LibDL)
if (LIBDL_INCLUDE_DIRS)
  add_definitions("-DHAVE_LIBDL")
  include_directories(${LIBDL_INCLUDE_DIRS})
  if (LIBDL_NEEDS_UNDERSCORE)
    add_definitions("-DLIBDL_NEEDS_UNDERSCORE")
  endif()
endif()

# google-glog
find_package(Glog REQUIRED)
if (LIBGLOG_STATIC)
  add_definitions("-DGOOGLE_GLOG_DLL_DECL=")
endif()
include_directories(${LIBGLOG_INCLUDE_DIR})

# inotify checks
find_package(Libinotify)
if (LIBINOTIFY_INCLUDE_DIR)
  include_directories(${LIBINOTIFY_INCLUDE_DIR})
endif()

# pcre checks
find_package(PCRE)
include_directories(${PCRE_INCLUDE_DIR})

# libevent checks
find_package(LibEvent REQUIRED)
include_directories(${LIBEVENT_INCLUDE_DIR})

set(CMAKE_REQUIRED_LIBRARIES "${LIBEVENT_LIB}")
CHECK_FUNCTION_EXISTS("evhttp_bind_socket_with_fd" HAVE_CUSTOM_LIBEVENT)
if(HAVE_CUSTOM_LIBEVENT)
        message("Using custom LIBEVENT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_CUSTOM_LIBEVENT")
endif()
set(CMAKE_REQUIRED_LIBRARIES)

# libXed
if (ENABLE_XED)
  find_package(LibXed)
  if (LibXed_FOUND)
    include_directories(${LibXed_INCLUDE_DIR})
  endif()
  add_definitions("-DHAVE_LIBXED")
else()
  message(STATUS "XED is disabled")
endif()

# CURL checks
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
if (CURL_STATIC)
  add_definitions("-DCURL_STATICLIB")
endif()

set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
CHECK_FUNCTION_EXISTS("curl_multi_select" HAVE_CURL_MULTI_SELECT)
CHECK_FUNCTION_EXISTS("curl_multi_wait" HAVE_CURL_MULTI_WAIT)
if (HAVE_CURL_MULTI_SELECT)
  add_definitions("-DHAVE_CURL_MULTI_SELECT")
endif()
if (HAVE_CURL_MULTI_WAIT)
  add_definitions("-DHAVE_CURL_MULTI_WAIT")
endif()
set(CMAKE_REQUIRED_LIBRARIES)

# LibXML2 checks
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
add_definitions(${LIBXML2_DEFINITIONS})

# libsqlite3
find_package(LibSQLite)
if (LIBSQLITE3_INCLUDE_DIR)
  include_directories(${LIBSQLITE3_INCLUDE_DIR})
endif ()

# fastlz
find_package(FastLZ)
if (FASTLZ_INCLUDE_DIR)
  include_directories(${FASTLZ_INCLUDE_DIR})
endif()

# ICU
find_package(ICU REQUIRED)
if (ICU_FOUND)
  if (ICU_VERSION VERSION_LESS "4.2")
    unset(ICU_FOUND CACHE)
    unset(ICU_INCLUDE_DIRS CACHE)
    unset(ICU_LIBRARIES CACHE)
    message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
  endif ()
  include_directories(${ICU_INCLUDE_DIRS})
  if (ICU_STATIC)
    add_definitions("-DU_EXPORT=")
    add_definitions("-DU_IMPORT=")
  endif()
  # Everything is either in the `icu61` namespace or `icu` namespace, depending
  # on another definition. There's an implicit `using namespace WHATEVER;` in
  # ICU4c < 61.1, but now that's opt-in rather than opt-out.
  add_definitions("-DU_USING_ICU_NAMESPACE=1")
endif (ICU_FOUND)

# jemalloc/tmalloc and profiler
if (USE_GOOGLE_HEAP_PROFILER OR USE_GOOGLE_CPU_PROFILER)
  FIND_LIBRARY(GOOGLE_PROFILER_LIB profiler)
  FIND_PATH(GOOGLE_PROFILER_INCLUDE_DIR NAMES google/profiler.h)
  if (GOOGLE_PROFILER_INCLUDE_DIR)
    include_directories(${GOOGLE_PROFILER_INCLUDE_DIR})
  endif()
  if (GOOGLE_PROFILER_LIB)
    message(STATUS "Found Google profiler: ${GOOGLE_PROFILER_LIB}")
    if (USE_GOOGLE_CPU_PROFILER)
      set(GOOGLE_CPU_PROFILER_ENABLED 1)
    endif()
  else()
    message(STATUS "Can't find Google profiler")
  endif()
endif()

if (USE_GOOGLE_HEAP_PROFILER AND GOOGLE_PROFILER_LIB)
  FIND_LIBRARY(GOOGLE_TCMALLOC_FULL_LIB tcmalloc)
  if (GOOGLE_TCMALLOC_FULL_LIB)
    message(STATUS "Found full tcmalloc: ${GOOGLE_TCMALLOC_FULL_LIB}")
    set(GOOGLE_HEAP_PROFILER_ENABLED 1)
    set(GOOGLE_TCMALLOC_ENABLED 1)
  else()
    message(STATUS "Can't find full tcmalloc - heap profiling is disabled")
  endif()
endif()

if(USE_JEMALLOC AND NOT GOOGLE_TCMALLOC_ENABLED)
  add_definitions(-DUSE_JEMALLOC=1)
  set(JEMALLOC_ENABLED 1)
else()
  add_definitions(-DNO_JEMALLOC=1)
endif()

if (USE_TCMALLOC AND NOT JEMALLOC_ENABLED AND NOT GOOGLE_TCMALLOC_ENABLED)
  FIND_LIBRARY(GOOGLE_TCMALLOC_MIN_LIB tcmalloc_minimal)
  if (GOOGLE_TCMALLOC_MIN_LIB)
    message(STATUS "Found minimal tcmalloc: ${GOOGLE_TCMALLOC_MIN_LIB}")
    set(GOOGLE_TCMALLOC_ENABLED 1)
  else()
    message(STATUS "Can't find minimal tcmalloc")
  endif()
endif()

if (GOOGLE_TCMALLOC_ENABLED)
  add_definitions(-DGOOGLE_TCMALLOC=1)
else()
  add_definitions(-DNO_TCMALLOC=1)
endif()
if (GOOGLE_HEAP_PROFILER_ENABLED)
  add_definitions(-DGOOGLE_HEAP_PROFILER=1)
endif()
if (GOOGLE_CPU_PROFILER_ENABLED)
  add_definitions(-DGOOGLE_CPU_PROFILER=1)
endif()

# HHProf
if (JEMALLOC_ENABLED AND ENABLE_HHPROF)
  add_definitions(-DENABLE_HHPROF=1)
endif()

# tbb libs
find_package(TBB REQUIRED)
if (${TBB_INTERFACE_VERSION} LESS 5005)
  unset(TBB_FOUND CACHE)
  unset(TBB_INCLUDE_DIRS CACHE)
  unset(TBB_LIBRARIES CACHE)
  message(FATAL_ERROR "TBB is too old, please install at least 3.0(5005), preferably 4.0(6000) or higher")
endif()
include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})

# OpenSSL libs
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# LibreSSL explicitly refuses to support RAND_egd()
SET(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
SET(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#include <openssl/rand.h>
int main() {
  return RAND_egd(\"/dev/null\");
}" OPENSSL_HAVE_RAND_EGD)
if (NOT OPENSSL_HAVE_RAND_EGD)
  add_definitions("-DOPENSSL_NO_RAND_EGD")
endif()
CHECK_CXX_SOURCE_COMPILES("#include <openssl/ssl.h>
int main() {
  return SSL_set_alpn_protos(nullptr, nullptr, 0);
}" OPENSSL_HAVE_ALPN)
SET(CMAKE_REQUIRED_INCLUDES)
SET(CMAKE_REQUIRED_LIBRARIES)


# ZLIB
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

# libpthreads
find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})
if (LIBPTHREAD_STATIC)
  add_definitions("-DPTW32_STATIC_LIB")
endif()

OPTION(
  NON_DISTRIBUTABLE_BUILD
  "Use libraries which may result in a binary that can not be legally distributed"
  OFF
)

# Either Readline or Editline (for hphpd)
if(NON_DISTRIBUTABLE_BUILD)
  find_package(Readline)
endif()
if (NOT READLINE_INCLUDE_DIR)
  find_package(Editline)
endif()

if (NON_DISTRIBUTABLE_BUILD AND READLINE_INCLUDE_DIR)
  if (READLINE_STATIC)
    add_definitions("-DREADLINE_STATIC")
  endif()
  include_directories(${READLINE_INCLUDE_DIR})
elseif (EDITLINE_INCLUDE_DIRS)
  add_definitions("-DUSE_EDITLINE")
  include_directories(${EDITLINE_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Could not find Readline or Editline")
endif()

if (NOT WINDOWS)
  find_package(LibDwarf REQUIRED)
  include_directories(${LIBDWARF_INCLUDE_DIRS})
  if (LIBDWARF_CONST_NAME)
    add_definitions("-DLIBDWARF_CONST_NAME")
  endif()
  if (LIBDWARF_USE_INIT_C)
    add_definitions("-DLIBDWARF_USE_INIT_C")
  endif()

  find_package(LibElf REQUIRED)
  include_directories(${LIBELF_INCLUDE_DIRS})
  if (ELF_GETSHDRSTRNDX)
    add_definitions("-DHAVE_ELF_GETSHDRSTRNDX")
  endif()
endif()

FIND_LIBRARY(CRYPT_LIB NAMES xcrypt crypt crypto)
if (LINUX OR FREEBSD)
  FIND_LIBRARY (RT_LIB rt)
endif()

if (LINUX)
  FIND_LIBRARY (CAP_LIB cap)

  if (NOT CAP_LIB)
    message(FATAL_ERROR "You need to install libcap")
  endif()
endif()

if (LINUX OR APPLE)
  FIND_LIBRARY (DL_LIB dl)
  FIND_LIBRARY (RESOLV_LIB resolv)
endif()

if (FREEBSD)
  FIND_LIBRARY (EXECINFO_LIB execinfo)
  if (NOT EXECINFO_LIB)
    message(FATAL_ERROR "You need to install libexecinfo")
  endif()
endif()

if (APPLE)
  find_library(KERBEROS_LIB NAMES gssapi_krb5)

  # This is required by Homebrew's libc. See
  # https://github.com/facebook/hhvm/pull/5728#issuecomment-124290712
  # for more info.
  find_package(Libpam)
  if (PAM_INCLUDE_PATH)
    include_directories(${PAM_INCLUDE_PATH})
  endif()
endif()

#find_package(BISON REQUIRED)
#find_package(FLEX REQUIRED)

#if (${FLEX_VERSION} VERSION_LESS 2.5.33)
#  message(FATAL_ERROR "Flex is too old, found ${FLEX_VERSION} and we need 2.5.33")
#endif()

include_directories(${HPHP_HOME}/hphp)

macro(hphp_link target)
  # oniguruma must be linked first for MacOS's linker to do the right thing -
  # that's handled in HPHPSetup.cmake
  #
  # That only handles linking - we still need to make sure that:
  # - oniguruma is built first, if needed (so we have the header files)
  # - we build with the header files in the include path
  if(APPLE)
    add_dependencies(${target} onig)
    target_include_directories(${target} PRIVATE $<TARGET_PROPERTY:onig,INTERFACE_INCLUDE_DIRECTORIES>)
  else()
    # Otherwise, the linker does the right thing, which sometimes means putting it after things that use it
    target_link_libraries(${target} onig)
  endif()

  if (LIBDL_LIBRARIES)
    target_link_libraries(${target} ${LIBDL_LIBRARIES})
  endif ()

  if (JEMALLOC_ENABLED)
    target_link_libraries(${target} jemalloc)
    add_dependencies(${target} jemalloc)
  endif ()

  if (GOOGLE_HEAP_PROFILER_ENABLED OR GOOGLE_CPU_PROFILER_ENABLED)
    target_link_libraries(${target} ${GOOGLE_PROFILER_LIB})
  endif()

  if (GOOGLE_HEAP_PROFILER_ENABLED)
    target_link_libraries(${target} ${GOOGLE_TCMALLOC_FULL_LIB})
  elseif (GOOGLE_TCMALLOC_ENABLED)
    target_link_libraries(${target} ${GOOGLE_TCMALLOC_MIN_LIB})
  endif()

  target_link_libraries(${target} boost)
  target_link_libraries(${target} libsodium)

  target_link_libraries(${target} ${PCRE_LIBRARY})
  target_link_libraries(${target} ${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
  target_link_libraries(${target} ${LIBEVENT_LIB})
  target_link_libraries(${target} ${CURL_LIBRARIES})
  target_link_libraries(${target} ${LIBGLOG_LIBRARY})
  if (LIBJSONC_LIBRARY)
    target_link_libraries(${target} ${LIBJSONC_LIBRARY})
  endif()

  if (LIBINOTIFY_LIBRARY)
    target_link_libraries(${target} ${LIBINOTIFY_LIBRARY})
  endif()

  if (LINUX)
    target_link_libraries(${target} ${CAP_LIB})
  endif()

  if (LINUX OR APPLE)
    target_link_libraries(${target} ${RESOLV_LIB})
    target_link_libraries(${target} ${DL_LIB})
  endif()

  if (FREEBSD)
    target_link_libraries(${target} ${EXECINFO_LIB})
  endif()

  if (APPLE)
    target_link_libraries(${target} ${LIBINTL_LIBRARIES})
    target_link_libraries(${target} ${KERBEROS_LIB})

    if (PAM_LIBRARY)
      target_link_libraries(${target} ${PAM_LIBRARY})
    endif()
  endif()

  if (LIBPTHREAD_LIBRARIES)
    target_link_libraries(${target} ${LIBPTHREAD_LIBRARIES})
  endif()

  target_link_libraries(${target} ${TBB_LIBRARIES})
  target_link_libraries(${target} ${OPENSSL_LIBRARIES})
  target_link_libraries(${target} ${ZLIB_LIBRARIES})

  target_link_libraries(${target} ${LIBXML2_LIBRARIES})

  target_link_libraries(${target} ${LBER_LIBRARIES})

  if (CRYPT_LIB)
    target_link_libraries(${target} ${CRYPT_LIB})
  endif()

  if (LINUX OR FREEBSD)
    target_link_libraries(${target} ${RT_LIB})
  endif()

  if (LIBSQLITE3_FOUND AND LIBSQLITE3_LIBRARY)
    target_link_libraries(${target} ${LIBSQLITE3_LIBRARY})
  else()
    target_link_libraries(${target} sqlite3)
  endif()

  target_link_libraries(${target} double-conversion)

  target_link_libraries(${target} lz4)
  target_link_libraries(${target} libzip)

  if (PCRE_LIBRARY)
    target_link_libraries(${target} ${PCRE_LIBRARY})
  else()
    target_link_libraries(${target} pcre)
  endif()

  if (LIBFASTLZ_LIBRARY)
    target_link_libraries(${target} ${LIBFASTLZ_LIBRARY})
  else()
    target_link_libraries(${target} fastlz)
  endif()

  target_link_libraries(${target} timelib)
  target_link_libraries(${target} folly)
  target_link_libraries(${target} wangle)
  target_link_libraries(${target} brotli)
  target_link_libraries(${target} libcompile_ffi_stubs)
  target_link_libraries(${target} librust_facts_ffi_stubs)
  target_link_libraries(${target} librust_parser_ffi_stubs)

  if (ENABLE_MCROUTER)
    target_link_libraries(${target} mcrouter)
  endif()

  if (NOT MSVC)
    target_link_libraries(${target} afdt)
  endif()
  target_link_libraries(${target} mbfl)

  if (EDITLINE_LIBRARIES)
    target_link_libraries(${target} ${EDITLINE_LIBRARIES})
  elseif (READLINE_LIBRARY)
    target_link_libraries(${target} ${READLINE_LIBRARY})
  endif()

  if (NOT WINDOWS)
    target_link_libraries(${target} ${LIBDWARF_LIBRARIES})
    target_link_libraries(${target} ${LIBELF_LIBRARIES})
  endif()

  if (LINUX)
    target_link_libraries(${target})
  endif()

  if (MSVC)
    target_link_libraries(${target} dbghelp.lib dnsapi.lib)
  endif()

# Check whether atomic operations require -latomic or not
# See https://github.com/facebook/hhvm/issues/5217
  include(CheckCXXSourceCompiles)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "-std=c++1y")
  CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
#include <iostream>
#include <stdint.h>
int main() {
    struct Test { int64_t val1; int64_t val2; };
    std::atomic<Test> s;
    // Do this to stop modern compilers from optimizing away the libatomic
    // calls in release builds, making this test always pass in release builds,
    // and incorrectly think that HHVM doesn't need linking against libatomic.
    bool (std::atomic<Test>::* volatile x)(void) const =
      &std::atomic<Test>::is_lock_free;
    std::cout << (s.*x)() << std::endl;
}
  " NOT_REQUIRE_ATOMIC_LINKER_FLAG)

  if(NOT "${NOT_REQUIRE_ATOMIC_LINKER_FLAG}")
      message(STATUS "-latomic is required to link hhvm")
      find_library(ATOMIC_LIBRARY NAMES atomic libatomic.so.1)
      target_link_libraries(${target} ${ATOMIC_LIBRARY})
  endif()
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})

  if (ENABLE_XED)
    if (LibXed_FOUND)
        target_link_libraries(${target} ${LibXed_LIBRARY})
    else()
        target_link_libraries(${target} xed)
    endif()
  endif()
endmacro()
