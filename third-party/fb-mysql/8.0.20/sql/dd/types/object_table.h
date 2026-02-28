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

#ifndef DD__OBJECT_TABLE_INCLUDED
#define DD__OBJECT_TABLE_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/string_type.h"  // dd::String_type

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table_definition;
class Properties;

///////////////////////////////////////////////////////////////////////////

/**
  This class represents all data dictionary table like mysql.tables,
  mysql.columns and more. This is the base class of all the classes
  defined in sql/dd/impl/tables/ headers. This class is also the base
  class of tables requested by the DDSE and by plugins.

  The server code should contain a Object_table subclass for each DD table
  which is a target table for at least one of the supported DD versions (i.e.,
  the DD versions from which this server can upgrade). So even if a previous
  DD version stops using a DD table, the later servers which can upgrade need
  to keep the Object_table subclass for that table. The motivation for that
  is to be able to recognize the table, and to be able to remove it.

  Instances of this class will contain one or two table definitions, depending
  on the context:

  - The actual table definition reflects the persistently stored DD table,
    i.e., what is reflected in the persistently stored mete data.
  - The target table definition reflects the DD table which the server
    is using during normal operation.

  If the actual DD version is different from the target DD version, upgrade
  is required. The actual table definition is used only in situations where
  we have an upgrade or downgrade.

  @note This class may be inherited along different paths
        for some subclasses due to the diamond shaped
        inheritance hierarchy; thus, direct subclasses
        must inherit this class virtually.
*/

class Object_table {
 public:
  /**
    Allocate a new Object_table instance on the heap.

    The new instance has the predefined options that all DD tables share:

      ENGINE=INNODB
      DEFAULT CHARSET=utf8
      COLLATE=utf8_bin
      ROW_FORMAT=DYNAMIC
      STATS_PERSISTENT=0
      TABLESPACE=mysql

    @note The object is owned by the caller.

    @returns pointer to new Object_table instance.
  */
  static Object_table *create_object_table();

  /**
    Get the table name used by the target definition for the dictionary table.

    @return table name.
  */
  virtual const String_type &name() const = 0;

  /**
    Get the target definition for the dictionary table.

    @note There are const and non-const variants.

    @return Pointer to the definition of the table.
  */
  virtual Object_table_definition *target_table_definition() = 0;

  virtual const Object_table_definition *target_table_definition() const = 0;

  /**
    Mark the target definition for the dictionary table as abandoned.

    @param last_dd_version  Last version where this object table was used.
  */
  virtual void set_abandoned(uint last_dd_version) const = 0;

  /**
    Check if the dictionary table is abandoned.

    @return   true if the table is abandoned.
  */
  virtual bool is_abandoned() const = 0;

  /**
    Get the actual definition for the dictionary table.

    The actual definition is the definition which is used by a DD table
    which is stored persistently. Normally, for an ordinary running server,
    the actual table definitions are equal to the target table definitions.
    In an upgrade context, they may differ.

    @return Pointer to the definition of the table.
  */
  virtual const Object_table_definition *actual_table_definition() const = 0;

  /**
    Set the actual definition for the dictionary table.

    @param table_def_properties  Actual table definition represented as
                                 a set of properties.

    @return false if no error.
  */
  virtual bool set_actual_table_definition(
      const Properties &table_def_properties) const = 0;

  /**
    Get the field ordinal position in the object table.

    @return Integer ordinal position.
  */
  virtual int field_number(const String_type &field_label) const = 0;

  /**
    Execute low level code for populating the table.

    @return Boolean operation outcome, false if success.
  */
  virtual bool populate(THD *thd) const = 0;

  /**
    Check if the table should be hidden.

    Most of Object tables (alias DD tables) are hidden from users,
    but some of them are expected to be visible (not hidden) to user and be
    able to update them, e.g., innodb_index_stats/innodb_table_stats.

    @returns true if the table should be hidden.
  */
  virtual bool is_hidden() const = 0;

  /**
    Mark the dictionary table as hidden or visible.

    @param hidden  Set to 'true' if the table should be hidden.
  */
  virtual void set_hidden(bool hidden) = 0;

 public:
  virtual ~Object_table() {}
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__OBJECT_TABLE_INCLUDED
