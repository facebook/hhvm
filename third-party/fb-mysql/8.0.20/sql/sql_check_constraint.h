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

#ifndef SQL_CHECK_CONSTRAINT_INCLUDED
#define SQL_CHECK_CONSTRAINT_INCLUDED

#include "lex_string.h"          // LEX_STRING
#include "sql/mem_root_array.h"  // Mem_root_array

class Alter_column;
class Alter_drop;
class Create_field;
class Item;
class String;
struct TABLE;
class THD;
class Value_generator;

/**
  Class to represent the check constraint specifications obtained from the SQL
  statement parse.
*/
class Sql_check_constraint_spec {
 public:
  /**
    Validate check constraint name, perform per item-type to check if the
    expression is allowed for the check constraint. Check expression is
    pre-validated at this stage. Validation of specific functions in expression
    is done later in the method open_table_from_share.

    @retval  false  Success.
    @retval  true   Failure.
  */
  bool pre_validate();

  /**
    Write check constraint expression into a String with proper syntax.

    @param[in]   thd   Thread handle.
    @param[out]  out   Check constraint expression.
  */
  void print_expr(THD *thd, String &out);

  /**
    Method to check if column "column_name" referred in the check constraint
    expression.

    @param[in]  column_name   Column name.

    @retval     true       If column name is referenced in the check expression.
    @retval     false      Otherwise.
  */
  bool expr_refers_column(const char *column_name);

 public:
  /// Name of the check constraint.
  LEX_STRING name{nullptr, 0};

  /// Check constraint expression.
  Item *check_expr{nullptr};

  /// Name of the column if check clause is defined at the column level.
  LEX_STRING column_name{nullptr, 0};

  /// Check constraint state (enforced/not enforced)
  bool is_enforced{true};

  /**
    During ALTER TABLE operation, the state of the Sql_check_constraint_spec
    instance(s) is set to alter mode in new table definition. In this
    mode, alias_name is stored to data-dictionary tables to avoid name
    conflicts. The name of the check constraint is updated to actual name after
    older table version is either dropped or when new version of table is
    renamed to actual table name.
  */
  bool is_alter_mode{false};

  /// Alias name for check constraints.
  LEX_STRING alias_name{nullptr, 0};
};

/**
  Class to represent check constraint in the TABLE_SHARE.

  The instance of Sql_check_constraint_share contains information as name,
  state and expression in string form. These informations are filled from
  the data-dictionary. The check expression is not in itemized (materialized)
  form here.
*/
class Sql_check_constraint_share {
 public:
  Sql_check_constraint_share() = default;

  Sql_check_constraint_share(const LEX_CSTRING &name,
                             const LEX_CSTRING &expr_str, bool is_enforced)
      : m_name(name), m_expr_str(expr_str), m_is_enforced(is_enforced) {}

  /// Constraint name.
  LEX_CSTRING &name() { return m_name; }
  /// Check expression in string form.
  LEX_CSTRING &expr_str() { return m_expr_str; }
  /// Check constraint state (enforced / not enforced)
  bool is_enforced() { return m_is_enforced; }

 private:
  /// Check constraint name.
  LEX_CSTRING m_name{nullptr, 0};

  /// Check constraint expression.
  LEX_CSTRING m_expr_str{nullptr, 0};

  /// Check constraint state.
  bool m_is_enforced{true};
};

/**
  Class to represent check constraint in the TABLE instance.

  The Sql_table_check_constraint is a Sql_check_constraint_share with reference
  to the parent TABLE instance and itemized (materialized) form of check
  constraint expression.
  Sql_table_check_constraint is prepared from the Sql_check_constraint_share of
  TABLE_SHARE instance.
*/
class Sql_table_check_constraint : public Sql_check_constraint_share {
 public:
  Sql_table_check_constraint() = default;

  Sql_table_check_constraint(const LEX_CSTRING &name,
                             const LEX_CSTRING &expr_str, bool is_enforced,
                             Value_generator *val_gen, TABLE *table)
      : Sql_check_constraint_share(name, expr_str, is_enforced),
        m_val_gen(val_gen),
        m_table(table) {}

  /// Value generator.
  Value_generator *value_generator() { return m_val_gen; }
  void set_value_generator(Value_generator *val_gen) { m_val_gen = val_gen; }

  /// Reference to owner table.
  TABLE *table() const { return m_table; }

 private:
  /// Value generator for the check constraint expression.
  Value_generator *m_val_gen{nullptr};

  /// Parent table reference.
  TABLE *m_table{nullptr};
};

/// Type for the list of Sql_check_constraint_spec elements.
using Sql_check_constraint_spec_list =
    Mem_root_array<Sql_check_constraint_spec *>;

/// Type for the list of Sql_check_constraint_share elements.
using Sql_check_constraint_share_list =
    Mem_root_array<Sql_check_constraint_share>;

/// Type for the list of Sql_table_check_constraint elements.
using Sql_table_check_constraint_list =
    Mem_root_array<Sql_table_check_constraint>;

/**
  Method to check if server is a slave server and master server is on a
  version not supporting check constraints feature. Check constraint support
  is introduced in server version 80016.

  Method is used by methods prepare_check_constraints_for_create() and
  prepare_check_constraints_for_alter(). Check constraints are not prepared
  (and specification list is cleared) when this method returns to true.
  In older versions, check constraint syntax was supported but check constraint
  feature was not supported. So if master is on older version and slave gets
  event with check constraint syntax then on slave supporting check constraint,
  query is parsed but during prepare time the specifications are ignored
  for the statement(event).

  @retval  true   if server is a slave server and master server is on a version
                  not supporting check constraints feature.
  @retval  false  Otherwise.
*/
bool is_slave_with_master_without_check_constraints_support(THD *thd);

/**
  Check if constraint expression refers to only "column_name" column of the
  table.

  @param[in]  check_expr    Check constraint expression.
  @param[in]  column_name   Column name.

  @retval     true       If expression refers to only "column_name".
  @retval     false      If expression refers to more than one column
                         or if expression does not refers to "column_name".
*/
bool check_constraint_expr_refers_to_only_column(Item *check_expr,
                                                 const char *column_name);

/**
  Helper class to check if column being dropped or removed in ALTER statement
  is in use by Check constraints.
*/
class Check_constraint_column_dependency_checker {
 public:
  explicit Check_constraint_column_dependency_checker(
      const Sql_check_constraint_spec_list &check_constraint_list)
      : m_check_constraint_list(check_constraint_list) {}

  /**
    Method to check if column being dropped is in use by check constraints.

    @param   drop    Instance of Alter_drop.

    @retval  true    If some check constraint uses the column being dropped.
    @retval  false   Otherwise.
  */
  bool operator()(const Alter_drop *drop);

  /**
    Method to check if column being renamed using RENAME COLUMN clause of the
    ALTER TABLE statement is in use by check constraints.

    @param   alter_column   Instance of Alter_column.

    @retval  true    If some check constraint uses the column being renamed.
    @retval  false   Otherwise.
  */
  bool operator()(const Alter_column *alter_column);

  /**
    Method to check if column being renamed using CHANGE [COLUMN] clause of the
    ALTER TABLE statement is in use by check constraints.

    @param   fld     Instance of Create_field.

    @retval  true    If some check constraint uses the column being renamed.
    @retval  false   Otherwise.
  */
  bool operator()(const Create_field &fld);

 private:
  /**
    Check if any check constraint uses "column_name".

    @param   column_name  Column name.

    @retval  true         If column is used by the check constraint.
    @retval  false        Otherwise.
  */
  bool any_check_constraint_uses_column(const char *column_name);

 private:
  /// Check constraint specification list.
  const Sql_check_constraint_spec_list &m_check_constraint_list;
};
#endif  // SQL_CHECK_CONSTRAINT_INCLUDED
