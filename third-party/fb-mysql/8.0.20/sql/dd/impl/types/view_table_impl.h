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

#ifndef DD__VIEW_TABLE_IMPL_INCLUDED
#define DD__VIEW_TABLE_IMPL_INCLUDED

#include <sys/types.h>
#include <new>

#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/weak_object_impl.h"  // dd::Weak_object_impl
#include "sql/dd/string_type.h"
#include "sql/dd/types/view_table.h"  // dd::View_table

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_key;
class Object_table;
class Open_dictionary_tables_ctx;
class View;
class View_impl;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class View_table_impl : public Weak_object_impl, public View_table {
 public:
  View_table_impl();

  View_table_impl(View_impl *view);

  View_table_impl(const View_table_impl &src, View_impl *parent);

  virtual ~View_table_impl() {}

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

  virtual void debug_print(String_type &outb) const;

  void set_ordinal_position(uint) {}

  virtual uint ordinal_position() const { return -1; }

 public:
  /////////////////////////////////////////////////////////////////////////
  // table_catalog.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_catalog() const { return m_table_catalog; }

  virtual void set_table_catalog(const String_type &table_catalog) {
    m_table_catalog = table_catalog;
  }

  /////////////////////////////////////////////////////////////////////////
  // table_schema.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_schema() const { return m_table_schema; }

  virtual void set_table_schema(const String_type &table_schema) {
    m_table_schema = table_schema;
  }

  /////////////////////////////////////////////////////////////////////////
  // table_name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_name() const { return m_table_name; }

  virtual void set_table_name(const String_type &table_name) {
    m_table_name = table_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // view.
  /////////////////////////////////////////////////////////////////////////

  virtual const View &view() const;

  virtual View &view();

 public:
  static View_table_impl *restore_item(View_impl *view) {
    return new (std::nothrow) View_table_impl(view);
  }

  static View_table_impl *clone(const View_table_impl &other, View_impl *view) {
    return new (std::nothrow) View_table_impl(other, view);
  }

 public:
  virtual Object_key *create_primary_key() const;
  virtual bool has_new_primary_key() const;

 private:
  String_type m_table_catalog;
  String_type m_table_schema;
  String_type m_table_name;

  // References to other objects
  View_impl *m_view;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__VIEW_TABLE_IMPL_INCLUDED
