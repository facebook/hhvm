/*
   Copyright (c) 2015, 2016, Oracle and/or its affiliates. All rights reserved.

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
  Hash accessor function for lexical scanners:
  * sql_lex.h, sql_lex.cc,
  * sql_lex_hints.h, sql_lex_hints.cc.
*/

#ifndef SQL_LEX_HASH_INCLUDED
#define SQL_LEX_HASH_INCLUDED

class Lex_hash {
 private:
  const unsigned char *hash_map;
  const unsigned int entry_max_len;

 public:
  Lex_hash(const unsigned char *hash_map_arg, unsigned int entry_max_len_arg)
      : hash_map(hash_map_arg), entry_max_len(entry_max_len_arg) {}

  const struct SYMBOL *get_hash_symbol(const char *s, unsigned int len) const;

  static const Lex_hash sql_keywords;
  static const Lex_hash sql_keywords_and_funcs;

  static const Lex_hash hint_keywords;
};

#endif /* SQL_LEX_HASH_INCLUDED */
