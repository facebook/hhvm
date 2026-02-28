# Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
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

# Targets below assume we have gcc and gcov version >= 9
# cmake <path> -DWITH_DEBUG=1 -DWITH_SYSTEM_LIBS=1 -DENABLE_GCOV=1
# make
# make fastcov-clean
# <run some tests>
# make fastcov-report
# make fastcov-html

FIND_PROGRAM(FASTCOV_EXECUTABLE NAMES fastcov.py fastcov)

IF(NOT FASTCOV_EXECUTABLE OR
    NOT CMAKE_COMPILER_IS_GNUCXX OR
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
  MESSAGE(WARNING "You should upgrade to gcc version >= 9 and fastcov")
  RETURN()
ENDIF()

FIND_PROGRAM(GCOV_EXECUTABLE NAMES gcov-9 gcov)
IF(NOT GCOV_EXECUTABLE)
  MESSAGE(FATAL_ERROR "gcov not found")
ENDIF()

EXECUTE_PROCESS(
  COMMAND ${GCOV_EXECUTABLE} --version
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE  stderr
  RESULT_VARIABLE result
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# gcov --version output samples on Linux:
# gcov (Debian 9-20190208-1) 9.0.1 20190208 (experimental)
# gcov (GCC) 8.3.1 20190223 (Red Hat 8.3.1-2)
STRING(REPLACE "\n" ";" GCOV_OUTPUT_LIST "${stdout}")
UNSET(GCOV_VERSION)
LIST(GET GCOV_OUTPUT_LIST 0 FIRST_LINE)
STRING(REGEX MATCH "gcov [(].*[)] ([0-9\.]+).*" XXX ${FIRST_LINE})
IF(CMAKE_MATCH_1)
  SET(GCOV_VERSION "${CMAKE_MATCH_1}")
ENDIF()

IF(GCOV_VERSION AND GCOV_VERSION VERSION_LESS 9)
  MESSAGE(FATAL_ERROR "${GCOV_EXECUTABLE} has version ${GCOV_VERSION}\n"
    "At least version 9 is required")
ENDIF()

# Add symlinks for files which contain #line 1 "foo.c" rather than full path.
# extra/duktape/duktape-2.3.0/src/duktape.c has #line directives for:
FILE(GLOB DUKTAPE_SRC_INPUT
  "${CMAKE_SOURCE_DIR}/extra/duktape/duktape-2.3.0/src-input/*.c")
FOREACH(FILE ${DUKTAPE_SRC_INPUT})
  GET_FILENAME_COMPONENT(filename "${FILE}" NAME)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${FILE} ${filename}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
ENDFOREACH()

FOREACH(FILE
    # InnoDB generated parsers are checked in as source.
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0blex.cc
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0blex.l
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0pars.cc
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0pars.y
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0tlex.cc
    ${CMAKE_SOURCE_DIR}/storage/innobase/fts/fts0tlex.l
    ${CMAKE_SOURCE_DIR}/storage/innobase/pars/lexyy.cc
    ${CMAKE_SOURCE_DIR}/storage/innobase/pars/pars0grm.cc
    ${CMAKE_SOURCE_DIR}/storage/innobase/pars/pars0grm.y
    ${CMAKE_SOURCE_DIR}/storage/innobase/pars/pars0lex.l
    # MySQL parsers
    ${CMAKE_SOURCE_DIR}/sql/debug_lo_parser.yy
    ${CMAKE_SOURCE_DIR}/sql/debug_lo_scanner.ll
    ${CMAKE_SOURCE_DIR}/sql/sql_hints.yy
    ${CMAKE_SOURCE_DIR}/sql/sql_yacc.yy
    )
  GET_FILENAME_COMPONENT(filename "${FILE}" NAME)
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${FILE} ${filename}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
ENDFOREACH()

# Ignore std, boost and 3rd-party code when doing coverage analysis.
SET(FASTCOV_EXCLUDE_LIST "--exclude")
FOREACH(FASTCOV_EXCLUDE
    "/usr/include"
    "${BOOST_INCLUDE_DIR}"
    "${BOOST_PATCHES_DIR}"
    ${GMOCK_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/extra/duktape"
    "${CMAKE_SOURCE_DIR}/extra/lz4"
    "${CMAKE_SOURCE_DIR}/extra/rapidjson"
    )
  LIST(APPEND FASTCOV_EXCLUDE_LIST "${FASTCOV_EXCLUDE}")
ENDFOREACH()

ADD_CUSTOM_TARGET(fastcov-clean
  COMMAND ${FASTCOV_EXECUTABLE} --gcov ${GCOV_EXECUTABLE} --zerocounters
  COMMENT "Running ${FASTCOV_EXECUTABLE} --zerocounters"
  VERBATIM
  )
ADD_CUSTOM_TARGET(fastcov-report
  COMMAND ${FASTCOV_EXECUTABLE} --gcov ${GCOV_EXECUTABLE}
          ${FASTCOV_EXCLUDE_LIST} --lcov -o report.info
  COMMENT "Running ${FASTCOV_EXECUTABLE} --lcov -o report.info"
  VERBATIM
  )
ADD_CUSTOM_TARGET(fastcov-html
  COMMAND genhtml -o code_coverage report.info
  COMMENT "Running genhtml -o code_coverage report.info"
  VERBATIM
  )
