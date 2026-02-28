# Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA 


# This file exports macros that emulate some functionality found  in GNU libtool
# on Unix systems. One such feature is convenience libraries. In this context,
# convenience library is a static library that can be linked to shared library
# On systems that force position-independent code, linking into shared library
# normally requires compilation with a special flag (often -fPIC).
# Some systems, like Windows or OSX do not need special compilation (Windows
# never uses PIC and OSX always uses it).
#
# The intention behind convenience libraries is simplify the build and to
# reduce excessive recompiles.

# Except for convenience libraries, this file provides macros to merge static
# libraries (we need it for mysqlclient) and to create shared library out of
# convenience libraries(again, for mysqlclient)


GET_FILENAME_COMPONENT(MYSQL_CMAKE_SCRIPT_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

INCLUDE(${MYSQL_CMAKE_SCRIPT_DIR}/cmake_parse_arguments.cmake)
# CREATE_EXPORT_FILE (VAR target api_functions)
# Internal macro, used on Windows to export API functions as dllexport.
# Returns a list of extra files that should be linked into the library
# (in the variable pointed to by VAR).
MACRO(CREATE_EXPORT_FILE VAR TARGET API_FUNCTIONS)
  SET(DUMMY ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_dummy.c)
  CONFIGURE_FILE_CONTENT("" ${DUMMY})
  IF(WIN32)
    SET(EXPORTS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_exports.def)
    SET(CONTENT "EXPORTS\n")
    FOREACH(FUNC ${API_FUNCTIONS})
      SET(CONTENT "${CONTENT} ${FUNC}\n")
    ENDFOREACH()
    CONFIGURE_FILE_CONTENT(${CONTENT} ${EXPORTS})
    SET(${VAR} ${DUMMY} ${EXPORTS})
  ELSE()
    SET(${VAR} ${DUMMY})
  ENDIF()
ENDMACRO()


# ADD_CONVENIENCE_LIBRARY(name source1...sourceN)
# Create an OBJECT library ${name}_objlib containing all object files.
# Create a STATIC library ${name} which can be used for linking.
#
# We use the OBJECT libraries for merging in MERGE_CONVENIENCE_LIBRARIES.
# For APPLE, we create a STATIC library only,
# see comments in MERGE_CONVENIENCE_LIBRARIES for Xcode
#
# Optional arguments:
# [COMPILE_DEFINITIONS ...] for TARGET_COMPILE_DEFINITIONS
# [COMPILE_OPTIONS ...]     for TARGET_COMPILE_OPTIONS
# [DEPENDENCIES ...]        for ADD_DEPENDENCIES
# [INCLUDE_DIRECTORIES ...] for TARGET_INCLUDE_DIRECTORIES
# [LINK_LIBRARIES ...]      for TARGET_LINK_LIBRARIES
MACRO(ADD_CONVENIENCE_LIBRARY)
  MYSQL_PARSE_ARGUMENTS(ARG
    "COMPILE_DEFINITIONS;COMPILE_OPTIONS;DEPENDENCIES;INCLUDE_DIRECTORIES;LINK_LIBRARIES"
    "EXCLUDE_FROM_ALL"
    ${ARGN}
    )
  LIST(GET ARG_DEFAULT_ARGS 0 TARGET)
  SET(SOURCES ${ARG_DEFAULT_ARGS})
  LIST(REMOVE_AT SOURCES 0)

  # For APPLE, we create a STATIC library only,
  IF(APPLE)
    SET(TARGET_LIB ${TARGET})
    ADD_LIBRARY(${TARGET} STATIC ${SOURCES})
  ELSE()
    SET(TARGET_LIB ${TARGET}_objlib)
    ADD_LIBRARY(${TARGET_LIB} OBJECT ${SOURCES})
    ADD_LIBRARY(${TARGET} STATIC $<TARGET_OBJECTS:${TARGET_LIB}>)
  ENDIF()

  # Collect all static libraries in the same directory
  SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive_output_directory)

  IF(ARG_EXCLUDE_FROM_ALL)
    SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_ALL TRUE)
    IF(WIN32)
      SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    ENDIF()
  ENDIF()

  # Add COMPILE_DEFINITIONS to _objlib
  IF(ARG_COMPILE_DEFINITIONS)
    TARGET_COMPILE_DEFINITIONS(${TARGET_LIB} PRIVATE
      ${ARG_COMPILE_DEFINITIONS})
  ENDIF()

  # Add COMPILE_OPTIONS to _objlib
  IF(ARG_COMPILE_OPTIONS)
    TARGET_COMPILE_OPTIONS(${TARGET_LIB} PRIVATE
      ${ARG_COMPILE_OPTIONS})
  ENDIF()

  # Add DEPENDENCIES to _objlib
  IF(ARG_DEPENDENCIES)
    ADD_DEPENDENCIES(${TARGET_LIB} ${ARG_DEPENDENCIES})
  ENDIF()

  # Add INCLUDE_DIRECTORIES to _objlib
  IF(ARG_INCLUDE_DIRECTORIES)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_LIB} PRIVATE
      ${ARG_INCLUDE_DIRECTORIES})
  ENDIF()

  # Add LINK_LIBRARIES to static lib
  IF(ARG_LINK_LIBRARIES)
    TARGET_LINK_LIBRARIES(${TARGET} ${ARG_LINK_LIBRARIES})
  ENDIF()

  # Keep track of known convenience libraries, in a global scope.
  SET(KNOWN_CONVENIENCE_LIBRARIES
    ${KNOWN_CONVENIENCE_LIBRARIES} ${TARGET} CACHE INTERNAL "" FORCE)

ENDMACRO()


# Create libs from libs.
# Merge static libraries, creates shared libraries out of convenience libraries.
MACRO(MERGE_LIBRARIES_SHARED)
  MYSQL_PARSE_ARGUMENTS(ARG
    "EXPORTS;OUTPUT_NAME;COMPONENT"
    "SKIP_INSTALL;EXCLUDE_FROM_ALL"
    ${ARGN}
    )
  LIST(GET ARG_DEFAULT_ARGS 0 TARGET)
  SET(LIBS ${ARG_DEFAULT_ARGS})
  LIST(REMOVE_AT LIBS 0)

  FOREACH(LIB ${LIBS})
    LIST(FIND KNOWN_CONVENIENCE_LIBRARIES ${LIB} FOUNDIT)
    IF(FOUNDIT LESS 0)
      MESSAGE(STATUS "Known libs : ${KNOWN_CONVENIENCE_LIBRARIES}")
      MESSAGE(FATAL_ERROR "Unknown static library ${LIB} FOUNDIT ${FOUNDIT}")
    ENDIF()
  ENDFOREACH()

  CREATE_EXPORT_FILE(SRC ${TARGET} "${ARG_EXPORTS}")
  IF(UNIX)
    # Mark every export as explicitly needed, so that ld won't remove the
    # .a files containing them. This has a similar effect as
    # --Wl,--no-whole-archive, but is more focused.
    FOREACH(SYMBOL ${ARG_EXPORTS})
      IF(APPLE)
        SET(export_link_flags "${export_link_flags} -Wl,-u,_${SYMBOL}")
      ELSE()
        SET(export_link_flags "${export_link_flags} -Wl,-u,${SYMBOL}")
      ENDIF()
    ENDFOREACH()
  ENDIF()

  IF(NOT ARG_SKIP_INSTALL)
    ADD_VERSION_INFO(${TARGET} SHARED SRC)
  ENDIF()
  ADD_LIBRARY(${TARGET} SHARED ${SRC})

  IF(ARG_EXCLUDE_FROM_ALL)
    IF(NOT ARG_SKIP_INSTALL)
      MESSAGE(FATAL_ERROR "EXCLUDE_FROM_ALL requires SKIP_INSTALL")
    ENDIF()
    SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_ALL TRUE)
    IF(WIN32)
      SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    ENDIF()
  ENDIF()

  # Collect all dynamic libraries in the same directory
  SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory)
  IF(WIN32_CLANG AND WITH_ASAN)
    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE
      "${ASAN_LIB_DIR}/clang_rt.asan_dll_thunk-x86_64.lib")
  ENDIF()

  IF(WIN32)
    # This must be a cmake bug on windows ...
    # Anyways, with this the .dll ends up in the desired directory.
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/library_output_directory)
  ENDIF()

  TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${LIBS})
  IF(ARG_OUTPUT_NAME)
    SET_TARGET_PROPERTIES(
      ${TARGET} PROPERTIES OUTPUT_NAME "${ARG_OUTPUT_NAME}")
  ENDIF()
  SET_TARGET_PROPERTIES(
    ${TARGET} PROPERTIES LINK_FLAGS "${export_link_flags}")

  IF(APPLE AND HAVE_CRYPTO_DYLIB AND HAVE_OPENSSL_DYLIB)
    ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
      COMMAND install_name_tool -change
      "${CRYPTO_VERSION}" "@loader_path/${CRYPTO_VERSION}"
      $<TARGET_SONAME_FILE:${TARGET}>
      COMMAND install_name_tool -change
      "${OPENSSL_VERSION}" "@loader_path/${OPENSSL_VERSION}"
      $<TARGET_SONAME_FILE:${TARGET}>
      )
    # All executables have dependencies:  "@loader_path/../lib/xxx.dylib
    # Create a symlink so that this works for Xcode also.
    IF(NOT BUILD_IS_SINGLE_CONFIG)
      ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        $<TARGET_SONAME_FILE_DIR:${TARGET}> lib
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/runtime_output_directory
        )
    ENDIF()
  ENDIF()

  IF(NOT ARG_SKIP_INSTALL)
    IF(ARG_COMPONENT)
      SET(COMP COMPONENT ${ARG_COMPONENT})
    ENDIF()
    MYSQL_INSTALL_TARGETS(${TARGET} DESTINATION "${INSTALL_LIBDIR}" ${COMP})
  ENDIF()
ENDMACRO()


FUNCTION(GET_DEPENDEND_OS_LIBS target result)
  SET(deps ${${target}_LIB_DEPENDS})
  IF(deps)
    FOREACH(lib ${deps})
      # Filter out keywords for used for debug vs optimized builds
      IF(NOT lib MATCHES "general" AND
          NOT lib MATCHES "debug" AND
          NOT lib MATCHES "optimized")
        LIST(FIND KNOWN_CONVENIENCE_LIBRARIES ${lib} FOUNDIT)
        IF(FOUNDIT LESS 0)
          SET(ret ${ret} ${lib})
        ENDIF()
      ENDIF()
    ENDFOREACH()
  ENDIF()
  SET(${result} ${ret} PARENT_SCOPE)
ENDFUNCTION()


MACRO(MERGE_CONVENIENCE_LIBRARIES)
  MYSQL_PARSE_ARGUMENTS(ARG
    "OUTPUT_NAME;COMPONENT"
    "SKIP_INSTALL;EXCLUDE_FROM_ALL"
    ${ARGN}
    )
  LIST(GET ARG_DEFAULT_ARGS 0 TARGET)
  SET(LIBS ${ARG_DEFAULT_ARGS})
  LIST(REMOVE_AT LIBS 0)

  SET(SOURCE_FILE
    ${CMAKE_BINARY_DIR}/archive_output_directory/${TARGET}_depends.c)
  ADD_LIBRARY(${TARGET} STATIC ${SOURCE_FILE})

  IF(ARG_EXCLUDE_FROM_ALL)
    IF(NOT ARG_SKIP_INSTALL)
      MESSAGE(FATAL_ERROR "EXCLUDE_FROM_ALL requires SKIP_INSTALL")
    ENDIF()
    SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_ALL TRUE)
    IF(WIN32)
      SET_PROPERTY(TARGET ${TARGET} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    ENDIF()
  ENDIF()

  # Collect all static libraries in the same directory
  SET_TARGET_PROPERTIES(${TARGET} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive_output_directory)

  # Go though the list of libraries.
  # Known convenience libraries should have type "STATIC_LIBRARY"
  SET(OSLIBS)
  SET(MYLIBS)
  FOREACH(LIB ${LIBS})
    GET_TARGET_PROPERTY(LIB_TYPE ${LIB} TYPE)
    IF(LIB_TYPE STREQUAL "STATIC_LIBRARY")
      LIST(FIND KNOWN_CONVENIENCE_LIBRARIES ${LIB} FOUNDIT)
      IF(FOUNDIT LESS 0)
        MESSAGE(STATUS "Known libs : ${KNOWN_CONVENIENCE_LIBRARIES}")
        MESSAGE(FATAL_ERROR "Unknown static library ${LIB} FOUNDIT ${FOUNDIT}")
      ELSE()
        ADD_DEPENDENCIES(${TARGET} ${LIB})
        LIST(APPEND MYLIBS ${LIB})
        GET_DEPENDEND_OS_LIBS(${LIB} LIB_OSLIBS)
        IF(LIB_OSLIBS)
          # MESSAGE(STATUS "GET_DEPENDEND_OS_LIBS ${LIB} : ${LIB_OSLIBS}")
          LIST(APPEND OSLIBS ${LIB_OSLIBS})
        ENDIF()
      ENDIF()
    ELSE()
      # 3rd party library like libz.so. This is a usage error of this macro.
      MESSAGE(FATAL_ERROR "Unknown 3rd party lib ${LIB}")
    ENDIF()
    # MESSAGE(STATUS "LIB ${LIB} LIB_TYPE ${LIB_TYPE}")
  ENDFOREACH()

  # Make the generated dummy source file depended on all static input
  # libs. If input lib changes,the source file is touched
  # which causes the desired effect (relink).
  ADD_CUSTOM_COMMAND(
    OUTPUT  ${SOURCE_FILE}
    COMMAND ${CMAKE_COMMAND}  -E touch ${SOURCE_FILE}
    DEPENDS ${MYLIBS}
    )

  # For Xcode the merging of TARGET_OBJECTS does not work.
  # Rather than having a special implementation for Xcode only,
  # we always use libtool directly for merging libraries.
  IF(APPLE)
    SET(STATIC_LIBS_STRING)
    FOREACH(LIB ${MYLIBS})
      STRING_APPEND(STATIC_LIBS_STRING " $<TARGET_FILE:${LIB}>")
    ENDFOREACH()
    # Convert string to list
    STRING(REGEX REPLACE "[ ]+" ";" STATIC_LIBS_STRING "${STATIC_LIBS_STRING}" )
    ADD_CUSTOM_COMMAND(TARGET ${TARGET} POST_BUILD
      COMMAND rm $<TARGET_FILE:${TARGET}>
      COMMAND /usr/bin/libtool -static -no_warning_for_no_symbols
      -o $<TARGET_FILE:${TARGET}>
      ${STATIC_LIBS_STRING}
      )
  ELSE()
    FOREACH(LIB ${MYLIBS})
      TARGET_SOURCES(${TARGET} PRIVATE $<TARGET_OBJECTS:${LIB}_objlib>)
    ENDFOREACH()
  ENDIF()

  # On Windows, ssleay32.lib/libeay32.lib or libssl.lib/libcrypto.lib
  # must be merged into mysqlclient.lib
  IF(WIN32 AND ${TARGET} STREQUAL "mysqlclient")
    SET(LINKER_EXTRA_FLAGS "")
    FOREACH(LIB ${SSL_LIBRARIES})
      STRING_APPEND(LINKER_EXTRA_FLAGS " ${LIB}")
    ENDFOREACH()
    SET_TARGET_PROPERTIES(${TARGET}
      PROPERTIES STATIC_LIBRARY_FLAGS "${LINKER_EXTRA_FLAGS}")
  ENDIF()

  IF(OSLIBS)
    LIST(REMOVE_DUPLICATES OSLIBS)
    TARGET_LINK_LIBRARIES(${TARGET} PRIVATE ${OSLIBS})
    MESSAGE(STATUS "Library ${TARGET} depends on OSLIBS ${OSLIBS}")
  ENDIF()

  MESSAGE(STATUS "MERGE_CONVENIENCE_LIBRARIES TARGET ${TARGET}")
  MESSAGE(STATUS "MERGE_CONVENIENCE_LIBRARIES LIBS ${LIBS}")

  IF(NOT ARG_SKIP_INSTALL)
    IF(ARG_COMPONENT)
      SET(COMP COMPONENT ${ARG_COMPONENT})
    ENDIF()
    IF(INSTALL_STATIC_LIBRARIES)
      MYSQL_INSTALL_TARGETS(${TARGET} DESTINATION "${INSTALL_LIBDIR}" ${COMP})
    ENDIF()
  ENDIF()
ENDMACRO()
