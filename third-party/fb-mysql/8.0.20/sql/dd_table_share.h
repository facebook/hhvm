#ifndef DD_TABLE_SHARE_INCLUDED
#define DD_TABLE_SHARE_INCLUDED
/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <sys/types.h>

#include "m_ctype.h"
#include "my_inttypes.h"
#include "my_sys.h"            // get_charset
#include "sql/dd/object_id.h"  // dd::Object_id

class Field;
class KEY_PART_INFO;
class THD;
struct TABLE_SHARE;
enum enum_field_types : int;

namespace dd {
class Table;

enum class enum_column_types;
}  // namespace dd

/**
  Read the table definition from the data-dictionary.

  @param thd        Thread handler
  @param share      Fill this with table definition
  @param table_def  A data-dictionary Table-object describing
                    table to be used for opening, instead of reading
                    information from DD.

  @note
    This function is called when the table definition is not cached in
    table_def_cache.
    The data is returned in 'share', which is alloced by
    alloc_table_share().. The code assumes that share is initialized.

  @returns
   false   OK
   true    Error
*/
bool open_table_def(THD *thd, TABLE_SHARE *share, const dd::Table &table_def);

/**
  Read the table definition from the data-dictionary.

  @param thd        Thread handler
  @param share      Fill this with table definition
  @param table_def  A data-dictionary Table-object describing
                    table to be used for opening.

  @note
    This function is called from InnoDB, and will suppress errors
    due to:
      Invalid collations.
      Missing FTS parser.

  @returns
   false   OK
   true    Error
*/
bool open_table_def_suppress_invalid_meta_data(THD *thd, TABLE_SHARE *share,
                                               const dd::Table &table_def);

/* Map from new to old field type. */
enum_field_types dd_get_old_field_type(dd::enum_column_types type);

static inline CHARSET_INFO *dd_get_mysql_charset(dd::Object_id dd_cs_id) {
  return get_charset(static_cast<uint>(dd_cs_id), MYF(0));
}

/**
  Check if the given key_part is suitable to be promoted as part of
  primary key.

  @param key_part    - pointer to KEY_PART_INTO which we are checking.
  @param table_field - Pointer to Field of column used by key_part.

  @returns
   true  - Is suitable for primary key.
   false - if not.
*/
bool is_suitable_for_primary_key(KEY_PART_INFO *key_part, Field *table_field);

#endif  // DD_TABLE_SHARE_INCLUDED
