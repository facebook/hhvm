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

#ifndef DD__SCHEMA_IMPL_INCLUDED
#define DD__SCHEMA_IMPL_INCLUDED

#include <stdio.h>
#include <new>
#include <string>

#include "my_inttypes.h"
#include "sql/dd/impl/properties_impl.h"           // Properties_impl
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/properties.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/entity_object_table.h"  // dd::Entity_object_table
#include "sql/dd/types/schema.h"               // dd::Schema
#include "sql/sql_time.h"                      // gmt_time_to_local_time

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Event;
class Function;
class Object_table;
class Open_dictionary_tables_ctx;
class Procedure;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Table;
class View;
class Weak_object;
class Object_table;

///////////////////////////////////////////////////////////////////////////

class Schema_impl : public Entity_object_impl, public Schema {
 public:
  Schema_impl();

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // Default collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id default_collation_id() const {
    return m_default_collation_id;
  }

  virtual void set_default_collation_id(Object_id default_collation_id) {
    m_default_collation_id = default_collation_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // Default encryption.
  /////////////////////////////////////////////////////////////////////////

  virtual bool default_encryption() const {
    return m_default_encryption == enum_encryption_type::ET_YES;
  }

  virtual void set_default_encryption(bool default_encryption) {
    m_default_encryption = default_encryption ? enum_encryption_type::ET_YES
                                              : enum_encryption_type::ET_NO;
  }

  /////////////////////////////////////////////////////////////////////////
  // created
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong created(bool convert_time) const {
    return convert_time ? gmt_time_to_local_time(m_created) : m_created;
  }

  virtual void set_created(ulonglong created) { m_created = created; }

  /////////////////////////////////////////////////////////////////////////
  // last_altered
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong last_altered(bool convert_time) const {
    return convert_time ? gmt_time_to_local_time(m_last_altered)
                        : m_last_altered;
  }

  virtual void set_last_altered(ulonglong last_altered) {
    m_last_altered = last_altered;
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
  // options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const { return *m_options; }

  virtual Properties &options() { return *m_options; }

  virtual bool set_options_raw(const String_type &options_raw);

  virtual bool set_db_metadata(const String_type &metadata);
  virtual String_type get_db_metadata() const noexcept;

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
  virtual Event *create_event(THD *thd) const;

  virtual Function *create_function(THD *thd) const;

  virtual Procedure *create_procedure(THD *thd) const;

  virtual Table *create_table(THD *thd) const;

  virtual View *create_view(THD *thd) const;

  virtual View *create_system_view(THD *thd) const;

 public:
  virtual void debug_print(String_type &outb) const {
    char outbuf[1024];
    snprintf(outbuf, sizeof(outbuf) / sizeof(char),
             "SCHEMA OBJECT: id= {OID: %lld}, name= %s, "
             "collation_id={OID: %lld},"
             "m_created= %llu, m_last_altered= %llu,"
             "m_default_encryption= %d, "
             "se_private_data= %s",
             id(), name().c_str(), m_default_collation_id, m_created,
             m_last_altered, static_cast<int>(m_default_encryption),
             m_se_private_data.raw_string().c_str());
    outb = String_type(outbuf);
  }

 private:
  // Fields
  ulonglong m_created;
  ulonglong m_last_altered;
  enum_encryption_type m_default_encryption;

  // The se_private_data column of a schema might be used by several storage
  // engines at the same time as the schema is not associated with any specific
  // engine. So to avoid any naming conflicts, we have the convention that the
  // keys should be prefixed with the engine name.
  Properties_impl m_se_private_data;

  // References to other objects
  Object_id m_default_collation_id;

  std::unique_ptr<Properties> m_options;

  Schema_impl(const Schema_impl &src);

  Schema *clone() const { return new Schema_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__SCHEMA_IMPL_INCLUDED
