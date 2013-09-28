cmake_minimum_required(VERSION 2.6)
project(timelib C)

include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(TestBigEndian)

MACRO(TIME_CHECK_INCLUDE include_file)
	string(REGEX REPLACE "[./]"
	         "_" _include_var ${include_file})
	string(TOUPPER "HAVE_${_include_var}" _include_var)

	CHECK_INCLUDE_FILES("${include_file}" ${_include_var})
	IF(NOT ${_include_var})
		message(STATUS "Checking for ${include_file} - failed")
	ENDIF()
ENDMACRO()

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

TIME_CHECK_INCLUDE("sys/types.h")
TIME_CHECK_INCLUDE("inttypes.h")
TIME_CHECK_INCLUDE("stdint.h")
TIME_CHECK_INCLUDE("string.h")
TIME_CHECK_INCLUDE("stdlib.h")
TIME_CHECK_INCLUDE("locale.h")
TIME_CHECK_INCLUDE("limits.h")

CHECK_TYPE_SIZE("int32_t" HAVE_INT32_T)
CHECK_TYPE_SIZE("uint32_t" HAVE_UINT32_T)

CHECK_FUNCTION_EXISTS("strtoll" HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS("atoll" HAVE_ATOLL)
CHECK_FUNCTION_EXISTS("strftime" HAVE_STRFTIME)
CHECK_FUNCTION_EXISTS("setlocale" HAVE_SETLOCALE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/timelib_config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/timelib_config.h)

add_definitions(-DNDEBUG)
set(TIMELIB_SOURCES astro.c dow.c interval.c parse_date.c parse_iso_intervals.c parse_tz.c timelib.c tm2unixtime.c unixtime2tm.c)

add_library(timelib STATIC ${TIMELIB_SOURCES})

