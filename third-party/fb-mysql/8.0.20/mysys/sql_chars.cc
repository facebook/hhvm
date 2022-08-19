/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql_chars.h"

#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"

static void hint_lex_init_maps(CHARSET_INFO *cs,
                               enum hint_lex_char_classes *hint_map) {
  size_t i;
  for (i = 0; i < 256; i++) {
    if (my_ismb1st(cs, i))
      hint_map[i] = HINT_CHR_MB;
    else if (my_isalpha(cs, i))
      hint_map[i] = HINT_CHR_IDENT;
    else if (my_isdigit(cs, i))
      hint_map[i] = HINT_CHR_DIGIT;
    else if (my_isspace(cs, i)) {
      DBUG_ASSERT(!my_ismb1st(cs, i));
      hint_map[i] = HINT_CHR_SPACE;
    } else
      hint_map[i] = HINT_CHR_CHAR;
  }
  hint_map[(uchar)'*'] = HINT_CHR_ASTERISK;
  hint_map[(uchar)'@'] = HINT_CHR_AT;
  hint_map[(uchar)'`'] = HINT_CHR_BACKQUOTE;
  hint_map[(uchar)'"'] = HINT_CHR_DOUBLEQUOTE;
  hint_map[(uchar)'_'] = HINT_CHR_IDENT;
  hint_map[(uchar)'$'] = HINT_CHR_IDENT;
  hint_map[(uchar)'/'] = HINT_CHR_SLASH;
  hint_map[(uchar)'\n'] = HINT_CHR_NL;
  hint_map[(uchar)'\''] = HINT_CHR_QUOTE;
}

bool init_state_maps(CHARSET_INFO *cs) {
  uint i;
  uchar *ident_map;
  enum my_lex_states *state_map = nullptr;

  lex_state_maps_st *lex_state_maps = (lex_state_maps_st *)my_once_alloc(
      sizeof(lex_state_maps_st), MYF(MY_WME));

  if (lex_state_maps == nullptr) return true;  // OOM

  cs->state_maps = lex_state_maps;
  state_map = lex_state_maps->main_map;

  if (!(cs->ident_map = ident_map = (uchar *)my_once_alloc(256, MYF(MY_WME))))
    return true;  // OOM

  hint_lex_init_maps(cs, lex_state_maps->hint_map);

  /* Fill state_map with states to get a faster parser */
  for (i = 0; i < 256; i++) {
    if (my_isalpha(cs, i))
      state_map[i] = MY_LEX_IDENT;
    else if (my_isdigit(cs, i))
      state_map[i] = MY_LEX_NUMBER_IDENT;
    else if (my_ismb1st(cs, i))
      /* To get whether it's a possible leading byte for a charset. */
      state_map[i] = MY_LEX_IDENT;
    else if (my_isspace(cs, i))
      state_map[i] = MY_LEX_SKIP;
    else
      state_map[i] = MY_LEX_CHAR;
  }
  state_map[(uchar)'_'] = state_map[(uchar)'$'] = MY_LEX_IDENT;
  state_map[(uchar)'\''] = MY_LEX_STRING;
  state_map[(uchar)'.'] = MY_LEX_REAL_OR_POINT;
  state_map[(uchar)'>'] = state_map[(uchar)'='] = state_map[(uchar)'!'] =
      MY_LEX_CMP_OP;
  state_map[(uchar)'<'] = MY_LEX_LONG_CMP_OP;
  state_map[(uchar)'&'] = state_map[(uchar)'|'] = MY_LEX_BOOL;
  state_map[(uchar)'#'] = MY_LEX_COMMENT;
  state_map[(uchar)';'] = MY_LEX_SEMICOLON;
  state_map[(uchar)':'] = MY_LEX_SET_VAR;
  state_map[0] = MY_LEX_EOL;
  state_map[(uchar)'/'] = MY_LEX_LONG_COMMENT;
  state_map[(uchar)'*'] = MY_LEX_END_LONG_COMMENT;
  state_map[(uchar)'@'] = MY_LEX_USER_END;
  state_map[(uchar)'`'] = MY_LEX_USER_VARIABLE_DELIMITER;
  state_map[(uchar)'"'] = MY_LEX_STRING_OR_DELIMITER;

  /*
    Create a second map to make it faster to find identifiers
  */
  for (i = 0; i < 256; i++) {
    ident_map[i] = (uchar)(state_map[i] == MY_LEX_IDENT ||
                           state_map[i] == MY_LEX_NUMBER_IDENT);
  }

  /* Special handling of hex and binary strings */
  state_map[(uchar)'x'] = state_map[(uchar)'X'] = MY_LEX_IDENT_OR_HEX;
  state_map[(uchar)'b'] = state_map[(uchar)'B'] = MY_LEX_IDENT_OR_BIN;
  state_map[(uchar)'n'] = state_map[(uchar)'N'] = MY_LEX_IDENT_OR_NCHAR;

  return false;
}
