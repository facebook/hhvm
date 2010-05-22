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

# pcre checks
find_package(PCRE REQUIRED)

# libevent checks
find_package(LibEvent REQUIRED)
include_directories(${LIBEVENT_INCLUDE_DIR})

set(CMAKE_REQUIRED_LIBRARIES "${LIBEVENT_LIB}")
CHECK_FUNCTION_EXISTS("evhttp_bind_socket_with_fd" HAVE_CUSTOM_LIBEVENT)
if (NOT HAVE_CUSTOM_LIBEVENT)
	message(SEND_ERROR "Custom libevent is required with HipHop patches")
	unset(HAVE_CUSTOM_LIBEVENT CACHE)
	unset(LIBEVENT_INCLUDE_DIR CACHE)
	unset(LIBEVENT_LIB CACHE)
	unset(LibEvent_FOUND CACHE)
endif ()
set(CMAKE_REQUIRED_LIBRARIES)

# GD checks
find_package(GD REQUIRED)

option(WANT_FB_LIBMCC "want FB Memcache" 0)
option(WANT_FB_LIBFML "want FB libfbml" 0)

if (WANT_FB_LIBMCC)
	add_definitions(-DHPHP_WITH_LIBMCC)
	message(FATAL_ERROR Need to add libmcc and libch for linking)
else ()
	# nothing for now
endif()

if (WANT_FB_LIBFBML)
	message(FATAL_ERROR Need to find the mozilla stuff for linking)
else ()
	# nothing for now
endif()

# CURL checks
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
CHECK_FUNCTION_EXISTS("curl_multi_select" HAVE_CUSTOM_CURL)
if (NOT HAVE_CUSTOM_CURL)
        message(SEND_ERROR "Custom libcurl is required with HipHop patches ${HAVE_CUSTOM_CURL}")
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
include_directories("${HPHP_HOME}/src/third_party/xhp/xhp")
include_directories("${HPHP_HOME}/src/third_party/libafdt/src")
include_directories("${HPHP_HOME}/src/third_party/libmbfl")
include_directories("${HPHP_HOME}/src/third_party/libmbfl/mbfl")
include_directories("${HPHP_HOME}/src/third_party/libmbfl/filter")

# ICU

find_package(ICU REQUIRED)
if (ICU_FOUND)
	if (ICU_VERSION VERSION_LESS "4.2")
		message(SEND_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
		unset(ICU_FOUND CACHE)
		unset(ICU_INCLUDE_DIRS CACHE)
		unset(ICU_LIBRARIES CACHE)
	endif (ICU_VERSION VERSION_LESS "4.2")
	include_directories(${ICU_INCLUDE_DIRS})
endif (ICU_FOUND)

# (google heap OR cpu profiler) AND libunwind 
FIND_LIBRARY(UNWIND_LIB unwind)

# Google tmalloc
FIND_LIBRARY(GOOGLE_TCMALLOC_LIB tcmalloc)
FIND_LIBRARY(GOOGLE_TCMALLOC_MINIMAL_LIB tcmalloc_minimal)
FIND_LIBRARY(GOOGLE_PROFILER_LIB profiler)

# tbb libs
find_package(TBB REQUIRED)
if (${TBB_INTERFACE_VERSION} LESS 3016)
	message(SEND_ERROR "TBB is too old, please install a newer version")
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

#oniguruma
find_package(ONIGURUMA REQUIRED)
include_directories(${ONIGURUMA_INCLUDE_DIRS})

#LINK_LIBS = -lpthread $(BFD_LIBS) -lrt -lstdc++ -lresolv
#-lcrypto -lcrypt


FIND_LIBRARY (CAP_LIB cap)

if (CAP_LIB STREQUAL "CAP_LIB-NOTFOUND")
  message(FATAL_ERROR "You need to install libcap")
endif()

# potentially make it look in a different directory for the google tools
FIND_LIBRARY (BFD_LIB bfd)
FIND_LIBRARY (BINUTIL_LIB iberty)
FIND_LIBRARY (DL_LIB dl)

if (BFD_LIB STREQUAL "BFD_LIB-NOTFOUND")
	message(FATAL_ERROR "You need to install binutils")
endif()

if (BINUTIL_LIB STREQUAL "BINUTIL_LIB-NOTFOUND")
	message(FATAL_ERROR "You need to install binutils")
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

	target_link_libraries(${target} ${CAP_LIB})
	target_link_libraries(${target} ${BFD_LIB})
	target_link_libraries(${target} ${BINUTIL_LIB})
	target_link_libraries(${target} ${DL_LIB})
	target_link_libraries(${target} pthread)
	target_link_libraries(${target} ${TBB_LIBRARIES})
	target_link_libraries(${target} ${OPENSSL_LIBRARIES})
	target_link_libraries(${target} ${ZLIB_LIBRARIES})

	target_link_libraries(${target} ${LIBXML2_LIBRARIES})
	target_link_libraries(${target} ${EXPAT_LIBRARY})
	target_link_libraries(${target} ${ONIGURUMA_LIBRARIES})
	target_link_libraries(${target} ${Mcrypt_LIB})
	target_link_libraries(${target} ${GD_LIBRARY})
	
	target_link_libraries(${target} timelib)
	target_link_libraries(${target} sqlite3)
	target_link_libraries(${target} xhp)
	target_link_libraries(${target} afdt)
	target_link_libraries(${target} mbfl)
endmacro()
