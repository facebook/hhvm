/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LEXER_YYSTYPE_INCLUDED
#define LEXER_YYSTYPE_INCLUDED

#include "lex_string.h"
#include "sql/lex_symbol.h"

class PT_hint_list;
struct CHARSET_INFO;

union Lexer_yystype {
  LEX_STRING lex_str;
  LEX_SYMBOL keyword;
  const CHARSET_INFO *charset;
  PT_hint_list *optimizer_hints;
  LEX_CSTRING hint_string;
};

#endif  // LEXER_YYSTYPE_INCLUDED
