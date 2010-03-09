CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4 FATAL_ERROR)
PROJECT(hphp C CXX)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	IF("$ENV{HPHP_FORCE_32BIT}" STREQUAL "")
		message(FATAL_ERROR "HipHop currently only supports 64-bit machines")
	ENDIF()
ENDIF()

IF("$ENV{HPHP_HOME}" STREQUAL "")
	message(FATAL_ERROR "You should set the HPHP_HOME environmental")
ENDIF()

file(TO_CMAKE_PATH "$ENV{HPHP_HOME}" HPHP_HOME)

SET(CMAKE_MODULE_PATH "${HPHP_HOME}/CMake" ${CMAKE_MODULE_PATH})

include("${HPHP_HOME}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)

IF(NOT EXISTS "src/third_party/libmbfl/mbfl/mbfilter.h")
	message(FATAL_ERROR "Please setup the git submodule to pull dependencies")
ENDIF()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

