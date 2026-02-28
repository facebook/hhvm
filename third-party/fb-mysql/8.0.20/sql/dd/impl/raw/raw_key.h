/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__RAW_KEY_INCLUDED
#define DD__RAW_KEY_INCLUDED

#include "my_base.h"        // key_part_map
#include "sql/sql_const.h"  // MAX_KEY_LENGTH

namespace dd {

///////////////////////////////////////////////////////////////////////////

struct Raw_key {
  uchar key[MAX_KEY_LENGTH];
  int index_no;
  int key_len;

  key_part_map keypart_map;

  Raw_key(int p_index_no, int p_key_len, key_part_map p_keypart_map)
      : index_no(p_index_no), key_len(p_key_len), keypart_map(p_keypart_map) {}
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__RAW_KEY_INCLUDED
