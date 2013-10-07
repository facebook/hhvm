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

set(RECURSIVE_SOURCE_SUBDIRS runtime/base runtime/debugger runtime/eval runtime/ext runtime/server runtime/vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})

	auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND C_SOURCES ${files})

	auto_sources(files "*.S" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND ASM_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

if(NOT LINUX)
	add_definitions(-DNO_HARDWARE_COUNTERS)
	list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/runtime/vm/debug/elfwriter.cpp)
endif()

# Not working with off-the-shelf libevent
list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/server/server-name-indication.cpp")

# Waiting on OSS fix :(
list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/runtime/vm/runtime-type-profiler.cpp)

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

ADD_LIBRARY(hphp_runtime_static STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")

hphp_link(hphp_runtime_static)
add_dependencies(hphp_runtime_static hphp_parser)

add_subdirectory("tools/bootstrap")

add_subdirectory(vixl)
add_subdirectory(neo)
add_subdirectory(parser)
add_subdirectory(zend)
add_subdirectory(util)

add_subdirectory(compiler)
add_subdirectory(runtime/ext_hhvm)
add_subdirectory(hhvm)
add_subdirectory(system)

if (NOT "$ENV{HPHP_NOTEST}" STREQUAL "1")
	add_subdirectory(test)
endif ()
