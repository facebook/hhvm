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

#ifndef DD__ROUTINE_INCLUDED
#define DD__ROUTINE_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/impl/raw/object_keys.h"  // IWYU pragma: keep
#include "sql/dd/types/entity_object.h"   // dd::Entity_object
#include "sql/dd/types/view.h"            // dd::Column::enum_security_type

struct MDL_key;
struct CHARSET_INFO;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Routine_impl;
class Primary_id_key;
class Void_key;
class Parameter;
class Properties;
class Routine_name_key;

namespace tables {
class Routines;
}

///////////////////////////////////////////////////////////////////////////

/**
  Abstract base class for functions and procedures.

  @note This class may be inherited along different paths
        for some subclasses due to the diamond shaped
        inheritance hierarchy; thus, direct subclasses
        must inherit this class virtually.
*/

class Routine : virtual public Entity_object {
 public:
  typedef Routine_impl Impl;
  typedef Routine Cache_partition;
  typedef tables::Routines DD_table;
  typedef Primary_id_key Id_key;
  typedef Routine_name_key Name_key;
  typedef Void_key Aux_key;
  typedef Collection<Parameter *> Parameter_collection;

  // We need a set of functions to update a preallocated key.
  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }

  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_routine_name_key(key, schema_id(), name());
  }

  virtual bool update_routine_name_key(Name_key *key, Object_id schema_id,
                                       const String_type &name) const = 0;

  virtual bool update_aux_key(Aux_key *) const { return true; }

 public:
  enum enum_routine_type { RT_FUNCTION = 1, RT_PROCEDURE };

  enum enum_sql_data_access {
    SDA_CONTAINS_SQL = 1,
    SDA_NO_SQL,
    SDA_READS_SQL_DATA,
    SDA_MODIFIES_SQL_DATA
  };

 public:
  virtual ~Routine() {}

 public:
  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const = 0;
  virtual void set_schema_id(Object_id schema_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // routine type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_routine_type type() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // definition/utf8.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definition() const = 0;
  virtual void set_definition(const String_type &definition) = 0;

  virtual const String_type &definition_utf8() const = 0;
  virtual void set_definition_utf8(const String_type &definition_utf8) = 0;

  /////////////////////////////////////////////////////////////////////////
  // parameter_str
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &parameter_str() const = 0;
  virtual void set_parameter_str(const String_type &parameter_str) = 0;

  /////////////////////////////////////////////////////////////////////////
  // is_deterministic.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_deterministic() const = 0;
  virtual void set_deterministic(bool deterministic) = 0;

  /////////////////////////////////////////////////////////////////////////
  // sql data access.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_sql_data_access sql_data_access() const = 0;
  virtual void set_sql_data_access(enum_sql_data_access sda) = 0;

  /////////////////////////////////////////////////////////////////////////
  // security type.
  /////////////////////////////////////////////////////////////////////////

  virtual View::enum_security_type security_type() const = 0;
  virtual void set_security_type(View::enum_security_type st) = 0;

  /////////////////////////////////////////////////////////////////////////
  // sql_mode
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong sql_mode() const = 0;
  virtual void set_sql_mode(ulonglong sm) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definer.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definer_user() const = 0;
  virtual const String_type &definer_host() const = 0;
  virtual void set_definer(const String_type &username,
                           const String_type &hostname) = 0;

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
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;
  virtual void set_comment(const String_type &comment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // parameter collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Parameter *add_parameter() = 0;

  virtual const Parameter_collection &parameters() const = 0;

  /**
    Allocate a new object graph and invoke the copy contructor for
    each object. Only used in unit testing.

    @return pointer to dynamically allocated copy
  */
  virtual Routine *clone() const = 0;

  static void create_mdl_key(enum_routine_type type,
                             const String_type &schema_name,
                             const String_type &name, MDL_key *key);
  static const CHARSET_INFO *name_collation();
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__ROUTINE_INCLUDED
