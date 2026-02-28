/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__INDEX_STAT_INCLUDED
#define DD__INDEX_STAT_INCLUDED

#include "sql/dd/types/entity_object.h"        // Entity_object
#include "sql/dd/types/entity_object_table.h"  // Entity_object_table

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Composite_4char_key;
class Index_stat_impl;

namespace tables {
class Index_stats;
}

///////////////////////////////////////////////////////////////////////////

class Index_stat : virtual public Entity_object {
 public:
  typedef Index_stat_impl Impl;
  typedef tables::Index_stats DD_table;
  typedef Composite_4char_key Name_key;

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
  // index name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &index_name() const = 0;
  virtual void set_index_name(const String_type &index_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // column name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &column_name() const = 0;
  virtual void set_column_name(const String_type &column_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // cardinality.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong cardinality() const = 0;
  virtual void set_cardinality(ulonglong cardinality) = 0;

  /////////////////////////////////////////////////////////////////////////
  // cached_time.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong cached_time() const = 0;
  virtual void set_cached_time(ulonglong cached_time) = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__INDEX_STAT_INCLUDED
