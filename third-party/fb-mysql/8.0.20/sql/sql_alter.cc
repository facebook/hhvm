/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_alter.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_access
#include "sql/create_field.h"
#include "sql/dd/types/trigger.h"  // dd::Trigger
#include "sql/derror.h"            // ER_THD
#include "sql/error_handler.h"     // Strict_error_handler
// mysql_exchange_partition
#include "sql/log.h"
#include "sql/mysqld.h"              // lower_case_table_names
#include "sql/parse_tree_helpers.h"  // is_identifier
#include "sql/sql_class.h"           // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_servers.h"
#include "sql/sql_table.h"  // mysql_alter_table,
#include "sql/table.h"
#include "template_utils.h"  // delete_container_pointers

bool has_external_data_or_index_dir(partition_info &pi);

Alter_info::Alter_info(const Alter_info &rhs, MEM_ROOT *mem_root)
    : drop_list(mem_root, rhs.drop_list.begin(), rhs.drop_list.end()),
      alter_list(mem_root, rhs.alter_list.begin(), rhs.alter_list.end()),
      key_list(mem_root, rhs.key_list.begin(), rhs.key_list.end()),
      alter_rename_key_list(mem_root, rhs.alter_rename_key_list.begin(),
                            rhs.alter_rename_key_list.end()),
      alter_index_visibility_list(mem_root,
                                  rhs.alter_index_visibility_list.begin(),
                                  rhs.alter_index_visibility_list.end()),
      alter_constraint_enforcement_list(
          mem_root, rhs.alter_constraint_enforcement_list.begin(),
          rhs.alter_constraint_enforcement_list.end()),
      check_constraint_spec_list(mem_root,
                                 rhs.check_constraint_spec_list.begin(),
                                 rhs.check_constraint_spec_list.end()),
      create_list(rhs.create_list, mem_root),
      flags(rhs.flags),
      keys_onoff(rhs.keys_onoff),
      partition_names(rhs.partition_names, mem_root),
      num_parts(rhs.num_parts),
      requested_algorithm(rhs.requested_algorithm),
      requested_lock(rhs.requested_lock),
      with_validation(rhs.with_validation),
      new_db_name(rhs.new_db_name),
      new_table_name(rhs.new_table_name) {
  /*
    Make deep copies of used objects.
    This is not a fully deep copy - clone() implementations
    of Create_list do not copy string constants. At the same length the only
    reason we make a copy currently is that ALTER/CREATE TABLE
    code changes input Alter_info definitions, but string
    constants never change.
  */
  List_iterator<Create_field> it(create_list);
  Create_field *el;
  while ((el = it++)) it.replace(el->clone(mem_root));

  /* partition_names are not deeply copied currently */
}

Alter_table_ctx::Alter_table_ctx()
    : datetime_field(nullptr),
      error_if_not_empty(false),
      tables_opened(0),
      db(nullptr),
      table_name(nullptr),
      alias(nullptr),
      new_db(nullptr),
      new_name(nullptr),
      new_alias(nullptr),
      fk_info(nullptr),
      fk_count(0),
      fk_max_generated_name_number(0)
#ifndef DBUG_OFF
      ,
      tmp_table(false)
#endif
{
}

Alter_table_ctx::Alter_table_ctx(THD *thd, TABLE_LIST *table_list,
                                 uint tables_opened_arg, const char *new_db_arg,
                                 const char *new_name_arg)
    : datetime_field(nullptr),
      error_if_not_empty(false),
      tables_opened(tables_opened_arg),
      new_db(new_db_arg),
      new_name(new_name_arg),
      fk_info(nullptr),
      fk_count(0),
      fk_max_generated_name_number(0)
#ifndef DBUG_OFF
      ,
      tmp_table(false)
#endif
{
  /*
    Assign members db, table_name, new_db and new_name
    to simplify further comparisions: we want to see if it's a RENAME
    later just by comparing the pointers, avoiding the need for strcmp.
  */
  db = table_list->db;
  table_name = table_list->table_name;
  alias = (lower_case_table_names == 2) ? table_list->alias : table_name;

  if (!new_db || !my_strcasecmp(table_alias_charset, new_db, db)) new_db = db;

  if (new_name) {
    DBUG_PRINT("info", ("new_db.new_name: '%s'.'%s'", new_db, new_name));

    if (lower_case_table_names ==
        1)  // Convert new_name/new_alias to lower case
    {
      my_casedn_str(files_charset_info, const_cast<char *>(new_name));
      new_alias = new_name;
    } else if (lower_case_table_names == 2)  // Convert new_name to lower case
    {
      my_stpcpy(new_alias_buff, new_name);
      new_alias = (const char *)new_alias_buff;
      my_casedn_str(files_charset_info, const_cast<char *>(new_name));
    } else
      new_alias = new_name;  // LCTN=0 => case sensitive + case preserving

    if (!is_database_changed() &&
        !my_strcasecmp(table_alias_charset, new_name, table_name)) {
      /*
        Source and destination table names are equal:
        make is_table_renamed() more efficient.
      */
      new_alias = table_name;
      new_name = table_name;
    }
  } else {
    new_alias = alias;
    new_name = table_name;
  }

  snprintf(tmp_name, sizeof(tmp_name), "%s-%lx_%x", tmp_file_prefix,
           current_pid, thd->thread_id());
  /* Safety fix for InnoDB */
  if (lower_case_table_names) my_casedn_str(files_charset_info, tmp_name);

  if (table_list->table->s->tmp_table == NO_TMP_TABLE) {
    build_table_filename(path, sizeof(path) - 1, db, table_name, "", 0);

    build_table_filename(new_path, sizeof(new_path) - 1, new_db, new_name, "",
                         0);

    build_table_filename(new_filename, sizeof(new_filename) - 1, new_db,
                         new_name, reg_ext, 0);

    build_table_filename(tmp_path, sizeof(tmp_path) - 1, new_db, tmp_name, "",
                         FN_IS_TMP);
  } else {
    /*
      We are not filling path, new_path and new_filename members if
      we are altering temporary table as these members are not used in
      this case. This fact is enforced with assert.
    */
    build_tmptable_filename(thd, tmp_path, sizeof(tmp_path));
#ifndef DBUG_OFF
    tmp_table = true;
#endif
  }

  /* Initialize MDL requests on new table name and database if necessary. */
  if (table_list->table->s->tmp_table == NO_TMP_TABLE) {
    if (is_table_renamed()) {
      MDL_REQUEST_INIT(&target_mdl_request, MDL_key::TABLE, new_db, new_name,
                       MDL_EXCLUSIVE, MDL_TRANSACTION);

      if (is_database_changed()) {
        MDL_REQUEST_INIT(&target_db_mdl_request, MDL_key::SCHEMA, new_db, "",
                         MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
      }
    }
  }
}

Alter_table_ctx::~Alter_table_ctx() {}

bool Sql_cmd_alter_table::execute(THD *thd) {
  /* Verify that none one of the DISCARD and IMPORT flags are set. */
  DBUG_ASSERT(!thd_tablespace_op(thd));
  DBUG_EXECUTE_IF("delay_alter_table_by_one_second", { my_sleep(1000000); });

  LEX *lex = thd->lex;
  /* first SELECT_LEX (have special meaning for many of non-SELECTcommands) */
  SELECT_LEX *select_lex = lex->select_lex;
  /* first table of first SELECT_LEX */
  TABLE_LIST *first_table = select_lex->get_table_list();
  /*
    Code in mysql_alter_table() may modify its HA_CREATE_INFO argument,
    so we have to use a copy of this structure to make execution
    prepared statement- safe. A shallow copy is enough as no memory
    referenced from this structure will be modified.
    @todo move these into constructor...
  */
  HA_CREATE_INFO create_info(*lex->create_info);
  Alter_info alter_info(*m_alter_info, thd->mem_root);
  ulong priv = 0;
  ulong priv_needed = ALTER_ACL;
  bool result;

  DBUG_TRACE;

  if (thd->is_fatal_error()) /* out of memory creating a copy of alter_info */
    return true;

  {
    partition_info *part_info = thd->lex->part_info;
    if (part_info != nullptr && has_external_data_or_index_dir(*part_info) &&
        check_access(thd, FILE_ACL, any_db, nullptr, nullptr, false, false))

      return true;
  }
  /*
    We also require DROP priv for ALTER TABLE ... DROP PARTITION, as well
    as for RENAME TO, as being done by SQLCOM_RENAME_TABLE
  */
  if (alter_info.flags &
      (Alter_info::ALTER_DROP_PARTITION | Alter_info::ALTER_RENAME))
    priv_needed |= DROP_ACL;

  /* Must be set in the parser */
  DBUG_ASSERT(alter_info.new_db_name.str);
  DBUG_ASSERT(!(alter_info.flags & Alter_info::ALTER_EXCHANGE_PARTITION));
  DBUG_ASSERT(!(alter_info.flags & Alter_info::ALTER_ADMIN_PARTITION));
  if (check_access(thd, priv_needed, first_table->db,
                   &first_table->grant.privilege,
                   &first_table->grant.m_internal, false, false) ||
      check_access(thd, INSERT_ACL | CREATE_ACL, alter_info.new_db_name.str,
                   &priv,
                   nullptr, /* Don't use first_tab->grant with sel_lex->db */
                   false, false))
    return true; /* purecov: inspected */

  /* If it is a merge table, check privileges for merge children. */
  if (create_info.merge_list.first) {
    /*
      The user must have (SELECT_ACL | UPDATE_ACL | DELETE_ACL) on the
      underlying base tables, even if there are temporary tables with the same
      names.

      From user's point of view, it might look as if the user must have these
      privileges on temporary tables to create a merge table over them. This is
      one of two cases when a set of privileges is required for operations on
      temporary tables (see also CREATE TABLE).

      The reason for this behavior stems from the following facts:

        - For merge tables, the underlying table privileges are checked only
          at CREATE TABLE / ALTER TABLE time.

          In other words, once a merge table is created, the privileges of
          the underlying tables can be revoked, but the user will still have
          access to the merge table (provided that the user has privileges on
          the merge table itself).

        - Temporary tables shadow base tables.

          I.e. there might be temporary and base tables with the same name, and
          the temporary table takes the precedence in all operations.

        - For temporary MERGE tables we do not track if their child tables are
          base or temporary. As result we can't guarantee that privilege check
          which was done in presence of temporary child will stay relevant later
          as this temporary table might be removed.

      If SELECT_ACL | UPDATE_ACL | DELETE_ACL privileges were not checked for
      the underlying *base* tables, it would create a security breach as in
      Bug#12771903.
    */

    if (check_table_access(thd, SELECT_ACL | UPDATE_ACL | DELETE_ACL,
                           create_info.merge_list.first, false, UINT_MAX,
                           false))
      return true;
  }

  if (check_grant(thd, priv_needed, first_table, false, UINT_MAX, false))
    return true; /* purecov: inspected */

  if (alter_info.new_table_name.str &&
      !test_all_bits(priv, INSERT_ACL | CREATE_ACL)) {
    // Rename of table
    DBUG_ASSERT(alter_info.flags & Alter_info::ALTER_RENAME);
    TABLE_LIST tmp_table;
    tmp_table.table_name = alter_info.new_table_name.str;
    tmp_table.db = alter_info.new_db_name.str;
    tmp_table.grant.privilege = priv;
    if (check_grant(thd, INSERT_ACL | CREATE_ACL, &tmp_table, false, UINT_MAX,
                    false))
      return true; /* purecov: inspected */
  }

  /* Don't yet allow changing of symlinks with ALTER TABLE */
  if (create_info.data_file_name)
    push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_OPTION_IGNORED,
                        ER_THD(thd, WARN_OPTION_IGNORED), "DATA DIRECTORY");
  if (create_info.index_file_name)
    push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_OPTION_IGNORED,
                        ER_THD(thd, WARN_OPTION_IGNORED), "INDEX DIRECTORY");
  create_info.data_file_name = create_info.index_file_name = nullptr;

  thd->enable_slow_log = opt_log_slow_admin_statements;

  /* Push Strict_error_handler for alter table*/
  Strict_error_handler strict_handler;
  if (!thd->lex->is_ignore() && thd->install_strict_handler())
    thd->push_internal_handler(&strict_handler);

  result = mysql_alter_table(thd, alter_info.new_db_name.str,
                             alter_info.new_table_name.str, &create_info,
                             first_table, &alter_info);

  if (!thd->lex->is_ignore() && thd->install_strict_handler())
    thd->pop_internal_handler();
  return result;
}

bool Sql_cmd_discard_import_tablespace::execute(THD *thd) {
  /* Verify that exactly one of the DISCARD and IMPORT flags are set. */
  DBUG_ASSERT((m_alter_info->flags & Alter_info::ALTER_DISCARD_TABLESPACE) ^
              (m_alter_info->flags & Alter_info::ALTER_IMPORT_TABLESPACE));

  /*
    Verify that none of the other flags are set, except for
    ALTER_ALL_PARTITION, which may be set or not, and is
    therefore masked away along with the DISCARD/IMPORT flags.
  */
  DBUG_ASSERT(!(m_alter_info->flags & ~(Alter_info::ALTER_DISCARD_TABLESPACE |
                                        Alter_info::ALTER_IMPORT_TABLESPACE |
                                        Alter_info::ALTER_ALL_PARTITION)));

  /* first SELECT_LEX (have special meaning for many of non-SELECTcommands) */
  SELECT_LEX *select_lex = thd->lex->select_lex;
  /* first table of first SELECT_LEX */
  TABLE_LIST *table_list = select_lex->get_table_list();

  if (check_access(thd, ALTER_ACL, table_list->db, &table_list->grant.privilege,
                   &table_list->grant.m_internal, false, false))
    return true;

  if (check_grant(thd, ALTER_ACL, table_list, false, UINT_MAX, false))
    return true;

  thd->enable_slow_log = opt_log_slow_admin_statements;

  /*
    Check if we attempt to alter mysql.slow_log or
    mysql.general_log table and return an error if
    it is the case.
    TODO: this design is obsolete and will be removed.
  */
  enum_log_table_type table_kind =
      query_logger.check_if_log_table(table_list, false);

  if (table_kind != QUERY_LOG_NONE) {
    /* Disable alter of enabled query log tables */
    if (query_logger.is_log_table_enabled(table_kind)) {
      my_error(ER_BAD_LOG_STATEMENT, MYF(0), "ALTER");
      return true;
    }
  }

  /*
    Add current database to the list of accessed databases
    for this statement. Needed for MTS.
  */
  thd->add_to_binlog_accessed_dbs(table_list->db);

  return mysql_discard_or_import_tablespace(thd, table_list);
}

bool Sql_cmd_secondary_load_unload::execute(THD *thd) {
  // One of the SECONDARY_LOAD/SECONDARY_UNLOAD flags must have been set.
  DBUG_ASSERT(
      ((m_alter_info->flags & Alter_info::ALTER_SECONDARY_LOAD) == 0) !=
      ((m_alter_info->flags & Alter_info::ALTER_SECONDARY_UNLOAD) == 0));

  // No other flags should've been set.
  DBUG_ASSERT(!(m_alter_info->flags & ~(Alter_info::ALTER_SECONDARY_LOAD |
                                        Alter_info::ALTER_SECONDARY_UNLOAD)));

  TABLE_LIST *table_list = thd->lex->select_lex->get_table_list();

  if (check_access(thd, ALTER_ACL, table_list->db, &table_list->grant.privilege,
                   &table_list->grant.m_internal, false, false))
    return true;

  if (check_grant(thd, ALTER_ACL, table_list, false, UINT_MAX, false))
    return true;

  return mysql_secondary_load_or_unload(thd, table_list);
}
