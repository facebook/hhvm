cmake_minimum_required(VERSION 2.6)
project(sqlite3 C)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

set(SQLITE_DEFINITIONS -DSQLITE_ENABLE_FTS3;-DSQLITE_ENABLE_RTREE -DNDEBUG)

MACRO(CHECK_INCLUDE_FILE_DEFINE_MACRO include_file)
	string(REGEX REPLACE "[./]"
		"_" _include_var ${include_file})
	string(TOUPPER "HAVE_${_include_var}" _include_var)

	CHECK_INCLUDE_FILES("${include_file}" ${_include_var})
	IF(NOT ${_include_var})
		message(STATUS "Checking for ${include_file} - failed")
	ENDIF()

	IF(${_include_var})
		list (APPEND SQLITE_DEFINITIONS -D${_include_var}=1)
	endif ()
ENDMACRO()

MACRO(CHECK_FUNCTION_EXISTS_DEFINE_MACRO function_name)
	string(TOUPPER "HAVE_${function_name}" _function_name)

	CHECK_FUNCTION_EXISTS("${function_name}" ${_function_name})

	IF(${_function_name})
		list (APPEND SQLITE_DEFINITIONS -D${_function_name}=1)
	endif ()
ENDMACRO()

CHECK_INCLUDE_FILE_DEFINE_MACRO("sys/types.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("sys/stat.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("stdlib.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("string.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("memory.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("strings.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("inttypes.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("stdint.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("unistd.h")
CHECK_INCLUDE_FILE_DEFINE_MACRO("dlfcn.h")

CHECK_FUNCTION_EXISTS_DEFINE_MACRO("fdatasync")
CHECK_FUNCTION_EXISTS_DEFINE_MACRO("usleep")
CHECK_FUNCTION_EXISTS_DEFINE_MACRO("fullfsync")
CHECK_FUNCTION_EXISTS_DEFINE_MACRO("localtime_r")
CHECK_FUNCTION_EXISTS_DEFINE_MACRO("gmtime_r")

CHECK_LIBRARY_EXISTS (pthread pthread_create "" HAVE_PTHREAD)
IF(${HAVE_PTHREAD})
	list (APPEND SQLITE_DEFINITIONS -DSQLITE_THREADSAFE=1)
endif ()

add_definitions(${SQLITE_DEFINITIONS})

mark_as_advanced(SQLITE_DEFINITIONS)

add_library(sqlite3 STATIC sqlite3.c)
