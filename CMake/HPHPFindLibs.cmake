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
find_package(Boost 1.48.0 COMPONENTS system program_options filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
# Boost 1.49 supports a better flat_multimap, but 1.48 is good enough
if (Boost_VERSION GREATER 104899)
	add_definitions("-DHAVE_BOOST1_49")
endif()


# features.h
FIND_PATH(FEATURES_HEADER features.h)
if (FEATURES_HEADER)
	add_definitions("-DHAVE_FEATURES_H=1")
endif()

# google-glog
find_package(Glog REQUIRED)
include_directories(${LIBGLOG_INCLUDE_DIR})

# inotify checks
find_package(Libinotify)
if (LIBINOTIFY_INCLUDE_DIR)
	include_directories(${LIBINOTIFY_INCLUDE_DIR})
endif()

# iconv checks
find_package(Libiconv REQUIRED)
include_directories(${LIBICONV_INCLUDE_DIR})
if (LIBICONV_CONST)
  message(STATUS "Using const for input to iconv() call")
  add_definitions("-DICONV_CONST=const")
endif()

# mysql checks
find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR})
MYSQL_SOCKET_SEARCH()
if (MYSQL_UNIX_SOCK_ADDR)
  add_definitions(-DPHP_MYSQL_UNIX_SOCK_ADDR="${MYSQL_UNIX_SOCK_ADDR}")
endif()

# libmemcached checks
find_package(Libmemcached REQUIRED)
if (LIBMEMCACHED_VERSION VERSION_LESS "0.39")
  unset(LIBMEMCACHED_INCLUDE_DIR CACHE)
  unset(LIBMEMCACHED_LIBRARY CACHE)
  unset(LIBMEMCACHED_VERSION CACHE)
  message(FATAL_ERROR "libmemcache is too old, found ${LIBMEMCACHED_VERSION} and we need 0.39")
endif ()
include_directories(${LIBMEMCACHED_INCLUDE_DIR})

# pcre checks
find_package(PCRE REQUIRED)
include_directories(${PCRE_INCLUDE_DIRS})

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

find_package(LibUODBC)
if (LIBUODBC_INCLUDE_DIRS)
	include_directories(${LIBUODBC_INCLUDE_DIRS})
	add_definitions("-DHAVE_UODBC")
endif ()

# GD checks
add_definitions(-DPNG_SKIP_SETJMP_CHECK)
find_package(LibVpx)
if (LIBVPX_INCLUDE_DIRS)
	include_directories(${LIBVPX_INCLUDE_DIRS})
	add_definitions("-DHAVE_GD_WEBP")
endif()
find_package(LibJpeg)
if (LIBJPEG_INCLUDE_DIRS)
	include_directories(${LIBJPEG_INCLUDE_DIRS})
	add_definitions("-DHAVE_GD_JPG")
endif()
find_package(LibPng)
if (LIBPNG_INCLUDE_DIRS)
	include_directories(${LIBPNG_INCLUDE_DIRS})
	add_definitions("-DHAVE_GD_PNG")
endif()
find_package(Freetype)
if (FREETYPE_INCLUDE_DIRS)
	include_directories(${FREETYPE_INCLUDE_DIRS})
	add_definitions("-DHAVE_LIBFREETYPE -DHAVE_GD_FREETYPE -DENABLE_GD_TTF")
endif()

# libXed
find_package(LibXed)
if (LibXed_INCLUDE_DIR AND LibXed_LIBRARY)
	include_directories(${LibXed_INCLUDE_DIR})
	add_definitions("-DHAVE_LIBXED")
endif()

# CURL checks
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

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

find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# SQLite3 + timelib are bundled in HPHP sources
include_directories("${HPHP_HOME}/hphp/third_party/libsqlite3")
include_directories("${HPHP_HOME}/hphp/third_party/timelib")
include_directories("${HPHP_HOME}/hphp/third_party/libafdt/src")
include_directories("${HPHP_HOME}/hphp/third_party/libmbfl")
include_directories("${HPHP_HOME}/hphp/third_party/libmbfl/mbfl")
include_directories("${HPHP_HOME}/hphp/third_party/libmbfl/filter")
include_directories("${HPHP_HOME}/hphp/third_party/lz4")
include_directories("${HPHP_HOME}/hphp/third_party/double-conversion/src")
include_directories("${HPHP_HOME}/hphp/third_party/folly")
include_directories("${HPHP_HOME}/hphp/third_party/libzip")

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
endif (ICU_FOUND)

# jemalloc/tmalloc and profiler
if (USE_GOOGLE_HEAP_PROFILER OR USE_GOOGLE_CPU_PROFILER)
	FIND_LIBRARY(GOOGLE_PROFILER_LIB profiler)
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
int main(void) {
#if !defined(JEMALLOC_VERSION_MAJOR) || (JEMALLOC_VERSION_MAJOR < 3)
# error \"jemalloc version >= 3.0.0 required\"
#endif
	return 0;
}" JEMALLOC_VERSION_3)
		set (CMAKE_REQUIRED_INCLUDES)

		if (JEMALLOC_VERSION_3)
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

# mcrypt libs
find_package(Mcrypt REQUIRED)
include_directories(${Mcrypt_INCLUDE_DIR})

# OpenSSL libs
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# ZLIB
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIR})
add_definitions(${BZIP2_DEFINITIONS})

# oniguruma
find_package(ONIGURUMA REQUIRED)
include_directories(${ONIGURUMA_INCLUDE_DIRS})

# LDAP
find_package(Ldap REQUIRED)
include_directories(${LDAP_INCLUDE_DIR})

# ncurses, readline and history
#set(CURSES_NEED_NCURSES true)
find_package(Ncurses REQUIRED)
include_directories(${NCURSES_INCLUDE_PATH})

# libpthreads
find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})

# Either Readline or Editline (for hphpd)
find_package(Readline)
find_package(Editline)
if (READLINE_INCLUDE_DIR)
	include_directories(${READLINE_INCLUDE_DIR})
elseif (EDITLINE_INCLUDE_DIRS)
	add_definitions("-DUSE_EDITLINE")
	include_directories(${EDITLINE_INCLUDE_DIRS})
else()
	message(FATAL_ERROR "Could not find Readline or Editline")
endif()

find_package(CClient REQUIRED)
include_directories(${CCLIENT_INCLUDE_PATH})

find_package(LibDwarf REQUIRED)
include_directories(${LIBDWARF_INCLUDE_DIRS})

find_package(LibElf REQUIRED)
include_directories(${LIBELF_INCLUDE_DIRS})
if (ELF_GETSHDRSTRNDX)
        add_definitions("-DHAVE_ELF_GETSHDRSTRNDX")
endif()

CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/utf8.h" U8T_DECOMPOSE RECENT_CCLIENT)
if (NOT RECENT_CCLIENT)
	unset(RECENT_CCLIENT CACHE)
	message(FATAL_ERROR "Your version of c-client is too old, you need 2007")
endif()


if (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.c")
	CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.c" auth_gss CCLIENT_HAS_GSS)
elseif (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.h")
    CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.h" auth_gss CCLIENT_HAS_GSS)
endif()

find_package(Libpam)
if (PAM_INCLUDE_PATH)
	include_directories(${PAM_INCLUDE_PATH})
endif()

if (NOT CCLIENT_HAS_GSS)
	add_definitions(-DSKIP_IMAP_GSS=1)
endif()

if (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.c")
	CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.c" ssl_onceonlyinit CCLIENT_HAS_SSL)
elseif (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.h")
	CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.h" ssl_onceonlyinit CCLIENT_HAS_SSL)
endif()

if (NOT CCLIENT_HAS_SSL)
	add_definitions(-DSKIP_IMAP_SSL=1)
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

FIND_LIBRARY (BFD_LIB bfd)
FIND_LIBRARY (BINUTIL_LIB iberty)

if (NOT BFD_LIB)
	message(FATAL_ERROR "You need to install binutils")
endif()

if (NOT BINUTIL_LIB)
	message(FATAL_ERROR "You need to install binutils")
endif()

if (FREEBSD)
	FIND_LIBRARY (EXECINFO_LIB execinfo)
	if (NOT EXECINFO_LIB)
		message(FATAL_ERROR "You need to install libexecinfo")
	endif()
endif()

if (APPLE)
	find_library(LIBINTL_LIBRARIES NAMES intl libintl)
	if (LIBINTL_INCLUDE_DIR)
		include_directories(${LIBINTL_INCLUDE_DIR})
	endif()
	find_library(KERBEROS_LIB NAMES gssapi_krb5)
endif()

#find_package(BISON REQUIRED)
#find_package(FLEX REQUIRED)

#if (${FLEX_VERSION} VERSION_LESS 2.5.33)
#	message(FATAL_ERROR "Flex is too old, found ${FLEX_VERSION} and we need 2.5.33")
#endif()

include_directories(${HPHP_HOME}/hphp)
include_directories(${HPHP_HOME}/hphp/system/gen)

macro(hphp_link target)
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
	target_link_libraries(${target} ${PCRE_LIBRARY})
	target_link_libraries(${target} ${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
	target_link_libraries(${target} ${LIBEVENT_LIB})
	target_link_libraries(${target} ${CURL_LIBRARIES})
	target_link_libraries(${target} ${LIBGLOG_LIBRARY})

if (LibXed_LIBRARY)
	target_link_libraries(${target} ${LibXed_LIBRARY})
endif()

if (LIBINOTIFY_LIBRARY)
	target_link_libraries(${target} ${LIBINOTIFY_LIBRARY})
endif()

if (LIBICONV_LIBRARY)
	target_link_libraries(${target} ${LIBICONV_LIBRARY})
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
endif()

	target_link_libraries(${target} ${BFD_LIB})
	target_link_libraries(${target} ${BINUTIL_LIB})
if (${LIBPTHREAD_LIBRARIES})
	target_link_libraries(${target} ${LIBPTHREAD_LIBRARIES})
endif()
	target_link_libraries(${target} ${TBB_LIBRARIES})
	target_link_libraries(${target} ${OPENSSL_LIBRARIES})
	target_link_libraries(${target} ${ZLIB_LIBRARIES})
	target_link_libraries(${target} ${BZIP2_LIBRARIES})

	target_link_libraries(${target} ${LIBXML2_LIBRARIES})
	target_link_libraries(${target} ${EXPAT_LIBRARY})
	target_link_libraries(${target} ${ONIGURUMA_LIBRARIES})
	target_link_libraries(${target} ${Mcrypt_LIB})
	target_link_libraries(${target} ${GD_LIBRARY})
if (FREETYPE_LIBRARIES)
	target_link_libraries(${target} ${FREETYPE_LIBRARIES})
endif()
if (LIBJPEG_LIBRARIES)
	target_link_libraries(${target} ${LIBJPEG_LIBRARIES})
endif()
if (LIBPNG_LIBRARIES)
	target_link_libraries(${target} ${LIBPNG_LIBRARIES})
endif()
if (LIBVPX_LIBRARIES)
	target_link_libraries(${target} ${LIBVPX_LIBRARIES})
endif()
if (LIBUODBC_LIBRARIES)
	target_link_libraries(${target} ${LIBUODBC_LIBRARIES})
endif()
	target_link_libraries(${target} ${LDAP_LIBRARIES})
	target_link_libraries(${target} ${LBER_LIBRARIES})

	target_link_libraries(${target} ${LIBMEMCACHED_LIBRARY})

	target_link_libraries(${target} ${CRYPT_LIB})
	if (LINUX OR FREEBSD)
		target_link_libraries(${target} ${RT_LIB})
	endif()

	target_link_libraries(${target} timelib)
	target_link_libraries(${target} sqlite3)
	target_link_libraries(${target} lz4)
	target_link_libraries(${target} double-conversion)
	target_link_libraries(${target} folly)
	target_link_libraries(${target} zip_static)

	target_link_libraries(${target} afdt)
	target_link_libraries(${target} mbfl)

	if (READLINE_LIBRARY)
		target_link_libraries(${target} ${READLINE_LIBRARY})
	elseif (EDITLINE_LIBRARIES)
		target_link_libraries(${target} ${EDITLINE_LIBRARIES})
	endif()

	target_link_libraries(${target} ${NCURSES_LIBRARY})
	target_link_libraries(${target} ${CCLIENT_LIBRARY})

	if (PAM_LIBRARY)
		target_link_libraries(${target} ${PAM_LIBRARY})
	endif()

        target_link_libraries(${target} ${LIBDWARF_LIBRARIES})
        target_link_libraries(${target} ${LIBELF_LIBRARIES})

endmacro()
