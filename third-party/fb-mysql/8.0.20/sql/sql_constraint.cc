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

#include "sql/sql_constraint.h"

#include "my_sys.h"              // my_error
#include "mysqld_error.h"        // ER_*
#include "sql/dd/types/table.h"  // dd::Table
#include "sql/field.h"           // Field
#include "sql/key.h"             // KEY
#include "sql/sql_alter.h"       // Alter_info
#include "sql/sql_class.h"       // THD
#include "sql/table.h"           // TABLE

KEY *Constraint_type_resolver::is_primary_or_unique_constraint(
    const TABLE *src_table, const char *name) {
  if (src_table->key_info == nullptr) return nullptr;

  for (KEY *key_info = src_table->key_info;
       key_info < (src_table->key_info + src_table->s->keys); key_info++) {
    if ((key_info->flags & HA_NOSAME) &&
        !my_strcasecmp(system_charset_info, key_info->name, name))
      return key_info;
  }

  return nullptr;
}

bool Constraint_type_resolver::is_referential_constraint(
    const dd::Table *dd_src_table, const char *name) {
  if (dd_src_table == nullptr) return false;

  for (const dd::Foreign_key *fk : dd_src_table->foreign_keys()) {
    if (!my_strcasecmp(system_charset_info, fk->name().c_str(), name))
      return true;
  }

  return false;
}

bool Constraint_type_resolver::is_check_constraint(const TABLE *src_table,
                                                   const char *name) {
  if (src_table->table_check_constraint_list == nullptr) return false;

  for (Sql_table_check_constraint &table_cc :
       *src_table->table_check_constraint_list) {
    if (!my_strcasecmp(system_charset_info, table_cc.name().str, name))
      return true;
  }

  return false;
}

Drop_constraint_type_resolver::~Drop_constraint_type_resolver() {
  if (!is_type_resolution_needed()) return;

  // Erase Alter_drop elements added while resolving constraint type.
  m_alter_info->drop_list.erase(
      m_alter_info->drop_list.cbegin() + m_first_fixed_alter_drop_pos,
      m_alter_info->drop_list.cend());

  // Reset Alter_info::flags.
  m_alter_info->flags |= m_flags;
}

bool Drop_constraint_type_resolver::is_type_resolution_needed() const {
  return (m_alter_info->flags & Alter_info::DROP_ANY_CONSTRAINT);
}

bool Drop_constraint_type_resolver::resolve_constraint_type(
    THD *thd, const TABLE *src_table, const dd::Table *dd_src_table,
    const Alter_drop *drop) {
  Alter_drop::drop_type type = Alter_drop::ANY_CONSTRAINT;
  uint16_t found_count = 0;

  // PRIMARY and UNIQUE constraint.
  KEY *key_info = is_primary_or_unique_constraint(src_table, drop->name);
  if (key_info != nullptr) {
    found_count++;
    type = Alter_drop::KEY;
    m_flags |= ((m_alter_info->flags ^ Alter_info::ALTER_DROP_INDEX) &
                Alter_info::ALTER_DROP_INDEX);
  }

  // REFERENTIAL constraint.
  if (is_referential_constraint(dd_src_table, drop->name)) {
    found_count++;
    type = Alter_drop::FOREIGN_KEY;
    m_flags |= ((m_alter_info->flags ^ Alter_info::DROP_FOREIGN_KEY) &
                Alter_info::DROP_FOREIGN_KEY);
  }

  // CHECK constraint.
  if ((found_count < 2) && is_check_constraint(src_table, drop->name)) {
    found_count++;
    type = Alter_drop::CHECK_CONSTRAINT;
    m_flags |= ((m_alter_info->flags ^ Alter_info::DROP_CHECK_CONSTRAINT) &
                Alter_info::DROP_CHECK_CONSTRAINT);
  }

  if (found_count == 2) {
    my_error(ER_MULTIPLE_CONSTRAINTS_WITH_SAME_NAME, MYF(0), drop->name,
             "DROP");
    return true;
  }
  if (type == Alter_drop::ANY_CONSTRAINT) {
    my_error(ER_CONSTRAINT_NOT_FOUND, MYF(0), drop->name);
    return true;
  }

  // Add new Alter_drop element with actual type to drop_list.
  Alter_drop *new_drop = new (thd->mem_root) Alter_drop(type, drop->name);
  if (!new_drop) return true;  // OOM error.
  m_alter_info->drop_list.push_back(new_drop);

  if (key_info == nullptr) return false;

  /*
    If constraint is a PRIMARY or UNIQUE then check if it is a functional
    index. In that case, add hidden generated columns of functional index
    to the drop list.
  */
  for (uint i = 0; i < key_info->user_defined_key_parts; i++) {
    if (key_info->key_part[i].field->is_field_for_functional_index()) {
      Alter_drop *column_drop = new (thd->mem_root) Alter_drop(
          Alter_drop::COLUMN, key_info->key_part[i].field->field_name);
      if (!column_drop) return true;  // OOM error.
      m_alter_info->drop_list.push_back(column_drop);
      m_flags |= ((m_alter_info->flags ^ Alter_info::ALTER_DROP_COLUMN) &
                  Alter_info::ALTER_DROP_COLUMN);
    }
  }

  return false;
}

bool Drop_constraint_type_resolver::resolve_constraints_type(
    THD *thd, const TABLE *src_table, const dd::Table *dd_src_table) {
  m_first_fixed_alter_drop_pos = m_alter_info->drop_list.size();

  for (const Alter_drop *drop : m_alter_info->drop_list) {
    if (drop->type != Alter_drop::ANY_CONSTRAINT) continue;

    if (resolve_constraint_type(thd, src_table, dd_src_table, drop))
      return true;
  }

  // Update Alter_info flags.
  m_alter_info->flags |= m_flags;
  return false;
}

Enforce_constraint_type_resolver::~Enforce_constraint_type_resolver() {
  if (!is_type_resolution_needed()) return;

  /*
    Erase Alter_constraint_enforcement elements added while resolving
    constraint type.
  */
  m_alter_info->alter_constraint_enforcement_list.erase(
      m_alter_info->alter_constraint_enforcement_list.cbegin() +
          m_first_fixed_alter_constraint_pos,
      m_alter_info->alter_constraint_enforcement_list.cend());

  // Reset Alter_info::flags.
  m_alter_info->flags |= m_flags;
}

bool Enforce_constraint_type_resolver::is_type_resolution_needed() const {
  return (m_alter_info->flags & (Alter_info::ENFORCE_ANY_CONSTRAINT |
                                 Alter_info::SUSPEND_ANY_CONSTRAINT));
}

bool Enforce_constraint_type_resolver::resolve_constraint_type(
    THD *thd, const TABLE *src_table, const dd::Table *dd_src_table,
    const Alter_constraint_enforcement *alter_constraint) {
  Alter_constraint_enforcement::Type type =
      Alter_constraint_enforcement::Type::ANY_CONSTRAINT;

  // CHECK constraint.
  if (is_check_constraint(src_table, alter_constraint->name)) {
    type = Alter_constraint_enforcement::Type::CHECK_CONSTRAINT;
    ulonglong check_cons_flag = alter_constraint->is_enforced
                                    ? Alter_info::ENFORCE_CHECK_CONSTRAINT
                                    : Alter_info::SUSPEND_CHECK_CONSTRAINT;
    m_flags |= ((m_alter_info->flags ^ check_cons_flag) & check_cons_flag);
  }

  // PRIMARY, UNIQUE or REFERENTIAL constraint.
  if ((is_primary_or_unique_constraint(src_table, alter_constraint->name) !=
       nullptr) ||
      is_referential_constraint(dd_src_table, alter_constraint->name)) {
    if (type != Alter_constraint_enforcement::Type::ANY_CONSTRAINT)
      my_error(ER_MULTIPLE_CONSTRAINTS_WITH_SAME_NAME, MYF(0),
               alter_constraint->name, "ALTER");
    else {
      // Enforcement state alter is supported for only CHECK constraints.
      my_error(ER_ALTER_CONSTRAINT_ENFORCEMENT_NOT_SUPPORTED, MYF(0),
               alter_constraint->name);
    }
    return true;
  }

  if (type == Alter_constraint_enforcement::Type::ANY_CONSTRAINT) {
    my_error(ER_CONSTRAINT_NOT_FOUND, MYF(0), alter_constraint->name);
    return true;
  }

  /*
    Add new Alter_constraint_enforcement element with the actual type to
    alter_constraint_enforcement list.
  */
  Alter_constraint_enforcement *new_alter_constraint =
      new (thd->mem_root) Alter_constraint_enforcement(
          type, alter_constraint->name, alter_constraint->is_enforced);
  if (!new_alter_constraint) return true;  // OOM error.
  m_alter_info->alter_constraint_enforcement_list.push_back(
      new_alter_constraint);
  return false;
}

bool Enforce_constraint_type_resolver::resolve_constraints_type(
    THD *thd, const TABLE *src_table, const dd::Table *dd_src_table) {
  m_first_fixed_alter_constraint_pos =
      m_alter_info->alter_constraint_enforcement_list.size();

  for (const Alter_constraint_enforcement *alter_constraint :
       m_alter_info->alter_constraint_enforcement_list) {
    if (alter_constraint->type !=
        Alter_constraint_enforcement::Type::ANY_CONSTRAINT)
      continue;

    if (resolve_constraint_type(thd, src_table, dd_src_table, alter_constraint))
      return true;
  }

  // Update Alter_info flags.
  m_alter_info->flags |= m_flags;
  return false;
}
