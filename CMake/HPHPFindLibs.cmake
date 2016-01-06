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

# boost checks
find_package(Boost 1.51.0 COMPONENTS system program_options filesystem context REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions("-DHAVE_BOOST1_49")


# features.h
FIND_PATH(FEATURES_HEADER features.h)
if (FEATURES_HEADER)
  add_definitions("-DHAVE_FEATURES_H=1")
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

# mysql checks - if we're using async mysql, we use webscalesqlclient from
# third-party/ instead
if (ENABLE_ASYNC_MYSQL)
  include_directories(
    ${TP_DIR}/re2/src/
    ${TP_DIR}/squangle/src/
    ${TP_DIR}/webscalesqlclient/src/include/
  )
  set(MYSQL_CLIENT_LIB_DIR ${TP_DIR}/webscalesqlclient/src/)
  set(MYSQL_CLIENT_LIBS
    ${MYSQL_CLIENT_LIB_DIR}/libmysql/libwebscalesqlclient_r.a
  )
else()
  find_package(MySQL REQUIRED)
  link_directories(${MYSQL_LIB_DIR})
  include_directories(${MYSQL_INCLUDE_DIR})
endif()
MYSQL_SOCKET_SEARCH()
if (MYSQL_UNIX_SOCK_ADDR)
  add_definitions(-DPHP_MYSQL_UNIX_SOCK_ADDR="${MYSQL_UNIX_SOCK_ADDR}")
else ()
  message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
endif ()

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
find_package(LibXed)
if (LibXed_INCLUDE_DIR AND LibXed_LIBRARY)
  include_directories(${LibXed_INCLUDE_DIR})
  add_definitions("-DHAVE_LIBXED")
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
if (LIBSQLITE_INCLUDE_DIR)
  include_directories(${LIBSQLITE_INCLUDE_DIR})
endif ()

# libdouble-conversion
find_package(DoubleConversion)
if (DOUBLE_CONVERSION_INCLUDE_DIR)
  include_directories(${DOUBLE_CONVERSION_INCLUDE_DIR})
endif ()

# liblz4
find_package(LZ4)
if (LZ4_INCLUDE_DIR)
  include_directories(${LZ4_INCLUDE_DIR})
endif()

# fastlz
find_package(FastLZ)
if (FASTLZ_INCLUDE_DIR)
  include_directories(${FASTLZ_INCLUDE_DIR})
endif()

# libzip
find_package(LibZip)
if (LIBZIP_INCLUDE_DIR_ZIP AND LIBZIP_INCLUDE_DIR_ZIPCONF)
  if (LIBZIP_VERSION VERSION_LESS "0.11")
    unset(LIBZIP_FOUND CACHE)
    unset(LIBZIP_LIBRARY CACHE)
    unset(LIBZIP_INCLUDE_DIR_ZIP CACHE)
    unset(LIBZIP_INCLUDE_DIR_ZIPCONF CACHE)
    message(STATUS "libzip is too old, found ${LIBZIP_VERSION} and we need 0.11+, using third-party bundled libzip")
  else ()
    include_directories(${LIBZIP_INCLUDE_DIR_ZIP} ${LIBZIP_INCLUDE_DIR_ZIPCONF})
    message(STATUS "Found libzip: ${LIBZIP_LIBRARY} ${LIBZIP_VERSION}")
  endif ()
else ()
  message(STATUS "Using third-party bundled libzip")
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

if (USE_JEMALLOC AND NOT GOOGLE_TCMALLOC_ENABLED)
  FIND_LIBRARY(JEMALLOC_LIB NAMES jemalloc)
  FIND_PATH(JEMALLOC_INCLUDE_DIR NAMES jemalloc/jemalloc.h)

  if (JEMALLOC_INCLUDE_DIR AND JEMALLOC_LIB)
    include_directories(${JEMALLOC_INCLUDE_DIR})

    set (CMAKE_REQUIRED_INCLUDES ${JEMALLOC_INCLUDE_DIR})
    INCLUDE(CheckCXXSourceCompiles)
    CHECK_CXX_SOURCE_COMPILES("
#include <jemalloc/jemalloc.h>

#define JEMALLOC_VERSION_NUMERIC ((JEMALLOC_VERSION_MAJOR << 24) | (JEMALLOC_VERSION_MINOR << 16) | (JEMALLOC_VERSION_BUGFIX << 8) | JEMALLOC_VERSION_NDEV)

#if JEMALLOC_VERSION_NUMERIC < 0x03050100
# error jemalloc version >= 3.5.1 required
#endif

int main(void) { return 0; }" JEMALLOC_VERSION_MINIMUM)
    set (CMAKE_REQUIRED_INCLUDES)

    if (JEMALLOC_VERSION_MINIMUM)
      message(STATUS "Found jemalloc: ${JEMALLOC_LIB}")
      set(JEMALLOC_ENABLED 1)
    else()
      message(STATUS "Found jemalloc, but it was too old")
    endif()
  endif()
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

if (JEMALLOC_ENABLED)
  add_definitions(-DUSE_JEMALLOC=1)
else()
  add_definitions(-DNO_JEMALLOC=1)
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
SET(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#include <openssl/rand.h>
int main() {
  return RAND_egd(\"/dev/null\");
}" OPENSSL_HAVE_RAND_EGD)
if (NOT OPENSSL_HAVE_RAND_EGD)
  add_definitions("-DOPENSSL_NO_RAND_EGD")
endif()


# ZLIB
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

# oniguruma
find_package(ONIGURUMA REQUIRED)
include_directories(${ONIGURUMA_INCLUDE_DIRS})
if (ONIGURUMA_STATIC)
  add_definitions("-DONIG_EXTERN=extern")
endif()

# libpthreads
find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})
if (LIBPTHREAD_STATIC)
  add_definitions("-DPTW32_STATIC_LIB")
endif()

# Either Readline or Editline (for hphpd)
find_package(Readline)
find_package(Editline)
if (EDITLINE_INCLUDE_DIRS)
  add_definitions("-DUSE_EDITLINE")
  include_directories(${EDITLINE_INCLUDE_DIRS})
elseif (READLINE_INCLUDE_DIR)
  if (READLINE_STATIC)
    add_definitions("-DREADLINE_STATIC")
  endif()
  include_directories(${READLINE_INCLUDE_DIR})
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

# LLVM. Disabled in OSS for now: t5056266
# find_package(LLVM)
# if (LIBLLVM_INCLUDE_DIR)
#   include_directories(LIBLLVM_INCLUDE_DIR)
#   add_definitions("-DUSE_LLVM")
# endif()

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
  # oniguruma must remain first for OS X to work -- see below for a somewhat
  # dogscience explanation. If you deeply understand this, feel free to fix
  # properly; in particular, two-level namespaces on OS X should allow us to
  # say *which* copy of the disputed functions we want, but I don' t know
  # how to get that to work.
  #
  # oniguruma has some of its own implementations of POSIX regex functions,
  # like regcomp() an regexec(). We use onig everywhere, for both its own
  # sepcial functions and for the POSIX replacements. This means that the
  # linker needs to pick the implementions of the POSIX regex functions from
  # onig, not libc.
  #
  # On Linux, that works out fine, since the linker sees onig on the link
  # line before (implicitly) libc. However, on OS X, despide the manpage for
  # ld claiming otherwise about indirect dylib dependencies, as soon as we
  # include one of the libs here that pull in libSystem.B, the linker will
  # pick the implementations of those functions from libc, not from onig.
  # And since we've included the onig headers, which have very slightly
  # different definintions for some of the key data structures, things go
  # quite awry -- this manifests as infinite loops or crashes when calling
  # the PHP split() function.
  #
  # So make sure to link onig first, so its implementations are picked.
  target_link_libraries(${target} ${ONIGURUMA_LIBRARIES})

  if (LIBDL_LIBRARIES)
    target_link_libraries(${target} ${LIBDL_LIBRARIES})
  endif ()

  if (GOOGLE_HEAP_PROFILER_ENABLED OR GOOGLE_CPU_PROFILER_ENABLED)
    target_link_libraries(${target} ${GOOGLE_PROFILER_LIB})
  endif()

  if (JEMALLOC_ENABLED)
    target_link_libraries(${target} ${JEMALLOC_LIB})
  endif()

  if (GOOGLE_HEAP_PROFILER_ENABLED)
    target_link_libraries(${target} ${GOOGLE_TCMALLOC_FULL_LIB})
  elseif (GOOGLE_TCMALLOC_ENABLED)
    target_link_libraries(${target} ${GOOGLE_TCMALLOC_MIN_LIB})
  endif()

  target_link_libraries(${target} ${Boost_LIBRARIES})
  target_link_libraries(${target} ${MYSQL_CLIENT_LIBS})
  if (ENABLE_ASYNC_MYSQL)
    target_link_libraries(${target} squangle)
  endif()
  target_link_libraries(${target} ${PCRE_LIBRARY})
  target_link_libraries(${target} ${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
  target_link_libraries(${target} ${LIBEVENT_LIB})
  target_link_libraries(${target} ${CURL_LIBRARIES})
  target_link_libraries(${target} ${LIBGLOG_LIBRARY})
  if (LIBJSONC_LIBRARY)
    target_link_libraries(${target} ${LIBJSONC_LIBRARY})
  endif()

  if (LibXed_LIBRARY)
    target_link_libraries(${target} ${LibXed_LIBRARY})
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

  if (DOUBLE_CONVERSION_LIBRARY)
    target_link_libraries(${target} ${DOUBLE_CONVERSION_LIBRARY})
  else()
    target_link_libraries(${target} double-conversion)
  endif()

  if (LZ4_LIBRARY)
    target_link_libraries(${target} ${LZ4_LIBRARY})
  else()
    target_link_libraries(${target} lz4)
  endif()

  if (LIBZIP_LIBRARY)
    target_link_libraries(${target} ${LIBZIP_LIBRARY})
  else()
    target_link_libraries(${target} zip_static)
  endif()

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

  if (LIBLLVM_LIBRARY)
    target_link_libraries(${target} ${LIBLLVM_LIBRARY})
  endif()

  if (LINUX)
    target_link_libraries(${target} -Wl,--wrap=pthread_create -Wl,--wrap=pthread_exit -Wl,--wrap=pthread_join)
  endif()

  if (MSVC)
    target_link_libraries(${target} dbghelp.lib dnsapi.lib)
  endif()
endmacro()
