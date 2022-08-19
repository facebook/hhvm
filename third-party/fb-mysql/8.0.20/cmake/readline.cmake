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

# cmake -DWITH_EDITLINE=system|bundled
# bundled is the default

MACRO (MYSQL_CHECK_MULTIBYTE)
  SET(CMAKE_EXTRA_INCLUDE_FILES wchar.h)
  CHECK_TYPE_SIZE(mbstate_t SIZEOF_MBSTATE_T)
  SET(CMAKE_EXTRA_INCLUDE_FILES)
  IF(SIZEOF_MBSTATE_T)
    SET(HAVE_MBSTATE_T 1)
  ENDIF()

  CHECK_C_SOURCE_COMPILES("
  #include <langinfo.h>
  int main(int ac, char **av)
  {
    char *cs = nl_langinfo(CODESET);
    return 0;
  }"
  HAVE_LANGINFO_CODESET)
  
  CHECK_FUNCTION_EXISTS(wcsdup HAVE_WCSDUP)

  SET(CMAKE_EXTRA_INCLUDE_FILES wchar.h)
  CHECK_TYPE_SIZE(wchar_t SIZEOF_WCHAR_T)
  IF(SIZEOF_WCHAR_T)
    SET(HAVE_WCHAR_T 1)
  ENDIF()

  SET(CMAKE_EXTRA_INCLUDE_FILES wctype.h)
  CHECK_TYPE_SIZE(wint_t SIZEOF_WINT_T)
  IF(SIZEOF_WINT_T)
    SET(HAVE_WINT_T 1)
  ENDIF()
  SET(CMAKE_EXTRA_INCLUDE_FILES)

ENDMACRO()

MACRO (FIND_CURSES)
 FIND_PACKAGE(Curses) 
 MARK_AS_ADVANCED(CURSES_CURSES_H_PATH CURSES_FORM_LIBRARY CURSES_HAVE_CURSES_H)
 IF(NOT CURSES_FOUND)
   SET(ERRORMSG "Curses library not found. Please install appropriate package,
    remove CMakeCache.txt and rerun cmake.")
   IF(LINUX)
     SET(ERRORMSG ${ERRORMSG} 
       "On Debian/Ubuntu, package name is libncurses5-dev, on Redhat and derivates " 
       "it is ncurses-devel.")
   ENDIF()
   MESSAGE(FATAL_ERROR ${ERRORMSG})
 ENDIF()

 IF(CURSES_HAVE_CURSES_H)
   SET(HAVE_CURSES_H 1 CACHE INTERNAL "")
 ELSEIF(CURSES_HAVE_NCURSES_H)
   SET(HAVE_NCURSES_H 1 CACHE INTERNAL "")
 ENDIF()

 IF(LINUX)
   # -Wl,--as-needed breaks linking with -lcurses, e.g on Fedora 
   # Lower-level libcurses calls are exposed by libtinfo
   CHECK_LIBRARY_EXISTS(${CURSES_LIBRARY} tputs "" HAVE_TPUTS_IN_CURSES)
   IF(NOT HAVE_TPUTS_IN_CURSES)
     CHECK_LIBRARY_EXISTS(tinfo tputs "" HAVE_TPUTS_IN_TINFO)
     IF(HAVE_TPUTS_IN_TINFO)
       SET(CURSES_LIBRARY tinfo)
     ENDIF()
   ENDIF() 
 ENDIF()
ENDMACRO()

MACRO (MYSQL_USE_BUNDLED_EDITLINE)
  SET(WITH_EDITLINE "bundled" CACHE STRING "By default use bundled editline")
  SET(USE_LIBEDIT_INTERFACE 1)
  SET(HAVE_HIST_ENTRY 1)
  SET(EDITLINE_INCLUDE_DIR
    ${CMAKE_SOURCE_DIR}/extra/libedit/libedit-20190324-3.1/src/editline)
  INCLUDE_DIRECTORIES(BEFORE SYSTEM ${EDITLINE_INCLUDE_DIR})
  SET(EDITLINE_LIBRARY edit)
  FIND_CURSES()
  ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/extra/libedit/libedit-20190324-3.1/src)
ENDMACRO()

MACRO (FIND_SYSTEM_EDITLINE)
  FIND_PATH(FOUND_EDITLINE_READLINE
    NAMES editline/readline.h
  )
  IF(FOUND_EDITLINE_READLINE)
    SET(EDITLINE_INCLUDE_DIR "${FOUND_EDITLINE_READLINE}/editline")
  ELSE()
    # Different path on FreeBSD
    FIND_PATH(FOUND_EDIT_READLINE_READLINE
      NAMES edit/readline/readline.h
    )
    IF(FOUND_EDIT_READLINE_READLINE)
      SET(EDITLINE_INCLUDE_DIR "${FOUND_EDIT_READLINE_READLINE}/edit/readline")
    ENDIF()
  ENDIF()

  FIND_LIBRARY(EDITLINE_LIBRARY
    NAMES
    edit
  )
  MARK_AS_ADVANCED(EDITLINE_INCLUDE_DIR EDITLINE_LIBRARY)

  MESSAGE(STATUS "EDITLINE_INCLUDE_DIR ${EDITLINE_INCLUDE_DIR}")
  MESSAGE(STATUS "EDITLINE_LIBRARY ${EDITLINE_LIBRARY}")

  INCLUDE(CheckCXXSourceCompiles)
  IF(EDITLINE_LIBRARY AND EDITLINE_INCLUDE_DIR)
    CMAKE_PUSH_CHECK_STATE()

    SET(CMAKE_REQUIRED_INCLUDES ${EDITLINE_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(SYSTEM ${EDITLINE_INCLUDE_DIR})
    LIST(APPEND CMAKE_REQUIRED_LIBRARIES ${EDITLINE_LIBRARY})
    CHECK_CXX_SOURCE_COMPILES("
    #include <stdio.h>
    #include <readline.h>
    int main(int argc, char **argv)
    {
       HIST_ENTRY entry;
       return 0;
    }"
    EDITLINE_HAVE_HIST_ENTRY)

    CHECK_CXX_SOURCE_COMPILES("
    #include <stdio.h>
    #include <readline.h>
    int main(int argc, char **argv)
    {
      typedef int MYFunction(const char*, int);
      MYFunction* myf= rl_completion_entry_function;
      int res= (myf)(NULL, 0);
      completion_matches(0,0);
      return res;
    }"
    EDITLINE_HAVE_COMPLETION_INT)

    CHECK_CXX_SOURCE_COMPILES("
    #include <stdio.h>
    #include <readline.h>
    int main(int argc, char **argv)
    {
      typedef char* MYFunction(const char*, int);
      MYFunction* myf= rl_completion_entry_function;
      char *res= (myf)(NULL, 0);
      completion_matches(0,0);
      return res != NULL;
    }"
    EDITLINE_HAVE_COMPLETION_CHAR)

    IF(EDITLINE_HAVE_COMPLETION_INT OR EDITLINE_HAVE_COMPLETION_CHAR)
      SET(HAVE_HIST_ENTRY ${EDITLINE_HAVE_HIST_ENTRY})
      SET(USE_LIBEDIT_INTERFACE 1)
      SET(EDITLINE_FOUND 1)
      IF(EDITLINE_HAVE_COMPLETION_CHAR)
        SET(USE_NEW_EDITLINE_INTERFACE 1)
      ENDIF()
    ENDIF()
    CMAKE_POP_CHECK_STATE()
  ENDIF()
ENDMACRO()


IF (NOT WITH_EDITLINE AND NOT WIN32)
  SET(WITH_EDITLINE "bundled" CACHE STRING "By default use bundled editline")
ENDIF()

MACRO (MYSQL_CHECK_EDITLINE)
  IF (NOT WIN32)
    MYSQL_CHECK_MULTIBYTE()

    IF(WITH_EDITLINE STREQUAL "bundled") 
      MYSQL_USE_BUNDLED_EDITLINE()
    ELSEIF(WITH_EDITLINE STREQUAL "system")
      FIND_SYSTEM_EDITLINE()
      IF(NOT EDITLINE_FOUND)
        MESSAGE(FATAL_ERROR "Cannot find system editline libraries.") 
      ENDIF()
    ELSE()
      MESSAGE(FATAL_ERROR "WITH_EDITLINE must be bundled or system")
    ENDIF()
  ENDIF(NOT WIN32)
ENDMACRO()
