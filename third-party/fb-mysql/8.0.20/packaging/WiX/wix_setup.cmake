# Copyright (c) 2010, 2018, Oracle and/or its affiliates. All rights reserved.
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

SET(WIX_REQUIRED_VERSION "V3.11")

# Need an extra indirection to access ENV(ProgramFiles(x86))
SET(MYENV "ProgramFiles(x86)")

# Look in various paths for 'heat.exe'
# Different installations have different layouts.
MACRO(FIND_WIX_PATH VERSION)
  FOREACH(path
    "$ENV{ProgramFiles}/WiX Toolset ${VERSION}"
    "$ENV{ProgramFiles}/WiX Toolset ${VERSION}/bin"
    "$ENV{${MYENV}}/WiX Toolset ${VERSION}"
    "$ENV{${MYENV}}/WiX Toolset ${VERSION}/bin")
      FIND_PATH(WIX_DIR heat.exe "${path}")
      MESSAGE(STATUS "WIX_DIR ${WIX_DIR} path ${path}")
      IF(WIX_DIR)
        BREAK()
      ENDIF()
  ENDFOREACH()
ENDMACRO()

FIND_WIX_PATH(${WIX_REQUIRED_VERSION})

# Finally, look in environment
IF(NOT WIX_DIR)
  FIND_PATH(WIX_DIR heat.exe "$ENV{WIX}")
  MESSAGE(STATUS "WIX_DIR ${WIX_DIR} looked at $ENV{WIX}\n")
ENDIF()

IF(NOT WIX_DIR)
  IF(NOT _WIX_DIR_CHECKED)
    SET(_WIX_DIR_CHECKED 1 CACHE INTERNAL "")
    MESSAGE(STATUS "Cannot find wix 3, installer project will not be generated")
  ENDIF()
  RETURN()
ENDIF()

FIND_PROGRAM(HEAT_EXECUTABLE heat ${WIX_DIR})
FIND_PROGRAM(CANDLE_EXECUTABLE candle ${WIX_DIR})
FIND_PROGRAM(LIGHT_EXECUTABLE light ${WIX_DIR})

FUNCTION (CREATE_WIX_LICENCE_AND_RTF license_file)
  # WiX wants the license text as rtf; if there is no rtf license,
  # we create a fake one from the plain text LICENSE file.
  IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf")
   SET(LICENSE_RTF "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf" PARENT_SCOPE)
  ELSE()
    FILE(READ ${license_file} CONTENTS)
    STRING(REGEX REPLACE "\n" "\\\\par\n" CONTENTS "${CONTENTS}")
    STRING(REGEX REPLACE "\t" "\\\\tab" CONTENTS "${CONTENTS}")
    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.rtf" "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Courier New;}}\\viewkind4\\uc1\\pard\\lang1031\\f0\\fs15")
    FILE(APPEND "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.rtf" "${CONTENTS}")
    FILE(APPEND "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.rtf" "\n}\n")
    SET(LICENSE_RTF "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.rtf" PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()
