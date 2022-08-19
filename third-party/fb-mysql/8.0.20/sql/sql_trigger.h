#ifndef SQL_TRIGGER_INCLUDED
#define SQL_TRIGGER_INCLUDED

/*
   Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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

///////////////////////////////////////////////////////////////////////////

/**
  @file

  @brief
  This file contains declarations of global public functions which are used
  directly from parser/executioner to perform basic operations on triggers
  (CREATE TRIGGER, DROP TRIGGER, ALTER TABLE, DROP TABLE, ...)
*/

///////////////////////////////////////////////////////////////////////////

#include "lex_string.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"  // SQLCOM_CREATE_TRIGGER, SQLCOM_DROP_TRIGGER
#include "sql/mdl.h"        // enum_mdl_type
#include "sql/sql_cmd.h"    // Sql_cmd

class THD;
struct TABLE;
struct TABLE_LIST;

namespace dd {
class Table;
}
///////////////////////////////////////////////////////////////////////////

/**
  Find trigger's table from trigger identifier.

  @param[in] thd                    Thread context.
  @param[in] db_name                Schema name.
  @param[in] trigger_name           Trigger name.
  @param[in] continue_if_not_exist  true if SQL statement contains
                                    "IF EXISTS" clause. That means a warning
                                    instead of error should be thrown if trigger
                                    with given name does not exist.
  @param[out] table                 Pointer to TABLE_LIST object for the
                                    table trigger.

  @return Operation status
    @retval false On success.
    @retval true  Otherwise.
*/

bool get_table_for_trigger(THD *thd, const LEX_CSTRING &db_name,
                           const LEX_STRING &trigger_name,
                           bool continue_if_not_exist, TABLE_LIST **table);

/**
  Check for table with triggers that old database name and new database name
  are the same. This functions is called while handling the statement
  RENAME TABLE to ensure that table moved within the same database.

  @param[in] db_name     Schema name.
  @param[in] table       Table.
  @param[in] new_db_name New schema name

  @note
    Set error ER_TRG_IN_WRONG_SCHEMA in Diagnostics_area in case
    function returns true.

  @return Operation status
    @retval false Either there is no triggers assigned to a table or
                  old and new schema name are the same.
    @retval true  Old and new schema name aren't the same.
*/

bool check_table_triggers_are_not_in_the_same_schema(const char *db_name,
                                                     const dd::Table &table,
                                                     const char *new_db_name);

/**
  Acquire either exclusive or shared MDL lock for a trigger
  in specified schema.

  @param[in] thd                     Current thread context
  @param[in] db                      Schema name
  @param[in] trg_name                Trigger name
  @param[in] trigger_name_mdl_type   Type of MDL to acquire for trigger name

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool acquire_mdl_for_trigger(THD *thd, const char *db, const char *trg_name,
                             enum_mdl_type trigger_name_mdl_type);

/**
  Acquire exclusive MDL lock for a trigger in specified schema.

  @param[in] thd         Current thread context
  @param[in] db          Schema name
  @param[in] trg_name    Trigger name

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

inline bool acquire_exclusive_mdl_for_trigger(THD *thd, const char *db,
                                              const char *trg_name) {
  return acquire_mdl_for_trigger(thd, db, trg_name, MDL_EXCLUSIVE);
}

/**
  Acquire shared MDL lock for a trigger in specified schema.

  @param[in] thd         Current thread context
  @param[in] db          Schema name
  @param[in] trg_name    Trigger name

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

inline bool acquire_shared_mdl_for_trigger(THD *thd, const char *db,
                                           const char *trg_name) {
  return acquire_mdl_for_trigger(thd, db, trg_name, MDL_SHARED_HIGH_PRIO);
}

/**
  Drop statistics from performance schema for every trigger
  associated with a table.

  @param schema_name Name of schema containing the table.
  @param table       Table reference, for that associated
                     triggers statistics has to be deleted.
*/

#ifdef HAVE_PSI_SP_INTERFACE
void remove_all_triggers_from_perfschema(const char *schema_name,
                                         const dd::Table &table);
#endif
///////////////////////////////////////////////////////////////////////////

/**
  This class has common code for CREATE/DROP TRIGGER statements.
*/

class Sql_cmd_ddl_trigger_common : public Sql_cmd {
 public:
  /**
    Set a table associated with a trigger.

    @param trigger_table  a table associated with a trigger.
  */

  void set_table(TABLE_LIST *trigger_table) { m_trigger_table = trigger_table; }

 protected:
  Sql_cmd_ddl_trigger_common() : m_trigger_table(nullptr) {}

  bool check_trg_priv_on_subj_table(THD *thd, TABLE_LIST *table) const;
  TABLE *open_and_lock_subj_table(THD *thd, TABLE_LIST *tables,
                                  MDL_ticket **mdl_ticket) const;

  /**
    Restore original state of meta-data locks.

    @param thd         current thread context
    @param mdl_ticket  granted metadata lock
  */

  void restore_original_mdl_state(THD *thd, MDL_ticket *mdl_ticket) const;

  TABLE_LIST *m_trigger_table;
};

/**
  This class implements the CREATE TRIGGER statement.
*/

class Sql_cmd_create_trigger : public Sql_cmd_ddl_trigger_common {
 public:
  /**
    Return the command code for CREATE TRIGGER
  */

  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_CREATE_TRIGGER;
  }

  virtual bool execute(THD *thd);
};

/**
  This class implements the DROP TRIGGER statement.
*/

class Sql_cmd_drop_trigger : public Sql_cmd_ddl_trigger_common {
 public:
  /**
    Return the command code for DROP TRIGGER
  */

  virtual enum_sql_command sql_command_code() const {
    return SQLCOM_DROP_TRIGGER;
  }

  virtual bool execute(THD *thd);
};

#endif /* SQL_TRIGGER_INCLUDED */
