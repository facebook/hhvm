/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__EVENT_INCLUDED
#define DD__EVENT_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/impl/raw/object_keys.h"  // IWYU pragma: keep
#include "sql/dd/types/entity_object.h"   // dd::Entity_object

typedef long my_time_t;
struct MDL_key;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Event_impl;
class Void_key;
class Item_name_key;

namespace tables {
class Events;
}

///////////////////////////////////////////////////////////////////////////

class Event : virtual public Entity_object {
 public:
  typedef Event_impl Impl;
  typedef Event Cache_partition;
  typedef tables::Events DD_table;
  typedef Primary_id_key Id_key;
  typedef Item_name_key Name_key;
  typedef Void_key Aux_key;

  // We need a set of functions to update a preallocated key.
  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }

  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_name_key(key, schema_id(), name());
  }

  static bool update_name_key(Name_key *key, Object_id schema_id,
                              const String_type &name);

  virtual bool update_aux_key(Aux_key *) const { return true; }

 public:
  enum enum_interval_field {
    IF_YEAR = 1,
    IF_QUARTER,
    IF_MONTH,
    IF_DAY,
    IF_HOUR,
    IF_MINUTE,
    IF_WEEK,
    IF_SECOND,
    IF_MICROSECOND,
    IF_YEAR_MONTH,
    IF_DAY_HOUR,
    IF_DAY_MINUTE,
    IF_DAY_SECOND,
    IF_HOUR_MINUTE,
    IF_HOUR_SECOND,
    IF_MINUTE_SECOND,
    IF_DAY_MICROSECOND,
    IF_HOUR_MICROSECOND,
    IF_MINUTE_MICROSECOND,
    IF_SECOND_MICROSECOND
  };

  enum enum_event_status { ES_ENABLED = 1, ES_DISABLED, ES_SLAVESIDE_DISABLED };

  enum enum_on_completion { OC_DROP = 1, OC_PRESERVE };

 public:
  virtual ~Event() {}

 public:
  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const = 0;
  virtual void set_schema_id(Object_id schema_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definer.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definer_user() const = 0;
  virtual const String_type &definer_host() const = 0;
  virtual void set_definer(const String_type &username,
                           const String_type &hostname) = 0;

  /////////////////////////////////////////////////////////////////////////
  // time_zone.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &time_zone() const = 0;
  virtual void set_time_zone(const String_type &time_zone) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definition/utf8.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definition() const = 0;
  virtual void set_definition(const String_type &definition) = 0;

  virtual const String_type &definition_utf8() const = 0;
  virtual void set_definition_utf8(const String_type &definition_utf8) = 0;

  /////////////////////////////////////////////////////////////////////////
  // execute_at.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t execute_at() const = 0;
  virtual void set_execute_at(my_time_t execute_at) = 0;

  virtual void set_execute_at_null(bool is_null) = 0;
  virtual bool is_execute_at_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // interval_value.
  /////////////////////////////////////////////////////////////////////////

  virtual uint interval_value() const = 0;
  virtual void set_interval_value(uint interval_value) = 0;

  virtual void set_interval_value_null(bool is_null) = 0;
  virtual bool is_interval_value_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // interval_field.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_interval_field interval_field() const = 0;
  virtual void set_interval_field(enum_interval_field interval_field) = 0;

  virtual void set_interval_field_null(bool is_null) = 0;
  virtual bool is_interval_field_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // sql_mode
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong sql_mode() const = 0;
  virtual void set_sql_mode(ulonglong sm) = 0;

  /////////////////////////////////////////////////////////////////////////
  // starts.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t starts() const = 0;
  virtual void set_starts(my_time_t starts) = 0;

  virtual void set_starts_null(bool is_null) = 0;
  virtual bool is_starts_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // ends.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t ends() const = 0;
  virtual void set_ends(my_time_t ends) = 0;

  virtual void set_ends_null(bool is_null) = 0;
  virtual bool is_ends_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // event_status.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_event_status event_status() const = 0;
  virtual void set_event_status(enum_event_status event_status) = 0;

  virtual void set_event_status_null(bool is_null) = 0;
  virtual bool is_event_status_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // on_completion.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_on_completion on_completion() const = 0;
  virtual void set_on_completion(enum_on_completion on_completion) = 0;

  /////////////////////////////////////////////////////////////////////////
  // created.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong created(bool convert_time) const = 0;
  virtual void set_created(ulonglong created) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last altered.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong last_altered(bool convert_time) const = 0;
  virtual void set_last_altered(ulonglong last_altered) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last_executed.
  /////////////////////////////////////////////////////////////////////////

  virtual my_time_t last_executed() const = 0;
  virtual void set_last_executed(my_time_t last_executed) = 0;

  virtual void set_last_executed_null(bool is_null) = 0;
  virtual bool is_last_executed_null() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;
  virtual void set_comment(const String_type &comment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // originator.
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong originator() const = 0;
  virtual void set_originator(ulonglong originator) = 0;

  /////////////////////////////////////////////////////////////////////////
  // collations.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id client_collation_id() const = 0;
  virtual void set_client_collation_id(Object_id client_collation_id) = 0;

  virtual Object_id connection_collation_id() const = 0;
  virtual void set_connection_collation_id(
      Object_id connection_collation_id) = 0;

  virtual Object_id schema_collation_id() const = 0;
  virtual void set_schema_collation_id(Object_id schema_collation_id) = 0;

  /**
    Allocate a new object graph and invoke the copy contructor for
    each object. Only used in unit testing.

    @return pointer to dynamically allocated copy
  */
  virtual Event *clone() const = 0;

  static void create_mdl_key(const String_type &schema_name,
                             const String_type &name, MDL_key *key);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__EVENT_INCLUDED
