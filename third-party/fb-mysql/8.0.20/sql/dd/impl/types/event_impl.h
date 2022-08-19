/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__EVENT_IMPL_INCLUDED
#define DD__EVENT_IMPL_INCLUDED

#include <sys/types.h>
#include <new>
#include <string>

#include "my_inttypes.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/event.h"  // dd::Event
#include "sql/sql_time.h"        // gmt_time_to_local_time

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Open_dictionary_tables_ctx;
class Weak_object;
class Object_table;

class Event_impl : public Entity_object_impl, public Event {
 public:
  Event_impl();
  Event_impl(const Event_impl &);

  virtual ~Event_impl() {}

 public:
  virtual const Object_table &object_table() const;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

  virtual void debug_print(String_type &outb) const;

 public:
  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const { return m_schema_id; }

  virtual void set_schema_id(Object_id schema_id) { m_schema_id = schema_id; }

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
  // time_zone
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &time_zone() const { return m_time_zone; }

  virtual void set_time_zone(const String_type &time_zone) {
    m_time_zone = time_zone;
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
  // execute_at.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t execute_at() const { return m_execute_at; }

  virtual void set_execute_at(my_time_t execute_at) {
    m_execute_at = execute_at;
  }

  virtual void set_execute_at_null(bool is_null) {
    m_is_execute_at_null = is_null;
  }

  virtual bool is_execute_at_null() const { return m_is_execute_at_null; }

  /////////////////////////////////////////////////////////////////////////
  // interval_value.
  /////////////////////////////////////////////////////////////////////////

  virtual uint interval_value() const { return m_interval_value; }

  virtual void set_interval_value(uint interval_value) {
    m_interval_value = interval_value;
  }

  virtual void set_interval_value_null(bool is_null) {
    m_is_interval_value_null = is_null;
  }

  virtual bool is_interval_value_null() const {
    return m_is_interval_value_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // interval_field
  /////////////////////////////////////////////////////////////////////////

  virtual enum_interval_field interval_field() const {
    return m_interval_field;
  }

  virtual void set_interval_field(enum_interval_field interval_field) {
    m_interval_field = interval_field;
  }

  virtual void set_interval_field_null(bool is_null) {
    m_is_interval_field_null = is_null;
  }

  virtual bool is_interval_field_null() const {
    return m_is_interval_field_null;
  }

  /////////////////////////////////////////////////////////////////////////
  // sql_mode
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong sql_mode() const { return m_sql_mode; }

  virtual void set_sql_mode(ulonglong sm) { m_sql_mode = sm; }

  /////////////////////////////////////////////////////////////////////////
  // starts.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t starts() const { return m_starts; }

  virtual void set_starts(my_time_t starts) { m_starts = starts; }

  virtual void set_starts_null(bool is_null) { m_is_starts_null = is_null; }

  virtual bool is_starts_null() const { return m_is_starts_null; }

  /////////////////////////////////////////////////////////////////////////
  // ends.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t ends() const { return m_ends; }

  virtual void set_ends(my_time_t ends) { m_ends = ends; }

  virtual void set_ends_null(bool is_null) { m_is_ends_null = is_null; }

  virtual bool is_ends_null() const { return m_is_ends_null; }

  /////////////////////////////////////////////////////////////////////////
  // event_status
  /////////////////////////////////////////////////////////////////////////

  virtual enum_event_status event_status() const { return m_event_status; }

  virtual void set_event_status(enum_event_status event_status) {
    m_event_status = event_status;
  }

  virtual void set_event_status_null(bool is_null) {
    m_is_event_status_null = is_null;
  }

  virtual bool is_event_status_null() const { return m_is_event_status_null; }

  /////////////////////////////////////////////////////////////////////////
  // on_completion
  /////////////////////////////////////////////////////////////////////////

  virtual enum_on_completion on_completion() const { return m_on_completion; }

  virtual void set_on_completion(enum_on_completion on_completion) {
    m_on_completion = on_completion;
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
  // last_executed.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t last_executed() const { return m_last_executed; }

  virtual void set_last_executed(my_time_t last_executed) {
    m_is_last_executed_null = false;
    m_last_executed = last_executed;
  }

  virtual void set_last_executed_null(bool is_null) {
    m_is_last_executed_null = is_null;
  }

  virtual bool is_last_executed_null() const { return m_is_last_executed_null; }

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

  /////////////////////////////////////////////////////////////////////////
  // originator
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong originator() const { return m_originator; }

  virtual void set_originator(ulonglong originator) {
    m_originator = originator;
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
  enum_interval_field m_interval_field;
  enum_event_status m_event_status;
  enum_on_completion m_on_completion;

  ulonglong m_sql_mode;
  ulonglong m_created;
  ulonglong m_last_altered;
  ulonglong m_originator;
  uint m_interval_value;

  my_time_t m_execute_at;
  my_time_t m_starts;
  my_time_t m_ends;
  my_time_t m_last_executed;

  bool m_is_execute_at_null;
  bool m_is_interval_value_null;
  bool m_is_interval_field_null;
  bool m_is_starts_null;
  bool m_is_ends_null;
  bool m_is_event_status_null;
  bool m_is_last_executed_null;

  String_type m_time_zone;
  String_type m_definition;
  String_type m_definition_utf8;
  String_type m_definer_user;
  String_type m_definer_host;
  String_type m_comment;

  // References.

  Object_id m_schema_id;
  Object_id m_client_collation_id;
  Object_id m_connection_collation_id;
  Object_id m_schema_collation_id;

  Event *clone() const { return new Event_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__EVENT_IMPL_INCLUDED
