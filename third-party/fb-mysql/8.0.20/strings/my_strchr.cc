/* Copyright (c) 2005, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"

/**
  Calculate the length of the initial segment of 'str' which consists
  entirely of characters not in 'reject'.

  @param  cs              Pointer to charset info.
  @param  str             Pointer to multi-byte string.
  @param  str_end         Pointer to end of multi-byte string.
  @param  reject          Pointer to start of single-byte reject string.
  @param  reject_length   Length of single-byte reject string.

  @return Length of segment of multi-byte string that doesn't contain
          any character of the single-byte reject string or zero if an
          invalid encoding of a character of the multi-byte string is
          found.

  @note The reject string points to single-byte characters so it is
  only possible to find the first occurrence of a single-byte
  character.  Multi-byte characters in 'str' are treated as not
  matching any character in the reject string.
  This method returns zero if an invalid encoding of any character
  in the string 'str' using charset 'cs' is found.

  @todo should be moved to CHARSET_INFO if it's going to be called
  frequently.

  @internal The implementation builds on the assumption that 'str' is long,
  while 'reject' is short. So it compares each character in string
  with the characters in 'reject' in a tight loop over the characters
  in 'reject'.
*/

size_t my_strcspn(const CHARSET_INFO *cs, const char *str, const char *str_end,
                  const char *reject, size_t reject_length) {
  const char *ptr_str, *ptr_reject;
  const char *reject_end = reject + reject_length;
  uint mbl = 0;

  for (ptr_str = str; ptr_str < str_end; ptr_str += mbl) {
    mbl = my_mbcharlen_ptr(cs, ptr_str, str_end);

    if (mbl == 0) return 0;

    if (mbl == 1) {
      for (ptr_reject = reject; ptr_reject < reject_end; ++ptr_reject) {
        if (*ptr_reject == *ptr_str) return (size_t)(ptr_str - str);
      }
    }
  }
  return (size_t)(ptr_str - str);
}
