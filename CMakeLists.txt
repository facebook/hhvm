CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
PROJECT(hphp C CXX ASM)

IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "HHVM requires a 64bit OS")
ENDIF()

set(HPHP_HOME "$ENV{HPHP_HOME}")
if (NOT HPHP_HOME)
	set(HPHP_HOME "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
message("Using HPHP_HOME == ${HPHP_HOME}")

IF(NOT EXISTS "${HPHP_HOME}/CMake/HPHPSetup.cmake")
	message(FATAL_ERROR "Invalid HPHP_HOME. Set it to the root of your hhvm tree, or run `cmake .` from there.")
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)
include(cotire)

set_property(DIRECTORY
    PROPERTY COTIRE_PREFIX_HEADER_IGNORE_PATH
        "/usr/include/stdlib.h"
        "/usr/include/x86_64-linux-gnu/bits"
        "/usr/include/c-client"
        "/usr/include/jemalloc"
        "/usr/include/onigposix.h"
        "/usr/include/oniguruma.h"
        "/usr/include/png.h"
        "/usr/include/ldap.h"
        "/usr/include/libxml"
        "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")

add_subdirectory(hphp)
