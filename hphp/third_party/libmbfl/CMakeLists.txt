cmake_minimum_required(VERSION 2.6)
project(libmbfl C CXX)

include(CheckIncludeFiles)
include(CheckFunctionExists)

MACRO(MBFL_CHECK_INCLUDE include_file)
	CHECK_INCLUDE_FILES("${include_file}" INCLUDE_CHECK_${include_file})
	IF(NOT INCLUDE_CHECK_${include_file})
		message(STATUS "Checking for ${include_file} - failed")
	ENDIF()
ENDMACRO()

MBFL_CHECK_INCLUDE("stdlib.h")
MBFL_CHECK_INCLUDE("stddef.h")
MBFL_CHECK_INCLUDE("assert.h")

CHECK_FUNCTION_EXISTS("strcasecmp" HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS("strchr" HAVE_STRCHR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbfl)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/nls)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filters)

add_subdirectory(nls)
add_subdirectory(filters)
add_subdirectory(mbfl)