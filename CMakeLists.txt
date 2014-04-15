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

if (NOT EXISTS "${HPHP_HOME}/third-party/CMakeLists.txt")
	message(FATAL_ERROR "third-party/CMakeLists.txt missing. Try updating your submodule with:
rm -r third-party
git submodule update --init --recursive
")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)
include(cotire)

FIND_PATH(LIBC_INCLUDE_PATH stdlib.h)
FIND_PATH(ARCH_INCLUDE_PATH ${CMAKE_LIBRARY_ARCHITECTURE})
SET(ARCH_INCLUDE_PATH "${ARCH_INCLUDE_PATH}/${CMAKE_LIBRARY_ARCHITECTURE}")

add_subdirectory(third-party)
add_subdirectory(hphp)
