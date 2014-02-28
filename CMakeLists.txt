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

FIND_PATH(LIBC_INCLUDE_PATH stdlib.h)
FIND_PATH(ARCH_INCLUDE_PATH ${CMAKE_LIBRARY_ARCHITECTURE})
SET(ARCH_INCLUDE_PATH "${ARCH_INCLUDE_PATH}/${CMAKE_LIBRARY_ARCHITECTURE}")

set_property(DIRECTORY
    PROPERTY COTIRE_PREFIX_HEADER_IGNORE_PATH
        "${LIBC_INCLUDE_PATH}/stdlib.h"
        "${LIBC_INCLUDE_PATH}/ansidecl.h"
        "${LIBC_INCLUDE_PATH}/bfd.h"
        "${LIBC_INCLUDE_PATH}/libelf.h"
        "${LIBC_INCLUDE_PATH}/elf.h"
        "${LIBC_INCLUDE_PATH}/gelf.h"
        "${ARCH_INCLUDE_PATH}"
        ${CCLIENT_INCLUDE_PATH}
        "${JEMALLOC_INCLUDE_DIR}/jemalloc"
        "${ONIGURUMA_INCLUDE_DIR}/onigposix.h"
        "${ONIGURUMA_INCLUDE_DIR}/oniguruma.h"
        "${LIBPNG_INCLUDE_DIRS}/png.h"
        "${LDAP_INCLUDE_DIR}/ldap.h"
#        "/usr/include/libxml"
        "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")

add_subdirectory(hphp)
