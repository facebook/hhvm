/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__FOREIGN_KEY_INCLUDED
#define DD__FOREIGN_KEY_INCLUDED

#include "sql/dd/collection.h"           // dd::Collection
#include "sql/dd/sdi_fwd.h"              // dd::Sdi_wcontext
#include "sql/dd/types/entity_object.h"  // dd::Entity_object
#include "sql/dd/types/object_table.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Foreign_key_element;
class Foreign_key_impl;
class Index;
class Table;

namespace tables {
class Foreign_keys;
}

///////////////////////////////////////////////////////////////////////////

class Foreign_key : virtual public Entity_object {
 public:
  typedef Collection<Foreign_key_element *> Foreign_key_elements;
  typedef Foreign_key_impl Impl;
  typedef tables::Foreign_keys DD_table;

 public:
  enum enum_rule {
    RULE_NO_ACTION = 1,
    RULE_RESTRICT,
    RULE_CASCADE,
    RULE_SET_NULL,
    RULE_SET_DEFAULT
  };

  enum enum_match_option {
    OPTION_NONE = 1,
    OPTION_PARTIAL,
    OPTION_FULL,
  };

 public:
  virtual ~Foreign_key() {}

  /////////////////////////////////////////////////////////////////////////
  // parent table.
  /////////////////////////////////////////////////////////////////////////

  virtual const Table &table() const = 0;

  virtual Table &table() = 0;

  /////////////////////////////////////////////////////////////////////////
  // unique_constraint
  /////////////////////////////////////////////////////////////////////////

  // Note that setting "" as unique constraint name is interpreted as NULL
  // when storing this to the DD tables. And correspondingly, if NULL is
  // stored, and the dd::Foreign_key is fetched from the tables, then
  // unique_constraint_name() will return "".

  virtual const String_type &unique_constraint_name() const = 0;
  virtual void set_unique_constraint_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // match_option.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_match_option match_option() const = 0;
  virtual void set_match_option(enum_match_option match_option) = 0;

  /////////////////////////////////////////////////////////////////////////
  // update_rule.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_rule update_rule() const = 0;
  virtual void set_update_rule(enum_rule update_rule) = 0;

  /////////////////////////////////////////////////////////////////////////
  // delete_rule.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_rule delete_rule() const = 0;
  virtual void set_delete_rule(enum_rule delete_rule) = 0;

  /////////////////////////////////////////////////////////////////////////
  // the catalog name of the referenced table.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &referenced_table_catalog_name() const = 0;
  virtual void set_referenced_table_catalog_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // the schema name of the referenced table.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &referenced_table_schema_name() const = 0;
  virtual void set_referenced_table_schema_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // the name of the referenced table.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &referenced_table_name() const = 0;
  virtual void set_referenced_table_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Foreign key element collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Foreign_key_element *add_element() = 0;

  virtual const Foreign_key_elements &elements() const = 0;

  virtual Foreign_key_elements *elements() = 0;

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

// Class to hold de-normalized information about FKs to be added
// to dd::Table objects representing FK parents.

class Foreign_key_parent {
 public:
  Foreign_key_parent()
      : m_child_schema_name(),
        m_child_table_name(),
        m_fk_name(),
        m_update_rule(Foreign_key::enum_rule::RULE_NO_ACTION),
        m_delete_rule(Foreign_key::enum_rule::RULE_NO_ACTION) {}

  const String_type &child_schema_name() const { return m_child_schema_name; }

  void set_child_schema_name(const String_type &child_schema_name) {
    m_child_schema_name = child_schema_name;
  }

  const String_type &child_table_name() const { return m_child_table_name; }

  void set_child_table_name(const String_type &child_table_name) {
    m_child_table_name = child_table_name;
  }

  const String_type &fk_name() const { return m_fk_name; }

  void set_fk_name(const String_type &fk_name) { m_fk_name = fk_name; }

  Foreign_key::enum_rule update_rule() const { return m_update_rule; }

  void set_update_rule(Foreign_key::enum_rule update_rule) {
    m_update_rule = update_rule;
  }

  Foreign_key::enum_rule delete_rule() const { return m_delete_rule; }

  void set_delete_rule(Foreign_key::enum_rule delete_rule) {
    m_delete_rule = delete_rule;
  }

 private:
  String_type m_child_schema_name;
  String_type m_child_table_name;
  String_type m_fk_name;
  Foreign_key::enum_rule m_update_rule;
  Foreign_key::enum_rule m_delete_rule;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__FOREIGN_KEY_INCLUDED
