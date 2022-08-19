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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

FUNCTION(GET_NATIVE_FUNCTION_NAMES FILE_NAME RESULT_NAMES)
  FILE(READ ${FILE_NAME} FILE_CONTENT)
  STRING(REGEX MATCHALL "{\"[A-Z_0-9]+\",\( |\n\) *SQL_F[A-Z_0-9]*" RESULT ${FILE_CONTENT})
  STRING(REGEX REPLACE "{\"[A-Z_0-9]+\",\( |\n\) *SQL_F[A-Z_0-9]*_INTERNAL" "" RESULT ${RESULT})
  STRING(REGEX REPLACE "{(\"[A-Z_0-9]+\",)\( |\n\) *SQL_F[A-Z_0-9]*" "\\1\n" RESULT ${RESULT})

  SET(${RESULT_NAMES} ${RESULT} PARENT_SCOPE)
ENDFUNCTION()


FUNCTION(GET_SPECIAL_FUNCTION_NAMES FILE_NAME RESULT_NAMES)
  FILE(READ ${FILE_NAME} FILE_CONTENT)
  STRING(REGEX MATCHALL "{SYM_FN\\(\"[A-Z_0-9]+\"," RESULT ${FILE_CONTENT})
  STRING(REGEX REPLACE " *{SYM_FN\\((\"[A-Z_0-9]+\",)" "\\1\n" RESULT ${RESULT})

  SET(${RESULT_NAMES} ${RESULT} PARENT_SCOPE)
ENDFUNCTION()


FUNCTION(GET_OTHER_FUNCTION_NAMES FILE_NAME RESULT_NAMES)
  FILE(READ ${FILE_NAME} FILE_CONTENT)
  STRING(REGEX MATCHALL " *( |\\|) +[A-Z_0-9]+ +'\\(' +[a-z_0-9]*expr[a-z_0-9]* +'(,|\\))'" RESULT1 ${FILE_CONTENT})
  STRING(REGEX REPLACE " *( |\\|) +([A-Z_0-9]+) +'\\(' +[a-z_0-9]*expr[a-z_0-9]* +'(,|\\))'" "\"\\2\",\n" RESULT1 ${RESULT1})

  STRING(REGEX MATCHALL " *( |\\|) +[A-Z_0-9]+ +'\\(' +'\\)'" RESULT2 ${FILE_CONTENT})
  STRING(REGEX REPLACE " *( |\\|) +([A-Z_0-9]+) +'\\(' +'\\)'" "\"\\2\",\n" RESULT2 ${RESULT2})

  STRING(REGEX MATCHALL " *( |\\|) +[A-Z_0-9]+ +optional_braces" RESULT3 ${FILE_CONTENT})
  STRING(REGEX REPLACE " *( |\\|) +([A-Z_0-9]+) +optional_braces" "\"\\2\",\n" RESULT3 ${RESULT3})

  STRING(REGEX MATCHALL " *( |\\|) +[A-Z_0-9]+ +'\\(' +(interval_time_stamp|date_time_type|simple_ident_nospvar)" RESULT4 ${FILE_CONTENT})
  STRING(REGEX REPLACE " *( |\\|) +([A-Z_0-9]+) +'\\(' +(interval_time_stamp|date_time_type|simple_ident_nospvar)" "\"\\2\",\n" RESULT4 ${RESULT4})

  STRING(REGEX MATCHALL " *( |\\|) +[A-Z_0-9]+ +func_datetime_precision" RESULT5 ${FILE_CONTENT})
  STRING(REGEX REPLACE " *( |\\|) +([A-Z_0-9]+) +func_datetime_precision" "\"\\2\",\n" RESULT5 ${RESULT5})

  # fix some names
  STRING(REGEX REPLACE "\"([A-Z_0-9]+)_SYM\"" "\"\\1\"" RESULT ${RESULT1} ${RESULT2} ${RESULT3} ${RESULT4} ${RESULT5})
  STRING(REGEX REPLACE "TIMESTAMP_(ADD|DIFF)" "TIMESTAMP\\1" RESULT ${RESULT})

  # removal false possitives
  STRING(REGEX REPLACE "\"(RANGE|LIST|CHECK|AS|ROW|JSON_TABLE)\",\n" "" RESULT ${RESULT})

  SET(${RESULT_NAMES} ${RESULT} PARENT_SCOPE)
ENDFUNCTION()


GET_NATIVE_FUNCTION_NAMES(${PROJECT_SOURCE_DIR}/sql/item_create.cc NATIVE_MYSQL_FUNCTIONS)
GET_SPECIAL_FUNCTION_NAMES(${PROJECT_SOURCE_DIR}/sql/lex.h SPECIAL_MYSQL_FUNCTIONS)
GET_OTHER_FUNCTION_NAMES(${PROJECT_SOURCE_DIR}/sql/sql_yacc.yy OTHER_MYSQL_FUNCTIONS)


CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/xpl/mysql_function_names_t.cc.in
                ${MYSQLX_GENERATE_DIR}/mysql_function_names_t.cc)




