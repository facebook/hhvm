/*
   Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

/*
  Char classes for lexical scanners
*/

#ifndef SQL_LEX_CHARS_INCLUDED
#define SQL_LEX_CHARS_INCLUDED

#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_macros.h"

struct CHARSET_INFO;

enum MY_ATTRIBUTE((__packed__)) my_lex_states {
  MY_LEX_START,
  MY_LEX_CHAR,
  MY_LEX_IDENT,
  MY_LEX_IDENT_SEP,
  MY_LEX_IDENT_START,
  MY_LEX_REAL,
  MY_LEX_HEX_NUMBER,
  MY_LEX_BIN_NUMBER,
  MY_LEX_CMP_OP,
  MY_LEX_LONG_CMP_OP,
  MY_LEX_STRING,
  MY_LEX_COMMENT,
  MY_LEX_END,
  MY_LEX_NUMBER_IDENT,
  MY_LEX_INT_OR_REAL,
  MY_LEX_REAL_OR_POINT,
  MY_LEX_BOOL,
  MY_LEX_EOL,
  MY_LEX_LONG_COMMENT,
  MY_LEX_END_LONG_COMMENT,
  MY_LEX_SEMICOLON,
  MY_LEX_SET_VAR,
  MY_LEX_USER_END,
  MY_LEX_HOSTNAME,
  MY_LEX_SKIP,
  MY_LEX_USER_VARIABLE_DELIMITER,
  MY_LEX_SYSTEM_VAR,
  MY_LEX_IDENT_OR_KEYWORD,
  MY_LEX_IDENT_OR_HEX,
  MY_LEX_IDENT_OR_BIN,
  MY_LEX_IDENT_OR_NCHAR,
  MY_LEX_STRING_OR_DELIMITER
};

enum MY_ATTRIBUTE((__packed__)) hint_lex_char_classes {
  HINT_CHR_ASTERISK,     // [*]
  HINT_CHR_AT,           // [@]
  HINT_CHR_BACKQUOTE,    // [`]
  HINT_CHR_CHAR,         // default state
  HINT_CHR_DIGIT,        // [[:digit:]]
  HINT_CHR_DOUBLEQUOTE,  // ["]
  HINT_CHR_EOF,          // pseudo-class
  HINT_CHR_IDENT,        // [_$[:alpha:]]
  HINT_CHR_MB,           // multibyte character
  HINT_CHR_NL,           // \n
  HINT_CHR_QUOTE,        // [']
  HINT_CHR_SLASH,        // [/]
  HINT_CHR_SPACE         // [[:space:]] excluding \n
};

struct lex_state_maps_st {
  enum my_lex_states main_map[256];
  enum hint_lex_char_classes hint_map[256];
};

typedef struct lex_state_maps_st lex_state_maps_st;

bool init_state_maps(CHARSET_INFO *cs);

#endif /* SQL_LEX_CHARS_INCLUDED */
