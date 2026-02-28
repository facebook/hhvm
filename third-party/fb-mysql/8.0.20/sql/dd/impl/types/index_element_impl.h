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

#ifndef DD__INDEX_ELEMENT_IMPL_INCLUDED
#define DD__INDEX_ELEMENT_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <new>

#include "sql/dd/impl/types/index_impl.h"        // dd::Index_impl
#include "sql/dd/impl/types/weak_object_impl.h"  // dd::Weak_object_impl
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/index_element.h"  // dd::Index_element

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Index;
class Object_key;
class Object_table;
class Open_dictionary_tables_ctx;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class Index_element_impl : public Weak_object_impl, public Index_element {
 public:
  Index_element_impl()
      : m_ordinal_position(0),
        m_length(-1),
        m_order(Index_element::ORDER_ASC),
        m_hidden(false),
        m_index(nullptr),
        m_column(nullptr) {}

  Index_element_impl(Index_impl *index, Column *column)
      : m_ordinal_position(0),
        m_length(-1),
        m_order(Index_element::ORDER_ASC),
        m_hidden(false),
        m_index(index),
        m_column(column) {}

  Index_element_impl(const Index_element_impl &src, Index_impl *parent,
                     Column *column);

  virtual ~Index_element_impl() {}

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  void set_ordinal_position(uint ordinal_position) {
    m_ordinal_position = ordinal_position;
  }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // index.
  /////////////////////////////////////////////////////////////////////////

  virtual const Index &index() const { return *m_index; }

  virtual Index &index() { return *m_index; }

  /////////////////////////////////////////////////////////////////////////
  // column.
  /////////////////////////////////////////////////////////////////////////

  virtual const Column &column() const { return *m_column; }

  virtual Column &column() { return *m_column; }

  /////////////////////////////////////////////////////////////////////////
  // ordinal_position.
  /////////////////////////////////////////////////////////////////////////

  virtual uint ordinal_position() const { return m_ordinal_position; }

  /////////////////////////////////////////////////////////////////////////
  // length.
  /////////////////////////////////////////////////////////////////////////

  virtual uint length() const { return m_length; }

  virtual void set_length(uint length) { m_length = length; }

  virtual void set_length_null(bool) { m_length = (uint)-1; }

  virtual bool is_length_null() const { return m_length == (uint)-1; }

  /////////////////////////////////////////////////////////////////////////
  // is_hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_hidden() const { return m_hidden; }

  virtual void set_hidden(bool hidden) { m_hidden = hidden; }

  /////////////////////////////////////////////////////////////////////////
  // order.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_index_element_order order() const { return m_order; }

  virtual void set_order(enum_index_element_order order) { m_order = order; }

  virtual bool is_prefix() const;

 public:
  static Index_element_impl *restore_item(Index_impl *index) {
    return new (std::nothrow) Index_element_impl(index, nullptr);
  }

  static Index_element_impl *clone(const Index_element_impl &other,
                                   Index_impl *index);

 public:
  virtual void debug_print(String_type &outb) const;

 public:
  virtual Object_key *create_primary_key() const;
  virtual bool has_new_primary_key() const;

 private:
  // Fields
  uint m_ordinal_position;
  uint m_length;

  enum_index_element_order m_order;

  bool m_hidden;

  // References to other objects
  Index_impl *m_index;
  Column *m_column;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__INDEX_ELEMENT_IMPL_INCLUDED
