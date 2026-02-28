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

#ifndef DD__ENTITY_OBJECT_TABLE_INCLUDED
#define DD__ENTITY_OBJECT_TABLE_INCLUDED

#include "sql/dd/types/object_table.h"  // dd::Object_table

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Entity_object;
class Open_dictionary_tables_ctx;
class Raw_record;

///////////////////////////////////////////////////////////////////////////

/**
  This class represents DD table like mysql.schemata,
  mysql.tables, mysql.tablespaces and more. These corresponds to
  base DD table where the Entity_object's are persisted.

  This class does not represent table like mysql.columns,
  mysql.indexes which hold metadata child objects object of
  mysql.tables and are not directly created/searched/dropped
  without accessing mysql.tables or dd::Table Dictionary object.
*/
class Entity_object_table : virtual public Object_table {
 public:
  virtual ~Entity_object_table() {}

  virtual Entity_object *create_entity_object(
      const Raw_record &record) const = 0;

  virtual bool restore_object_from_record(Open_dictionary_tables_ctx *otx,
                                          const Raw_record &record,
                                          Entity_object **o) const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__ENTITY_OBJECT_TABLE_INCLUDED
