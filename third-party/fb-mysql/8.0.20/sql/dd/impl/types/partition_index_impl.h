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

#ifndef DD__PARTITION_INDEX_IMPL_INCLUDED
#define DD__PARTITION_INDEX_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <new>

#include "sql/dd/impl/properties_impl.h"
#include "sql/dd/impl/types/weak_object_impl.h"  // dd::Weak_object_impl
#include "sql/dd/object_id.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/index.h"
#include "sql/dd/types/partition_index.h"  // dd::Partition_index

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Index;
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

class Partition_index_impl : public Weak_object_impl, public Partition_index {
 public:
  Partition_index_impl();

  Partition_index_impl(Partition_impl *partition, Index *index);

  Partition_index_impl(const Partition_index_impl &src, Partition_impl *parent,
                       Index *index);

  virtual ~Partition_index_impl() {}

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  void debug_print(String_type &outb) const;

  void set_ordinal_position(uint) {}

  virtual uint ordinal_position() const { return -1; }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // Partition.
  /////////////////////////////////////////////////////////////////////////

  virtual const Partition &partition() const;

  virtual Partition &partition();

  Partition_impl &partition_impl() { return *m_partition; }

  /////////////////////////////////////////////////////////////////////////
  // Index.
  /////////////////////////////////////////////////////////////////////////

  virtual const Index &index() const;

  virtual Index &index();

  /////////////////////////////////////////////////////////////////////////
  // Options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const { return m_options; }

  virtual Properties &options() { return m_options; }

  virtual bool set_options(const Properties &options) {
    return m_options.insert_values(options);
  }

  virtual bool set_options(const String_type &options_raw) {
    return m_options.insert_values(options_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const {
    return m_se_private_data;
  }

  virtual Properties &se_private_data() { return m_se_private_data; }

  virtual bool set_se_private_data(const String_type &se_private_data_raw) {
    return m_se_private_data.insert_values(se_private_data_raw);
  }

  virtual bool set_se_private_data(const Properties &se_private_data) {
    return m_se_private_data.insert_values(se_private_data);
  }

  /////////////////////////////////////////////////////////////////////////
  // Tablespace.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id tablespace_id() const { return m_tablespace_id; }

  virtual void set_tablespace_id(Object_id tablespace_id) {
    m_tablespace_id = tablespace_id;
  }

 public:
  static Partition_index_impl *restore_item(Partition_impl *partition) {
    return new (std::nothrow) Partition_index_impl(partition, nullptr);
  }

  static Partition_index_impl *clone(const Partition_index_impl &other,
                                     Partition_impl *partition);

 public:
  virtual Object_key *create_primary_key() const;
  virtual bool has_new_primary_key() const;

 private:
  // Fields.

  Properties_impl m_options;
  Properties_impl m_se_private_data;

  // References to tightly-coupled objects.

  Partition_impl *m_partition;
  Index *m_index;

  // References to loosely-coupled objects.

  Object_id m_tablespace_id;
};

///////////////////////////////////////////////////////////////////////////

/**
  Used to sort Partition_index objects for the same partition in
  the same order as Index objects for the table.
*/

struct Partition_index_order_comparator {
  bool operator()(const dd::Partition_index *pi1,
                  const dd::Partition_index *pi2) const {
    return pi1->index().ordinal_position() < pi2->index().ordinal_position();
  }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PARTITION_INDEX_IMPL_INCLUDED
