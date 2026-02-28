/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DEFAULT_VALUES_INCLUDED
#define DEFAULT_VALUES_INCLUDED

#include <stddef.h>

#include "my_inttypes.h"

/** Forward declarations. */

class Create_field;
class THD;
template <class T>
class List;

namespace dd {
class Column;
class Table;
}  // namespace dd
struct TABLE;
struct TABLE_SHARE;

/**
  Find the largest field among a list of create fields.

  Iterates over the list of create fields and find the largest one, i.e.
  the field with largest pack length.

  @param   create_fields    List of create fields.
  @return                   Size of the largest field, as number of bytes.
*/

size_t max_pack_length(const List<Create_field> &create_fields);

/**
  Prepare the default value of a single column.

  This function creates a fake field using the submitted fake table
  object, which also has assigned a fake table share. The field is
  create in order to use existing code to store the default value. The
  value is stored into the submitted buffer, which has been allocated
  in advance (and will be deleted afterwards). The fake table and
  share are required by the field infrastructure.

  After storing the default value into the buffer, the value is read back
  and copied into the column object's default value field. For bit fields
  with leftover bits in the preamble, these bits are copied and appended
  onto the default value as the last byte.

  @param          thd       Thread context.
  @param          buf       Buffer to store the default value into. The buffer
                            size must be at least two bytes.
  @param          table     Fake table to use when storing the default value.
  @param          field     Create_field corresponding to the column object.
  @param [in,out] col_obj   Column for which to prepare the default value.

  @retval         true      Failure.
  @retval         false     Success.
*/

bool prepare_default_value(THD *thd, uchar *buf, TABLE *table,
                           const Create_field &field, dd::Column *col_obj);

/**
  Prepare the default value buffer for an empty record.

  This function prepares the buffer based on the objects retrieved from the
  data dictionary. The function will scan the columns of the submitted table
  objects and calculate the length of the record, the number of null bits,
  etc. The empty buffer is allocated, and all bits are set to 0, including
  used null bits, the actual field values, etc. Afterwards, the unused bits
  in the preamble, up to the next full byte border, are set to 1. Finally,
  the buffer is assigned to the 'TABLE_SHARE::default_values' field.

 @note This function does not fill in the actual default values, it just
        allocates and prepares the buffer.

  @param          thd          Thread context.
  @param          table        Table for which the default value buffer should
                               be prepared.
  @param [in,out] share        Table share for the table. The default value
                               buffer is assigned to the appropriate table
                               share field.

  @retval         true         Failure.
  @retval         false        Success.
*/

bool prepare_default_value_buffer_and_table_share(THD *thd,
                                                  const dd::Table &table,
                                                  TABLE_SHARE *share);

#endif /* DEFAULT_VALUES_INCLUDED */
