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

#ifndef DD__INDEX_STAT_IMPL_INCLUDED
#define DD__INDEX_STAT_IMPL_INCLUDED

#include <memory>
#include <new>
#include <string>

#include "my_inttypes.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/entity_object_table.h"
#include "sql/dd/types/index_stat.h"  // dd::Index_stat

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Charset;
class Object_key;
class Open_dictionary_tables_ctx;
class Raw_table;
class Weak_object;
class Object_table;

///////////////////////////////////////////////////////////////////////////

class Index_stat_impl : public Entity_object_impl, public Index_stat {
 public:
  Index_stat_impl() : m_cardinality(0), m_cached_time(0) {}

 public:
  virtual void debug_print(String_type &outb) const;

  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);
  virtual bool store_attributes(Raw_record *r);

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // schema name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &schema_name() const { return m_schema_name; }

  virtual void set_schema_name(const String_type &schema_name) {
    m_schema_name = schema_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // table name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_name() const { return m_table_name; }

  virtual void set_table_name(const String_type &table_name) {
    m_table_name = table_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // index name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &index_name() const { return m_index_name; }

  virtual void set_index_name(const String_type &index_name) {
    m_index_name = index_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // column name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &column_name() const { return m_column_name; }

  virtual void set_column_name(const String_type &column_name) {
    m_column_name = column_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // cardinality.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong cardinality() const { return m_cardinality; }

  virtual void set_cardinality(ulonglong cardinality) {
    m_cardinality = cardinality;
  }

  /////////////////////////////////////////////////////////////////////////
  // cached_time.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong cached_time() const { return m_cached_time; }

  virtual void set_cached_time(ulonglong cached_time) {
    m_cached_time = cached_time;
  }

 public:
  virtual Object_key *create_primary_key() const;
  virtual bool has_new_primary_key() const;

  // Fix "inherits ... via dominance" warnings
  virtual Entity_object_impl *impl() { return Entity_object_impl::impl(); }
  virtual const Entity_object_impl *impl() const {
    return Entity_object_impl::impl();
  }
  virtual Object_id id() const { return Entity_object_impl::id(); }
  virtual bool is_persistent() const {
    return Entity_object_impl::is_persistent();
  }
  virtual const String_type &name() const { return Entity_object_impl::name(); }
  virtual void set_name(const String_type &name) {
    Entity_object_impl::set_name(name);
  }

 private:
  // Fields
  String_type m_schema_name;
  String_type m_table_name;
  String_type m_index_name;
  String_type m_column_name;

  ulonglong m_cardinality;
  ulonglong m_cached_time;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__INDEX_STAT_IMPL_INCLUDED
