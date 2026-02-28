# Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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

CMAKE_POLICY(SET CMP0007 NEW)

MESSAGE(STATUS "Writing stdout to ${OUTPUT_FILE}")
MESSAGE(STATUS "Writing stderr to ${ERROR_FILE}")

EXECUTE_PROCESS(
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
  ERROR_FILE ${ERROR_FILE}
  OUTPUT_FILE ${OUTPUT_FILE}
  )

MESSAGE("Filtering out ignored warnings/errors")
MESSAGE(STATUS "Writing warnings/errors to ${TOFIX_FILE}")

# Read IGNORE_FILE and create a list of patterns that we should ignore in
# ERROR_FILE.
FILE(READ ${IGNORE_FILE} IGNORE_FILE_CONTENTS)
STRING(REPLACE ";" "\\\\;" IGNORE_FILE_CONTENTS ${IGNORE_FILE_CONTENTS})
STRING(REPLACE "\n" ";" IGNORE_FILE_LINES ${IGNORE_FILE_CONTENTS})
FOREACH(LINE ${IGNORE_FILE_LINES})
  STRING(REGEX MATCH "^[\r\n\t ]*#" MATCH_COMMENT ${LINE})
  STRING(REGEX MATCH "^[\r\n\t ]*$" MATCH_EMPTY ${LINE})
  IF(NOT (MATCH_COMMENT OR MATCH_EMPTY))
    MESSAGE(STATUS "Ignoring pattern ${LINE}")
    SET(IGNORE_LIST "${IGNORE_LIST};${LINE}")
  ENDIF()
ENDFOREACH()

# Convert ERROR_FILE contents to a list of lines
FILE(READ ${ERROR_FILE} ERROR_FILE_CONTENTS)
IF(ERROR_FILE_CONTENTS)
  STRING(REPLACE ";" "\\\\;" ERROR_FILE_CONTENTS ${ERROR_FILE_CONTENTS})
  STRING(REPLACE "\n" ";" ERROR_FILE_LINES ${ERROR_FILE_CONTENTS})
ENDIF()

FILE(REMOVE ${TOFIX_FILE})
FILE(REMOVE ${REGRESSION_FILE})
UNSET(FOUND_WARNINGS)
# See if we have any warnings/errors.
FOREACH(LINE ${ERROR_FILE_LINES})
  # Workaround for missing CONTINUE() in CMake version < 3.2
  SET(LOOP_CONTINUE 0)

  # Filter out information messages from dia.
  STRING(REGEX MATCH "^.*\\.dia --> dia_.*\\.png\$" DIA_STATUS ${LINE})
  STRING(LENGTH "${DIA_STATUS}" LEN_DIA_STATUS)
  IF (${LEN_DIA_STATUS} GREATER 0)
    SET (LOOP_CONTINUE 1)
  ENDIF()

  # Filter out git errors that occur if running on a tarball insted of a git
  # repo (doxygen_resources/doxygen-filter-mysqld calls git).
  STRING(REGEX MATCH "^Stopping at filesystem boundary \\(GIT_DISCOVERY_ACROSS_FILESYSTEM not set\\).\$" GIT_ERROR ${LINE})
  STRING(LENGTH "${GIT_ERROR}" LEN_GIT_ERROR)
  IF (${LEN_GIT_ERROR} GREATER 0)
    SET(LOOP_CONTINUE 1)
  ENDIF()
  STRING(REGEX MATCH "^fatal: Not a git repository \\(or any parent up to mount point " GIT_ERROR ${LINE})
  STRING(LENGTH "${GIT_ERROR}" LEN_GIT_ERROR)
  IF (${LEN_GIT_ERROR} GREATER 0)
    SET(LOOP_CONTINUE 1)
  ENDIF()

  IF(NOT ${LOOP_CONTINUE})
    STRING(REGEX MATCH "^(${SOURCE_DIR}/)(.*)" XXX ${LINE})
    IF(CMAKE_MATCH_1)
      SET(LINE ${CMAKE_MATCH_2})
    ELSE()
      GET_FILENAME_COMPONENT(SOURCE_DIR_REALPATH ${SOURCE_DIR} REALPATH)
      STRING(REGEX MATCH "^(${SOURCE_DIR_REALPATH}/)(.*)" XXX ${LINE})
      IF(CMAKE_MATCH_1)
	SET(LINE ${CMAKE_MATCH_2})
      ENDIF()
    ENDIF()

    # Check for known patterns. Known patterns are not reported as regressions.
    SET(IS_REGRESSION 1)
    FOREACH(IGNORE_PATTERN ${IGNORE_LIST})
      STRING(REGEX MATCH "${IGNORE_PATTERN}" IGNORED ${LINE})
      STRING(LENGTH "${IGNORED}" LEN_IGNORED)
      IF (${LEN_IGNORED} GREATER 0)
	# The line matches a pattern in IGNORE_FILE, so this is a known error.
	UNSET(IS_REGRESSION)
	BREAK()
      ENDIF()
    ENDFOREACH()

    # All errors go to TOFIX_FILE.
    FILE(APPEND ${TOFIX_FILE} "${LINE}\n")

    # Only regressions go to REGRESSION_FILE.
    IF (${IS_REGRESSION})
      MESSAGE(${LINE})
      FILE(APPEND ${REGRESSION_FILE} "${LINE}\n")
      SET(FOUND_WARNINGS 1)
    ENDIF()
  ENDIF()
ENDFOREACH()

# Only report regressions.
IF(FOUND_WARNINGS)
  MESSAGE("\n\nFound warnings/errors, see ${REGRESSION_FILE}")
ELSE()
  MESSAGE("No warnings/errors found")
ENDIF()
