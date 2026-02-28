/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__CHECK_CONSTRAINTS_INCLUDED
#define DD__CHECK_CONSTRAINTS_INCLUDED

#include "sql/dd/types/entity_object.h"  // dd::Entity_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Table;
class Check_constraint_impl;

namespace tables {
class Check_constraints;
}

///////////////////////////////////////////////////////////////////////////

class Check_constraint : virtual public Entity_object {
 public:
  typedef Check_constraint_impl Impl;
  typedef tables::Check_constraints DD_table;

 public:
  enum enum_constraint_state { CS_NOT_ENFORCED = 1, CS_ENFORCED };

 public:
  virtual ~Check_constraint() {}

  /////////////////////////////////////////////////////////////////////////
  // State. (enforced / not enforced)
  /////////////////////////////////////////////////////////////////////////

  virtual enum_constraint_state constraint_state() const = 0;
  virtual void set_constraint_state(bool is_enforced) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Check clause/utf8.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &check_clause() const = 0;
  virtual void set_check_clause(const String_type &check_clause) = 0;

  virtual const String_type &check_clause_utf8() const = 0;
  virtual void set_check_clause_utf8(const String_type &check_clause_utf8) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Alter mode ( true / false).
  // Set during ALTER TABLE operation. In alter mode, alias name is stored
  // in data-dictionary to avoid name conflicts.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_alter_mode() const = 0;
  virtual void set_alter_mode(bool alter_mode) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Alias check constriaint name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &alias_name() const = 0;
  virtual void set_alias_name(const String_type &alias_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Parent table.
  /////////////////////////////////////////////////////////////////////////

  virtual const Table &table() const = 0;

  virtual Table &table() = 0;

  /**
    Converts *this into json.

    Converts all member variables that are to be included in the sdi
    into json by transforming them appropriately and passing them to
    the rapidjson writer provided.

    @param wctx opaque context for data needed by serialization
    @param w rapidjson writer which will perform conversion to json

  */

  virtual void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const = 0;

  /**
    Re-establishes the state of *this by reading sdi information from
    the rapidjson DOM subobject provided.

    Cross-references encountered within this object are tracked in
    sdictx, so that they can be updated when the entire object graph
    has been established.

    @param rctx stores book-keeping information for the
    deserialization process
    @param val subobject of rapidjson DOM containing json
    representation of this object
    @retval false success
    @retval true  failure
  */

  virtual bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__CHECK_CONSTRAINTS_INCLUDED
