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

#ifndef DD__TABLE_STAT_INCLUDED
#define DD__TABLE_STAT_INCLUDED

#include "sql/dd/types/entity_object.h"        // dd::Entity_object
#include "sql/dd/types/entity_object_table.h"  // Entity_object_table

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Composite_char_key;
class Table_stat_impl;

namespace tables {
class Table_stats;
}

///////////////////////////////////////////////////////////////////////////

class Table_stat : virtual public Entity_object {
 public:
  typedef Table_stat_impl Impl;
  typedef tables::Table_stats DD_table;
  typedef Composite_char_key Name_key;

 public:
  /////////////////////////////////////////////////////////////////////////
  // schema name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &schema_name() const = 0;
  virtual void set_schema_name(const String_type &schema_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // table name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_name() const = 0;
  virtual void set_table_name(const String_type &table_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // table_rows.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong table_rows() const = 0;
  virtual void set_table_rows(ulonglong table_rows) = 0;

  /////////////////////////////////////////////////////////////////////////
  // avg_row_length.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong avg_row_length() const = 0;
  virtual void set_avg_row_length(ulonglong avg_row_length) = 0;

  /////////////////////////////////////////////////////////////////////////
  // data_length.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong data_length() const = 0;
  virtual void set_data_length(ulonglong data_length) = 0;

  /////////////////////////////////////////////////////////////////////////
  // max_data_length.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong max_data_length() const = 0;
  virtual void set_max_data_length(ulonglong max_data_length) = 0;

  /////////////////////////////////////////////////////////////////////////
  // index_length.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong index_length() const = 0;
  virtual void set_index_length(ulonglong index_length) = 0;

  /////////////////////////////////////////////////////////////////////////
  // data_free.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong data_free() const = 0;
  virtual void set_data_free(ulonglong data_free) = 0;

  /////////////////////////////////////////////////////////////////////////
  // auto_increment.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong auto_increment() const = 0;
  virtual void set_auto_increment(ulonglong auto_increment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // checksum.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong checksum() const = 0;
  virtual void set_checksum(ulonglong checksum) = 0;

  /////////////////////////////////////////////////////////////////////////
  // update_time.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong update_time() const = 0;
  virtual void set_update_time(ulonglong update_time) = 0;

  /////////////////////////////////////////////////////////////////////////
  // check_time.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong check_time() const = 0;
  virtual void set_check_time(ulonglong check_time) = 0;

  /////////////////////////////////////////////////////////////////////////
  // cached_time.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong cached_time() const = 0;
  virtual void set_cached_time(ulonglong cached_time) = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__TABLE_STAT_INCLUDED
