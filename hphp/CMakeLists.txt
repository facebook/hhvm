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

include(HPHPSetup)

# HHVM Build
SET(USE_HHVM TRUE)
SET(ENV{HHVM} 1)
ADD_DEFINITIONS("-DHHVM -DHHVM_BINARY=1 -DHHVM_PATH=\\\"${HPHP_HOME}/hphp/hhvm/hhvm\\\"")

set(RECURSIVE_SOURCE_SUBDIRS runtime/base runtime/eval runtime/ext runtime/vm system util)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})

	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})

	auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND C_SOURCES ${files})

endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

# Disable hardware counters off of Linux
if(NOT LINUX)
	add_definitions(-DNO_HARDWARE_COUNTERS)
	list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/runtime/base/hardware_counter.cpp)
endif()

# remove ext_hhvm, and anything in a test folder
foreach (file ${CXX_SOURCES})
       	if (${file} MATCHES "ext_hhvm")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
	if (${file} MATCHES "/test/")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
endforeach(file ${CXX_SOURCES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(third_party/libafdt)
add_subdirectory(third_party/libmbfl)
add_subdirectory(third_party/libsqlite3)
add_subdirectory(third_party/timelib)
add_subdirectory(third_party/lz4)
add_subdirectory(third_party/double-conversion)
add_subdirectory(third_party/folly)

foreach (CXX_FILE ${CXX_SOURCES})
	if(${CXX_FILE} MATCHES ".no.cpp$")
		SET_SOURCE_FILES_PROPERTIES(
			${CXX_FILE}
			PROPERTIES
			COMPILE_FLAGS -O0
		)
	endif()
endforeach()

add_custom_command(
  OUTPUT hphp_repo_schema.h hphp_build_info.cpp
  COMMAND hphp/util/generate_buildinfo.sh
  DEPENDS ${CXX_SOURCES} ${C_SOURCES}
  WORKING_DIRECTORY ${HPHP_HOME}
  COMMENT "Generating Repo Schema ID and Compiler ID"
  VERBATIM)

ADD_LIBRARY(hphp_runtime_static STATIC
            hphp_repo_schema.h hphp_build_info.cpp
            ${CXX_SOURCES} ${C_SOURCES})
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")

hphp_link(hphp_runtime_static)

add_subdirectory(compiler)
add_subdirectory(runtime/ext_hhvm)
add_subdirectory(hhvm)

if (NOT "$ENV{HPHP_NOTEST}" STREQUAL "1")
	add_subdirectory(test)
endif ()
