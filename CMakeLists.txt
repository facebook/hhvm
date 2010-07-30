CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4 FATAL_ERROR)
PROJECT(hphp C CXX)

IF("$ENV{HPHP_HOME}" STREQUAL "")
	message(FATAL_ERROR "You should set the HPHP_HOME environmental")
ENDIF()

file(TO_CMAKE_PATH "$ENV{HPHP_HOME}" HPHP_HOME)

IF(NOT IS_DIRECTORY ${HPHP_HOME})
	message(FATAL_ERROR "The value of HPHP_HOME does not exist")
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "32-bit support is experimental, things may be broke")
ENDIF()
