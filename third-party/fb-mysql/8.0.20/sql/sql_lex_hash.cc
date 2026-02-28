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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/sql_lex_hash.h"

#include <stddef.h>
#include <sys/types.h>

#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "sql/lex.h"
#include "sql/lex_hash.h"
#include "sql/lex_symbol.h"

const Lex_hash Lex_hash::sql_keywords(sql_keywords_map, sql_keywords_max_len);
const Lex_hash Lex_hash::sql_keywords_and_funcs(sql_keywords_and_funcs_map,
                                                sql_keywords_and_funcs_max_len);

const Lex_hash Lex_hash::hint_keywords(hint_keywords_map,
                                       hint_keywords_max_len);

/*
  The following data is based on the latin1 character set, and is only
  used when comparing keywords
*/

static const uchar to_upper_lex[] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  65,  66,  67,  68,  69,  70,  71,  72,
    73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88,  89,  90,  123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222,
    255};

inline int lex_casecmp(const char *s, const char *t, uint len) {
  while (len-- != 0 && to_upper_lex[(uchar)*s++] == to_upper_lex[(uchar)*t++])
    ;
  return (int)len + 1;
}

const SYMBOL *Lex_hash::get_hash_symbol(const char *s, unsigned int len) const {
  const char *cur_str = s;

  if (len == 0) {
    DBUG_PRINT("warning",
               ("get_hash_symbol() received a request for a zero-length symbol,"
                " which is probably a mistake."));
    return nullptr;
  }

  if (len > entry_max_len) return nullptr;

  uint32 cur_struct = uint4korr(hash_map + ((len - 1) * 4));

  for (;;) {
    uchar first_char = (uchar)cur_struct;

    if (first_char == 0) {
      uint16 ires = (uint16)(cur_struct >> 16);
      if (ires == array_elements(symbols)) return nullptr;
      const SYMBOL *res = symbols + ires;
      uint count = (uint)(cur_str - s);
      return lex_casecmp(cur_str, res->name + count, len - count) ? nullptr
                                                                  : res;
    }

    uchar cur_char = (uchar)to_upper_lex[(uchar)*cur_str];
    if (cur_char < first_char) return nullptr;
    cur_struct >>= 8;
    if (cur_char > (uchar)cur_struct) return nullptr;

    cur_struct >>= 8;
    cur_struct = uint4korr(hash_map +
                           (((uint16)cur_struct + cur_char - first_char) * 4));
    cur_str++;
  }
}
