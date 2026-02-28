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

#ifndef DD__VIEW_IMPL_INCLUDED
#define DD__VIEW_IMPL_INCLUDED

#include <sys/types.h>
#include <memory>
#include <new>

#include "my_inttypes.h"
#include "sql/dd/impl/properties_impl.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/abstract_table_impl.h"  // dd::Abstract_table_impl
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/view.h"          // dd::View
#include "sql/dd/types/view_routine.h"  // IWYU pragma: keep
#include "sql/dd/types/view_table.h"    // IWYU pragma: keep

namespace dd {
class Column;
class Open_dictionary_tables_ctx;
class Weak_object;
class Object_table;
}  // namespace dd

struct CHARSET_INFO;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class View_impl : public Abstract_table_impl, public View {
 public:
  View_impl();

  virtual ~View_impl() {}

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual void remove_children();

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  virtual void debug_print(String_type &outb) const;

 public:
  /////////////////////////////////////////////////////////////////////////
  // enum_table_type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_table_type type() const { return m_type; }

  /////////////////////////////////////////////////////////////////////////
  // regular/system view flag.
  /////////////////////////////////////////////////////////////////////////

  virtual void set_system_view(bool system_view) {
    m_type =
        system_view ? enum_table_type::SYSTEM_VIEW : enum_table_type::USER_VIEW;
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
  // check_option.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_check_option check_option() const { return m_check_option; }

  virtual void set_check_option(enum_check_option check_option) {
    m_check_option = check_option;
  }

  /////////////////////////////////////////////////////////////////////////
  // is_updatable.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_updatable() const { return m_is_updatable; }

  virtual void set_updatable(bool updatable) { m_is_updatable = updatable; }

  /////////////////////////////////////////////////////////////////////////
  // algorithm.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_algorithm algorithm() const { return m_algorithm; }

  virtual void set_algorithm(enum_algorithm algorithm) {
    m_algorithm = algorithm;
  }

  /////////////////////////////////////////////////////////////////////////
  // security_type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_security_type security_type() const { return m_security_type; }

  virtual void set_security_type(enum_security_type security_type) {
    m_security_type = security_type;
  }

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
  // Explicit list of column names.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &column_names() const { return m_column_names; }

  virtual Properties &column_names() { return m_column_names; }

  /////////////////////////////////////////////////////////////////////////
  // View_table collection.
  /////////////////////////////////////////////////////////////////////////

  virtual View_table *add_table();

  virtual const View_tables &tables() const { return m_tables; }

  /////////////////////////////////////////////////////////////////////////
  // View_routine collection.
  /////////////////////////////////////////////////////////////////////////

  virtual View_routine *add_routine();

  virtual const View_routines &routines() const { return m_routines; }

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
  virtual Object_id schema_id() const {
    return Abstract_table_impl::schema_id();
  }
  virtual void set_schema_id(Object_id schema_id) {
    Abstract_table_impl::set_schema_id(schema_id);
  }
  virtual uint mysql_version_id() const {
    return Abstract_table_impl::mysql_version_id();
  }
  virtual const Properties &options() const {
    return Abstract_table_impl::options();
  }
  virtual Properties &options() { return Abstract_table_impl::options(); }
  virtual bool set_options(const Properties &options) {
    return Abstract_table_impl::set_options(options);
  }
  virtual bool set_options(const String_type &options_raw) {
    return Abstract_table_impl::set_options(options_raw);
  }
  virtual ulonglong created(bool convert_time) const {
    return Abstract_table_impl::created(convert_time);
  }
  virtual void set_created(ulonglong created) {
    Abstract_table_impl::set_created(created);
  }
  virtual ulonglong last_altered(bool convert_time) const {
    return Abstract_table_impl::last_altered(convert_time);
  }
  virtual void set_last_altered(ulonglong last_altered) {
    Abstract_table_impl::set_last_altered(last_altered);
  }
  virtual Column *add_column() { return Abstract_table_impl::add_column(); }
  virtual const Column_collection &columns() const {
    return Abstract_table_impl::columns();
  }
  virtual Column_collection *columns() {
    return Abstract_table_impl::columns();
  }
  const Column *get_column(const String_type &name) const {
    return Abstract_table_impl::get_column(name);
  }
  Column *get_column(const String_type &name) {
    return Abstract_table_impl::get_column(name);
  }
  virtual enum_hidden_type hidden() const {
    return Abstract_table_impl::hidden();
  }
  virtual void set_hidden(enum_hidden_type hidden) {
    Abstract_table_impl::set_hidden(hidden);
  }

 private:
  enum_table_type m_type;
  bool m_is_updatable;
  enum_check_option m_check_option;
  enum_algorithm m_algorithm;
  enum_security_type m_security_type;

  String_type m_definition;
  String_type m_definition_utf8;
  String_type m_definer_user;
  String_type m_definer_host;

  Properties_impl m_column_names;

  // Collections.

  View_tables m_tables;
  View_routines m_routines;

  // References.

  Object_id m_client_collation_id;
  Object_id m_connection_collation_id;

  View_impl(const View_impl &src);
  View_impl *clone() const { return new View_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__VIEW_IMPL_INCLUDED
