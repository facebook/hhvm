/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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
  Functions to read and parse geometrical data.
  NOTE: These functions assumes that the string is end \0 terminated!
*/

#include "sql/gstream.h"

#include <string.h>
#include <sys/types.h>

#include "m_string.h"  // my_stpcpy
#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/psi_memory_key.h"

static inline bool is_numeric_beginning(const char *pc, const size_t len) {
  return (pc != nullptr &&
          ((*pc >= '0' && *pc <= '9') || *pc == '-' || *pc == '+' ||
           (*pc == '.' && len > 1 && pc[1] >= '0' && pc[1] <= '9')));
}

enum Gis_read_stream::enum_tok_types Gis_read_stream::get_next_toc_type() {
  skip_space();
  if (m_cur >= m_limit) return eostream;
  if (my_isvar_start(&my_charset_bin, *m_cur)) return word;
  if (is_numeric_beginning(m_cur, m_limit - m_cur)) return numeric;
  if (*m_cur == '(') return l_bra;
  if (*m_cur == ')') return r_bra;
  if (*m_cur == ',') return comma;
  return unknown;
}

bool Gis_read_stream::get_next_word(LEX_CSTRING *res) {
  skip_space();
  res->str = m_cur;
  /* The following will also test for \0 */
  if ((m_cur >= m_limit) || !my_isvar_start(&my_charset_bin, *m_cur))
    return true;

  /*
    We can't combine the following increment with my_isvar() because
    my_isvar() is a macro that would cause side effects
  */
  m_cur++;
  while ((m_cur < m_limit) && my_isvar(&my_charset_bin, *m_cur)) m_cur++;

  res->length = (uint32)(m_cur - res->str);
  return false;
}

/* Read a floating point number. */
bool Gis_read_stream::get_next_number(double *d) {
  const char *endptr;
  int err;

  skip_space();

  if ((m_cur >= m_limit) || !is_numeric_beginning(m_cur, m_limit - m_cur)) {
    set_error_msg("Numeric constant expected");
    return true;
  }

  *d = my_strntod(m_charset, m_cur, (uint)(m_limit - m_cur), &endptr, &err);
  if (err) return true;
  if (endptr) m_cur = endptr;
  return false;
}

bool Gis_read_stream::check_next_symbol(char symbol) {
  skip_space();
  if ((m_cur >= m_limit) || (*m_cur != symbol)) {
    char buff[32];
    my_stpcpy(buff, "'?' expected");
    buff[2] = symbol;
    set_error_msg(buff);
    return true;
  }
  m_cur++;
  return false;
}

/*
  Remember error message.
*/

void Gis_read_stream::set_error_msg(const char *msg) {
  size_t len = strlen(msg);  // ok in this context
  m_err_msg = (char *)my_realloc(key_memory_Gis_read_stream_err_msg, m_err_msg,
                                 (uint)len + 1, MYF(MY_ALLOW_ZERO_PTR));
  memcpy(m_err_msg, msg, len + 1);
}
