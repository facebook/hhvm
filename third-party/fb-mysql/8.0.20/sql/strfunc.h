/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef STRFUNC_INCLUDED
#define STRFUNC_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <cstring>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_inttypes.h"
#include "mysql/mysql_lex_string.h"  // MYSQL_LEX_CSTRING

class THD;
struct MEM_ROOT;
struct TYPELIB;

ulonglong find_set(const TYPELIB *lib, const char *x, size_t length,
                   const CHARSET_INFO *cs, const char **err_pos, uint *err_len,
                   bool *set_warning);
uint find_type(const TYPELIB *lib, const char *find, size_t length,
               bool part_match);
uint find_type2(const TYPELIB *lib, const char *find, size_t length,
                const CHARSET_INFO *cs);
uint check_word(TYPELIB *lib, const char *val, const char *end,
                const char **end_of_word);
char *flagset_to_string(THD *thd, LEX_STRING *result, ulonglong set,
                        const char *lib[]);
char *set_to_string(THD *thd, LEX_STRING *result, ulonglong set,
                    const char *lib[]);
char *set_to_string(THD *thd, LEX_STRING *result, ulonglong set,
                    const char *lib[], bool quoted);

size_t strconvert(const CHARSET_INFO *from_cs, const char *from,
                  CHARSET_INFO *to_cs, char *to, size_t to_length,
                  uint *errors);

/**
  convert a hex digit into number.
*/

inline int hexchar_to_int(char c) {
  if (c <= '9' && c >= '0') return c - '0';
  c |= 32;
  if (c <= 'f' && c >= 'a') return c - 'a' + 10;
  return -1;
}

/**
  Return a LEX_CSTRING handle to a std::string like (meaning someting
  which has the c_str() and length() member functions). Note that the
  std::string-like object retains ownership of the character array,
  and consquently the returned LEX_CSTRING is only valid as long as the
  std::string-like object is valid.

  @param s std::string-like object

  @return LEX_CSTRING handle to string
*/
template <class STDSTRINGLIKE_TYPE>
MYSQL_LEX_CSTRING lex_cstring_handle(const STDSTRINGLIKE_TYPE &s) {
  return {s.c_str(), s.length()};
}

/**
  Lowercase a string according to charset.

  @param ci pointer to charset for conversion
  @param s string to lower-case
  @retval modified argument if r-value
  @retval copy of modified argument if lvalue (meaningless, don't use)
 */
template <class STRLIKE_TYPE>
STRLIKE_TYPE casedn(const CHARSET_INFO *ci, STRLIKE_TYPE &&s) {
  s.resize(ci->casedn_multiply * s.size());
  s.resize(my_casedn_str(ci, &s.front()));
  return std::forward<STRLIKE_TYPE>(s);
}

/**
  Lowercase a string according to charset. Overload for const T& which
  copies argument and forwards to T&& overload.

  @param ci pointer to charset for conversion
  @param src string to lower-case
  @retval modified copy of argument
 */

template <class STRLIKE_TYPE>
STRLIKE_TYPE casedn(const CHARSET_INFO *ci, const STRLIKE_TYPE &src) {
  return casedn(ci, STRLIKE_TYPE{src});
}

/**
  Create a LEX_STRING in a MEM_ROOT and copy the given string
  into it.

  @param mem_root MEM_ROOT where to allocate the LEX_STRING.
  @param str      string to be copied into the LEX_STRING.
  @param length   length of str, in bytes

  @return  nullptr on failure, or pointer to the LEX_STRING object
*/
LEX_STRING *make_lex_string_root(MEM_ROOT *mem_root, const char *str,
                                 size_t length);

/**
  Copy the given string into a LEX_STRING, allocating it in the
  given MEM_ROOT.

  @param mem_root MEM_ROOT where to allocate the string.
  @param lex_str  LEX_STRING to fill with the copied string.
  @param str      string to be copied into the LEX_STRING.
  @param length   length of str, in bytes

  @return  true on failure (OOM), false otherwise.
*/
bool lex_string_strmake(MEM_ROOT *mem_root, LEX_STRING *lex_str,
                        const char *str, size_t length);

/**
  Copy the given string into a LEX_CSTRING, allocating it in the
  given MEM_ROOT.

  @param mem_root MEM_ROOT where to allocate the string.
  @param lex_str  LEX_CSTRING to fill with the copied string.
  @param str      string to be copied into the LEX_CSTRING.
  @param length   length of str, in bytes

  @return  true on failure (OOM), false otherwise.
*/
bool lex_string_strmake(MEM_ROOT *mem_root, LEX_CSTRING *lex_str,
                        const char *str, size_t length);

#endif /* STRFUNC_INCLUDED */
