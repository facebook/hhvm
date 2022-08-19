/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/* Insert of records */

#include "sql/sql_insert.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <map>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_table.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_grant_all_columns
#include "sql/binlog.h"
#include "sql/create_field.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd.h"            // dd::get_dictionary
#include "sql/dd/dictionary.h"    // dd::Dictionary
#include "sql/dd/types/column.h"  // dd::Column
#include "sql/dd/types/table.h"   // dd::Table
#include "sql/dd_sql_view.h"      // update_referencing_views_metadata
#include "sql/debug_sync.h"       // DEBUG_SYNC
#include "sql/derror.h"           // ER_THD
#include "sql/discrete_interval.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/lock.h"  // mysql_unlock_tables
#include "sql/mdl.h"
#include "sql/mysqld.h"  // stage_update
#include "sql/nested_join.h"
#include "sql/opt_explain.h"  // Modification_plan
#include "sql/opt_explain_format.h"
#include "sql/partition_info.h"  // partition_info
#include "sql/protocol.h"
#include "sql/query_options.h"
#include "sql/rpl_rli.h"    // Relay_log_info
#include "sql/rpl_slave.h"  // rpl_master_has_bug
#include "sql/sql_alter.h"
#include "sql/sql_array.h"
#include "sql/sql_base.h"  // setup_fields
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_optimizer.h"  // Prepare_error_tracker
#include "sql/sql_resolver.h"   // validate_gc_assignment
#include "sql/sql_show.h"       // store_create_info
#include "sql/sql_table.h"      // quick_rm_table
#include "sql/sql_update.h"     // records_are_comparable
#include "sql/sql_view.h"       // check_key_in_view
#include "sql/system_variables.h"
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_commit_stmt
#include "sql/transaction_info.h"
#include "sql/trigger_def.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_lock.h"

namespace dd {
class Table;
}  // namespace dd

static bool check_view_insertability(THD *thd, TABLE_LIST *view,
                                     const TABLE_LIST *insert_table_ref);

static void prepare_for_positional_update(TABLE *table, TABLE_LIST *tables);

/**
  Check that insert fields are from a single table of a multi-table view.

  @param fields            The insert fields to be checked.
  @param view              The view for insert.
  @param [out] insert_table_ref Reference to table to insert into

  This function is called to check that the fields being inserted into
  are from a single base table. This must be checked when the table to
  be inserted into is a multi-table view.

  @return false if success, true if an error was raised.
*/

static bool check_single_table_insert(List<Item> &fields, TABLE_LIST *view,
                                      TABLE_LIST **insert_table_ref) {
  // It is join view => we need to find the table for insert
  List_iterator_fast<Item> it(fields);
  Item *item;
  *insert_table_ref = nullptr;  // reset for call to check_single_table()
  table_map tables = 0;

  while ((item = it++)) tables |= item->used_tables();

  if (view->check_single_table(insert_table_ref, tables)) {
    my_error(ER_VIEW_MULTIUPDATE, MYF(0), view->view_db.str,
             view->view_name.str);
    return true;
  }
  DBUG_ASSERT(*insert_table_ref && (*insert_table_ref)->is_insertable());

  return false;
}

/**
  Check insert fields.

  @param thd          The current thread.
  @param table_list   The table for insert.
  @param fields       The insert fields.

  @return false if success, true if error

  Resolved reference to base table is returned in lex->insert_table_leaf.
*/

static bool check_insert_fields(THD *thd, TABLE_LIST *table_list,
                                List<Item> &fields) {
  LEX *const lex = thd->lex;

#ifndef DBUG_OFF
  TABLE_LIST *const saved_insert_table_leaf = lex->insert_table_leaf;
#endif

  TABLE *table = table_list->table;

  DBUG_ASSERT(table_list->is_insertable());

  if (fields.elements == 0) {
    /*
      No field list supplied, but a value list has been supplied.
      Use field list of table being updated.
    */
    DBUG_ASSERT(table);  // This branch is not reached with a view:

    lex->insert_table_leaf = table_list;

    Field_iterator_table_ref field_it;
    field_it.set(table_list);
    if (check_grant_all_columns(thd, INSERT_ACL, &field_it)) return true;
  } else {
    // INSERT with explicit field list.
    SELECT_LEX *select_lex = thd->lex->select_lex;
    Name_resolution_context *context = &select_lex->context;
    Name_resolution_context_state ctx_state;

    /* Save the state of the current name resolution context. */
    ctx_state.save_state(context, table_list);

    /*
      Perform name resolution only in the first table - 'table_list',
      which is the table that is inserted into.
    */
    TABLE_LIST *next_name_resolution_table =
        table_list->next_name_resolution_table;
    table_list->next_name_resolution_table = nullptr;
    table_list->next_local = nullptr;

    context->resolve_in_table_list_only(table_list);
    const bool res = setup_fields(thd, Ref_item_array(), fields, INSERT_ACL,
                                  nullptr, false, true);

    table_list->next_name_resolution_table = next_name_resolution_table;

    /* Restore the current context. */
    ctx_state.restore_state(context, table_list);

    if (res) return true;

    if (table_list->is_merged()) {
      if (check_single_table_insert(fields, table_list,
                                    &lex->insert_table_leaf))
        return true;
      table = lex->insert_table_leaf->table;
    } else {
      lex->insert_table_leaf = table_list;
    }

    // We currently don't check for unique columns when inserting via a view.
    const bool check_unique = !table_list->is_view();

    if (check_unique && bitmap_bits_set(table->write_set) < fields.elements) {
      for (auto i = fields.cbegin(); i != fields.cend(); ++i) {
        // Skipping views means that we only have FIELD_ITEM.
        const Item &item1 = *i;
        for (auto j = std::next(i); j != fields.cend(); ++j) {
          const Item &item2 = *j;
          if (item1.eq(&item2, true)) {
            my_error(ER_FIELD_SPECIFIED_TWICE, MYF(0), item1.item_name.ptr());
            break;
          }
        }
      }
      DBUG_ASSERT(thd->is_error());
      return true;
    }
  }
  /* Mark all generated columns for write*/
  if (table->vfield) table->mark_generated_columns(false);

  if (check_key_in_view(thd, table_list, lex->insert_table_leaf) ||
      (table_list->is_view() &&
       check_view_insertability(thd, table_list, lex->insert_table_leaf))) {
    my_error(ER_NON_INSERTABLE_TABLE, MYF(0), table_list->alias, "INSERT");
    return true;
  }

  DBUG_ASSERT(saved_insert_table_leaf == nullptr ||
              lex->insert_table_leaf == saved_insert_table_leaf);

  return false;
}

/**
  Check that table references are restricted to the supplied table map.
  The check can be skipped if the supplied table is a base table.

  @param view   Table being specified
  @param values Values whose used tables are to be matched against table map
  @param map    Table map to match against

  @return false if success, true if error
*/

static bool check_valid_table_refs(const TABLE_LIST *view, List<Item> &values,
                                   table_map map) {
  List_iterator_fast<Item> it(values);
  Item *item;

  if (!view->is_view())  // Ignore check if called with base table.
    return false;

  map |= PSEUDO_TABLE_BITS;

  while ((item = it++)) {
    if (item->used_tables() & ~map) {
      my_error(ER_VIEW_MULTIUPDATE, MYF(0), view->view_db.str,
               view->view_name.str);
      return true;
    }
  }
  return false;
}

/**
  Validates default value of fields which are not specified in
  the column list of INSERT statement.

  @note table->record[0] should be be populated with default values
        before calling this function.

  @param thd              thread context
  @param table            table to which values are inserted.

  @returns false if success, true if error
*/

bool validate_default_values_of_unset_fields(THD *thd, TABLE *table) {
  MY_BITMAP *write_set = table->write_set;
  DBUG_TRACE;

  for (Field **field = table->field; *field; field++) {
    if (!bitmap_is_set(write_set, (*field)->field_index) &&
        !((*field)->flags & NO_DEFAULT_VALUE_FLAG)) {
      if ((*field)->validate_stored_val(thd) && thd->is_error()) return true;
    }
  }

  return false;
}

/**
  Setup data for field BLOB/GEOMETRY field types for execution of
  "INSERT...UPDATE" statement. For a expression in 'UPDATE' clause
  like "a= VALUES(a)", let as call Field* referring 'a' as LHS_FIELD
  and Field* referring field 'a' in "VALUES(a)" as RHS_FIELD

  This function creates a separate copy of the blob value for RHS_FIELD,
  if the field is updated as well as accessed through VALUES()
  function in 'UPDATE' clause of "INSERT...UPDATE" statement.

  @param [in] thd
    Pointer to THD object.

  @param [in] fields
    List of fields representing LHS_FIELD of all expressions
    in 'UPDATE' clause.

  @param [in] mem_root
    MEM_ROOT for blob copy.

  @return - Can fail only when we are out of memory.
    @retval false   Success
    @retval true    Failure
*/

static bool mysql_prepare_blob_values(THD *thd, List<Item> &fields,
                                      MEM_ROOT *mem_root) {
  DBUG_TRACE;

  if (fields.elements <= 1) return false;

  // Collect LHS_FIELD's which are updated in a 'set'.
  // This 'set' helps decide if we need to make copy of BLOB value
  // or not.

  Prealloced_array<Field_blob *, 16> blob_update_field_set(
      PSI_NOT_INSTRUMENTED);
  if (blob_update_field_set.reserve(fields.elements)) return true;

  List_iterator_fast<Item> f(fields);
  Item *fld;
  while ((fld = f++)) {
    Item_field *field = fld->field_for_view_update();
    Field *lhs_field = field->field;

    if (lhs_field->type() == MYSQL_TYPE_BLOB ||
        lhs_field->type() == MYSQL_TYPE_GEOMETRY)
      blob_update_field_set.insert_unique(down_cast<Field_blob *>(lhs_field));
  }

  // Traverse through thd->lex->insert_update_values_map
  // and make copy of BLOB values in RHS_FIELD, if the same field is
  // modified (present in above 'set' prepared).
  if (thd->lex->has_values_map()) {
    std::map<Field *, Field *>::iterator iter;
    for (iter = thd->lex->begin_values_map();
         iter != thd->lex->end_values_map(); ++iter) {
      // Retrieve the Field_blob pointers from the map.
      // and initialize newly declared variables immediately.
      Field_blob *lhs_field = down_cast<Field_blob *>(iter->first);
      Field_blob *rhs_field = down_cast<Field_blob *>(iter->second);

      // Check if the Field_blob object is updated before making a copy.
      if (blob_update_field_set.count_unique(lhs_field) == 0) continue;

      // Copy blob value
      if (rhs_field->copy_blob_value(mem_root)) return true;
    }
  }

  return false;
}

bool Sql_cmd_insert_base::precheck(THD *thd) {
  /*
    Check that we have modify privileges for the first table and
    select privileges for the rest
  */
  ulong privilege = INSERT_ACL | (duplicates == DUP_REPLACE ? DELETE_ACL : 0) |
                    (update_value_list.elements ? UPDATE_ACL : 0);

  if (check_one_table_access(thd, privilege, lex->query_tables)) return true;

  return false;
}

/**
  Insert one or more rows from a VALUES list into a table

  @param thd   thread handler

  @returns false if success, true if error
*/

bool Sql_cmd_insert_values::execute_inner(THD *thd) {
  DBUG_TRACE;

  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_REPLACE ||
              thd->lex->sql_command == SQLCOM_INSERT);

  List_iterator_fast<List_item> its(insert_many_values);
  List_item *values;

  /*
    We have three alternative syntax rules for the INSERT statement:
    1) "INSERT (columns) VALUES ...", so non-listed columns need a default
    2) "INSERT VALUES (), ..." so all columns need a default;
    note that "VALUES (),(expr_1, ..., expr_n)" is not allowed, so checking
    emptiness of the first row is enough
    3) "INSERT VALUES (expr_1, ...), ..." so no defaults are needed; even if
    expr_i is "DEFAULT" (in which case the column is set by
    Item_default_value::save_in_field_inner()).
  */
  const bool manage_defaults = insert_field_list.elements != 0 ||  // 1)
                               value_count == 0;                   // 2)
  COPY_INFO info(COPY_INFO::INSERT_OPERATION, &insert_field_list,
                 manage_defaults, duplicates);
  COPY_INFO update(COPY_INFO::UPDATE_OPERATION, &update_field_list,
                   &update_value_list);

  SELECT_LEX *const select_lex = lex->select_lex;

  TABLE_LIST *const table_list = lex->insert_table;
  TABLE *const insert_table = lex->insert_table_leaf->table;

  if (duplicates == DUP_UPDATE || duplicates == DUP_REPLACE)
    prepare_for_positional_update(insert_table, table_list);

  /* Must be done before can_prune_insert, due to internal initialization. */
  if (info.add_function_default_columns(insert_table, insert_table->write_set))
    return true; /* purecov: inspected */
  if (duplicates == DUP_UPDATE && update.add_function_default_columns(
                                      insert_table, insert_table->write_set))
    return true; /* purecov: inspected */

  // Current error state inside and after the insert loop
  bool has_error = false;

  {  // Statement plan is available within these braces
    Modification_plan plan(
        thd, (lex->sql_command == SQLCOM_INSERT) ? MT_INSERT : MT_REPLACE,
        insert_table, nullptr, false, 0);
    DEBUG_SYNC(thd, "planned_single_insert");

    if (lex->is_explain()) {
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }

    insert_table->next_number_field = insert_table->found_next_number_field;

    if (thd->slave_thread) {
      /* Get SQL thread's rli, even for a slave worker thread */
      Relay_log_info *c_rli = thd->rli_slave->get_c_rli();
      DBUG_ASSERT(c_rli != nullptr);
      if (info.get_duplicate_handling() == DUP_UPDATE &&
          insert_table->next_number_field != nullptr &&
          rpl_master_has_bug(c_rli, 24432, true, nullptr, nullptr))
        return true;
    }

    THD_STAGE_INFO(thd, stage_update);
    if (duplicates == DUP_REPLACE &&
        (!insert_table->triggers ||
         !insert_table->triggers->has_delete_triggers()))
      insert_table->file->ha_extra(HA_EXTRA_WRITE_CAN_REPLACE);
    if (duplicates == DUP_UPDATE)
      insert_table->file->ha_extra(HA_EXTRA_INSERT_WITH_UPDATE);
    /*
      let's *try* to start bulk inserts. It won't necessary
      start them as insert_many_values.elements should be greater than
      some - handler dependent - threshold.
      We should not start bulk inserts if this statement uses
      functions or invokes triggers since they may access
      to the same table and therefore should not see its
      inconsistent state created by this optimization.
      So we call start_bulk_insert to perform nesessary checks on
      insert_many_values.elements, and - if nothing else - to initialize
      the code to make the call of end_bulk_insert() below safe.
    */
    if (duplicates != DUP_ERROR || lex->is_ignore())
      insert_table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
    /*
       This is a simple check for the case when the table has a trigger
       that reads from it, or when the statement invokes a stored function
       that reads from the table being inserted to.
       Engines can't handle a bulk insert in parallel with a read form the
       same table in the same connection.
    */
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES)
      insert_table->file->ha_start_bulk_insert(insert_many_values.elements);

    insert_table->prepare_triggers_for_insert_stmt_or_event();
    insert_table->mark_columns_needed_for_insert(thd);
    /*
      Count warnings for all inserts. For single row insert, generate an error
      if trying to set a NOT NULL field to NULL.
      Notice that policy must be reset before leaving this function.
    */
    thd->check_for_truncated_fields =
        ((insert_many_values.elements == 1 && !lex->is_ignore())
             ? CHECK_FIELD_ERROR_FOR_NULL
             : CHECK_FIELD_WARN);
    thd->num_truncated_fields = 0L;

    for (Field **next_field = insert_table->field; *next_field; ++next_field) {
      (*next_field)->reset_warnings();
    }

    while ((values = its++)) {
      Autoinc_field_has_explicit_non_null_value_reset_guard after_each_row(
          insert_table);

      if (insert_field_list.elements || !value_count) {
        restore_record(insert_table, s->default_values);  // Get empty record

        /*
          Check whether default values of the insert_field_list not specified in
          column list are correct or not.
        */
        if (validate_default_values_of_unset_fields(thd, insert_table)) {
          has_error = true;
          break;
        }
        if (fill_record_n_invoke_before_triggers(
                thd, &info, insert_field_list, *values, insert_table,
                TRG_EVENT_INSERT, insert_table->s->fields, true, nullptr)) {
          DBUG_ASSERT(thd->is_error());
          /*
            TODO: Convert warnings to errors if values_list.elements == 1
            and check that all items return warning in case of problem with
            storing field.
          */
          has_error = true;
          break;
        }

        if (check_that_all_fields_are_given_values(thd, insert_table,
                                                   table_list)) {
          DBUG_ASSERT(thd->is_error());
          has_error = true;
          break;
        }
      } else {
        restore_record(insert_table, s->default_values);  // Get empty record
        if (fill_record_n_invoke_before_triggers(
                thd, insert_table->field, *values, insert_table,
                TRG_EVENT_INSERT, insert_table->s->fields)) {
          DBUG_ASSERT(thd->is_error());
          has_error = true;
          break;
        }
      }

      const int check_result = table_list->view_check_option(thd);
      if (check_result == VIEW_CHECK_SKIP)
        continue;
      else if (check_result == VIEW_CHECK_ERROR) {
        has_error = true;
        break;
      }

      if (invoke_table_check_constraints(thd, insert_table)) {
        if (thd->is_error()) {
          has_error = true;
          break;
        }
        // continue when IGNORE clause is used.
        continue;
      }

      if (write_record(thd, insert_table, &info, &update)) {
        has_error = true;
        break;
      }
      thd->get_stmt_da()->inc_current_row_for_condition();
    }
  }  // Statement plan is available within these braces

  DBUG_ASSERT(has_error == thd->get_stmt_da()->is_error());

  /*
    Now all rows are inserted.  Time to update logs and sends response to
    user
  */
  {
    /* TODO: Only call this if insert_table->found_next_number_field.*/
    insert_table->file->ha_release_auto_increment();
    /*
      Make sure 'end_bulk_insert()' is called regardless of current error
    */
    int loc_error = 0;
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES)
      loc_error = insert_table->file->ha_end_bulk_insert();
    /*
      Report error if 'end_bulk_insert()' failed, and set 'has_error'
    */
    if (loc_error && !has_error) {
      /* purecov: begin inspected */
      myf error_flags = MYF(0);
      if (insert_table->file->is_fatal_error(loc_error))
        error_flags |= ME_FATALERROR;

      insert_table->file->print_error(loc_error, error_flags);
      has_error = true;
      /* purecov: end */
    }

    const bool transactional_table = insert_table->file->has_transactions();

    const bool changed MY_ATTRIBUTE((unused)) =
        info.stats.copied || info.stats.deleted || info.stats.updated;

    if (!has_error ||
        thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
      if (mysql_bin_log.is_open()) {
        int errcode = 0;
        if (!has_error) {
          /*
            [Guilhem wrote] Temporary errors may have filled
            thd->net.last_error/errno.  For example if there has
            been a disk full error when writing the row, and it was
            MyISAM, then thd->net.last_error/errno will be set to
            "disk full"... and the mysql_file_pwrite() will wait until free
            space appears, and so when it finishes then the
            write_row() was entirely successful
          */
          /* todo: consider removing */
          thd->clear_error();
        } else
          errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);

        /* bug#22725:

        A query which per-row-loop can not be interrupted with
        KILLED, like INSERT, and that does not invoke stored
        routines can be binlogged with neglecting the KILLED error.

        If there was no error (has_error == false) until after the end of
        inserting loop the KILLED flag that appeared later can be
        disregarded since previously possible invocation of stored
        routines did not result in any error due to the KILLED.  In
        such case the flag is ignored for constructing binlog event.
        */
        if (thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                              thd->query().length, transactional_table, false,
                              false, errcode))
          has_error = true;
      }
    }
    DBUG_ASSERT(
        transactional_table || !changed ||
        thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
  }
  /*
    We'll report to the client this id:
    - if the table contains an autoincrement column and we successfully
    inserted an autogenerated value, the autogenerated value.
    - if the table contains no autoincrement column and LAST_INSERT_ID(X) was
    called, X.
    - if the table contains an autoincrement column, and some rows were
    inserted, the id of the last "inserted" row (if IGNORE, that value may not
    have been really inserted but ignored).
  */
  ulonglong id =
      (thd->first_successful_insert_id_in_cur_stmt > 0)
          ? thd->first_successful_insert_id_in_cur_stmt
          : (thd->arg_of_last_insert_id_function
                 ? thd->first_successful_insert_id_in_prev_stmt
                 : ((insert_table->next_number_field && info.stats.copied)
                        ? insert_table->next_number_field->val_int()
                        : 0));
  insert_table->next_number_field = nullptr;

  // Remember to restore warning handling before leaving
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;

  DBUG_ASSERT(has_error == thd->get_stmt_da()->is_error());
  if (has_error) return true;

  if (insert_many_values.elements == 1 &&
      (!(thd->variables.option_bits & OPTION_WARNINGS) ||
       !thd->num_truncated_fields)) {
    my_ok(thd,
          info.stats.copied + info.stats.deleted +
              (thd->get_protocol()->has_client_capability(CLIENT_FOUND_ROWS)
                   ? info.stats.touched
                   : info.stats.updated),
          id);
  } else {
    char buff[160];
    ha_rows updated =
        thd->get_protocol()->has_client_capability(CLIENT_FOUND_ROWS)
            ? info.stats.touched
            : info.stats.updated;
    if (lex->is_ignore())
      snprintf(buff, sizeof(buff), ER_THD(thd, ER_INSERT_INFO),
               (long)info.stats.records,
               (long)(info.stats.records - info.stats.copied),
               (long)thd->get_stmt_da()->current_statement_cond_count());
    else
      snprintf(buff, sizeof(buff), ER_THD(thd, ER_INSERT_INFO),
               (long)info.stats.records, (long)(info.stats.deleted + updated),
               (long)thd->get_stmt_da()->current_statement_cond_count());
    my_ok(thd, info.stats.copied + info.stats.deleted + updated, id, buff);
  }

  /*
    If we have inserted into a VIEW, and the base table has
    AUTO_INCREMENT column, but this column is not accessible through
    a view, then we should restore LAST_INSERT_ID to the value it
    had before the statement.
  */
  if (table_list->is_view() && !table_list->contain_auto_increment)
    thd->first_successful_insert_id_in_cur_stmt =
        thd->first_successful_insert_id_in_prev_stmt;

  DBUG_EXECUTE_IF("after_mysql_insert", {
    const char act[] =
        "now "
        "wait_for signal.continue";
    DBUG_ASSERT(opt_debug_sync_timeout > 0);
    DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(act)));
  };);

  return false;
}

/**
  Additional check for insertability for VIEW

  A view is insertable if the following conditions are true:
  - All columns being inserted into are from a single table.
  - All not used columns in table have default values.
  - All columns in view are distinct (not referring to the same column).
  - All columns in view are insertable-into.

  @param thd              thread handler
  @param[in,out] view     reference to view being inserted into.
                          view->contain_auto_increment is true if and only if
                          the view contains an auto_increment field.
  @param insert_table_ref reference to underlying table being inserted into

  @retval false if success
  @retval true if table is not insertable-into (no error is reported)
*/

static bool check_view_insertability(THD *thd, TABLE_LIST *view,
                                     const TABLE_LIST *insert_table_ref) {
  DBUG_TRACE;

  const uint num = view->view_query()->select_lex->item_list.elements;
  TABLE *const table = insert_table_ref->table;
  MY_BITMAP used_fields;
  enum_mark_columns save_mark_used_columns = thd->mark_used_columns;

  const uint used_fields_buff_size = bitmap_buffer_size(table->s->fields);
  uint32 *const used_fields_buff = (uint32 *)thd->alloc(used_fields_buff_size);
  if (!used_fields_buff) return true; /* purecov: inspected */

  DBUG_ASSERT(view->table == nullptr && table != nullptr &&
              view->field_translation != nullptr);

  (void)bitmap_init(&used_fields, used_fields_buff, table->s->fields);
  bitmap_clear_all(&used_fields);

  view->contain_auto_increment = false;

  thd->mark_used_columns = MARK_COLUMNS_NONE;

  // No privilege checking is done for these columns
  Column_privilege_tracker column_privilege(thd, 0);

  /* check simplicity and prepare unique test of view */
  Field_translator *const trans_start = view->field_translation;
  Field_translator *const trans_end = trans_start + num;

  for (Field_translator *trans = trans_start; trans != trans_end; trans++) {
    if (trans->item == nullptr) continue;
    /*
      @todo
      This fix_fields() call is necessary for execution of prepared statements.
      When repeated preparation is eliminated the call can be deleted.
    */
    if (!trans->item->fixed && trans->item->fix_fields(thd, &trans->item))
      return true; /* purecov: inspected */

    // Extract the underlying base table column, if there is one
    Item_field *const field = trans->item->field_for_view_update();

    // No underlying base table column, view is not insertable-into
    if (field == nullptr) return true;

    if (field->field->auto_flags & Field::NEXT_NUMBER)
      view->contain_auto_increment = true;
    /* prepare unique test */
    /*
      remove collation (or other transparent for update function) if we have
      it
    */
    trans->item = field;
  }
  thd->mark_used_columns = save_mark_used_columns;

  /* unique test */
  for (Field_translator *trans = trans_start; trans != trans_end; trans++) {
    if (trans->item == nullptr) continue;
    /* Thanks to test above, we know that all columns are of type Item_field */
    Item_field *field = down_cast<Item_field *>(trans->item);
    /* check fields belong to table in which we are inserting */
    if (field->field->table == table &&
        bitmap_test_and_set(&used_fields, field->field->field_index))
      return true;
  }

  return false;
}

/**
  Recursive helper function for resolving join conditions for
  insertion into view for prepared statements.

  @param thd      Thread handler
  @param tr       Table structure which is traversed recursively

  @return false if success, true if error
*/
static bool fix_join_cond_for_insert(THD *thd, TABLE_LIST *tr) {
  if (tr->join_cond() && !tr->join_cond()->fixed) {
    Column_privilege_tracker column_privilege(thd, SELECT_ACL);

    if (tr->join_cond()->fix_fields(thd, nullptr))
      return true; /* purecov: inspected */
  }

  if (tr->nested_join == nullptr) return false;

  for (TABLE_LIST *ti : tr->nested_join->join_list) {
    if (fix_join_cond_for_insert(thd, ti)) return true; /* purecov: inspected */
  }
  return false;
}

/**
  Get extra info for tables we insert into

  @param table     table(TABLE object) we insert into,
                   might be NULL in case of view
  @param tables (TABLE_LIST object) or view we insert into
*/

static void prepare_for_positional_update(TABLE *table, TABLE_LIST *tables) {
  if (table) {
    table->prepare_for_position();
    return;
  }

  DBUG_ASSERT(tables->is_view());
  for (TABLE_LIST *tbl : *tables->view_tables) {
    prepare_for_positional_update(tbl->table, tbl);
  }

  return;
}

static bool allocate_column_bitmap(TABLE *table, MY_BITMAP **bitmap) {
  DBUG_TRACE;
  const uint number_bits = table->s->fields;
  MY_BITMAP *the_struct;
  my_bitmap_map *the_bits;

  DBUG_ASSERT(current_thd == table->in_use);
  if (multi_alloc_root(table->in_use->mem_root, &the_struct, sizeof(MY_BITMAP),
                       &the_bits, bitmap_buffer_size(number_bits),
                       NULL) == nullptr)
    return true;

  if (bitmap_init(the_struct, the_bits, number_bits) != 0) return true;

  *bitmap = the_struct;

  return false;
}

bool get_default_columns(TABLE *table, MY_BITMAP **m_function_default_columns) {
  if (allocate_column_bitmap(table, m_function_default_columns)) return true;
  /*
    Find columns with function default on insert or update, mark them in
    bitmap.
  */
  for (uint i = 0; i < table->s->fields; ++i) {
    Field *f = table->field[i];
    // if it's a default expression
    if (f->has_insert_default_general_value_expression()) {
      bitmap_set_bit(*m_function_default_columns, f->field_index);
    }
  }

  // Remove from map the fields that are explicitly specified
  bitmap_subtract(*m_function_default_columns, table->write_set);

  // If no bit left exit
  if (bitmap_is_clear_all(*m_function_default_columns)) return false;

  // For each default function that is used restore the flags
  for (uint i = 0; i < table->s->fields; ++i) {
    Field *f = table->field[i];
    if (bitmap_is_set(*m_function_default_columns, i)) {
      DBUG_ASSERT(f->m_default_val_expr != nullptr);
      // restore binlog safety flags
      table->in_use->lex->set_stmt_unsafe_flags(
          f->m_default_val_expr->get_stmt_unsafe_flags());
      // Mark the columns the expression reads in the table's read_set
      for (uint j = 0; j < table->s->fields; j++) {
        if (bitmap_is_set(&f->m_default_val_expr->base_columns_map, j)) {
          bitmap_set_bit(table->read_set, j);
        }
      }
    }
  }

  return false;
}

/**
  Prepare items in INSERT statement

  @param thd                   Thread handler

  WARNING
    You MUST set table->insert_values to 0 after calling this function
    before releasing the table object.

  @return false if success, true if error
*/

bool Sql_cmd_insert_base::prepare_inner(THD *thd) {
  DBUG_TRACE;

  Prepare_error_tracker tracker(thd);

  const bool select_insert = insert_many_values.elements == 0;

  // Number of update fields must match number of update values
  DBUG_ASSERT(update_field_list.elements == update_value_list.elements);

  SELECT_LEX_UNIT *const unit = lex->unit;
  SELECT_LEX *const select = lex->select_lex;

  Name_resolution_context *const context = &select->context;
  Name_resolution_context_state ctx_state;

  TABLE_LIST *const table_list = lex->query_tables;
  lex->insert_table = table_list;

  const bool insert_into_view = table_list->is_view();

  /*
    Save the state of the current name resolution context.
    Should be done only when select_insert is true, but compiler does not
    realize that.
  */
  ctx_state.save_state(context, table_list);

  DBUG_PRINT("enter",
             ("table_list %p, view %d", table_list, (int)insert_into_view));

  // This flag is used only for INSERT, make sure it is clear
  lex->in_update_value_clause = false;

  // first_select_table is the first table after the table inserted into
  TABLE_LIST *first_select_table = table_list->next_local;

  // Setup the insert table only
  table_list->next_local = nullptr;

  // The VALUES table should only be available from within the update
  // expressions (i.e. the rhs of ODKU updates). The context must be restored
  // before resolve_update_expressions for ODKU statements.
  TABLE_LIST *next_name_resolution_table =
      table_list->next_name_resolution_table;
  table_list->next_name_resolution_table = nullptr;

  if (select->setup_tables(thd, table_list, select_insert))
    return true; /* purecov: inspected */

  if (insert_into_view) {
    // Allowing semi-join would transform this table into a "join view"
    if (table_list->resolve_derived(thd, false)) return true;

    if (select->merge_derived(thd, table_list))
      return true; /* purecov: inspected */

    /*
      Require proper privileges for all leaf tables of the view.
      @todo - Check for target table only.
    */
    ulong privilege = INSERT_ACL |
                      (duplicates == DUP_REPLACE ? DELETE_ACL : 0) |
                      (update_value_list.elements ? UPDATE_ACL : 0);

    if (select->check_view_privileges(thd, privilege, privilege)) return true;
    /*
      On second preparation, we may need to resolve view condition generated
      when merging the view.
    */
    if (!select->first_execution && table_list->is_merged() &&
        fix_join_cond_for_insert(thd, table_list))
      return true; /* purecov: inspected */
  }

  /*
    Insertability test is spread across several places:
    - Target table or view must be insertable (checked below)
    - A view containing LIMIT has special key requirements
                                          (checked in check_insert_fields)
    - A view has special requirements with respect to columns being specified
                                          (checked in check_view_insertability)
    - All inserted columns must be from an insertable component of a view
                                          (checked in check_insert_fields)
    - For INSERT ... VALUES, target table must not be same as one selected from
                                          (checked in unique_table)
  */
  if (!table_list->is_insertable()) {
    my_error(ER_NON_INSERTABLE_TABLE, MYF(0), table_list->alias, "INSERT");
    return true;
  }

  if (insert_into_view && insert_field_list.elements == 0) {
    empty_field_list_on_rset = true;
    if (table_list->is_multiple_tables()) {
      my_error(ER_VIEW_NO_INSERT_FIELD_LIST, MYF(0), table_list->view_db.str,
               table_list->view_name.str);
      return true;
    }
    if (insert_view_fields(&insert_field_list, table_list)) return true;
  }

  // REPLACE for a JOIN view is not permitted.
  if (table_list->is_multiple_tables() && duplicates == DUP_REPLACE) {
    my_error(ER_VIEW_DELETE_MERGE_VIEW, MYF(0), table_list->view_db.str,
             table_list->view_name.str);
    return true;
  }

  if (duplicates == DUP_UPDATE) {
    // Must be allocated before Item::fix_fields()
    if (table_list->set_insert_values(thd->mem_root))
      return true; /* purecov: inspected */
  }

  /*
    INSERT ... VALUES () has no WHERE clause, HAVING clause, GROUP BY clause,
    ORDER BY clause nor LIMIT clause. Table list is empty, except when an alias
    name is given for VALUES, which is represented as a derived table.
  */
  DBUG_ASSERT(
      select_insert ||
      ((first_select_table == nullptr || first_select_table->is_derived()) &&
       select->where_cond() == nullptr && select->group_list.elements == 0 &&
       select->having_cond() == nullptr && !select->has_limit()));

  // Prepare the lists of columns and values in the statement.

  if (check_insert_fields(thd, table_list, insert_field_list)) return true;

  TABLE *const insert_table = lex->insert_table_leaf->table;

  uint field_count = insert_field_list.elements ? insert_field_list.elements
                                                : insert_table->s->fields;

  // If the SQL command was an INSERT without column list (like
  //  "INSERT INTO foo VALUES (1, 2);", we ignore any columns that is hidden
  // from the user.
  if (insert_field_list.elements == 0) {
    for (uint i = 0; i < insert_table->s->fields; ++i) {
      if (insert_table->s->field[i]->is_hidden_from_user()) field_count--;
    }
  }

  table_map map = lex->insert_table_leaf->map();

  List_iterator_fast<List_item> its(insert_many_values);
  List_item *values;
  uint value_list_counter = 0;
  while ((values = its++)) {
    value_list_counter++;
    /*
      Values for all fields in table must be specified, unless there is
      no field list and no value list is supplied (means all default values).
    */
    if (values->elements != field_count &&
        !(values->elements == 0 && insert_field_list.elements == 0)) {
      my_error(ER_WRONG_VALUE_COUNT_ON_ROW, MYF(0), value_list_counter);
      return true;
    }

    // Each set of values specified must have the same cardinality
    if (value_list_counter > 1 && value_count != values->elements) {
      my_error(ER_WRONG_VALUE_COUNT_ON_ROW, MYF(0), value_list_counter);
      return true;
    }
    // Assign value count in the Sql_cmd object
    value_count = values->elements;

    if (setup_fields(thd, Ref_item_array(), *values, SELECT_ACL, nullptr, false,
                     false))
      return true;

    if (check_valid_table_refs(table_list, *values, map))
      return true; /* purecov: inspected */

    if ((insert_table->has_gcol() ||
         insert_table->gen_def_fields_ptr != nullptr) &&
        validate_gc_assignment(&insert_field_list, values, insert_table))
      return true;
  }

  /*
    check_insert_fields() will usually mark all inserted columns in write_set,
    except when
    - an explicit column list is given, and a view is inserted into
      (fields from field_translation list have already been fixed in
       resolve_derived(), thus setup_fields() in check_insert_fields() will
       not process them), or
    - no columns where provided in field list
      (except when no values are given - this is a special case that implies
       that all columns are given default values, and default values are
       managed in a different manner, see COPY_INFO for details).
  */
  if ((insert_into_view || insert_field_list.elements == 0) &&
      (select_insert || value_count > 0))
    bitmap_set_all(insert_table->write_set);

  MY_BITMAP *function_default_columns = nullptr;
  get_default_columns(insert_table, &function_default_columns);

  // If an alias for VALUES is specified, prepare the derived values table. This
  // must be done after setting up the insert table to have access to insert
  // fields. If values_field_list is not empty, the table is already set up
  // (e.g. as part of a PREPARE statement).
  if (values_column_list != nullptr && values_field_list.is_empty() &&
      prepare_values_table(thd))
    return true;

  if (duplicates == DUP_UPDATE) {
    // Setup the columns to be updated
    if (setup_fields(thd, Ref_item_array(), update_field_list, UPDATE_ACL,
                     nullptr, false, true))
      return true;

    if (check_valid_table_refs(table_list, update_field_list, map)) return true;
  }

  if (table_list->is_merged()) {
    Column_privilege_tracker column_privilege(thd, SELECT_ACL);

    if (table_list->prepare_check_option(thd))
      return true; /* purecov: inspected */

    if (duplicates == DUP_REPLACE && table_list->prepare_replace_filter(thd))
      return true; /* purecov: inspected */
  }

  /*
    In ON DUPLICATE KEY clause, it is possible to refer to fields from
    the selected tables also, if the query expression is not a VALUES clause,
    not a UNION and the query block is not explicitly grouped.

    This has implications if ON DUPLICATE KEY values contain subqueries,
    due to the way SELECT_LEX::apply_local_transforms() is called: it is
    usually triggered only on the outer-most query block. Such subqueries
    are attached to the last query block of the INSERT statement (relevant if
    this is an INSERT statement with a query expression containing UNION).

    If the query is INSERT VALUES, processing is quite simple:
    - resolve VALUES expressions (above)
    - resolve ON DUPLICATE KEY values with same name resolution context.
    - call apply_local_transforms() on outer query block.

    If the query is INSERT SELECT and the query expression contains UNION,
    processing is performed as follows:
    - resolve ON DUPLICATE KEY expressions with same name resolution context.
      In this case, it is OK to resolve any subqueries before the outer
      query block, because references from the expressions into the
      tables of the query expression are not allowed.
    - resolve the query expression with insert table excluded from name
      resolution context. This will implicitly call apply_local_transforms()
      on the outer query blocks and all subqueries in ON DUPLICATE KEY
      expressions, which are attached to the last query block of the UNION.

    If the query is INSERT SELECT and the query expression does not have
    a UNION, processing is performed as follows:
    - set skip_local_transforms for the outer query block to prevent
      apply_local_transforms() from being called.
    - resolve the query expression with insert table excluded from name
      resolution context.
    - if query block is not grouped, combine the name resolution context
      for the insert table and the query expression, so that ON DUPLICATE KEY
      expressions may refer to all those tables (otherwise restore
      name resolution context as insert table only).
    - resolve ON DUPLICATE KEY expressions.
    - call apply_local_transforms() on outer query block, which also
      contains references to any subqueries from ON DUPLICATE KEY expressions.
  */

  if (!select_insert) {
    // Duplicate tables in subqueries in VALUES clause are not allowed.
    TABLE_LIST *const duplicate =
        unique_table(lex->insert_table_leaf, table_list->next_global, true);
    if (duplicate != nullptr) {
      update_non_unique_table_error(table_list, "INSERT", duplicate);
      return true;
    }
  } else {
    ulong added_options = SELECT_NO_UNLOCK;

    // Is inserted table used somewhere in other parts of query
    if (unique_table(lex->insert_table_leaf, table_list->next_global, false)) {
      // Using same table for INSERT and SELECT, buffer the selection
      added_options |= OPTION_BUFFER_RESULT;
    }
    /*
      INSERT...SELECT...ON DUPLICATE KEY UPDATE/REPLACE SELECT/
      INSERT...IGNORE...SELECT can be unsafe, unless ORDER BY PRIMARY KEY
      clause is used in SELECT statement. We therefore use row based
      logging if mixed or row based logging is available.
      TODO: Check if the order of the output of the select statement is
      deterministic. Waiting for BUG#42415
    */
    if (lex->sql_command == SQLCOM_INSERT_SELECT && duplicates == DUP_UPDATE)
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_INSERT_SELECT_UPDATE);

    if (lex->sql_command == SQLCOM_INSERT_SELECT && lex->is_ignore())
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_INSERT_IGNORE_SELECT);

    if (lex->sql_command == SQLCOM_REPLACE_SELECT)
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_REPLACE_SELECT);

    result = new (thd->mem_root) Query_result_insert(
        table_list, insert_table, &insert_field_list, &insert_field_list,
        &update_field_list, &update_value_list, duplicates);
    if (result == nullptr) return true; /* purecov: inspected */

    if (unit->is_union()) {
      /*
        Update values may not have references to SELECT tables, so it is
        safe to resolve them before the query expression.
      */
      if (duplicates == DUP_UPDATE && resolve_update_expressions(thd))
        return true;
    } else {
      /*
        Delay apply_local_transforms() call until query block and any
        attached subqueries have been resolved.
      */
      select->skip_local_transforms = true;
    }

    // Remove the insert table from the first query block
    select->table_list.first = context->table_list =
        context->first_name_resolution_table = first_select_table;

    if (unit->prepare_limit(thd, unit->global_parameters()))
      return true; /* purecov: inspected */

    if (unit->prepare(thd, result, added_options, 0)) return true;

    /* Restore the insert table but not the name resolution context */
    if (first_select_table != select->table_list.first) {
      // If we have transformation of the top block table list
      // by SELECT_LEX::transform_grouped_to_derived, we must update:
      first_select_table = select->table_list.first;
      ctx_state.update_next_local(first_select_table);
    }
    select->table_list.first = context->table_list = table_list;
    table_list->next_local = first_select_table;

    if (field_count != unit->types.elements) {
      my_error(ER_WRONG_VALUE_COUNT_ON_ROW, MYF(0), 1);
      return true;
    }

    if ((insert_table->has_gcol() || insert_table->gen_def_fields_ptr) &&
        validate_gc_assignment(&insert_field_list,
                               unit->get_unit_column_types(), insert_table))
      return true;
  }

  // The insert table should be a separate name resolution context
  DBUG_ASSERT(table_list->next_name_resolution_table == nullptr);

  if (duplicates == DUP_UPDATE) {
    if (select_insert) {
      if (!unit->is_union() && !select->is_grouped()) {
        /*
          Make one context out of the two separate name resolution contexts:
          the INSERT table and the tables in the SELECT part,
          by concatenating the two lists:
        */
        table_list->next_name_resolution_table =
            context->first_name_resolution_table;
        context->first_name_resolution_table = table_list;
      } else {
        // Restore the original name resolution context (the insert table)
        ctx_state.restore_state(context, table_list);
      }
    } else {
      // Resolve Item_insert_values _after_ their corresponding fields have been
      // fixed, as their field arguments should be resolved as normal.
      if (values_table != nullptr) resolve_values_table_columns(thd);

      // Restore the derived reference table that points to VALUES.
      table_list->next_name_resolution_table = next_name_resolution_table;
    }

    if (!unit->is_union() && resolve_update_expressions(thd)) return true;
  }

  if (!unit->is_union() && select->apply_local_transforms(thd, false))
    return true; /* purecov: inspected */

  if (select_insert) {
    // Restore the insert table and the name resolution context
    select->table_list.first = context->table_list = table_list;
    table_list->next_local = first_select_table;
    ctx_state.restore_state(context, table_list);
  }

  if (insert_table->triggers) {
    /*
      We don't need to mark columns which are used by ON DELETE and
      ON UPDATE triggers, which may be invoked in case of REPLACE or
      INSERT ... ON DUPLICATE KEY UPDATE, since before doing actual
      row replacement or update write_record() will mark all table
      fields as used.
    */
    if (insert_table->triggers->mark_fields(TRG_EVENT_INSERT)) return true;
  }

  if (!select_insert && insert_table->part_info) {
    uint num_partitions = 0;
    enum partition_info::enum_can_prune can_prune_partitions =
        partition_info::PRUNE_NO;
    /*
      We have three alternative syntax rules for the INSERT statement:
      1) "INSERT (columns) VALUES ...", so non-listed columns need a default
      2) "INSERT VALUES (), ..." so all columns need a default;
      note that "VALUES (),(expr_1, ..., expr_n)" is not allowed, so checking
      emptiness of the first row is enough
      3) "INSERT VALUES (expr_1, ...), ..." so no defaults are needed; even if
      expr_i is "DEFAULT" (in which case the column is set by
      Item_default_value::save_in_field_inner()).
    */
    const bool manage_defaults = insert_field_list.elements != 0 ||  // 1)
                                 value_count == 0;                   // 2)
    COPY_INFO info(COPY_INFO::INSERT_OPERATION, &insert_field_list,
                   manage_defaults, duplicates);
    COPY_INFO update(COPY_INFO::UPDATE_OPERATION, &update_field_list,
                     &update_value_list);  // @todo FIX THIS
    /* Must be done before can_prune_insert, due to internal initialization. */
    if (info.add_function_default_columns(insert_table,
                                          insert_table->write_set))
      return true; /* purecov: inspected */
    if (duplicates == DUP_UPDATE && update.add_function_default_columns(
                                        insert_table, insert_table->write_set))
      return true; /* purecov: inspected */
    MY_BITMAP used_partitions;
    bool prune_needs_default_values = false;
    if (insert_table->part_info->can_prune_insert(
            thd, duplicates, update, update_field_list, insert_field_list,
            value_count == 0, &can_prune_partitions,
            &prune_needs_default_values, &used_partitions))
      return true; /* purecov: inspected */

    if (can_prune_partitions != partition_info::PRUNE_NO) {
      its.rewind();
      values = its++;
      num_partitions = insert_table->part_info->lock_partitions.n_bits;
      uint counter = 1;
      /*
        Pruning probably possible, all partitions is unmarked for read/lock,
        and we must now add them on row by row basis.

        Check the first INSERT value.

        PRUNE_DEFAULTS means the partitioning fields are only set to DEFAULT
        values, so we only need to check the first INSERT value, since all the
        rest will be in the same partition.
      */
      if (insert_table->part_info->set_used_partition(
              insert_field_list, *values, info, prune_needs_default_values,
              &used_partitions)) {
        can_prune_partitions = partition_info::PRUNE_NO;
        // set_used_partition may fail.
        if (thd->is_error()) return true;
      }

      while ((values = its++)) {
        counter++;

        /*
          We check pruning for each row until we will
          use all partitions, Even if the number of rows is much higher than the
          number of partitions.
          TODO: Cache the calculated part_id and reuse in
          ha_partition::write_row() if possible.
        */
        if (can_prune_partitions == partition_info::PRUNE_YES) {
          if (insert_table->part_info->set_used_partition(
                  insert_field_list, *values, info, prune_needs_default_values,
                  &used_partitions)) {
            can_prune_partitions = partition_info::PRUNE_NO;
            // set_used_partition may fail.
            if (thd->is_error()) return true;
          }
          if (!(counter % num_partitions)) {
            /*
              Check if we using all partitions in table after adding partition
              for current row to the set of used partitions. Do it only from
              time to time to avoid overhead from bitmap_is_set_all() call.
            */
            if (bitmap_is_set_all(&used_partitions))
              can_prune_partitions = partition_info::PRUNE_NO;
          }
        }
      }
    }

    if (can_prune_partitions != partition_info::PRUNE_NO) {
      /*
        Only lock the partitions we will insert into.
        And also only read from those partitions (duplicates etc.).
        If explicit partition selection 'INSERT INTO t PARTITION (p1)' is used,
        the new set of read/lock partitions is the intersection of read/lock
        partitions and used partitions, i.e only the partitions that exists in
        both sets will be marked for read/lock.
        It is also safe for REPLACE, since all potentially conflicting records
        always belong to the same partition as the one which we try to
        insert a row. This is because ALL unique/primary keys must
        include ALL partitioning columns.
      */
      bitmap_intersect(&insert_table->part_info->read_partitions,
                       &used_partitions);
      bitmap_intersect(&insert_table->part_info->lock_partitions,
                       &used_partitions);
    }
  }

  return false;
}

/**
  Prepare the derived table created as a VALUES alias.

  When an optional table alias is specified for the VALUES of an INSERT
  statement, we create a derived table to contain its values. The field
  translation of this derived table is pointed to the insert buffer of the table
  we are inserting into, by means of the Item_insert_value objects we create
  here. An Item_insert_value object will originally contain the corresponding
  Item_field of the insert table, which is then cloned during its fix_field
  implementation to move the underlying Field to insert_values instead.

  The derived table is initialized in Sql_cmd_insert_base::make_cmd, but we have
  to wait until after the insert table is resolved to create our field
  translation indirection. If the table alias is not given explicit column names
  in the query, it must take the column names of the insert table; this
  information is not available to us until the insert table is resolved.

  @param thd           Thread context.

  @returns false if success, true if error.
*/

bool Sql_cmd_insert_base::prepare_values_table(THD *thd) {
  // Insert_item_value items will be allocated for the field_translation of the
  // VALUES table. They should be persistent for prepared statements.
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  if (insert_field_list.elements == 0) {
    TABLE_LIST *insert_table = lex->select_lex->table_list.first;
    Field_iterator_table_ref it;
    it.set(insert_table);

    for (it.set(insert_table); !it.end_of_fields(); it.next()) {
      Item *item = it.create_item(current_thd);
      if (item == nullptr) return true;
      if (values_field_list.push_back(down_cast<Item_field *>(item)))
        return true;
    }
  } else {
    for (Item &item : insert_field_list) {
      Item_field *field = down_cast<Item_field *>(item.real_item());
      values_field_list.push_back(field);
    }
  }

  // If no column names were specified for the values table, we fill in names
  // corresponding to the same column of the insert table.
  if (values_column_list->empty()) {
    for (Item &item : values_field_list) {
      Item_field *field = down_cast<Item_field *>(&item);
      values_column_list->push_back(to_lex_cstring(field->field_name));
    }
  }

  if (check_duplicate_names(values_column_list, values_field_list, false))
    return true;
  values_table->set_derived_column_names(values_column_list);

  Field_translator *transl =
      pointer_cast<Field_translator *>(thd->stmt_arena->alloc(
          values_column_list->size() * sizeof(Field_translator)));
  if (transl == nullptr) return true;

  uint field_count = 0;
  for (Item &item : values_field_list) {
    Item_field *field_arg = down_cast<Item_field *>(&item);
    Item_insert_value *insert_value =
        new Item_insert_value(&thd->lex->current_select()->context, field_arg);
    if (insert_value == nullptr) return true;
    insert_value->context = &lex->select_lex->context;

    transl[field_count].name = values_column_list->at(field_count).str;
    transl[field_count++].item = insert_value;
  }

  values_table->field_translation = transl;
  values_table->field_translation_end = transl + field_count;

  return false;
}

/**
  Resolve the columns of the optional VALUES table to the insert_values of the
  table inserted into.

  Field_translation of this table contains Item_insert_value objects pointing to
  the Item_field objects of the insert table. Caller is responsible for fixing
  these fields before calling this function.

  @param thd           Thread context.

  @returns false if success, true if error.
*/

bool Sql_cmd_insert_base::resolve_values_table_columns(THD *thd) {
  // If insert_field_list is empty, we fill it by iterating over the fields of
  // the insert table in prepare_values_table(). These new Item_field objects
  // must be fixed as normal insert fields _before_ the Item_insert_value
  // objects in the VALUES table alias are fixed.
  if (insert_field_list.is_empty() &&
      check_insert_fields(thd, thd->lex->query_tables, values_field_list))
    return true;

  Field_translator *field_translation = values_table->field_translation;
  while (field_translation != values_table->field_translation_end) {
    Item_insert_value *item =
        down_cast<Item_insert_value *>(field_translation->item);

    // We are specifically fixing fields for references in update clauses.
    thd->lex->in_update_value_clause = true;
    if (item->fix_fields(thd, nullptr)) return true;
    thd->lex->in_update_value_clause = false;

    ++field_translation;
  }

  // Signal that field_translation is valid.
  if (!values_table->is_merged()) values_table->set_merged();

  return false;
}

/**
  Resolve ON DUPLICATE KEY UPDATE expressions.

  Caller is responsible for setting up the columns to be updated before
  calling this function.

  @param thd     Thread handler

  @returns false if success, true if error
*/

bool Sql_cmd_insert_base::resolve_update_expressions(THD *thd) {
  DBUG_TRACE;

  TABLE_LIST *const insert_table_ref = lex->query_tables;
  TABLE_LIST *const insert_table_leaf = lex->insert_table_leaf;

  const bool select_insert = insert_many_values.elements == 0;

  table_map map = lex->insert_table_leaf->map();

  lex->in_update_value_clause = true;

  if (setup_fields(thd, Ref_item_array(), update_value_list, SELECT_ACL,
                   nullptr, false, false))
    return true;

  if (check_valid_table_refs(insert_table_ref, update_value_list, map))
    return true;

  if (insert_table_leaf->table->has_gcol() &&
      validate_gc_assignment(&update_field_list, &update_value_list,
                             insert_table_leaf->table))
    return true;

  lex->in_update_value_clause = false;

  if (select_insert) {
    /*
      Traverse the update values list and substitute fields from the
      select for references (Item_ref objects) to them. This is done in
      order to get correct values from those fields when the select
      employs a temporary table.
    */
    SELECT_LEX *const select = lex->select_lex;
    List_iterator<Item> li(update_value_list);
    Item *item;

    while ((item = li++)) {
      Item *new_item = item->transform(&Item::update_value_transformer,
                                       pointer_cast<uchar *>(select));
      if (new_item == nullptr) return true;
      if (new_item != item) thd->change_item_tree((Item **)li.ref(), new_item);
    }
  }

  return false;
}

/**
  Check if there are more unique keys after the current one

  @param table  table that keys are checked for
  @param keynr  current key number

  @returns true if there are unique keys after the specified one
*/

static bool last_uniq_key(TABLE *table, uint keynr) {
  /*
    When an underlying storage engine informs that the unique key
    conflicts are not reported in the ascending order by setting
    the HA_DUPLICATE_KEY_NOT_IN_ORDER flag, we cannot rely on this
    information to determine the last key conflict.

    The information about the last key conflict will be used to
    do a replace of the new row on the conflicting row, rather
    than doing a delete (of old row) + insert (of new row).

    Hence check for this flag and disable replacing the last row
    by returning 0 always. Returning 0 will result in doing
    a delete + insert always.
  */
  if (table->file->ha_table_flags() & HA_DUPLICATE_KEY_NOT_IN_ORDER)
    return false; /* purecov: inspected */

  while (++keynr < table->s->keys)
    if (table->key_info[keynr].flags & HA_NOSAME) return false;
  return true;
}

/**
  Write a record to table with optional deletion of conflicting records,
  invoke proper triggers if needed.

  @param thd    thread context
  @param table  table to which record should be written
  @param info   COPY_INFO structure describing handling of duplicates and
                which is used for counting number of records inserted and
                deleted.
  @param update COPY_INFO structure describing the UPDATE part
                (only used for INSERT ON DUPLICATE KEY UPDATE)

  Once this record is written to the table buffer, any AFTER INSERT trigger
  will be invoked. If instead of inserting a new record we end up updating an
  old one, both ON UPDATE triggers will fire instead. Similarly both ON
  DELETE triggers will be invoked if are to delete conflicting records.

  Call thd->transaction.stmt.mark_modified_non_trans_table() if table is a
  non-transactional table.

  @note In ON DUPLICATE KEY UPDATE case this call may set
        TABLE::autoinc_field_has_explicit_non_null_value flag to true (even
        in case of failure) so its caller should make sure that it is reset
        appropriately (@sa fill_record()).

  @returns false if success, true if error
*/

bool write_record(THD *thd, TABLE *table, COPY_INFO *info, COPY_INFO *update) {
  int error, trg_error = 0;
  char *key = nullptr;
  MY_BITMAP *save_read_set, *save_write_set;
  ulonglong prev_insert_id = table->file->next_insert_id;
  ulonglong insert_id_for_cur_row = 0;
  MEM_ROOT mem_root;
  DBUG_TRACE;

  /* Here we are using separate MEM_ROOT as this memory should be freed once we
     exit write_record() function. This is marked as not instumented as it is
     allocated for very short time in a very specific case.
  */
  init_sql_alloc(PSI_NOT_INSTRUMENTED, &mem_root, 256, 0);
  info->stats.records++;
  save_read_set = table->read_set;
  save_write_set = table->write_set;

  const enum_duplicates duplicate_handling = info->get_duplicate_handling();

  /*
    Ignore pk violation checks if:
    (0) Blind replace is enabled (by setting enable_blind_replace sysvar)
    (1) This is a replace into statement (as identified by DUP_REPLACE)
    (2) The table has a well defined primary key (and no hidden pk)
    (3) The table has no secondary keys
    (4) The table has no triggers defined
  */
  if (enable_blind_replace &&                 /* 0 */
      thd->lex->duplicates == DUP_REPLACE &&  /* 1 */
      table->s->keys == 1 &&                  /* 2, 3 */
      table->s->primary_key != MAX_INDEXES && /* 2 */
      !table->triggers /* 4 */) {
    thd->lex->blind_replace_into = true;
  }

  if (duplicate_handling == DUP_REPLACE || duplicate_handling == DUP_UPDATE) {
    DBUG_ASSERT(duplicate_handling != DUP_UPDATE || update != nullptr);
    while ((error = table->file->ha_write_row(table->record[0]))) {
      uint key_nr;
      /*
        If we do more than one iteration of this loop, from the second one the
        row will have an explicit value in the autoinc field, which was set at
        the first call of handler::update_auto_increment(). So we must save
        the autogenerated value to avoid thd->insert_id_for_cur_row to become
        0.
      */
      if (table->file->insert_id_for_cur_row > 0)
        insert_id_for_cur_row = table->file->insert_id_for_cur_row;
      else
        table->file->insert_id_for_cur_row = insert_id_for_cur_row;
      bool is_duplicate_key_error;
      if (!table->file->is_ignorable_error(error)) goto err;
      is_duplicate_key_error =
          (error == HA_ERR_FOUND_DUPP_KEY || error == HA_ERR_FOUND_DUPP_UNIQUE);
      if (!is_duplicate_key_error) {
        /*
          We come here when we had an ignorable error which is not a duplicate
          key error. In this we ignore error if ignore flag is set, otherwise
          report error as usual. We will not do any duplicate key processing.
        */
        info->last_errno = error;
        table->file->print_error(error, MYF(0));
        /*
          If IGNORE option is used, handler errors will be downgraded
          to warnings and don't have to stop the iteration.
        */
        if (thd->is_error()) goto before_trg_err;
        goto ok_or_after_trg_err; /* Ignoring a not fatal error, return 0 */
      }
      if ((int)(key_nr = table->file->get_dup_key(error)) < 0) {
        error = HA_ERR_FOUND_DUPP_KEY; /* Database can't find key */
        goto err;
      }
      /*
        key index value is either valid in the range [0-MAX_KEY) or
        has value MAX_KEY as a marker for the case when no information
        about key can be found. In the last case we have to require
        that storage engine has the flag HA_DUPLICATE_POS turned on.
        If this invariant is false then DBUG_ASSERT will crash
        the server built in debug mode. For the server that was built
        without DEBUG we have additional check for the value of key_nr
        in the code below in order to report about error in any case.
      */
      DBUG_ASSERT(key_nr != MAX_KEY ||
                  (key_nr == MAX_KEY &&
                   (table->file->ha_table_flags() & HA_DUPLICATE_POS)));

      DEBUG_SYNC(thd, "write_row_replace");

      /* Read all columns for the row we are going to replace */
      table->use_all_columns();
      /*
        Don't allow REPLACE to replace a row when a auto_increment column
        was used.  This ensures that we don't get a problem when the
        whole range of the key has been used.
      */
      if (duplicate_handling == DUP_REPLACE && table->next_number_field &&
          key_nr == table->s->next_number_index && (insert_id_for_cur_row > 0))
        goto err;
      if (table->file->ha_table_flags() & HA_DUPLICATE_POS) {
        if (table->file->ha_rnd_pos(table->record[1], table->file->dup_ref))
          goto err;
      }
      /*
        If the key index is equal to MAX_KEY it's treated as unknown key case
        and we shouldn't try to locate key info.
      */
      else if (key_nr < MAX_KEY) {
        if (!key) {
          if (!(key = (char *)my_safe_alloca(table->s->max_unique_length,
                                             MAX_KEY_LENGTH))) {
            error = ENOMEM;
            goto err;
          }
        }
        /*
          If we convert INSERT operation internally to an UPDATE.
          An INSERT operation may update table->vfield for BLOB fields,
          So here we recalculate data for generated columns.
        */
        if (table->vfield) {
          update_generated_write_fields(table->write_set, table);
        }

        key_copy((uchar *)key, table->record[0], table->key_info + key_nr, 0);
        if ((error = (table->file->ha_index_read_idx_map(
                 table->record[1], key_nr, (uchar *)key, HA_WHOLE_KEY,
                 HA_READ_KEY_EXACT))))
          goto err;
      } else {
        /*
          For the server built in non-debug mode returns error if
          handler::get_dup_key() returned MAX_KEY as the value of key index.
        */
        error = HA_ERR_FOUND_DUPP_KEY; /* Database can't find key */
        goto err;
      }
      if (duplicate_handling == DUP_UPDATE) {
        int res = 0;
        /*
          We don't check for other UNIQUE keys - the first row
          that matches, is updated. If update causes a conflict again,
          an error is returned
        */
        DBUG_ASSERT(table->insert_values != nullptr);
        /*
          The insert has failed, store the insert_id generated for
          this row to be re-used for the next insert.
        */
        if (insert_id_for_cur_row > 0) prev_insert_id = insert_id_for_cur_row;

        store_record(table, insert_values);
        /*
          Special check for BLOB/GEOMETRY field in statements with
          "ON DUPLICATE KEY UPDATE" clause.
          See mysql_prepare_blob_values() function for more details.
        */
        if (mysql_prepare_blob_values(thd, *update->get_changed_columns(),
                                      &mem_root))
          goto before_trg_err;
        restore_record(table, record[1]);
        DBUG_ASSERT(update->get_changed_columns()->elements ==
                    update->update_values->elements);
        /*
          Reset TABLE::autoinc_field_has_explicit_non_null_value so we can
          figure out if ON DUPLICATE KEY UPDATE clause specifies value for
          auto-increment field as a side-effect of fill_record(). There is
          no need to clean-up this flag afterwards as this is responsibility
          of the caller.
        */
        table->autoinc_field_has_explicit_non_null_value = false;
        bool is_row_changed = false;
        if (fill_record_n_invoke_before_triggers(
                thd, update, *update->get_changed_columns(),
                *update->update_values, table, TRG_EVENT_UPDATE, 0, true,
                &is_row_changed))
          goto before_trg_err;

        bool insert_id_consumed = false;
        if (  // UPDATE clause specifies a value for the auto increment field
            table->autoinc_field_has_explicit_non_null_value &&
            // An auto increment value has been generated for this row
            (insert_id_for_cur_row > 0)) {
          // After-update value:
          const ulonglong auto_incr_val = table->next_number_field->val_int();
          if (auto_incr_val == insert_id_for_cur_row) {
            // UPDATE wants to use the generated value
            insert_id_consumed = true;
          } else if (table->file->auto_inc_interval_for_cur_row.in_range(
                         auto_incr_val)) {
            /*
              UPDATE wants to use one auto generated value which we have already
              reserved for another (previous or following) row. That may cause
              a duplicate key error if we later try to insert the reserved
              value. Such conflicts on auto generated values would be strange
              behavior, so we return a clear error now.
            */
            my_error(ER_AUTO_INCREMENT_CONFLICT, MYF(0));
            goto before_trg_err;
          }
        }

        if (!insert_id_consumed)
          table->file->restore_auto_increment(prev_insert_id);

        info->stats.touched++;
        if (is_row_changed) {
          /*
            CHECK OPTION for VIEW ... ON DUPLICATE KEY UPDATE ...
            It is safe to not invoke CHECK OPTION for VIEW if records are
            same. In this case the row is coming from the view and thus
            should satisfy the CHECK OPTION.
          */
          {
            const TABLE_LIST *inserted_view =
                table->pos_in_table_list->belong_to_view;
            if (inserted_view != nullptr) {
              res = inserted_view->view_check_option(thd);
              if (res == VIEW_CHECK_SKIP) goto ok_or_after_trg_err;
              if (res == VIEW_CHECK_ERROR) goto before_trg_err;
            }
          }

          /*
            Existing rows in table should normally satisfy CHECK constraints. So
            it should be safe to check constraints only for rows that has really
            changed (i.e. after compare_records()).

            In future, once addition/enabling of CHECK constraints without their
            validation is supported, we might encounter old rows which do not
            satisfy CHECK constraints currently enabled. However, rejecting
            no-op updates to such invalid pre-existing rows won't make them
            valid and is probably going to be confusing for users. So it makes
            sense to stick to current behavior.
          */
          if (invoke_table_check_constraints(thd, table)) {
            if (thd->is_error()) goto before_trg_err;
            // return false when IGNORE clause is used.
            goto ok_or_after_trg_err;
          }

          if ((error = table->file->ha_update_row(table->record[1],
                                                  table->record[0])) &&
              error != HA_ERR_RECORD_IS_THE_SAME) {
            info->last_errno = error;
            myf error_flags = MYF(0);
            if (table->file->is_fatal_error(error))
              error_flags |= ME_FATALERROR;
            table->file->print_error(error, error_flags);
            /*
              If IGNORE option is used, handler errors will be downgraded
              to warnings and don't  have to stop the iteration.
            */
            if (thd->is_error()) goto before_trg_err;
            goto ok_or_after_trg_err; /* Ignoring a not fatal error, return 0 */
          }

          if (error != HA_ERR_RECORD_IS_THE_SAME)
            info->stats.updated++;
          else
            error = 0;
          /*
            If ON DUP KEY UPDATE updates a row instead of inserting one, it's
            like a regular UPDATE statement: it should not affect the value of a
            next SELECT LAST_INSERT_ID() or mysql_insert_id().
            Except if LAST_INSERT_ID(#) was in the INSERT query, which is
            handled separately by THD::arg_of_last_insert_id_function.
          */
          insert_id_for_cur_row = table->file->insert_id_for_cur_row = 0;
          info->stats.copied++;
        }

        // Execute the 'AFTER, ON UPDATE' trigger
        trg_error = (table->triggers &&
                     table->triggers->process_triggers(thd, TRG_EVENT_UPDATE,
                                                       TRG_ACTION_AFTER, true));
        goto ok_or_after_trg_err;
      } else /* DUP_REPLACE */
      {
        TABLE_LIST *view = table->pos_in_table_list->belong_to_view;

        if (view && view->replace_filter) {
          const size_t record_length = table->s->reclength;

          void *record0_saved =
              my_malloc(PSI_NOT_INSTRUMENTED, record_length, MYF(MY_WME));

          if (!record0_saved) {
            error = ENOMEM;
            goto err;
          }

          // Save the record used for comparison.
          memcpy(record0_saved, table->record[0], record_length);

          // Preparing the record for comparison.
          memcpy(table->record[0], table->record[1], record_length);

          // Checking if the row being conflicted is visible by the view.
          bool found_row_in_view = view->replace_filter->val_int();

          // Restoring the record back.
          memcpy(table->record[0], record0_saved, record_length);

          my_free(record0_saved);

          if (!found_row_in_view) {
            my_error(ER_REPLACE_INACCESSIBLE_ROWS, MYF(0));
            goto err;
          }
        }

        /*
          The manual defines the REPLACE semantics that it is either
          an INSERT or DELETE(s) + INSERT; FOREIGN KEY checks in
          InnoDB do not function in the defined way if we allow MySQL
          to convert the latter operation internally to an UPDATE.
          We also should not perform this conversion if we have
          timestamp field with ON UPDATE which is different from DEFAULT.
          Another case when conversion should not be performed is when
          we have ON DELETE trigger on table so user may notice that
          we cheat here. Note that it is ok to do such conversion for
          tables which have ON UPDATE but have no ON DELETE triggers,
          we just should not expose this fact to users by invoking
          ON UPDATE triggers.
        */
        if (last_uniq_key(table, key_nr) &&
            !table->s->is_referenced_by_foreign_key() &&
            (!table->triggers || !table->triggers->has_delete_triggers())) {
          if ((error = table->file->ha_update_row(table->record[1],
                                                  table->record[0])) &&
              error != HA_ERR_RECORD_IS_THE_SAME)
            goto err;
          if (error != HA_ERR_RECORD_IS_THE_SAME)
            info->stats.deleted++;
          else
            error = 0;
          thd->record_first_successful_insert_id_in_cur_stmt(
              table->file->insert_id_for_cur_row);
          /*
            Since we pretend that we have done insert we should call
            its after triggers.
          */
          goto after_trg_n_copied_inc;
        } else {
          if (table->triggers &&
              table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                                TRG_ACTION_BEFORE, true))
            goto before_trg_err;
          if ((error = table->file->ha_delete_row(table->record[1]))) goto err;
          info->stats.deleted++;
          if (!table->file->has_transactions())
            thd->get_transaction()->mark_modified_non_trans_table(
                Transaction_ctx::STMT);
          if (table->triggers &&
              table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                                TRG_ACTION_AFTER, true)) {
            trg_error = 1;
            goto ok_or_after_trg_err;
          }
          /* Let us attempt do write_row() once more */
        }
      }
    }

    /*
        If more than one iteration of the above while loop is done, from the
       second one the row being inserted will have an explicit value in the
       autoinc field, which was set at the first call of
       handler::update_auto_increment(). This value is saved to avoid
       thd->insert_id_for_cur_row becoming 0. Use this saved autoinc value.
     */
    if (table->file->insert_id_for_cur_row == 0)
      table->file->insert_id_for_cur_row = insert_id_for_cur_row;

    thd->record_first_successful_insert_id_in_cur_stmt(
        table->file->insert_id_for_cur_row);
    /*
      Restore column maps if they where replaced during an duplicate key
      problem.
    */
    if (table->read_set != save_read_set || table->write_set != save_write_set)
      table->column_bitmaps_set(save_read_set, save_write_set);
  } else if ((error = table->file->ha_write_row(table->record[0]))) {
    DEBUG_SYNC(thd, "write_row_noreplace");
    info->last_errno = error;
    myf error_flags = MYF(0);
    if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;
    table->file->print_error(error, error_flags);
    /*
      If IGNORE option is used, handler errors will be downgraded
      to warnings and don't  have to stop the iteration.
    */
    if (thd->is_error()) goto before_trg_err;
    table->file->restore_auto_increment(prev_insert_id);
    goto ok_or_after_trg_err;
  }

after_trg_n_copied_inc:
  info->stats.copied++;
  thd->record_first_successful_insert_id_in_cur_stmt(
      table->file->insert_id_for_cur_row);
  trg_error =
      (table->triggers && table->triggers->process_triggers(
                              thd, TRG_EVENT_INSERT, TRG_ACTION_AFTER, true));

ok_or_after_trg_err:
  if (key) my_safe_afree(key, table->s->max_unique_length, MAX_KEY_LENGTH);
  if (!table->file->has_transactions())
    thd->get_transaction()->mark_modified_non_trans_table(
        Transaction_ctx::STMT);
  free_root(&mem_root, MYF(0));
  return trg_error;

err : {
  myf error_flags = MYF(0); /**< Flag for fatal errors */
  info->last_errno = error;
  if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

  table->file->print_error(error, error_flags);
}

before_trg_err:
  table->file->restore_auto_increment(prev_insert_id);
  if (key) my_safe_afree(key, table->s->max_unique_length, MAX_KEY_LENGTH);
  table->column_bitmaps_set(save_read_set, save_write_set);
  free_root(&mem_root, MYF(0));
  return true;
}

/**
  Check that all fields with arn't null_fields are used

  @param thd    thread handler
  @param entry
  @param table_list

  @returns true if all fields are given values
*/

bool check_that_all_fields_are_given_values(THD *thd, TABLE *entry,
                                            TABLE_LIST *table_list) {
  MY_BITMAP *write_set = entry->fields_set_during_insert;

  for (Field **field = entry->field; *field; field++) {
    if (!bitmap_is_set(write_set, (*field)->field_index) &&
        (((*field)->flags & NO_DEFAULT_VALUE_FLAG) &&
         ((*field)->m_default_val_expr == nullptr)) &&
        ((*field)->real_type() != MYSQL_TYPE_ENUM)) {
      bool view = false;
      if (table_list) {
        table_list = table_list->top_table();
        view = table_list->is_view();
      }
      if (view) {
        if ((*field)->type() == MYSQL_TYPE_GEOMETRY) {
          my_error(ER_NO_DEFAULT_FOR_VIEW_FIELD, MYF(0),
                   table_list->view_db.str, table_list->view_name.str);
        } else {
          (*field)->set_warning(
              Sql_condition::SL_WARNING, ER_NO_DEFAULT_FOR_VIEW_FIELD, 1,
              table_list->view_db.str, table_list->view_name.str);
        }
      } else {
        if ((*field)->type() == MYSQL_TYPE_GEOMETRY) {
          my_error(ER_NO_DEFAULT_FOR_FIELD, MYF(0), (*field)->field_name);
        } else {
          (*field)->set_warning(Sql_condition::SL_WARNING,
                                ER_NO_DEFAULT_FOR_FIELD, 1);
        }
      }
    }
  }
  bitmap_clear_all(write_set);
  return thd->is_error();
}

bool Query_result_insert::prepare(THD *thd, List<Item> &, SELECT_LEX_UNIT *u) {
  DBUG_TRACE;

  LEX *const lex = thd->lex;
  const enum_duplicates duplicate_handling = info.get_duplicate_handling();

  unit = u;

  table = lex->insert_table_leaf->table;

  if (info.add_function_default_columns(table, table->write_set)) return true;
  if ((duplicate_handling == DUP_UPDATE) &&
      update.add_function_default_columns(table, table->write_set))
    return true;

  restore_record(table, s->default_values);  // Get empty record
  table->next_number_field = table->found_next_number_field;

  if (thd->slave_thread) {
    /* Get SQL thread's rli, even for a slave worker thread */
    Relay_log_info *c_rli = thd->rli_slave->get_c_rli();
    DBUG_ASSERT(c_rli != nullptr);
    if (duplicate_handling == DUP_UPDATE &&
        table->next_number_field != nullptr &&
        rpl_master_has_bug(c_rli, 24432, true, nullptr, nullptr))
      return true;
  }

  thd->num_truncated_fields = 0;
  if (thd->lex->is_ignore() || duplicate_handling != DUP_ERROR)
    table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
  if (duplicate_handling == DUP_REPLACE &&
      (!table->triggers || !table->triggers->has_delete_triggers()))
    table->file->ha_extra(HA_EXTRA_WRITE_CAN_REPLACE);
  if (duplicate_handling == DUP_UPDATE)
    table->file->ha_extra(HA_EXTRA_INSERT_WITH_UPDATE);

  table->prepare_triggers_for_insert_stmt_or_event();
  table->mark_columns_needed_for_insert(thd);

  for (Field **next_field = table->field; *next_field; ++next_field) {
    (*next_field)->reset_warnings();
    (*next_field)->reset_tmp_null();
  }

  return false;
}

/**
  Set up the target table for execution.

  If the target table is the same as one of the source tables (INSERT SELECT),
  the target table is not finally prepared in the join optimization phase.
  Do the final preparation now.

  @returns false always
*/

bool Query_result_insert::start_execution(THD *thd) {
  DBUG_TRACE;
  if (thd->locked_tables_mode <= LTM_LOCK_TABLES && !thd->lex->is_explain()) {
    DBUG_ASSERT(!bulk_insert_started);
    // TODO: Is there no better estimation than 0 == Unknown number of rows?
    table->file->ha_start_bulk_insert((ha_rows)0);
    bulk_insert_started = true;
  }
  return false;
}

void Query_result_insert::cleanup(THD *thd) {
  DBUG_TRACE;
  if (table) {
    table->next_number_field = nullptr;
    table->file->ha_reset();
  }
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;
}

bool Query_result_insert::send_data(THD *thd, List<Item> &values) {
  DBUG_TRACE;
  bool error = false;

  Autoinc_field_has_explicit_non_null_value_reset_guard after_each_row(table);
  thd->check_for_truncated_fields = CHECK_FIELD_WARN;
  store_values(thd, values);
  thd->check_for_truncated_fields = CHECK_FIELD_ERROR_FOR_NULL;
  if (thd->is_error()) {
    return true;
  }

  /*
   TODO: This method is supposed to be called only once per-statement. But
         currently it is called for each row inserted by INSERT SELECT. Which
         looks like unnessary and results in resource wastage.
  */
  table->prepare_triggers_for_insert_stmt_or_event();
  table->mark_columns_needed_for_insert(thd);

  if (table_list)  // Not CREATE ... SELECT
  {
    switch (table_list->view_check_option(thd)) {
      case VIEW_CHECK_SKIP:
        return false;
      case VIEW_CHECK_ERROR:
        return true;
    }
  }

  if (invoke_table_check_constraints(thd, table)) {
    // return false when IGNORE clause is used.
    return thd->is_error();
  }

  error = write_record(thd, table, &info, &update);

  DEBUG_SYNC(thd, "create_select_after_write_rows_event");

  if (!error &&
      (table->triggers || info.get_duplicate_handling() == DUP_UPDATE)) {
    /*
      Restore fields of the record since it is possible that they were
      changed by ON DUPLICATE KEY UPDATE clause.
      If triggers exist then whey can modify some fields which were not
      originally touched by INSERT ... SELECT, so we have to restore
      their original values for the next row.
    */
    restore_record(table, s->default_values);
  }
  if (!error && table->next_number_field) {
    /*
      If no value has been autogenerated so far, we need to remember the
      value we just saw, we may need to send it to client in the end.
    */
    if (thd->first_successful_insert_id_in_cur_stmt == 0)  // optimization
      autoinc_value_of_last_inserted_row = table->next_number_field->val_int();
    /*
      Clear auto-increment field for the next record, if triggers are used
      we will clear it twice, but this should be cheap.
    */
    table->next_number_field->reset();
  }

  return error;
}

void Query_result_insert::store_values(THD *thd, List<Item> &values) {
  if (fields->elements) {
    restore_record(table, s->default_values);
    if (!validate_default_values_of_unset_fields(thd, table))
      fill_record_n_invoke_before_triggers(thd, &info, *fields, values, table,
                                           TRG_EVENT_INSERT, table->s->fields,
                                           true, nullptr);
  } else
    fill_record_n_invoke_before_triggers(thd, table->field, values, table,
                                         TRG_EVENT_INSERT, table->s->fields);

  check_that_all_fields_are_given_values(thd, table, table_list);
}

void Query_result_insert::send_error(THD *, uint errcode, const char *err) {
  DBUG_TRACE;

  my_message(errcode, err, MYF(0));
}

bool Query_result_insert::stmt_binlog_is_trans() const {
  return table->file->has_transactions();
}

bool Query_result_insert::send_eof(THD *thd) {
  int error;
  ulonglong id, row_count;
  bool changed MY_ATTRIBUTE((unused));
  THD::killed_state killed_status = thd->killed;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("trans_table=%d, table_type='%s'",
              table->file->has_transactions(), table->file->table_type()));

  error = (bulk_insert_started ? table->file->ha_end_bulk_insert() : 0);
  bulk_insert_started = false;
  if (!error && thd->is_error()) error = thd->get_stmt_da()->mysql_errno();

  changed = (info.stats.copied || info.stats.deleted || info.stats.updated);

  /*
    INSERT ... SELECT on non-transactional table which changes any rows
    must be marked as unsafe to rollback.
  */
  DBUG_ASSERT(
      table->file->has_transactions() || !changed ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));

  /*
    Write to binlog before commiting transaction.  No statement will
    be written by the binlog_query() below in RBR mode.  All the
    events are in the transaction cache and will be written when
    ha_autocommit_or_rollback() is issued below.
  */
  if (mysql_bin_log.is_open() &&
      (!error ||
       thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT))) {
    int errcode = 0;
    if (!error)
      thd->clear_error();
    else
      errcode = query_error_code(thd, killed_status == THD::NOT_KILLED);
    if (thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                          thd->query().length, stmt_binlog_is_trans(), false,
                          false, errcode)) {
      table->file->ha_release_auto_increment();
      return true;
    }
  }
  table->file->ha_release_auto_increment();

  if (error) {
    myf error_flags = MYF(0);
    if (table->file->is_fatal_error(my_errno())) error_flags |= ME_FATALERROR;

    table->file->print_error(my_errno(), error_flags);
    return true;
  }

  /*
    For the strict_mode call of push_warning above results to set
    error in Diagnostic_area. Therefore it is necessary to check whether
    the error was set and leave method if it is true. If we didn't do
    so we would failed later when my_ok is called.
  */
  if (thd->get_stmt_da()->is_error()) return true;

  char buff[160];
  if (thd->lex->is_ignore())
    snprintf(buff, sizeof(buff), ER_THD(thd, ER_INSERT_INFO),
             (long)info.stats.records,
             (long)(info.stats.records - info.stats.copied),
             (long)thd->get_stmt_da()->current_statement_cond_count());
  else
    snprintf(buff, sizeof(buff), ER_THD(thd, ER_INSERT_INFO),
             (long)info.stats.records,
             (long)(info.stats.deleted + info.stats.updated),
             (long)thd->get_stmt_da()->current_statement_cond_count());
  row_count = info.stats.copied + info.stats.deleted +
              (thd->get_protocol()->has_client_capability(CLIENT_FOUND_ROWS)
                   ? info.stats.touched
                   : info.stats.updated);
  id = (thd->first_successful_insert_id_in_cur_stmt > 0)
           ? thd->first_successful_insert_id_in_cur_stmt
           : (thd->arg_of_last_insert_id_function
                  ? thd->first_successful_insert_id_in_prev_stmt
                  : (info.stats.copied ? autoinc_value_of_last_inserted_row
                                       : 0));
  my_ok(thd, row_count, id, buff);

  /*
    If we have inserted into a VIEW, and the base table has
    AUTO_INCREMENT column, but this column is not accessible through
    a view, then we should restore LAST_INSERT_ID to the value it
    had before the statement.
  */
  if (table_list != nullptr && table_list->is_view() &&
      !table_list->contain_auto_increment)
    thd->first_successful_insert_id_in_cur_stmt =
        thd->first_successful_insert_id_in_prev_stmt;

  return false;
}

void Query_result_insert::abort_result_set(THD *thd) {
  DBUG_TRACE;
  /*
    If the creation of the table failed (due to a syntax error, for
    example), no table will have been opened and therefore 'table'
    will be NULL. In that case, we still need to execute the rollback
    and the end of the function.
   */
  if (table) {
    bool changed MY_ATTRIBUTE((unused));
    bool transactional_table;
    /*
      Try to end the bulk insert which might have been started before.
      We don't need to do this if we are in prelocked mode (since we
      don't use bulk insert in this case). Also we should not do this
      if tables are not locked yet (bulk insert is not started yet
      in this case).
    */
    if (bulk_insert_started) {
      table->file->ha_end_bulk_insert();
      bulk_insert_started = false;
    }

    /*
      If at least one row has been inserted/modified and will stay in
      the table (the table doesn't have transactions) we must write to
      the binlog (and the error code will make the slave stop).

      For many errors (example: we got a duplicate key error while
      inserting into a MyISAM table), no row will be added to the table,
      so passing the error to the slave will not help since there will
      be an error code mismatch (the inserts will succeed on the slave
      with no error).

      If table creation failed, the number of rows modified will also be
      zero, so no check for that is made.
    */
    changed = (info.stats.copied || info.stats.deleted || info.stats.updated);
    transactional_table = table->file->has_transactions();
    if (thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
      if (mysql_bin_log.is_open()) {
        int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
        /* error of writing binary log is ignored */
        (void)thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                                thd->query().length, transactional_table, false,
                                false, errcode);
      }
    }
    DBUG_ASSERT(
        transactional_table || !changed ||
        thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
    table->file->ha_release_auto_increment();
  }
}

/***************************************************************************
  CREATE TABLE (SELECT) ...
***************************************************************************/

/**
  Create table from lists of fields and items (or just return TABLE
  object for pre-opened existing table). Used by CREATE SELECT.

  Let "source table" be the table in the SELECT part.

  Let "source table columns" be the set of columns in the SELECT list.

  An interesting peculiarity in the syntax CREATE TABLE (@<columns@>) SELECT is
  that function defaults are stripped from the the source table columns, but
  not from the additional columns defined in the CREATE TABLE part. The first
  @c TIMESTAMP column there is also subject to promotion to @c TIMESTAMP @c
  DEFAULT @c CURRENT_TIMESTAMP @c ON @c UPDATE @c CURRENT_TIMESTAMP, as usual.


  @param [in] thd               Thread object
  @param [in] create_info       Create information (like MAX_ROWS, ENGINE or
                                temporary table flag)
  @param [in] create_table      Pointer to TABLE_LIST object providing database
                                and name for table to be created or to be open
  @param [in,out] alter_info    Initial list of columns and indexes for the
                                table to be created
  @param [in] items             The source table columns. Corresponding column
                                definitions (Create_field's) will be added to
                                the end of alter_info->create_list.
  @param  [out] post_ddl_ht     Set to handlerton for table's SE, if this SE
                                supports atomic DDL, so caller can call SE
                                post DDL hook after committing transaction.

  @note
    This function assumes that either table exists and was pre-opened and
    locked at open_and_lock_tables() stage (and in this case we just emit
    error or warning and return pre-opened TABLE object) or an exclusive
    metadata lock was acquired on table so we can safely create, open and
    lock table in it (we don't acquire metadata lock if this create is
    for temporary table).

  @note
    Since this function contains some logic specific to CREATE TABLE ...
    SELECT it should be changed before it can be used in other contexts.

  @retval non-zero  Pointer to TABLE object for table created or opened
  @retval 0         Error
*/

static TABLE *create_table_from_items(THD *thd, HA_CREATE_INFO *create_info,
                                      TABLE_LIST *create_table,
                                      Alter_info *alter_info, List<Item> *items,
                                      handlerton **post_ddl_ht) {
  TABLE tmp_table;  // Used during 'Create_field()'
  TABLE_SHARE share;
  TABLE *table = nullptr;
  uint select_field_count = items->elements;
  /* Add selected items to field list */
  List_iterator_fast<Item> it(*items);
  Item *item;

  DBUG_TRACE;

  tmp_table.s = &share;
  init_tmp_table_share(thd, &share, "", 0, "", "", nullptr);

  tmp_table.s->db_create_options = 0;
  tmp_table.s->db_low_byte_first = (create_info->db_type == myisam_hton ||
                                    create_info->db_type == heap_hton);
  tmp_table.set_not_started();

  if (!thd->variables.explicit_defaults_for_timestamp)
    promote_first_timestamp_column(&alter_info->create_list);

  while ((item = it++)) {
    Create_field *cr_field = generate_create_field(thd, item, &tmp_table);
    if (cr_field == nullptr) {
      return nullptr; /* purecov: deadcode */
    }

    // Array columns may be returned if show_hidden_columns is enabled. Raise an
    // error instead of attempting to create array columns in the new table.
    DBUG_EXECUTE("show_hidden_columns", {
      if (cr_field->is_array) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "Creating tables with array columns.");
        return nullptr;
      }
    });
    DBUG_ASSERT(!cr_field->is_array);

    alter_info->create_list.push_back(cr_field);
  }

  /*
    Acquire SU meta data locks for the tables referenced
    in the FK constraints.
  */
  if (!(create_info->options & HA_LEX_CREATE_TMP_TABLE) &&
      (create_info->db_type->flags & HTON_SUPPORTS_FOREIGN_KEYS)) {
    /*
      CREATE TABLE SELECT fails under LOCK TABLES at open_tables() time
      if target table doesn't exist already. So we don't need to handle
      LOCK TABLES case here by checking that parent tables for new FKs
      are properly locked and there are no orphan child tables for which
      table being created will become parent.
    */
    DBUG_ASSERT(thd->locked_tables_mode != LTM_LOCK_TABLES &&
                thd->locked_tables_mode != LTM_PRELOCKED_UNDER_LOCK_TABLES);

    MDL_request_list mdl_requests;

    if (collect_fk_parents_for_new_fks(
            thd, create_table->db, create_table->table_name, alter_info,
            MDL_SHARED_UPGRADABLE, nullptr, &mdl_requests, nullptr) ||
        collect_fk_names_for_new_fks(thd, create_table->db,
                                     create_table->table_name, alter_info,
                                     create_info->db_type,
                                     0,  // No pre-existing FKs
                                     &mdl_requests))

      return nullptr;

    if (!mdl_requests.is_empty() &&
        thd->mdl_context.acquire_locks_nsec(
            &mdl_requests, thd->variables.lock_wait_timeout_nsec))
      return nullptr;
  }

  // Prepare check constraints.
  if (prepare_check_constraints_for_create(
          thd, create_table->db, create_table->table_name, alter_info))
    return nullptr;

  DEBUG_SYNC(thd, "create_table_select_before_create");

  /*
    Create and lock table.

    Note that we either creating (or opening existing) temporary table or
    creating base table on which name we have exclusive lock. So code below
    should not cause deadlocks or races.

    We don't log the statement, it will be logged later.

    If this is a HEAP table, the automatic DELETE FROM which is written to the
    binlog when a HEAP table is opened for the first time since startup, must
    not be written: 1) it would be wrong (imagine we're in CREATE SELECT: we
    don't want to delete from it) 2) it would be written before the CREATE
    TABLE, which is a wrong order. So we keep binary logging disabled when we
    open_table().
  */
  {
    if (!mysql_create_table_no_lock(
            thd, create_table->db, create_table->table_name, create_info,
            alter_info, select_field_count, true, nullptr, post_ddl_ht)) {
      DEBUG_SYNC(thd, "create_table_select_before_open");

      if (!(create_info->options & HA_LEX_CREATE_TMP_TABLE)) {
        Open_table_context ot_ctx(thd, MYSQL_OPEN_REOPEN);
        /*
          Here we open the destination table, on which we already have
          an exclusive metadata lock.
        */
        if (open_table(thd, create_table, &ot_ctx)) {
          /* Play safe, remove table share for the table from the cache. */
          tdc_remove_table(thd, TDC_RT_REMOVE_ALL, create_table->db,
                           create_table->table_name, false);

          if (!(create_info->db_type->flags & HTON_SUPPORTS_ATOMIC_DDL))
            quick_rm_table(thd, create_info->db_type, create_table->db,
                           create_table->table_name, 0);
        } else
          table = create_table->table;
      } else {
        if (open_temporary_table(thd, create_table)) {
          /*
            This shouldn't happen as creation of temporary table should make
            it preparable for open. Anyway we can't drop temporary table if
            we are unable to fint it.
          */
          DBUG_ASSERT(0);
        } else {
          table = create_table->table;
        }
      }
    }
    if (!table)  // open failed
      return nullptr;
  }
  return table;
}

Query_result_create::Query_result_create(TABLE_LIST *table_arg,
                                         HA_CREATE_INFO *create_info_par,
                                         Alter_info *alter_info_arg,
                                         List<Item> &select_fields,
                                         enum_duplicates duplic,
                                         TABLE_LIST *select_tables_arg)
    : Query_result_insert(nullptr,  // table_list_par
                          nullptr,  // table_par
                          nullptr,  // target_columns
                          &select_fields,
                          nullptr,  // update_fields
                          nullptr,  // update_values
                          duplic),
      create_table(table_arg),
      create_info(create_info_par),
      select_tables(select_tables_arg),
      alter_info(alter_info_arg),
      m_plock(nullptr),
      m_post_ddl_ht(nullptr) {}

/**
  Create the new table from the selected items.

  @param thd     Thread handle.
  @param values  List of items to be used as new columns
  @param u       Select

  @returns false if success, true if error.
*/

bool Query_result_create::prepare(THD *thd, List<Item> &values,
                                  SELECT_LEX_UNIT *u) {
  DBUG_TRACE;

  unit = u;
  DBUG_ASSERT(create_table->table == nullptr);

  DEBUG_SYNC(thd, "create_table_select_before_check_if_exists");

  if (!(table = create_table_from_items(thd, create_info, create_table,
                                        alter_info, &values, &m_post_ddl_ht)))
    /* abort() deletes table */
    return true;

  if (table->s->fields < values.elements) {
    my_error(ER_WRONG_VALUE_COUNT_ON_ROW, MYF(0), 1L);
    return true;
  }
  /* First field to copy */
  field = table->field + table->s->fields - values.elements;
  for (Field **f = field; *f; f++) {
    if ((*f)->gcol_info) {
      /*
        Generated columns are not allowed to be given a value for CREATE TABLE
        .. SELECT statment.
      */
      my_error(ER_NON_DEFAULT_VALUE_FOR_GENERATED_COLUMN, MYF(0),
               (*f)->field_name, (*f)->table->s->table_name.str);
      return true;
    }
  }

  // Turn off function defaults for columns filled from SELECT list:
  bool retval = info.ignore_last_columns(table, values.elements);

  return retval;
}

/**
  Lock the newly created table and prepare it for insertion.

  @returns false if success, true if error
*/

bool Query_result_create::start_execution(THD *thd) {
  DBUG_TRACE;
  DEBUG_SYNC(thd, "create_table_select_before_lock");

  MYSQL_LOCK *extra_lock = nullptr;

  table->reginfo.lock_type = TL_WRITE;

  /*
    mysql_lock_tables() below should never fail with request to reopen table
    since it won't wait for the table lock (we have exclusive metadata lock on
    the table) and thus can't get aborted.
  */
  if (!(extra_lock = mysql_lock_tables(thd, &table, 1, 0)) ||
      binlog_show_create_table(thd)) {
    if (extra_lock) {
      mysql_unlock_tables(thd, extra_lock);
      extra_lock = nullptr;
    }
    return true;
  }
  if (extra_lock) {
    DBUG_ASSERT(m_plock == nullptr);

    if (create_info->options & HA_LEX_CREATE_TMP_TABLE)
      m_plock = &m_lock;
    else
      m_plock = &thd->extra_lock;

    *m_plock = extra_lock;
  }
  /* Mark all fields that are given values */
  for (Field **f = field; *f; f++) {
    bitmap_set_bit(table->write_set, (*f)->field_index);
    bitmap_set_bit(table->fields_set_during_insert, (*f)->field_index);
  }

  // Set up an empty bitmap of function defaults
  if (info.add_function_default_columns(table, table->write_set)) return true;

  if (info.add_function_default_columns(table, table->fields_set_during_insert))
    return true;

  table->next_number_field = table->found_next_number_field;

  restore_record(table, s->default_values);  // Get empty record
  thd->num_truncated_fields = 0;

  const enum_duplicates duplicate_handling = info.get_duplicate_handling();

  if (thd->lex->is_ignore() || duplicate_handling != DUP_ERROR)
    table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
  if (duplicate_handling == DUP_REPLACE &&
      (!table->triggers || !table->triggers->has_delete_triggers()))
    table->file->ha_extra(HA_EXTRA_WRITE_CAN_REPLACE);
  if (duplicate_handling == DUP_UPDATE)
    table->file->ha_extra(HA_EXTRA_INSERT_WITH_UPDATE);
  if (thd->locked_tables_mode <= LTM_LOCK_TABLES) {
    table->file->ha_start_bulk_insert((ha_rows)0);
    bulk_insert_started = true;
  }

  enum_check_fields save_check_for_truncated_fields =
      thd->check_for_truncated_fields;
  thd->check_for_truncated_fields = CHECK_FIELD_WARN;

  if (check_that_all_fields_are_given_values(thd, table, table_list))
    return true;

  thd->check_for_truncated_fields = save_check_for_truncated_fields;

  table->mark_columns_needed_for_insert(thd);
  return false;
}

/*
  For row-based replication, the CREATE-SELECT statement is written
  in two pieces: the first one contain the CREATE TABLE statement
  necessary to create the table and the second part contain the rows
  that should go into the table.

  For non-temporary tables, the start of the CREATE-SELECT
  implicitly commits the previous transaction, and all events
  forming the statement will be stored the transaction cache. At end
  of the statement, the entire statement is committed as a
  transaction, and all events are written to the binary log.

  On the master, the table is locked for the duration of the
  statement, but since the CREATE part is replicated as a simple
  statement, there is no way to lock the table for accesses on the
  slave.  Hence, we have to hold on to the CREATE part of the
  statement until the statement has finished.
*/

int Query_result_create::binlog_show_create_table(THD *thd) {
  DBUG_TRACE;

  TABLE_LIST *save_next_global = create_table->next_global;
  create_table->next_global = select_tables;
  int error = thd->decide_logging_format(create_table);
  create_table->next_global = save_next_global;

  if (error) return error;

  create_table->table->set_binlog_drop_if_temp(
      !thd->is_current_stmt_binlog_disabled() &&
      !thd->is_current_stmt_binlog_format_row());

  if (!thd->is_current_stmt_binlog_format_row() || table->s->tmp_table)
    return 0;

  /*
    Note 1: In RBR mode, we generate a CREATE TABLE statement for the
    created table by calling store_create_info() (behaves as SHOW
    CREATE TABLE). The 'CREATE TABLE' event will be put in the
    binlog statement cache with an Anonymous_gtid_log_event, and
    any subsequent events (e.g., table-map events and rows event)
    will be put in the binlog transaction cache with an
    Anonymous_gtid_log_event. So that the 'CREATE...SELECT'
    statement is logged as:
      Anonymous_gtid_log_event
      CREATE TABLE event
      Anonymous_gtid_log_event
      BEGIN
      rows event
      COMMIT

    We write the CREATE TABLE statement here and not in prepare()
    since there potentially are sub-selects or accesses to information
    schema that will do a close_thread_tables(), destroying the
    statement transaction cache.
  */

  char buf[2048];
  String query(buf, sizeof(buf), system_charset_info);
  int result;
  TABLE_LIST tmp_table_list(table);

  query.length(0);  // Have to zero it since constructor doesn't

  result = store_create_info(thd, &tmp_table_list, &query, create_info,
                             /* show_database */ true);
  DBUG_ASSERT(result == 0); /* store_create_info() always return 0 */

  if (mysql_bin_log.is_open()) {
    DEBUG_SYNC(thd, "create_select_before_write_create_event");
    /*
      Binary log layer has special code to handle rollback of CREATE TABLE
      SELECT in RBR mode - it truncates statement cache in this case.
      So it is OK that we disregard that SE is transactional and might even
      support atomic DDL below.
    */
    int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
    result =
        thd->binlog_query(THD::STMT_QUERY_TYPE, query.ptr(), query.length(),
                          /* is_trans */ false,
                          /* direct */ true,
                          /* suppress_use */ false, errcode);
    DEBUG_SYNC(thd, "create_select_after_write_create_event");
  }
  return result;
}

void Query_result_create::store_values(THD *thd, List<Item> &values) {
  /*
    Evaluate function defaults and default expressions for the columns defined
    in the CREATE TABLE SELECT.
    fill_record_n_invoke_before_triggers version which does *not* set function
    default and default expression automagically is called from here. As a
    result function default and default expression is not evaluated for the
    columns defined in CREATE TABLE SELECT. Hence calling set_function_defaults
    explicitly.
  */
  if (info.function_defaults_apply_on_columns(table->write_set))
    info.set_function_defaults(table);

  fill_record_n_invoke_before_triggers(thd, field, values, table,
                                       TRG_EVENT_INSERT, table->s->fields);
}

void Query_result_create::send_error(THD *thd, uint errcode, const char *err) {
  DBUG_TRACE;

  DBUG_PRINT("info",
             ("Current statement %s row-based",
              thd->is_current_stmt_binlog_format_row() ? "is" : "is NOT"));
  DBUG_PRINT("info",
             ("Current table (at %p) %s a temporary (or non-existant) table",
              table, table && !table->s->tmp_table ? "is NOT" : "is"));
  /*
    This will execute any rollbacks that are necessary before writing
    the transcation cache.

    We disable the binary log since nothing should be written to the
    binary log.  This disabling is important, since we potentially do
    a "roll back" of non-transactional tables by removing the table,
    and the actual rollback might generate events that should not be
    written to the binary log.

  */
  Disable_binlog_guard binlog_guard(thd);
  Query_result_insert::send_error(thd, errcode, err);
}

bool Query_result_create::stmt_binlog_is_trans() const {
  /*
    Binary logging code assumes that CREATE TABLE statements are
    written to transactional cache iff they support atomic DDL.
  */
  return (table->s->db_type()->flags & HTON_SUPPORTS_ATOMIC_DDL);
}

bool Query_result_create::send_eof(THD *thd) {
  /*
    The routine that writes the statement in the binary log
    is in Query_result_insert::send_eof(). For that reason, we
    mark the flag at this point.
  */
  if (create_info->options & HA_LEX_CREATE_TMP_TABLE)
    thd->get_transaction()->mark_created_temp_table(Transaction_ctx::STMT);

  bool error = false;

  /*
    For non-temporary tables, we update the unique_constraint_name for
    the FKs of referencing tables, after acquiring exclusive metadata locks.
    We also need to upgrade the SU locks on referenced tables to be exclusive
    before invalidating the referenced tables.
  */
  Foreign_key_parents_invalidator fk_invalidator;

  if (!(create_info->options & HA_LEX_CREATE_TMP_TABLE) &&
      (create_info->db_type->flags & HTON_SUPPORTS_FOREIGN_KEYS)) {
    MDL_request_list mdl_requests;

    if ((!dd::get_dictionary()->is_dd_table_name(create_table->db,
                                                 create_table->table_name) &&
         collect_fk_children(thd, create_table->db, create_table->table_name,
                             create_info->db_type, MDL_EXCLUSIVE,
                             &mdl_requests)) ||
        collect_fk_parents_for_new_fks(thd, create_table->db,
                                       create_table->table_name, alter_info,
                                       MDL_EXCLUSIVE, create_info->db_type,
                                       &mdl_requests, &fk_invalidator) ||
        (!mdl_requests.is_empty() &&
         thd->mdl_context.acquire_locks_nsec(
             &mdl_requests, thd->variables.lock_wait_timeout_nsec)))
      error = true;
    else {
      dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
      const dd::Table *new_table = nullptr;
      if (thd->dd_client()->acquire(create_table->db, create_table->table_name,
                                    &new_table))
        error = true;
      else {
        DBUG_ASSERT(new_table != nullptr);
        /*
          If we are to support FKs for storage engines which don't support
          atomic DDL we need to decide what to do for such SEs in case of
          failure to update children definitions and adjust code accordingly.
        */
        DBUG_ASSERT(create_info->db_type->flags & HTON_SUPPORTS_ATOMIC_DDL);

        if (adjust_fk_children_after_parent_def_change(
                thd, create_table->db, create_table->table_name,
                create_info->db_type, new_table, nullptr) ||
            adjust_fk_parents(thd, create_table->db, create_table->table_name,
                              true, nullptr))
          error = true;
      }
    }
  }

  {
    Uncommitted_tables_guard uncommitted_tables(thd);

    /*
      We can rollback target table creation by dropping it even for SEs which
      don't support atomic DDL. So there is no need to commit changes to
      metadata of dependent views below.
      Moreover, doing these intermediate commits can be harmful as in RBR mode
      they will flush CREATE TABLE event and row events to the binary log
      which, in case of later error, will create discrepancy with rollback of
      statement by target table removal.
      Such intermediate commits also wipe out transaction's unsafe-to-rollback
      flags which leads to broken assertions in Query_result_insert::send_eof().
    */
    if (!error)
      error = update_referencing_views_metadata(thd, create_table, false,
                                                &uncommitted_tables);
  }

  if (!error) error = Query_result_insert::send_eof(thd);
  if (error)
    abort_result_set(thd);
  else {
    bool commit_error = false;
    /*
      Do an implicit commit at end of statement for non-temporary tables.
      This can fail in which case rollback will be done automatically.
      For storage engines supporting atomic DDL this will revert table
      creation in SE, data-dictionary and binlog changes.
      For other storage engines we might end-up with partially consistent
      state between data-dictionary, SE, data in table and binary log.
      However this should be extremely rare.
    */
    if (!table->s->tmp_table) {
      thd->get_stmt_da()->set_overwrite_status(true);
      commit_error = trans_commit_stmt(thd) || trans_commit_implicit(thd);
      thd->get_stmt_da()->set_overwrite_status(false);
    }

    if (m_plock) {
      mysql_unlock_tables(thd, *m_plock);
      *m_plock = nullptr;
      m_plock = nullptr;
    }

    if (commit_error) {
      DBUG_ASSERT(!table->s->tmp_table);
      DBUG_ASSERT(table == thd->open_tables);
      close_thread_table(thd, &thd->open_tables);
      /*
        Remove TABLE and TABLE_SHARE objects for the table which creation
        might have been rolled back from the caches.
      */
      tdc_remove_table(thd, TDC_RT_REMOVE_ALL, create_table->db,
                       create_table->table_name, false);
    }

    if (m_post_ddl_ht) m_post_ddl_ht->post_ddl(thd);

    fk_invalidator.invalidate(thd);
  }
  return error;
}

/**
  Close and drop just created table in CREATE TABLE ... SELECT in case
  of error.

  @note Here we assume that the table to be closed is open only by the
        calling thread, so we needn't wait until other threads close the
        table. We also assume that the table is first in thd->open_ables
        and a data lock on it, if any, has been released.
*/

void Query_result_create::drop_open_table(THD *thd) {
  DBUG_TRACE;

  if (table->s->tmp_table) {
    /*
      Call reset here since SE may depend on this to reset its state
      properly. Normally this is done when calling
      mark_tmp_table_for_reuse(table); at the end of a statement using
      temporary tables. In a Query_result_set_insert object it is done
      by the cleanup() member function.  For a non-temporary table
      this is done by close_thread_table(). Calling ha_reset() from
      close_temporary_table() is not an options since this function
      gets called at times (boot) when is data structures needed by
      handler::reset() have not yet been initialized.
    */
    table->file->ha_reset();
    close_temporary_table(thd, table, true, true);
  } else {
    DBUG_ASSERT(table == thd->open_tables);

    handlerton *table_type = table->s->db_type();

    table->file->ha_extra(HA_EXTRA_PREPARE_FOR_DROP);
    close_thread_table(thd, &thd->open_tables);
    /*
      Remove TABLE and TABLE_SHARE objects for the table we have failed
      to create from the caches. This also nicely covers the case when
      addition of table to data-dictionary was not even committed.
    */
    tdc_remove_table(thd, TDC_RT_REMOVE_ALL, create_table->db,
                     create_table->table_name, false);

    if (!(table_type->flags & HTON_SUPPORTS_ATOMIC_DDL)) {
      /*
        Removal of table by quick_rm_table() below commits statement
        transaction as a side-effect. If the statement is not rolled back here
        then binlog cache (containing log(s) of new table and inserts) will be
        written to the binlog file.

        We are not allowed to rollback a statement transactions inside stored
        function or trigger. OTOH in such contexts only creation of temporary
        tables is allowed.
      */
      trans_rollback_stmt(thd);
      if (thd->transaction_rollback_request) trans_rollback_implicit(thd);

      quick_rm_table(thd, table_type, create_table->db,
                     create_table->table_name, 0);
    }
#ifdef HAVE_PSI_TABLE_INTERFACE
    else {
      /* quick_rm_table() was not called, so remove the P_S table share here. */
      PSI_TABLE_CALL(drop_table_share)
      (false, create_table->db, strlen(create_table->db),
       create_table->table_name, strlen(create_table->table_name));
    }
#endif
  }
}

void Query_result_create::abort_result_set(THD *thd) {
  DBUG_TRACE;

  /*
    In Query_result_insert::abort_result_set() we roll back the statement,
    including truncating the transaction cache of the binary log. To do this, we
    pretend that the statement is transactional, even though it might
    be the case that it was not.

    We roll back the statement prior to deleting the table and prior
    to releasing the lock on the table, since there might be potential
    for failure if the rollback is executed after the drop or after
    unlocking the table.

    We also roll back the statement regardless of whether the creation
    of the table succeeded or not, since we need to reset the binary
    log state.
  */
  {
    Disable_binlog_guard binlog_guard(thd);
    Query_result_insert::abort_result_set(thd);
    thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::STMT);
  }
  /* possible error of writing binary log is ignored deliberately */
  (void)thd->binlog_flush_pending_rows_event(true, true);

  if (m_plock) {
    mysql_unlock_tables(thd, *m_plock);
    *m_plock = nullptr;
    m_plock = nullptr;
  }

  if (table) {
    drop_open_table(thd);
    table = nullptr;  // Safety
  }

  if (!(create_info->options & HA_LEX_CREATE_TMP_TABLE)) {
    trans_rollback_stmt(thd);
    /*
      Rollback transaction both to clear THD::transaction_rollback_request
      (if it is set) and to synchronize DD state in cache and on disk (as
      statement rollback doesn't clear DD cache of modified uncommitted
      objects).
    */
    trans_rollback_implicit(thd);
    if (m_post_ddl_ht) m_post_ddl_ht->post_ddl(thd);
  }
}

bool Sql_cmd_insert_base::accept(THD *thd, Select_lex_visitor *visitor) {
  // Columns
  if (insert_field_list.elements) {
    List_iterator<Item> it_field(insert_field_list);
    while (Item *field = it_field++)
      if (walk_item(field, visitor)) return true;
  }

  if (insert_many_values.elements > 0) {
    // INSERT...VALUES statement
    List_iterator<List_item> it_row(insert_many_values);
    while (List_item *row = it_row++) {
      List_iterator<Item> it_col(*row);
      while (Item *item = it_col++)
        if (walk_item(item, visitor)) return true;
    }
  } else {
    // INSERT...SELECT statement
    if (thd->lex->select_lex->accept(visitor)) return true;
  }

  // Update list (on duplicate update)
  List_iterator<Item> it_value(update_value_list), it_column(update_field_list);
  Item *value, *column;
  while ((column = it_column++) && (value = it_value++))
    if (walk_item(column, visitor) || walk_item(value, visitor)) return true;

  return visitor->visit(thd->lex->select_lex);
}
