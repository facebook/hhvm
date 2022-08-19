/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef status_var_h
#define status_var_h

/**
  Declarations for SHOW STATUS support in plugins
*/
enum enum_mysql_show_type {
  SHOW_UNDEF,
  SHOW_BOOL,
  SHOW_INT,       ///< shown as _unsigned_ int
  SHOW_LONG,      ///< shown as _unsigned_ long
  SHOW_LONGLONG,  ///< shown as _unsigned_ longlong
  SHOW_CHAR,
  SHOW_CHAR_PTR,
  SHOW_ARRAY,
  SHOW_FUNC,
  SHOW_DOUBLE,
  /*
    This include defines server-only values of the enum.
    Using them in plugins is not supported.
  */
  SHOW_KEY_CACHE_LONG,
  SHOW_KEY_CACHE_LONGLONG,
  SHOW_LONG_STATUS,
  SHOW_DOUBLE_STATUS,
  SHOW_HAVE,
  SHOW_MY_BOOL,
  SHOW_HA_ROWS,
  SHOW_SYS,
  SHOW_LONG_NOFLUSH,
  SHOW_LONGLONG_STATUS,
  SHOW_LEX_STRING,
  /*
    Support for signed values are extended for plugins.
  */
  SHOW_SIGNED_INT,
  SHOW_SIGNED_LONG,
  SHOW_SIGNED_LONGLONG,
  SHOW_TIMER,
  SHOW_TIMER_STATUS
};

/**
  Status variable scope.
  Only GLOBAL status variable scope is available in plugins.
*/
enum enum_mysql_show_scope {
  SHOW_SCOPE_UNDEF,
  SHOW_SCOPE_GLOBAL,
  /* Server-only values. Not supported in plugins. */
  SHOW_SCOPE_SESSION,
  SHOW_SCOPE_ALL
};

/**
  SHOW STATUS Server status variable
*/
struct SHOW_VAR {
  const char *name;
  char *value;
  enum enum_mysql_show_type type;
  enum enum_mysql_show_scope scope;
};

#define SHOW_VAR_MAX_NAME_LEN 64
#define SHOW_VAR_FUNC_BUFF_SIZE 1024
#ifdef __cplusplus
class THD;
#define MYSQL_THD THD *
#else
#define MYSQL_THD void *
#endif
typedef int (*mysql_show_var_func)(MYSQL_THD, SHOW_VAR *, char *);

#endif
