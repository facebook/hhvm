cmake_minimum_required(VERSION 2.6)
project(libafdt CXX C)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCSourceRuns)
include(CheckCSourceCompiles)

MACRO(AFDT_CHECK_INCLUDE include_file)
	CHECK_INCLUDE_FILE("${include_file}" INCLUDE_CHECK_${include_file})
	IF(NOT INCLUDE_CHECK_${include_file})
		message(STATUS "Checking for ${include_file} - failed")
	ENDIF()
ENDMACRO()

AFDT_CHECK_INCLUDE("arpa/inet.h")
AFDT_CHECK_INCLUDE("netinet/in.h")
AFDT_CHECK_INCLUDE("stddef.h")
AFDT_CHECK_INCLUDE("stdint.h")
AFDT_CHECK_INCLUDE("string.h")
AFDT_CHECK_INCLUDE("sys/socket.h")
AFDT_CHECK_INCLUDE("sys/time.h")
AFDT_CHECK_INCLUDE("unistd.h")

CHECK_TYPE_SIZE("size_t" HAVE_SIZE_T)
CHECK_TYPE_SIZE("ssize_t" HAVE_SSIZE_T)
CHECK_TYPE_SIZE("uint32_t" HAVE_UINT32_T)
CHECK_TYPE_SIZE("uint8_t" HAVE_UINT8_T)

CHECK_SYMBOL_EXISTS("AF_LOCAL" "sys/socket.h" HAVE_DECL_AF_LOCAL)
CHECK_SYMBOL_EXISTS("PF_LOCAL" "sys/socket.h" HAVE_DECL_PF_LOCAL)

# need to check malloc

CHECK_FUNCTION_EXISTS("memset" HAVE_MEMSET)
CHECK_FUNCTION_EXISTS("socket" HAVE_SOCKET)
CHECK_FUNCTION_EXISTS("strerror" HAVE_STRERROR)

find_package(LibEvent)

IF(LibEvent_FOUND)
	set(CMAKE_REQUIRED_INCLUDES "${LIBEVENT_INCLUDE_DIR}")
	set(CMAKE_REQUIRED_LIBRARIES "${LIBEVENT_LIB}")
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/CMake/CheckLibeventVersion.c VERSION_CHECK_SOURCE)

	CHECK_SYMBOL_EXISTS("event_get_version_number" "event.h" HAVE_EVENT_GET_VERSION_NUMBER)

	IF(HAVE_EVENT_GET_VERSION_NUMBER)
		SET(CMAKE_REQUIRED_DEFINITIONS
			-DHAVE_EVENT_GET_VERSION_NUMBER
			-DEVENT_VERSION_WANTED=0x2000900)
		CHECK_C_SOURCE_COMPILES("${VERSION_CHECK_SOURCE}" HAVE_LIBEVENT_209)
		set(HAVE_LIBEVENT_145 1)
		set(HAVE_LIBEVENT_143 1)
		message(STATUS "Found libevent 2.0.9+")
	ELSE()
		# < version 2
		SET(CMAKE_REQUIRED_DEFINITIONS
			-DEVENT_VERSION_WANTED=0x1040500)
		CHECK_C_SOURCE_COMPILES("${VERSION_CHECK_SOURCE}" HAVE_LIBEVENT_145)
		IF(NOT HAVE_LIBEVENT_145)
			SET(CMAKE_REQUIRED_DEFINITIONS -DEVENT_VERSION_WANTED=0x1040300)
			CHECK_C_SOURCE_COMPILES("${VERSION_CHECK_SOURCE}" HAVE_LIBEVENT_143)
			IF(NOT LIBEVENT_VERSION_143)
				message(FATAL_ERROR "Couldn't find at least libevent 1.4.3+")
			ENDIF()
		ELSE()
			set(HAVE_LIBEVENT_143 1)
			message(STATUS "Found libevent 1.4.5+")
		ENDIF()
	ENDIF()
	set(ASYNC_SOURCE src/async.c)
	include_directories(${LIBEVENT_INCLUDE_DIR})
ENDIF()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
add_library(afdt STATIC src/lowlevel.c src/strlcpy.c src/sync.c src/util.c ${ASYNC_SOURCE})

IF(LibEvent_FOUND)
	target_link_libraries(afdt ${LIBEVENT_LIB})
ENDIF()
