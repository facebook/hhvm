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

#ifndef DD__INDEX_IMPL_INCLUDED
#define DD__INDEX_IMPL_INCLUDED

#include <sys/types.h>
#include <memory>
#include <new>

#include "sql/dd/impl/properties_impl.h"           // Properties_impl
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/properties.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/index.h"          // dd::Index
#include "sql/dd/types/index_element.h"  // IWYU pragma: keep

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Object_table;
class Open_dictionary_tables_ctx;
class Properties;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Table;
class Table_impl;
class Weak_object;

///////////////////////////////////////////////////////////////////////////

class Index_impl : public Entity_object_impl, public Index {
 public:
  Index_impl();

  Index_impl(Table_impl *table);

  Index_impl(const Index_impl &src, Table_impl *parent);

  virtual ~Index_impl();

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  void debug_print(String_type &outb) const;

  virtual void set_ordinal_position(uint ordinal_position) {
    m_ordinal_position = ordinal_position;
  }

  virtual uint ordinal_position() const { return m_ordinal_position; }

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // Table.
  /////////////////////////////////////////////////////////////////////////

  virtual const Table &table() const;

  virtual Table &table();

  /* non-virtual */ const Table_impl &table_impl() const { return *m_table; }

  /* non-virtual */ Table_impl &table_impl() { return *m_table; }

  /////////////////////////////////////////////////////////////////////////
  // is_generated
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_generated() const { return m_is_generated; }

  virtual void set_generated(bool generated) { m_is_generated = generated; }

  /////////////////////////////////////////////////////////////////////////
  // is_hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_hidden() const { return m_hidden; }

  virtual void set_hidden(bool hidden) { m_hidden = hidden; }

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

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

  /////////////////////////////////////////////////////////////////////////
  // Engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const { return m_engine; }

  virtual void set_engine(const String_type &engine) { m_engine = engine; }

  /////////////////////////////////////////////////////////////////////////
  // Index type.
  /////////////////////////////////////////////////////////////////////////

  virtual Index::enum_index_type type() const { return m_type; }

  virtual void set_type(Index::enum_index_type type) { m_type = type; }

  /////////////////////////////////////////////////////////////////////////
  // Index algorithm.
  /////////////////////////////////////////////////////////////////////////

  virtual Index::enum_index_algorithm algorithm() const { return m_algorithm; }

  virtual void set_algorithm(Index::enum_index_algorithm algorithm) {
    m_algorithm = algorithm;
  }

  virtual bool is_algorithm_explicit() const { return m_is_algorithm_explicit; }

  virtual void set_algorithm_explicit(bool alg_expl) {
    m_is_algorithm_explicit = alg_expl;
  }

  virtual bool is_visible() const { return m_is_visible; }

  virtual void set_visible(bool is_visible) { m_is_visible = is_visible; }

  /////////////////////////////////////////////////////////////////////////
  // Index-element collection
  /////////////////////////////////////////////////////////////////////////

  virtual Index_element *add_element(Column *c);

  virtual const Index_elements &elements() const { return m_elements; }

  virtual bool is_candidate_key() const;

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

 public:
  static Index_impl *restore_item(Table_impl *table) {
    return new (std::nothrow) Index_impl(table);
  }

  static Index_impl *clone(const Index_impl &other, Table_impl *table) {
    return new (std::nothrow) Index_impl(other, table);
  }

 private:
  // Fields.

  bool m_hidden;
  bool m_is_generated;

  uint m_ordinal_position;

  String_type m_comment;
  Properties_impl m_options;
  Properties_impl m_se_private_data;

  Index::enum_index_type m_type;
  Index::enum_index_algorithm m_algorithm;
  bool m_is_algorithm_explicit;
  bool m_is_visible;

  String_type m_engine;

  // References to tightly-coupled objects.

  Table_impl *m_table;

  Index_elements m_elements;

  // References to loosely-coupled objects.

  Object_id m_tablespace_id;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__INDEX_IMPL_INCLUDED
