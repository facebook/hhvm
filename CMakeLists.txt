CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4 FATAL_ERROR)
PROJECT(hphp C CXX)

IF("$ENV{HPHP_HOME}" STREQUAL "")
	message(FATAL_ERROR "You should set the HPHP_HOME environmental")
ENDIF()

file(TO_CMAKE_PATH "$ENV{HPHP_HOME}" HPHP_HOME)

IF(NOT IS_DIRECTORY ${HPHP_HOME})
	message(FATAL_ERROR "The value of HPHP_HOME does not exist")
ENDIF()

IF(NOT EXISTS "${HPHP_HOME}/LICENSE.PHP")
	message(FATAL_ERROR "The value of HPHP_HOME in incorrect")
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32-bit support is currently unsupported, check back with a later version of HipHop")
ENDIF()

if ("$ENV{USE_HHVM}" STREQUAL "1")
	message("Building for HHVM")
endif()
if ("$ENV{USE_HPHPC}" STREQUAL "1")
	message("Building for HPHPc")
endif()
