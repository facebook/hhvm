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

#ifndef DD__OBJECT_TABLE_DEFINITION_INCLUDED
#define DD__OBJECT_TABLE_DEFINITION_INCLUDED

#include <vector>

#include "my_inttypes.h"
#include "sql/dd/string_type.h"  // dd::String_type

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Properties;
class Table;

///////////////////////////////////////////////////////////////////////////

/**
  The purpose of this interface is to enable retrieving the SQL statements
  necessary to create and populate a DD table. An Object_table instance
  may use one or more instances implementing this interface to keep track
  of the table definitions corresponding to the supported DD versions.
*/

class Object_table_definition {
 public:
  virtual ~Object_table_definition() {}

  /**
    Set the name of the table.

    @param name    Table name.
  */
  virtual void set_table_name(const String_type &name) = 0;

  /**
    Add a field to the object table definition.

    @param field_number      Positional index of the field.
    @param field_name        Label which can be used to refer to the field.
    @param field_definition  Complete field definition with name, type etc.
  */
  virtual void add_field(int field_number, const String_type &field_name,
                         const String_type field_definition) = 0;

  /**
    Add an index to the object table definition.

    @param index_number      Positional index (sic!) of the index.
    @param index_name        Label which can be used to refer to the index.
    @param index_definition  Complete index definition.
  */
  virtual void add_index(int index_number, const String_type &index_name,
                         const String_type &index_definition) = 0;

  /**
    Get the SQL DDL statement for creating the dictionary table.

    @return String containing the SQL DDL statement for the target table.
  */
  virtual String_type get_ddl() const = 0;

  /**
    Get the SQL DML statements for populating the table.

    @return Vector of strings containing SQL DML statements
  */
  virtual const std::vector<String_type> &get_dml() const = 0;

  /**
    Store the elements of the object table definition into a property
    object.

    @param [out] table_def_properties  Properties object containing the
                                       definition.
  */
  virtual void store_into_properties(
      Properties *table_def_properties) const = 0;

  /**
    Restore the elements of the object table definition from a property
    object.

    @param table_def_properties  Properties object containing the
                                 definition.
    @return Operation outcome, false if no error.
  */
  virtual bool restore_from_properties(
      const Properties &table_def_properties) = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__OBJECT_TABLE_DEFINITION_INCLUDED
