/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__ROUTINE_IMPL_INCLUDED
#define DD__ROUTINE_IMPL_INCLUDED

#include <stddef.h>
#include <memory>  // std::unique_ptr
#include <string>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/parameter.h"  // dd::Parameter
#include "sql/dd/types/routine.h"    // dd::Routine
#include "sql/dd/types/view.h"
#include "sql/sql_time.h"  // gmt_time_to_local_time

namespace dd {
class Open_dictionary_tables_ctx;
class Parameter;
class Parameter_collection;
class Weak_object;
class Object_table;

///////////////////////////////////////////////////////////////////////////

class Routine_impl : public Entity_object_impl, virtual public Routine {
 public:
  Routine_impl();

  virtual ~Routine_impl();

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  virtual void debug_print(String_type &outb) const;

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const { return m_schema_id; }

  virtual void set_schema_id(Object_id schema_id) { m_schema_id = schema_id; }

  /////////////////////////////////////////////////////////////////////////
  // routine Partition type
  /////////////////////////////////////////////////////////////////////////

  virtual enum_routine_type type() const { return m_routine_type; }

  virtual void set_type(enum_routine_type routine_type) {
    m_routine_type = routine_type;
  }

  /////////////////////////////////////////////////////////////////////////
  // definition/utf8.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definition() const { return m_definition; }

  virtual void set_definition(const String_type &definition) {
    m_definition = definition;
  }

  virtual const String_type &definition_utf8() const {
    return m_definition_utf8;
  }

  virtual void set_definition_utf8(const String_type &definition_utf8) {
    m_definition_utf8 = definition_utf8;
  }

  /////////////////////////////////////////////////////////////////////////
  // parameter_str
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &parameter_str() const { return m_parameter_str; }

  virtual void set_parameter_str(const String_type &parameter_str) {
    m_parameter_str = parameter_str;
  }

  /////////////////////////////////////////////////////////////////////////
  // is_deterministic.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_deterministic() const { return m_is_deterministic; }

  virtual void set_deterministic(bool deterministic) {
    m_is_deterministic = deterministic;
  }

  /////////////////////////////////////////////////////////////////////////
  // sql data access.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_sql_data_access sql_data_access() const {
    return m_sql_data_access;
  }

  virtual void set_sql_data_access(enum_sql_data_access sda) {
    m_sql_data_access = sda;
  }

  /////////////////////////////////////////////////////////////////////////
  // security_type.
  /////////////////////////////////////////////////////////////////////////

  virtual View::enum_security_type security_type() const {
    return m_security_type;
  }

  virtual void set_security_type(View::enum_security_type security_type) {
    m_security_type = security_type;
  }

  /////////////////////////////////////////////////////////////////////////
  // sql_mode
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong sql_mode() const { return m_sql_mode; }

  virtual void set_sql_mode(ulonglong sm) { m_sql_mode = sm; }

  /////////////////////////////////////////////////////////////////////////
  // definer.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definer_user() const { return m_definer_user; }

  virtual const String_type &definer_host() const { return m_definer_host; }

  virtual void set_definer(const String_type &username,
                           const String_type &hostname) {
    m_definer_user = username;
    m_definer_host = hostname;
  }

  /////////////////////////////////////////////////////////////////////////
  // collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id client_collation_id() const {
    return m_client_collation_id;
  }

  virtual void set_client_collation_id(Object_id client_collation_id) {
    m_client_collation_id = client_collation_id;
  }

  virtual Object_id connection_collation_id() const {
    return m_connection_collation_id;
  }

  virtual void set_connection_collation_id(Object_id connection_collation_id) {
    m_connection_collation_id = connection_collation_id;
  }

  virtual Object_id schema_collation_id() const {
    return m_schema_collation_id;
  }

  virtual void set_schema_collation_id(Object_id schema_collation_id) {
    m_schema_collation_id = schema_collation_id;
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
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

  /////////////////////////////////////////////////////////////////////////
  // Parameter collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Parameter *add_parameter();

  virtual const Parameter_collection &parameters() const {
    return m_parameters;
  }

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
  enum_routine_type m_routine_type;
  enum_sql_data_access m_sql_data_access;
  View::enum_security_type m_security_type;

  bool m_is_deterministic;

  ulonglong m_sql_mode;
  ulonglong m_created;
  ulonglong m_last_altered;

  String_type m_definition;
  String_type m_definition_utf8;
  String_type m_parameter_str;
  String_type m_definer_user;
  String_type m_definer_host;
  String_type m_comment;

  // Collections.

  Parameter_collection m_parameters;

  // References.

  Object_id m_schema_id;
  Object_id m_client_collation_id;
  Object_id m_connection_collation_id;
  Object_id m_schema_collation_id;

 protected:
  Routine_impl(const Routine_impl &src);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__ROUTINE_IMPL_INCLUDED
