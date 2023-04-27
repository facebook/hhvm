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

#ifndef SQL_CONSTRAINT_INCLUDED
#define SQL_CONSTRAINT_INCLUDED

#include "my_inttypes.h"  // ulonglong

class Alter_constraint_enforcement;
class Alter_drop;
class Alter_info;
class KEY;
struct TABLE;
class THD;
namespace dd {
class Table;
}

/**
  Base class to resolve constraints type for the constraints specified in the
  ALTER TABLE ... DROP/ALTER CONSTRAINT operation.

  For a constraint specified in the DROP/ALTER CONSTRAINT operations of a ALTER
  TABLE statement, actual constraint type is unknown. This is the class to
  resolve the actual constraint type (PRIMARY, UNIQUE, FOREIGN KEY or CHECK) of
  a constraint by name. Changes to Alter_info instance are reverted in the
  destructor to make operation safe for re-execution of a stored routines and
  prepared statements.
*/
class Constraint_type_resolver {
 public:
  explicit Constraint_type_resolver(Alter_info *alter_info)
      : m_alter_info(alter_info) {}
  virtual ~Constraint_type_resolver() = default;

  /**
    Method to check if constraint type resolution is needed.

    @retval true  if ALTER TABLE statement has DROP/ALTER CONSTRAINT operations.
    @retval false Otherwise.
  */
  virtual bool is_type_resolution_needed() const = 0;

  /**
    Method to resolve constraints type.

    @param            thd                      Thread handle.
    @param            src_table                TABLE instance of a source table.
    @param            dd_src_table             Data-dictionary table instance of
                                               a source table.

    @retval           false                    Success.
    @retval           true                     Failure.
  */
  virtual bool resolve_constraints_type(THD *thd, const TABLE *src_table,
                                        const dd::Table *dd_src_table) = 0;

 protected:
  /**
    Helper method to check if "name" type is PRIMARY or UNIQUE constraint.

    @param  src_table      TABLE instance of a source table.
    @param  name           Constraint name.

    @retval  non-nullptr   Returns pointer to KEY instance if "name" is PRIMARY
                           or UNIQUE constraint.
    @retval  nullptr       Otherwise.
  */
  KEY *is_primary_or_unique_constraint(const TABLE *src_table,
                                       const char *name);

  /**
    Helper method to check if "name" type is REFERENTIAL constraint.

    @param  dd_src_table   Data-dictionary table instance of a source table.
    @param  name           Constraint name.

    @retval true           If "name" type is REFERENTIAL.
    @retval false          Otherwise.
  */
  bool is_referential_constraint(const dd::Table *dd_src_table,
                                 const char *name);

  /**
    Helper method to check if "name" type is CHECK constraint.

    @param  src_table      TABLE instance of a source table.
    @param  name           Constraint name.

    @retval true           If "name" type is CHECK.
    @retval false          Otherwise.
  */
  bool is_check_constraint(const TABLE *src_table, const char *name);

 protected:
  /// Alter_info instance describing table being altered.
  Alter_info *m_alter_info;
};

/**
  Class to resolve constraints type for the constraints specified in the
  ALTER TABLE ... DROP CONSTRAINT operation.

  For a constraint specified in the DROP CONSTRAINT operations of a ALTER TABLE
  statement, actual constraint type is unknown. This is the class to resolve
  actual constraint type (PRIMARY, UNIQUE, FOREIGN KEY or CHECK) by constraint
  name. Alter_drop elements with actual constraint type are added to the
  Alter_info::Alter_drop list. To make changes safe for re-execution of a stored
  routines and prepared statements, elements added to Alter_drop list are
  removed in the destructor.
*/
class Drop_constraint_type_resolver final : public Constraint_type_resolver {
 public:
  explicit Drop_constraint_type_resolver(Alter_info *alter_info)
      : Constraint_type_resolver(alter_info) {}
  ~Drop_constraint_type_resolver() override;

  /**
    Method to check if constraint type resolution is needed.

    @retval true  if ALTER TABLE statement has DROP CONSTRAINT operations.
    @retval false Otherwise.
  */
  bool is_type_resolution_needed() const override;

  /**
    Method to resolve constraints type.

    @param            thd                      Thread handle.
    @param            src_table                TABLE instance of a source table.
    @param            dd_src_table             Data-dictionary table instance of
                                               a source table.

    @retval           false                    Success.
    @retval           true                     Failure.
  */
  bool resolve_constraints_type(THD *thd, const TABLE *src_table,
                                const dd::Table *dd_src_table) override;

 private:
  /**
    Method to resolve constraint type.

    @param            thd                      Thread handle.
    @param            src_table                TABLE instance of a source table.
    @param            dd_src_table             Data-dictionary table instance of
                                               a source table.
    @param            drop                     Alter_drop instance for which
                                               type should be resolved.

    @retval           false                    Success.
    @retval           true                     Failure.
  */
  bool resolve_constraint_type(THD *thd, const TABLE *src_table,
                               const dd::Table *dd_src_table,
                               const Alter_drop *drop);

 private:
  /// Flags set in Alter_info::flags while fixing type for constraint.
  ulonglong m_flags{0};

  /*
    Alter_drop element with actual type is added to the Alter_info::drop_list
    for each DROP CONSTRAINT operation. Member holds the position of the first
    Alter_drop element with actual type in the Alter_info::drop_list list.
  */
  uint m_first_fixed_alter_drop_pos{0};
};

/**
  Class to resolve constraints type for the constraints specified in the
  ALTER TABLE ... ALTER CONSTRAINT operation.

  For a constraint specified in the ALTER CONSTRAINT operations of a ALTER TABLE
  statement, actual constraint type is unknown. This is the class to resolve
  actual constraint type (PRIMARY, UNIQUE, FOREIGN KEY or CHECK) by constraint
  name. Alter_constraint_enforcement elements with actual constraint type are
  added to the alter_constraint_enforcement_list. To make changes safe for
  re-execution of stored routines and prepared statements, elements added to
  list are removed in the destructor.
*/
class Enforce_constraint_type_resolver final : public Constraint_type_resolver {
 public:
  explicit Enforce_constraint_type_resolver(Alter_info *alter_info)
      : Constraint_type_resolver(alter_info) {}
  ~Enforce_constraint_type_resolver() override;

  /**
    Method to check if constraint type resolution is needed.

    @retval true  if ALTER TABLE statement has ALTER CONSTRAINT operations.
    @retval false Otherwise.
  */
  bool is_type_resolution_needed() const override;

  /**
    Method to resolve constraints type.

    @param            thd                      Thread handle.
    @param            src_table                TABLE instance of a source table.
    @param            dd_src_table             Data-dictionary table instance of
                                               a source table.

    @retval           false                    Success.
    @retval           true                     Failure.
  */
  bool resolve_constraints_type(THD *thd, const TABLE *src_table,
                                const dd::Table *dd_src_table) override;

 private:
  /**
    Method to resolve constraint type.

    @param            thd                      Thread handle.
    @param            src_table                TABLE instance of a source table.
    @param            dd_src_table             Data-dictionary table instance of
                                               a source table.
    @param            alter_constraint         Alter_constraint_enforcement
                                               instance for which type should be
                                               resolved.

    @retval           false                    Success.
    @retval           true                     Failure.
  */
  bool resolve_constraint_type(
      THD *thd, const TABLE *src_table, const dd::Table *dd_src_table,
      const Alter_constraint_enforcement *alter_constraint);

 private:
  /// Flags set in Alter_info::flags while fixing type for check constraint.
  ulonglong m_flags{0};

  /*
    Alter_constraint_enforcement element with actual type is added to the
    Alter_info::alter_constraint_enforcement_list for each ALTER CONSTRAINT
    operation. Member holds position of the first Alter_constraint_enforcement
    element with actual type in the alter_constraint_enforcement_list.
  */
  uint m_first_fixed_alter_constraint_pos{0};
};
#endif  // SQL_CONSTRAINT_INCLUDED
