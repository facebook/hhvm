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

# We require rapidjson version 1.1.0 or higher.
# -DWITH_RAPIDJSON=bundled is the default

IF(MY_COMPILER_IS_SUNPRO)
  # Oracle Developer Studio 12.x isn't detected as c++11 compliant by rapidjson
  # automatically.
  ADD_DEFINITIONS(-DRAPIDJSON_HAS_CXX11_RANGE_FOR=1)
  ADD_DEFINITIONS(-DRAPIDJSON_HAS_CXX11_NOEXCEPT=1)
  ADD_DEFINITIONS(-DRAPIDJSON_HAS_CXX11_RVALUE_REFS=1)
ENDIF()

MACRO(WRONG_RAPIDJSON_VERSION)
  MESSAGE(FATAL_ERROR "rapidjson version 1.1.0 or higher is required.")
ENDMACRO()

SET(rapidjson_regex_check_code "
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
int main() {
  using namespace rapidjson;
  Document schema_doc;
  schema_doc.Parse(\"{\\\\\"type\\\\\":\\\\\"string\\\\\",\\\\\"pattern\\\\\":\\\\\"a\\\\\"}\");
  SchemaDocument schema(schema_doc);
  SchemaValidator validator(schema);
  Document doc;
  doc.Parse(\"\\\\\"b\\\\\"\");
  return doc.Accept(validator) ? 1 : 0;
}"
)

MACRO (CHECK_RAPIDJSON_VERSION)
  FILE(STRINGS "${RAPIDJSON_INCLUDE_DIR}/rapidjson/rapidjson.h"
    RAPIDJSON_MAJOR_VERSION_NUMBER
    REGEX "^#define RAPIDJSON_MAJOR_VERSION ([0-9]+.*)"
  )

  FILE(STRINGS "${RAPIDJSON_INCLUDE_DIR}/rapidjson/rapidjson.h"
    RAPIDJSON_MINOR_VERSION_NUMBER
    REGEX "^#define RAPIDJSON_MINOR_VERSION ([0-9]+.*)"
  )

  STRING(REGEX MATCH "([0-9])"
    RAPIDJSON_MAJOR_VERSION_NUMBER "${RAPIDJSON_MAJOR_VERSION_NUMBER}")
  STRING(REGEX MATCH "([0-9])"
    RAPIDJSON_MINOR_VERSION_NUMBER "${RAPIDJSON_MINOR_VERSION_NUMBER}")

  MESSAGE(STATUS "RAPIDJSON_MAJOR_VERSION is ${RAPIDJSON_MAJOR_VERSION_NUMBER}")
  MESSAGE(STATUS "RAPIDJSON_MINOR_VERSION is ${RAPIDJSON_MINOR_VERSION_NUMBER}")

  SET(RAPIDJSON_FULL_VERSION
      "${RAPIDJSON_MAJOR_VERSION_NUMBER}.${RAPIDJSON_MINOR_VERSION_NUMBER}")

  IF (RAPIDJSON_FULL_VERSION VERSION_LESS "1.1")
    WRONG_RAPIDJSON_VERSION()
  ENDIF()
ENDMACRO()

MACRO (FIND_SYSTEM_RAPIDJSON)
  FIND_PATH(PATH_TO_RAPIDJSON NAMES rapidjson/rapidjson.h)
  IF (PATH_TO_RAPIDJSON)
    SET(SYSTEM_RAPIDJSON_FOUND 1)
    SET(RAPIDJSON_INCLUDE_DIR ${PATH_TO_RAPIDJSON})
  ENDIF()
ENDMACRO()

MACRO (MYSQL_CHECK_RAPIDJSON)
  IF (NOT WITH_RAPIDJSON OR
      NOT WITH_RAPIDJSON STREQUAL "system")
    SET(WITH_RAPIDJSON "bundled")
  ENDIF()

  IF (WITH_RAPIDJSON STREQUAL "bundled")
    SET(RAPIDJSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extra/rapidjson/include)
  ELSEIF(WITH_RAPIDJSON STREQUAL "system")
    FIND_SYSTEM_RAPIDJSON()
    IF (NOT SYSTEM_RAPIDJSON_FOUND)
      MESSAGE(FATAL_ERROR "Cannot find system rapidjson libraries. You need to "
        "install the required libraries:\n"
        "  Debian/Ubuntu:              apt install rapidjson-dev\n"
        "  RedHat/Fedora/Oracle Linux: yum install rapidjson-devel\n"
        "  SuSE:                       zypper install rapidjson\n"
        "You can also use the bundled version by specifyng "
        "-DWITH_RAPIDJSON=bundled.")
    ENDIF()
    # Check if the system version includes the necessary fixes for std::regex.
    CMAKE_PUSH_CHECK_STATE()
    SET(CMAKE_REQUIRED_INCLUDES ${RAPIDJSON_INCLUDE_DIR})
    SET(CMAKE_REQUIRED_DEFINITIONS "-DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0 "
                                   "-DRAPIDJSON_SCHEMA_USE_STDREGEX=1")
    CHECK_CXX_SOURCE_RUNS("${rapidjson_regex_check_code}"
                          HAVE_RAPIDJSON_WITH_STD_REGEX)
    CMAKE_POP_CHECK_STATE()
    IF (NOT HAVE_RAPIDJSON_WITH_STD_REGEX)
      MESSAGE(FATAL_ERROR "System rapidjson lacks some fixes required for "
              "support of regular expressions. See extra/RAPIDJSON-README "
              "for details.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "WITH_RAPIDJSON must be bundled or system")
  ENDIF()

  MESSAGE(STATUS "RAPIDJSON_INCLUDE_DIR ${RAPIDJSON_INCLUDE_DIR}")
  CHECK_RAPIDJSON_VERSION()
  IF(WITH_RAPIDJSON STREQUAL "bundled")
    INCLUDE_DIRECTORIES(BEFORE SYSTEM ${RAPIDJSON_INCLUDE_DIR})
  ELSE()
    INCLUDE_DIRECTORIES(SYSTEM ${RAPIDJSON_INCLUDE_DIR})
  ENDIF()
ENDMACRO()

ADD_DEFINITIONS(-DRAPIDJSON_NO_SIZETYPEDEFINE)
ADD_DEFINITIONS(-DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0)
ADD_DEFINITIONS(-DRAPIDJSON_SCHEMA_USE_STDREGEX=1)
