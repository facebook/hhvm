#ifndef GSTREAM_INCLUDED
#define GSTREAM_INCLUDED

/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>

#include "lex_string.h"
#include "m_ctype.h" /* my_charset_latin1, my_charset_bin */
#include "m_string.h"
#include "mysql/service_mysql_alloc.h"

class THD;

class Gis_read_stream {
 public:
  enum enum_tok_types { unknown, eostream, word, numeric, l_bra, r_bra, comma };

  Gis_read_stream(THD *thd, const CHARSET_INFO *charset, const char *buffer,
                  int size)
      : m_cur(buffer),
        m_limit(buffer + size),
        m_err_msg(nullptr),
        m_charset(charset),
        m_thd(thd) {}
  Gis_read_stream() : m_cur(NullS), m_limit(NullS), m_err_msg(NullS) {}
  ~Gis_read_stream() { my_free(m_err_msg); }

  enum enum_tok_types get_next_toc_type();
  bool get_next_word(LEX_CSTRING *);
  bool get_next_number(double *);
  bool check_next_symbol(char);

  bool is_end_of_stream() { return get_next_toc_type() == eostream; }

  inline void skip_space() {
    while ((m_cur < m_limit) && my_isspace(&my_charset_latin1, *m_cur)) m_cur++;
  }
  /* Skip next character, if match. Return 1 if no match */
  inline bool skip_char(char skip) {
    skip_space();
    if ((m_cur >= m_limit) || *m_cur != skip)
      return true; /* Didn't find char */
    m_cur++;
    return false;
  }
  void set_error_msg(const char *msg);

  // caller should free this pointer
  char *get_error_msg() {
    char *err_msg = m_err_msg;
    m_err_msg = NullS;
    return err_msg;
  }

  THD *thd() { return m_thd; }

 protected:
  const char *m_cur;
  const char *m_limit;
  char *m_err_msg;
  const CHARSET_INFO *m_charset;

 private:
  THD *m_thd;
};

#endif /* GSTREAM_INCLUDED */
