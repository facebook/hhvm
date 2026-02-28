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

#ifndef DD__ABSTRACT_TABLE_INCLUDED
#define DD__ABSTRACT_TABLE_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/collection.h"           // dd::Collection
#include "sql/dd/object_id.h"            // dd::Object_id
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Abstract_table_impl;
class Column;
class Item_name_key;
class Primary_id_key;
class Properties;
class Se_private_id_key;

namespace tables {
class Tables;
}

///////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// enum_table_type.
/////////////////////////////////////////////////////////////////////////

enum class enum_table_type {
  INVALID_TABLE,
  BASE_TABLE,
  USER_VIEW,
  SYSTEM_VIEW
};

/**
  Abstract base class for tables and views.

  @note This class may be inherited along different paths
        for some subclasses due to the diamond shaped
        inheritance hierarchy; thus, direct subclasses
        must inherit this class virtually.
*/

class Abstract_table : virtual public Entity_object {
 public:
  typedef Abstract_table_impl Impl;
  typedef Abstract_table Cache_partition;
  typedef tables::Tables DD_table;
  typedef Primary_id_key Id_key;
  typedef Item_name_key Name_key;
  typedef Se_private_id_key Aux_key;
  typedef Collection<Column *> Column_collection;

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
  virtual ~Abstract_table() {}

 public:
  /**
    Enumeration type which indicates whether the table is hidden,
    and if yes then which type of hidden table it is.
  */
  enum enum_hidden_type {
    /* Normal, user-visible table. */
    HT_VISIBLE = 1,
    /* Hidden. System (e.g. data-dictionary) table. */
    HT_HIDDEN_SYSTEM,
    /*
      Hidden. Table which is implicitly created and dropped by SE.
      For example, InnoDB's auxiliary table for FTS.
    */
    HT_HIDDEN_SE,
    /*
      Hidden. Temporary table created by ALTER TABLE implementation.
    */
    HT_HIDDEN_DDL
  };

  /////////////////////////////////////////////////////////////////////////
  // schema.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id schema_id() const = 0;
  virtual void set_schema_id(Object_id schema_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // mysql_version_id.
  /////////////////////////////////////////////////////////////////////////

  virtual uint mysql_version_id() const = 0;
  // virtual void set_mysql_version_id(uint mysql_version_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const = 0;
  virtual Properties &options() = 0;
  virtual bool set_options(const Properties &options) = 0;
  virtual bool set_options(const String_type &options_raw) = 0;

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

  virtual enum_table_type type() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_hidden_type hidden() const = 0;
  virtual void set_hidden(enum_hidden_type hidden) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Column collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Column *add_column() = 0;

  virtual const Column_collection &columns() const = 0;

  virtual Column_collection *columns() = 0;

  virtual const Column *get_column(const String_type &name) const = 0;

  /**
    Allocate a new object graph and invoke the copy contructor for
    each object.

    @return pointer to dynamically allocated copy
  */
  virtual Abstract_table *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////
}  // namespace dd

#endif  // DD__ABSTRACT_TABLE_INCLUDED
