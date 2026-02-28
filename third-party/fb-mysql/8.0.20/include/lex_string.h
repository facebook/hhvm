/*
   Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LEX_STRING_INCLUDED
#define LEX_STRING_INCLUDED

#include "mysql/mysql_lex_string.h"

#ifdef __cplusplus
#include <string>
#endif

/*
  LEX_STRING -- a pair of a C-string and its length.
  (it's part of the plugin API as a MYSQL_LEX_STRING)
  Ditto LEX_CSTRING/MYSQL_LEX_CSTRING.
*/

typedef struct MYSQL_LEX_STRING LEX_STRING;
typedef struct MYSQL_LEX_CSTRING LEX_CSTRING;

#ifdef __cplusplus

static inline std::string to_string(const LEX_STRING &str) {
  return std::string(str.str, str.length);
}

static inline std::string to_string(const LEX_CSTRING &str) {
  return std::string(str.str, str.length);
}

#endif  // defined(__cplusplus)

#endif
