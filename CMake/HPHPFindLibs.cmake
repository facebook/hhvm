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

# boost checks

find_package(Boost 1.37.0 COMPONENTS system;program_options;filesystem REQUIRED)
if (BOOST_VERSION EQUAL 104200)
	# Boost bug #3942 prevents us using 1.42
	message(FATAL_ERROR "Boost 1.42 is not compatible with HipHop")
endif()

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# mysql checks
find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR})

# libmemcached checks
find_package(Libmemcached REQUIRED)
if (LIBMEMCACHED_VERSION VERSION_LESS "0.39")
  message(FATAL_ERROR "libmemcache is too old, found ${LIBMEMCACHED_VERSION} and we need 0.39")
  unset(LIBMEMCACHED_INCLUDE_DIR)
  unset(LIBMEMCACHED_LIBRARIES)
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
if (NOT HAVE_CUSTOM_LIBEVENT)
	message(FATAL_ERROR "Custom libevent is required with HipHop patches")
	unset(HAVE_CUSTOM_LIBEVENT CACHE)
	unset(LIBEVENT_INCLUDE_DIR CACHE)
	unset(LIBEVENT_LIB CACHE)
	unset(LibEvent_FOUND CACHE)
endif ()
set(CMAKE_REQUIRED_LIBRARIES)

# GD checks
find_package(GD REQUIRED)

option(WANT_FB_LIBMCC "want FB Memcache" 0)

if (WANT_FB_LIBMCC)
	add_definitions(-DHPHP_WITH_LIBMCC)
	message(FATAL_ERROR Need to add libmcc and libch for linking)
else ()
	# nothing for now
endif()

# CURL checks
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
CHECK_FUNCTION_EXISTS("curl_multi_select" HAVE_CUSTOM_CURL)
if (NOT HAVE_CUSTOM_CURL)
        message(FATAL_ERROR "Custom libcurl is required with HipHop patches ${HAVE_CUSTOM_CURL}")
		unset(HAVE_CUSTOM_CURL CACHE)
		unset(CURL_INCLUDE_DIR CACHE)
		unset(CURL_LIBRARIES CACHE)
		unset(CURL_FOUND CACHE)
endif ()
set(CMAKE_REQUIRED_LIBRARIES)

# LibXML2 checks
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
add_definitions(${LIBXML2_DEFINITIONS})

find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# SQLite3 + timelib are bundled in HPHP sources
include_directories("${HPHP_HOME}/src/third_party/libsqlite3")
include_directories("${HPHP_HOME}/src/third_party/timelib")
include_directories("${HPHP_HOME}/src/third_party/libafdt/src")
include_directories("${HPHP_HOME}/src/third_party/libmbfl")
include_directories("${HPHP_HOME}/src/third_party/libmbfl/mbfl")
include_directories("${HPHP_HOME}/src/third_party/libmbfl/filter")

FIND_LIBRARY(XHP_LIB xhp)
FIND_PATH(XHP_INCLUDE_DIR xhp_preprocess.hpp)

if (XHP_LIB AND XHP_INCLUDE_DIR)
	include_directories(${XHP_INCLUDE_DIR})
	set(SKIP_BUNDLED_XHP ON)
else()
	include_directories("${HPHP_HOME}/src/third_party/xhp/xhp")
endif()

# ICU
find_package(ICU REQUIRED)
if (ICU_FOUND)
	if (ICU_VERSION VERSION_LESS "4.2")
		message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
		unset(ICU_FOUND CACHE)
		unset(ICU_INCLUDE_DIRS CACHE)
		unset(ICU_LIBRARIES CACHE)
	endif (ICU_VERSION VERSION_LESS "4.2")
	include_directories(${ICU_INCLUDE_DIRS})
endif (ICU_FOUND)

# (google heap OR cpu profiler) AND libunwind 
FIND_LIBRARY(UNWIND_LIB unwind)

# Google tmalloc
option(USE_TCMALLOC "Use tcmalloc" ON)

if (USE_TCMALLOC)
	FIND_LIBRARY(GOOGLE_TCMALLOC_LIB tcmalloc_minimal)

	set(HAVE_TCMALLOC 0)
	if (NOT GOOGLE_TCMALLOC_LIB)
		message(STATUS "Skipping TCmalloc")
	else()
		message(STATUS "Found tcmalloc: ${GOOGLE_TCMALLOC_LIB}")
		add_definitions(-DGOOGLE_TCMALLOC=1)
		set(HAVE_TCMALLOC 1)
	endif()
endif()

# tbb libs
find_package(TBB REQUIRED)
if (${TBB_INTERFACE_VERSION} LESS 3016)
	message(FATAL_ERROR "TBB is too old, please install a newer version")
	unset(TBB_FOUND CACHE)
	unset(TBB_INCLUDE_DIRS CACHE)
	unset(TBB_LIBRARIES CACHE)
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

# oniguruma
find_package(ONIGURUMA REQUIRED)
include_directories(${ONIGURUMA_INCLUDE_DIRS})

# LDAP
find_package(Ldap REQUIRED)
include_directories(${LDAP_INCLUDE_DIR})

# ncuses, readline and history
#set(CURSES_NEED_NCURSES true)
find_package(Ncurses REQUIRED)
include_directories(${NCURSES_INCLUDE_PATH})

find_package(Readline REQUIRED)
include_directories(${READLINE_INCLUDE_DIR})


if (LINUX OR FREEBSD)
	FIND_LIBRARY (CRYPT_LIB crypt)
	FIND_LIBRARY (RT_LIB rt)
elseif (APPLE)
	FIND_LIBRARY (CRYPT_LIB crypto)
	FIND_LIBRARY (ICONV_LIB iconv)
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
endif()

#find_package(BISON REQUIRED)
#find_package(FLEX REQUIRED)

#if (${FLEX_VERSION} VERSION_LESS 2.5.33)
#	message(FATAL_ERROR "Flex is too old, found ${FLEX_VERSION} and we need 2.5.33")
#endif()

include_directories(${HPHP_HOME}/src)
include_directories(${HPHP_HOME}/src/system/gen)

macro(hphp_link target)
	target_link_libraries(${target} ${Boost_LIBRARIES})
	target_link_libraries(${target} ${MYSQL_CLIENT_LIBS})
	target_link_libraries(${target} ${PCRE_LIBRARY})
	target_link_libraries(${target} ${ICU_LIBRARIES} ${ICU_I18N_LIBRARIES})
	target_link_libraries(${target} ${LIBEVENT_LIB})
	target_link_libraries(${target} ${CURL_LIBRARIES})

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

	target_link_libraries(${target} ${BFD_LIB})
	target_link_libraries(${target} ${BINUTIL_LIB})
	target_link_libraries(${target} pthread)
	target_link_libraries(${target} ${TBB_LIBRARIES})
	target_link_libraries(${target} ${OPENSSL_LIBRARIES})
	target_link_libraries(${target} ${ZLIB_LIBRARIES})

	target_link_libraries(${target} ${LIBXML2_LIBRARIES})
	target_link_libraries(${target} ${EXPAT_LIBRARY})
	target_link_libraries(${target} ${ONIGURUMA_LIBRARIES})
	target_link_libraries(${target} ${Mcrypt_LIB})
	target_link_libraries(${target} ${GD_LIBRARY})

	target_link_libraries(${target} ${LDAP_LIBRARIES})
	target_link_libraries(${target} ${LBER_LIBRARIES})

	target_link_libraries(${target} ${LIBMEMCACHED_LIBRARIES})

	if (LINUX OR FREEBSD)
		target_link_libraries(${target} ${CRYPT_LIB})
		target_link_libraries(${target} ${RT_LIB})
	elseif (APPLE)
		target_link_libraries(${target} ${CRYPTO_LIB})
		target_link_libraries(${target} ${ICONV_LIB})	
	endif()

	if (USE_TCMALLOC AND HAVE_TCMALLOC)
		target_link_libraries(${target} ${GOOGLE_TCMALLOC_LIB})
	endif()

	target_link_libraries(${target} timelib)
	target_link_libraries(${target} sqlite3)

	if (SKIP_BUNDLED_XHP)
		target_link_libraries(${target} ${XHP_LIB})
	else()
		target_link_libraries(${target} xhp)
	endif()

	target_link_libraries(${target} afdt)
	target_link_libraries(${target} mbfl)

	target_link_libraries(${target} ${READLINE_LIBRARY})
	target_link_libraries(${target} ${NCURSES_LIBRARY})

endmacro()
