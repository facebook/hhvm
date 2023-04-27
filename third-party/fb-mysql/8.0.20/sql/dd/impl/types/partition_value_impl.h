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

#ifndef DD__PARTITION_VALUE_IMPL_INCLUDED
#define DD__PARTITION_VALUE_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <new>

#include "sql/dd/impl/types/weak_object_impl.h"  // dd::Weak_object_impl
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/partition_value.h"  // dd::Partition_value

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_key;
class Object_table;
class Open_dictionary_tables_ctx;
class Partition;
class Partition_impl;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class Partition_value_impl : public Weak_object_impl, public Partition_value {
 public:
  Partition_value_impl()
      : m_max_value(false),
        m_null_value(false),
        m_list_num(0),
        m_column_num(0),
        m_partition(nullptr) {}

  Partition_value_impl(Partition_impl *partition)
      : m_max_value(false),
        m_null_value(false),
        m_list_num(0),
        m_column_num(0),
        m_partition(partition) {}

  Partition_value_impl(const Partition_value_impl &src, Partition_impl *parent);

  virtual ~Partition_value_impl() {}

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  void set_ordinal_position(uint) {}

  virtual uint ordinal_position() const { return -1; }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // index.
  /////////////////////////////////////////////////////////////////////////

  virtual const Partition &partition() const;

  virtual Partition &partition();

  /////////////////////////////////////////////////////////////////////////
  // list_num.
  /////////////////////////////////////////////////////////////////////////

  virtual uint list_num() const { return m_list_num; }

  virtual void set_list_num(uint list_num) { m_list_num = list_num; }

  /////////////////////////////////////////////////////////////////////////
  // column_num.
  /////////////////////////////////////////////////////////////////////////

  virtual uint column_num() const { return m_column_num; }

  virtual void set_column_num(uint column_num) { m_column_num = column_num; }

  /////////////////////////////////////////////////////////////////////////
  // value.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &value_utf8() const { return m_value_utf8; }

  virtual void set_value_utf8(const String_type &value) {
    m_value_utf8 = value;
  }

  ////////////////////////////////////////////////////////////////
  // max_value.
  /////////////////////////////////////////////////////////////////////////

  virtual bool max_value() const { return m_max_value; }

  virtual void set_max_value(bool max_value) { m_max_value = max_value; }

  ////////////////////////////////////////////////////////////////
  // null_value.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_value_null() const { return m_null_value; }

  virtual void set_value_null(bool is_null) { m_null_value = is_null; }

  /////////////////////////////////////////////////////////////////////////

 public:
  static Partition_value_impl *restore_item(Partition_impl *partition) {
    return new (std::nothrow) Partition_value_impl(partition);
  }

  static Partition_value_impl *clone(const Partition_value_impl &other,
                                     Partition_impl *partition) {
    return new (std::nothrow) Partition_value_impl(other, partition);
  }

 public:
  virtual void debug_print(String_type &outb) const;

 public:
  virtual Object_key *create_primary_key() const;
  virtual bool has_new_primary_key() const;

 private:
  // Fields
  bool m_max_value;
  bool m_null_value;

  uint m_list_num;
  uint m_column_num;

  String_type m_value_utf8;

  // References to other objects
  Partition_impl *m_partition;
};

///////////////////////////////////////////////////////////////////////////

/**
  Used to sort Partition_value objects for the same partition first
  according to list number and then according to the column number.
*/

struct Partition_value_order_comparator {
  bool operator()(const dd::Partition_value *pv1,
                  const dd::Partition_value *pv2) const {
    return ((pv1->list_num() < pv2->list_num()) ||
            (pv1->list_num() == pv2->list_num() &&
             pv1->column_num() < pv2->column_num()));
  }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PARTITION_VALUE_IMPL_INCLUDED
