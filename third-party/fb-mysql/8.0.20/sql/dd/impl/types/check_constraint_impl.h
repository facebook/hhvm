/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__CHECK_CONSTRAINT_IMPL_INCLUDED
#define DD__CHECK_CONSTRAINT_IMPL_INCLUDED

#include <new>

#include "m_ctype.h"                               // my_strcasecmp
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/check_constraint.h"  // dd::Check_constraint

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;
class Raw_record;
class Sdi_rcontext;
class Sdi_wcontext;
class Table;
class Table_impl;

///////////////////////////////////////////////////////////////////////////

class Check_constraint_impl : public Entity_object_impl,
                              public Check_constraint {
 public:
  Check_constraint_impl();

  Check_constraint_impl(Table_impl *table);

  Check_constraint_impl(const Check_constraint_impl &src, Table_impl *parent);

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  const Object_table &object_table() const override;

  bool validate() const override;

  bool store(Open_dictionary_tables_ctx *otx) override;

  bool store_attributes(Raw_record *r) override;

  bool restore_attributes(const Raw_record &r) override;

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const override;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) override;

  void debug_print(String_type &outb) const override;

  void set_ordinal_position(uint) {}

  virtual uint ordinal_position() const { return -1; }

 public:
  /////////////////////////////////////////////////////////////////////////
  // State.
  /////////////////////////////////////////////////////////////////////////

  enum_constraint_state constraint_state() const override {
    return m_constraint_state;
  }

  void set_constraint_state(bool is_enabled) override {
    m_constraint_state = is_enabled ? CS_ENFORCED : CS_NOT_ENFORCED;
  }

  /////////////////////////////////////////////////////////////////////////
  // Check clause/utf8.
  /////////////////////////////////////////////////////////////////////////

  const String_type &check_clause() const override { return m_check_clause; }

  void set_check_clause(const String_type &check_clause) override {
    m_check_clause = check_clause;
  }

  const String_type &check_clause_utf8() const override {
    return m_check_clause_utf8;
  }

  void set_check_clause_utf8(const String_type &check_clause_utf8) override {
    m_check_clause_utf8 = check_clause_utf8;
  }

  /////////////////////////////////////////////////////////////////////////
  // Alter mode ( true / false).
  // Set during ALTER TABLE operation. In alter mode, alias_name is stored
  // in data-dictionary to avoid name conflicts.
  /////////////////////////////////////////////////////////////////////////

  bool is_alter_mode() const override { return m_alter_mode; }

  void set_alter_mode(bool alter_mode) override { m_alter_mode = alter_mode; }

  /////////////////////////////////////////////////////////////////////////
  // Alias check constriaint name.
  /////////////////////////////////////////////////////////////////////////

  const String_type &alias_name() const override { return m_alias_name; }

  void set_alias_name(const String_type &alias_name) override {
    m_alias_name = alias_name;
  }

  /////////////////////////////////////////////////////////////////////////
  // Table.
  /////////////////////////////////////////////////////////////////////////

  const Table &table() const override;

  Table &table() override;

 public:
  static Check_constraint_impl *restore_item(Table_impl *table) {
    return new (std::nothrow) Check_constraint_impl(table);
  }

  static Check_constraint_impl *clone(const Check_constraint_impl &other,
                                      Table_impl *table) {
    return new (std::nothrow) Check_constraint_impl(other, table);
  }

  // Fix "inherits ... via dominance" warnings
  Entity_object_impl *impl() override { return Entity_object_impl::impl(); }
  const Entity_object_impl *impl() const override {
    return Entity_object_impl::impl();
  }
  Object_id id() const override { return Entity_object_impl::id(); }
  bool is_persistent() const override {
    return Entity_object_impl::is_persistent();
  }
  const String_type &name() const override {
    return Entity_object_impl::name();
  }
  void set_name(const String_type &name) override {
    Entity_object_impl::set_name(name);
  }

 private:
  enum_constraint_state m_constraint_state{CS_ENFORCED};
  String_type m_check_clause;
  String_type m_check_clause_utf8;
  bool m_alter_mode{false};
  String_type m_alias_name;
  Object_id m_schema_id;

  // References to the Table object.
  Table_impl *m_table;
};

///////////////////////////////////////////////////////////////////////////

/** Class used to sort check constraints by name for the same table. */
struct Check_constraint_order_comparator {
  bool operator()(const dd::Check_constraint *cc1,
                  const dd::Check_constraint *cc2) const {
    return (my_strcasecmp(system_charset_info, cc1->name().c_str(),
                          cc2->name().c_str()) < 0);
  }
};

///////////////////////////////////////////////////////////////////////////
}  // namespace dd

#endif  // DD__CHECK_CONSTRAINT_IMPL_INCLUDED
