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

# find all the source files
set(RECURSIVE_SOURCE_SUBDIRS runtime/base runtime/eval runtime/ext system/gen util)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})

	auto_sources(files "*.no.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_NOOPT_SOURCES ${files})

	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}") 
	list(APPEND CXX_SOURCES ${files})

	auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND C_SOURCES ${files})

endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

if (EXISTS "${hphp_SOURCE_DIR}/HPHP_EXCLUDE_FILES.txt")
	FILE(READ ${hphp_SOURCE_DIR}/HPHP_EXCLUDE_FILES.txt HPHP_EXCLUDE_FILES)
	STRING(REGEX REPLACE "[\n\r]" ";" HPHP_EXCLUDE_FILES "${HPHP_EXCLUDE_FILES}")
else ()
	SET(HPHP_EXCLUDE_FILES)
endif ()

foreach (t ${HPHP_EXCLUDE_FILES})
	string(SUBSTRING ${t} 0 1 T_FIRST_CHAR)
	if (${T_FIRST_CHAR} STREQUAL "D")

		string(REGEX REPLACE "^[D]" "" SKIP_DIR "${t}")
		file(TO_CMAKE_PATH "${hphp_SOURCE_DIR}/${SKIP_DIR}" SKIP_DIR)
		string(LENGTH "${SKIP_DIR}" SKIP_DIR_LENGTH)
		
		message(STATUS "Let's remove ${SKIP_DIR}")

		foreach (file ${CXX_SOURCES})
			string(LENGTH "${file}" FILE_LENGTH)
			if (${FILE_LENGTH} GREATER ${SKIP_DIR_LENGTH})
				string(SUBSTRING ${file} 0 ${SKIP_DIR_LENGTH} FILE_PATH)
				if (${FILE_PATH} STREQUAL ${SKIP_DIR})
					list(REMOVE_ITEM CXX_SOURCES ${file})
				endif ()
			endif ()
		endforeach(file ${CXX_SOURCES})

	elseif (${T_FIRST_CHAR} STREQUAL "#")

		# doing nothing since this is a comment

	else ()
		list(FIND CXX_SOURCES "${hphp_SOURCE_DIR}/${t}" FOUND_T)
		if (FOUND_T GREATER -1)
			LIST(REMOVE_AT CXX_SOURCES ${FOUND_T})
		endif()
	endif ()
endforeach(t ${HPHP_EXCLUDE_FILES})

#FLEX_TARGET(HphpScanner ${CMAKE_CURRENT_SOURCE_DIR}/runtime/eval/parser/hphp.x ${CMAKE_CURRENT_SOURCE_DIR}/runtime/eval/parser/lex.eval_.cpp COMPILE_FLAGS " -w -i -Peval_")
#BISON_TARGET(HphpParser ${CMAKE_CURRENT_SOURCE_DIR}/runtime/eval/parser/hphp.y ${CMAKE_CURRENT_SOURCE_DIR}/runtime/eval/parser/hphp.tab.cpp  COMPILE_FLAGS " -v -d -p eval_")
#ADD_FLEX_BISON_DEPENDENCY(HphpScanner HphpParser)

# required to remove warning when a file is there twice
#list(REMOVE_ITEM CXX_SOURCES ${FLEX_HphpScanner_OUTPUTS} ${BISON_HphpParser_OUTPUTS})
#list(APPEND CXX_SOURCES ${FLEX_HphpScanner_OUTPUTS} ${BISON_HphpParser_OUTPUTS})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#add_library(sqlite3 STATIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsqlite3/sqlite3.c")

#auto_sources(TIMELIB_SOURCES "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/third_party/timelib")
#add_library(timelib STATIC ${TIMELIB_SOURCES})

add_subdirectory(third_party/xhp/xhp)
add_subdirectory(third_party/libafdt)
add_subdirectory(third_party/libmbfl)
add_subdirectory(third_party/libsqlite3)
add_subdirectory(third_party/timelib)

foreach (CXX_FILE ${CXX_SOURCES})
	if(${CXX_FILE} MATCHES ".no.cpp$")
		SET_SOURCE_FILES_PROPERTIES(
			${CXX_FILE}
			PROPERTIES
			COMPILE_FLAGS -O0
		)
	endif()
endforeach()

ADD_LIBRARY(hphp_runtime_static STATIC ${C_SOURCES} ${CXX_SOURCES})
#ADD_LIBRARY(hphp_runtime SHARED ${C_SOURCES} ${CXX_SOURCES})
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
#SET_TARGET_PROPERTIES(hphp_runtime PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

hphp_link(hphp_runtime_static)

add_subdirectory(compiler)

add_subdirectory(hphp)

add_subdirectory(test)
