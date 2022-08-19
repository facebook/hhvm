/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__COLUMN_STATISTIC_IMPL_INCLUDED
#define DD__COLUMN_STATISTIC_IMPL_INCLUDED

#include <stdio.h>
#include <algorithm>
#include <new>

#include "my_alloc.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/object_id.h"                      // dd::Object_id
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/column_statistics.h"  // dd::Column_statistics
#include "sql/histograms/histogram.h"
#include "sql/psi_memory_key.h"  // key_memory_DD_column_statistics

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Open_dictionary_tables_ctx;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Weak_object;
class Object_table;

///////////////////////////////////////////////////////////////////////////

class Column_statistics_impl final : public Entity_object_impl,
                                     public Column_statistics {
 public:
  Column_statistics_impl()
      : m_schema_name(), m_table_name(), m_column_name(), m_histogram(nullptr) {
    init_alloc_root(key_memory_DD_column_statistics, &m_mem_root, 256, 0);
  }

 private:
  Column_statistics_impl(const Column_statistics_impl &column_statistics)
      : Weak_object(column_statistics),
        Entity_object_impl(column_statistics),
        m_schema_name(column_statistics.m_schema_name),
        m_table_name(column_statistics.m_table_name),
        m_column_name(column_statistics.m_column_name),
        m_histogram(nullptr) {
    init_alloc_root(key_memory_DD_column_statistics, &m_mem_root, 256, 0);

    if (column_statistics.m_histogram != nullptr)
      m_histogram = column_statistics.m_histogram->clone(&m_mem_root);
  }

 public:
  virtual const Object_table &object_table() const override;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  bool validate() const override { return m_histogram == nullptr; }

  bool store_attributes(Raw_record *r) override;

  bool restore_attributes(const Raw_record &r) override;

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const override;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) override;

  const String_type &schema_name() const override { return m_schema_name; }

  void set_schema_name(const String_type &schema_name) override {
    m_schema_name = schema_name;
  }

  const String_type &table_name() const override { return m_table_name; }

  void set_table_name(const String_type &table_name) override {
    m_table_name = table_name;
  }

  const String_type &column_name() const override { return m_column_name; }

  void set_column_name(const String_type &column_name) override {
    m_column_name = column_name;
  }

  const histograms::Histogram *histogram() const override {
    return m_histogram;
  }

  void set_histogram(const histograms::Histogram *histogram) override {
    // Free any existing histogram data
    free_root(&m_mem_root, MYF(0));

    // Take responsibility for the MEM_ROOT of the histogram provided
    m_mem_root = std::move(*histogram->get_mem_root());
    m_histogram = std::move(histogram);
  }

  Entity_object_impl *impl() override { return Entity_object_impl::impl(); }

  const Entity_object_impl *impl() const override {
    return Entity_object_impl::impl();
  }

  Object_id id() const override { return Entity_object_impl::id(); }

  bool is_persistent() const override {
    return Entity_object_impl::is_persistent();
  }

  const String_type &name() const override {
    return Entity_object_impl::name();
  }

  void set_name(const String_type &name) override {
    Entity_object_impl::set_name(name);
  }

  void debug_print(String_type &outb) const override {
    char outbuf[1024];
    sprintf(outbuf,
            "COLUMN STATISTIC OBJECT: id= {OID: %lld}, name= %s, "
            "schema_name= %s, table_name= %s, column_name= %s",
            id(), name().c_str(), schema_name().c_str(), table_name().c_str(),
            column_name().c_str());
    outb = String_type(outbuf);
  }

 private:
  String_type m_schema_name;
  String_type m_table_name;
  String_type m_column_name;
  const histograms::Histogram *m_histogram;

  Column_statistics *clone() const override {
    return new Column_statistics_impl(*this);
  }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLUMN_STATISTIC_IMPL_INCLUDED
