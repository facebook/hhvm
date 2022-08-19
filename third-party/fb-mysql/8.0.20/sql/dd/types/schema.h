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

#ifndef DD__SCHEMA_INCLUDED
#define DD__SCHEMA_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/impl/raw/object_keys.h"  // IWYU pragma: keep
#include "sql/dd/properties.h"            // Properties
#include "sql/dd/sdi_fwd.h"               // RJ_Document
#include "sql/dd/types/entity_object.h"   // dd::Entity_object

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Item_name_key;
class Primary_id_key;
class Schema_impl;
class Table;
class View;
class Event;
class Function;
class Procedure;
class Void_key;
class Time_zone;
class Properties;

namespace tables {
class Schemata;
}

/////////////////////////////////////////////////////////////////////////
// enum_encryption_type.
/////////////////////////////////////////////////////////////////////////

enum class enum_encryption_type { ET_NO = 1, ET_YES };

///////////////////////////////////////////////////////////////////////////

class Schema : virtual public Entity_object {
 public:
  typedef Schema_impl Impl;
  typedef Schema Cache_partition;
  typedef tables::Schemata DD_table;
  typedef Primary_id_key Id_key;
  typedef Item_name_key Name_key;
  typedef Void_key Aux_key;

  // We need a set of functions to update a preallocated key.
  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }

  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_name_key(key, name());
  }

  static bool update_name_key(Name_key *key, const String_type &name);

  virtual bool update_aux_key(Aux_key *) const { return true; }

 public:
  /////////////////////////////////////////////////////////////////////////
  // Default collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id default_collation_id() const = 0;
  virtual void set_default_collation_id(Object_id default_collation_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Default encryption.
  /////////////////////////////////////////////////////////////////////////

  virtual bool default_encryption() const = 0;
  virtual void set_default_encryption(bool default_encryption) = 0;

  /////////////////////////////////////////////////////////////////////////
  // created
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong created(bool convert_time) const = 0;
  virtual void set_created(ulonglong created) = 0;

  /////////////////////////////////////////////////////////////////////////
  // last_altered
  /////////////////////////////////////////////////////////////////////////

  virtual ulonglong last_altered(bool convert_time) const = 0;
  virtual void set_last_altered(ulonglong last_altered) = 0;

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const = 0;

  virtual Properties &se_private_data() = 0;
  virtual bool set_se_private_data(const String_type &se_private_data_raw) = 0;
  virtual bool set_se_private_data(const Properties &se_private_data) = 0;

  /////////////////////////////////////////////////////////////////////////
  // options
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const = 0;

  virtual Properties &options() = 0;

  virtual bool set_options_raw(const String_type &options_raw) = 0;

  virtual bool set_db_metadata(const String_type &metadata) = 0;
  virtual String_type get_db_metadata() const noexcept = 0;

 public:
  virtual Event *create_event(THD *thd) const = 0;

  virtual Function *create_function(THD *thd) const = 0;

  virtual Procedure *create_procedure(THD *thd) const = 0;

  virtual Table *create_table(THD *thd) const = 0;

  virtual View *create_view(THD *thd) const = 0;

  virtual View *create_system_view(THD *thd) const = 0;

  /**
    Allocate a new object and invoke the copy contructor.

    @return pointer to dynamically allocated copy
  */
  virtual Schema *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__SCHEMA_INCLUDE
