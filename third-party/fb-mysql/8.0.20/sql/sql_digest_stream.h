/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_DIGEST_STREAM_H
#define SQL_DIGEST_STREAM_H

#include "sql/sql_digest.h"

union Lexer_yystype;

/**
  State data storage for @c digest_start, @c digest_add_token.
  This structure extends the @c sql_digest_storage structure
  with temporary state used only during parsing.
*/
struct sql_digest_state {
  /**
    Index, in the digest token array, of the last identifier seen.
    Reduce rules used in the digest computation can not
    apply to tokens seen before an identifier.
    @sa digest_add_token
  */
  int m_last_id_index;
  sql_digest_storage m_digest_storage;

  inline void reset(unsigned char *token_array, uint length) {
    m_last_id_index = 0;
    m_digest_storage.reset(token_array, length);
  }

  inline bool is_empty() { return m_digest_storage.is_empty(); }
};
typedef struct sql_digest_state sql_digest_state;

sql_digest_state *digest_add_token(sql_digest_state *state, uint token,
                                   Lexer_yystype *yylval);

sql_digest_state *digest_reduce_token(sql_digest_state *state, uint token_left,
                                      uint token_right);

#endif
