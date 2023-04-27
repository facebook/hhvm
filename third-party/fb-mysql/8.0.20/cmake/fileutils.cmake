# Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.
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

FUNCTION(GET_FILE_SIZE FILE_NAME OUTPUT_SIZE)
  IF(WIN32)
    FILE(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/cmake/filesize.bat" FILESIZE_BAT)
    FILE(TO_NATIVE_PATH "${FILE_NAME}" NATIVE_FILE_NAME)

    EXECUTE_PROCESS(
      COMMAND "${FILESIZE_BAT}" "${NATIVE_FILE_NAME}"
      RESULT_VARIABLE COMMAND_RESULT
      OUTPUT_VARIABLE RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  ELSEIF(APPLE OR FREEBSD)
    EXEC_PROGRAM(stat ARGS -f '%z' ${FILE_NAME} OUTPUT_VARIABLE RESULT)
  ELSE()
    EXEC_PROGRAM(stat ARGS -c '%s' ${FILE_NAME} OUTPUT_VARIABLE RESULT)
  ENDIF()
  SET(${OUTPUT_SIZE} ${RESULT} PARENT_SCOPE)
ENDFUNCTION()


IF(WIN32)
  GET_FILENAME_COMPONENT(CMAKE_LINKER_PATH "${CMAKE_LINKER}" DIRECTORY)
  FIND_PROGRAM(DUMPBIN_EXECUTABLE dumpbin PATHS "${CMAKE_LINKER_PATH}")

  # TODO: implement for macOS (otool) Unix (ldd)
  FUNCTION(FIND_OBJECT_DEPENDENCIES FILE_NAME RETURN_VALUE)
    SET(${RETURN_VALUE} PARENT_SCOPE)
    IF(WIN32 AND DUMPBIN_EXECUTABLE)
      EXECUTE_PROCESS(COMMAND
        "${DUMPBIN_EXECUTABLE}" "/dependents" "${FILE_NAME}"
        OUTPUT_VARIABLE DUMPBIN_OUTPUT
        RESULT_VARIABLE DUMPBIN_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
      STRING(REPLACE "\n" ";" DUMPBIN_OUTPUT_LIST "${DUMPBIN_OUTPUT}")
      SET(DEPENDENCIES)
      FOREACH(LINE ${DUMPBIN_OUTPUT_LIST})
        STRING(REGEX MATCH "^[\r\n\t ]*([A-Za-z0-9_]*\.dll)" UNUSED ${LINE})
        IF(CMAKE_MATCH_1)
          LIST(APPEND DEPENDENCIES ${CMAKE_MATCH_1})
        ENDIF()
      ENDFOREACH()
      SET(${RETURN_VALUE} ${DEPENDENCIES} PARENT_SCOPE)
    ENDIF()
  ENDFUNCTION()
ENDIF()

IF(LINUX)
  FUNCTION(FIND_OBJECT_DEPENDENCIES FILE_NAME RETURN_VALUE)
    SET(${RETURN_VALUE} PARENT_SCOPE)
    EXECUTE_PROCESS(COMMAND
      objdump -p "${FILE_NAME}"
      OUTPUT_VARIABLE OBJDUMP_OUTPUT
      RESULT_VARIABLE OBJDUMP_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    STRING(REPLACE "\n" ";" OBJDUMP_OUTPUT_LIST "${OBJDUMP_OUTPUT}")
    SET(DEPENDENCIES)
    FOREACH(LINE ${OBJDUMP_OUTPUT_LIST})
      STRING(REGEX MATCH
        "^[ ]+NEEDED[ ]+([-_A-Za-z0-9\\.]+)" UNUSED ${LINE})
      IF(CMAKE_MATCH_1)
        LIST(APPEND DEPENDENCIES ${CMAKE_MATCH_1})
      ENDIF()
    ENDFOREACH()
    SET(${RETURN_VALUE} ${DEPENDENCIES} PARENT_SCOPE)
  ENDFUNCTION()

  FUNCTION(FIND_SONAME FILE_NAME RETURN_VALUE)
    SET(${RETURN_VALUE} PARENT_SCOPE)
    EXECUTE_PROCESS(COMMAND
      objdump -p "${FILE_NAME}"
      OUTPUT_VARIABLE OBJDUMP_OUTPUT
      RESULT_VARIABLE OBJDUMP_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    STRING(REPLACE "\n" ";" OBJDUMP_OUTPUT_LIST "${OBJDUMP_OUTPUT}")
    FOREACH(LINE ${OBJDUMP_OUTPUT_LIST})
      STRING(REGEX MATCH
        "^[ ]+SONAME[ ]+([-_A-Za-z0-9\\.]+)" UNUSED ${LINE})
      IF(CMAKE_MATCH_1)
        SET(${RETURN_VALUE} ${CMAKE_MATCH_1} PARENT_SCOPE)
      ENDIF()
    ENDFOREACH()
  ENDFUNCTION()

  FUNCTION(VERIFY_CUSTOM_LIBRARY_DEPENDENCIES)
    FOREACH(lib ${KNOWN_CUSTOM_LIBRARIES})
      FOREACH(lib_needs ${NEEDED_${lib}})
        GET_FILENAME_COMPONENT(library_name_we "${lib_needs}" NAME_WE)
        SET(SONAME ${SONAME_${library_name_we}})
        IF(SONAME)
          MESSAGE(STATUS
            "${lib} needs ${lib_needs} from ${library_name_we}")
          IF(NOT "${lib_needs}" STREQUAL "${SONAME}")
            MESSAGE(WARNING "${library_name_we} provides ${SONAME}")
          ENDIF()
        ENDIF()
      ENDFOREACH()
    ENDFOREACH()
  ENDFUNCTION()

ENDIF()
