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

#ifndef DD__ABSTRACT_TABLE_IMPL_INCLUDED
#define DD__ABSTRACT_TABLE_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>  // std::unique_ptr

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/dd/impl/properties_impl.h"  // Properties_impl
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/properties.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"  // dd::Abstract_table
#include "sql/dd/types/column.h"          // IWYU pragma: keep
#include "sql/sql_time.h"                 // gmt_time_to_local_time

class Time_zone;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;
class Sdi_rcontext;
class Sdi_wcontext;
class Weak_object;

class Abstract_table_impl : public Entity_object_impl,
                            virtual public Abstract_table {
 public:
  virtual const Object_table &object_table() const;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

 protected:
  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

 public:
  virtual void debug_print(String_type &outb) const;

 public:
  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const { return m_schema_id; }

  virtual void set_schema_id(Object_id schema_id) { m_schema_id = schema_id; }

  /////////////////////////////////////////////////////////////////////////
  // mysql_version_id.
  // Primarily intended for debugging, but can be used as a last-resort
  // version check for SE data and other items, but in general other
  // mechanisms should be preferred.
  /////////////////////////////////////////////////////////////////////////

  virtual uint mysql_version_id() const { return m_mysql_version_id; }

  // TODO: Commented out as it is not needed as we either use the value
  // assigned by the constructor, or restore a value from the TABLES
  // table. It may be necessary when implementing upgrade.
  // virtual void set_mysql_version_id(uint mysql_version_id)
  //{ m_mysql_version_id= mysql_version_id; }

  /////////////////////////////////////////////////////////////////////////
  // options.
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
  // created.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong created(bool convert_time) const {
    return convert_time ? gmt_time_to_local_time(m_created) : m_created;
  }

  virtual void set_created(ulonglong created) { m_created = created; }

  /////////////////////////////////////////////////////////////////////////
  // last altered.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong last_altered(bool convert_time) const {
    return convert_time ? gmt_time_to_local_time(m_last_altered)
                        : m_last_altered;
  }

  virtual void set_last_altered(ulonglong last_altered) {
    m_last_altered = last_altered;
  }

  /////////////////////////////////////////////////////////////////////////
  // hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_hidden_type hidden() const { return m_hidden; }

  virtual void set_hidden(enum_hidden_type hidden) { m_hidden = hidden; }

  /////////////////////////////////////////////////////////////////////////
  // Column collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Column *add_column();

  virtual const Column_collection &columns() const { return m_columns; }

  virtual Column_collection *columns() { return &m_columns; }

  const Column *get_column(Object_id column_id) const;

  Column *get_column(Object_id column_id);

  const Column *get_column(const String_type &name) const;

  Column *get_column(const String_type &name);

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

 protected:
  Abstract_table_impl();

  virtual ~Abstract_table_impl() {}

 private:
  // Fields.

  uint m_mysql_version_id;

  // TODO-POST-MERGE-TO-TRUNK:
  // Add new field m_last_checked_for_upgrade

  ulonglong m_created;
  ulonglong m_last_altered;

  enum_hidden_type m_hidden;

  Properties_impl m_options;

  // References to tightly-coupled objects.

  Column_collection m_columns;

  // References to other objects.

  Object_id m_schema_id;

 protected:
  Abstract_table_impl(const Abstract_table_impl &src);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__ABSTRACT_TABLE_IMPL_INCLUDED
