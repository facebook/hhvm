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

# This should be REQUIRED, but we have to support source tarball build.
# https://dev.mysql.com/doc/refman/8.0/en/source-installation.html
FIND_PACKAGE(BISON)

IF(NOT BISON_FOUND)
  MESSAGE(WARNING "No bison found!!")
  RETURN()
ENDIF()

IF(BISON_VERSION VERSION_LESS "2.1")
  MESSAGE(FATAL_ERROR
    "Bison version ${BISON_VERSION} is old. Please update to version 2.1 or higher"
    )
ELSE()
  IF(BISON_VERSION VERSION_LESS "2.4")
    # Don't use --warnings since unsupported
    SET(BISON_FLAGS_WARNINGS "" CACHE INTERNAL "BISON 2.x flags")
  ELSEIF(BISON_VERSION VERSION_LESS "3.0")
    # Enable all warnings
    SET(BISON_FLAGS_WARNINGS
      "--warnings=all"
      CACHE INTERNAL "BISON 2.x flags")
  ELSE()
    # TODO: replace with "--warnings=all"
    # For the backward compatibility with 2.x, suppress warnings:
    # * no-yacc: for --yacc
    # * no-empty-rule: for empty rules without %empty
    # * no-precedence: for useless precedence or/and associativity rules
    SET(BISON_FLAGS_WARNINGS
      "--warnings=all,no-yacc,no-empty-rule,no-precedence"
      CACHE INTERNAL "BISON 3.x flags")
  ENDIF()
ENDIF()
