/* Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

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

// Support for derived tables.

#include "sql/sql_derived.h"

#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/mem_root_array.h"
#include "sql/opt_trace.h"  // opt_trace_disable_etc
#include "sql/query_options.h"
#include "sql/sql_base.h"  // EXTRA_RECORD
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_resolver.h"   // check_right_lateral_join
#include "sql/sql_tmp_table.h"  // Tmp tables
#include "sql/sql_union.h"      // Query_result_union
#include "sql/sql_view.h"       // check_duplicate_names
#include "sql/table.h"
#include "sql/table_function.h"
#include "sql/thd_raii.h"
#include "thr_lock.h"

class Opt_trace_context;

/**
   Produces, from the first tmp TABLE object, a clone TABLE object for
   TABLE_LIST 'tl', to have a single materialization of multiple references to
   a CTE.

   How sharing of a single tmp table works
   =======================================

   There are several scenarios.
   (1) Non-recursive CTE referenced only once: nothing special.
   (2) Non-recursive CTE referenced more than once:
   - multiple TABLEs, one TABLE_SHARE.
   - The first ref in setup_materialized_derived() calls
   create_tmp_table(); others call open_table_from_share().
   - The first ref in create_derived() calls instantiate_tmp_table()
   (which calls handler::create() then open_tmp_table()); others call
   open_tmp_table(). open_tmp_table() calls handler::open().
   - The first ref in materialize_derived() evaluates the subquery and does
   all writes to the tmp table.
   - Finally all refs set up a read access method (table scan, index scan,
   index lookup, etc) and do reads, possibly interlaced (example: a
   nested-loop join of two references to the CTE).
   - The storage engine (MEMORY or InnoDB) must be informed of the uses above;
   this is done by having TABLE_SHARE::ref_count>=2 for every handler::open()
   call.
   (3) Recursive CTE, referenced once or more than once:
   All of (2) applies, where the set of refs is the non-recursive
   ones (a recursive ref is a ref appearing in the definition of a recursive
   CTE). Additionally:
   - recursive refs do not call setup_materialized_derived(),
   create_derived(), materialize_derived().
   - right after a non-recursive ref has been in setup_materialized_derived(),
   its recursive refs are replaced with clones of that ref, made with
   open_table_from_share().
   - the first non-recursive ref in materialized_derived() initiates the
   with-recursive algorithm:
     * its recursive refs call open_tmp_table().
     * Then writes (to the non-recursive ref) and reads (from the recursive
     refs) happen interlaced.
   - a particular recursive ref is the UNION table, if UNION DISTINCT is
   present in the CTE's definition: there is a single TABLE for it,
   writes/reads to/from it happen interlaced (writes are done by
   Query_result_union::send_data(); reads are done by the fake_select_lex's
   JOIN).
   - Finally all non-recursive refs set up a read access method and do reads,
   possibly interlaced.
   - The storage engine (MEMORY or InnoDB) must be informed of the uses above;
   this is done by having TABLE_SHARE::ref_count>=2 for every handler::open()
   call.
   - The Server code handling tmp table creation must also be informed:
   see how Query_result_union::create_result_table() disables PK promotion.

   How InnoDB manages the uses above
   =================================

   The storage engine needs to take measures so that inserts and reads
   don't corrupt each other's behaviour. In InnoDB that means two things
   (@see row_search_no_mvcc()):
   (a) A certain way to use its cursor when reading
   (b) Making the different handlers inform each other when one insertion
   modifies the structure of the index tree (e.g. splits a page; this
   triggers a refreshing of all read cursors).

   Requirements on tmp tables used to write/read CTEs
   ==================================================

   The internal tmp table must support a phase where table scans and
   insertions happen interlaced, either issued from a single TABLE or from
   multiple TABLE clones. If from a single TABLE, that object does repetitions
   of {"write rows" then "init scan / read rows / close scan"}. If from
   multiple TABLEs, one does "write rows", every other one does "init scan /
   read rows / close scan".
   During this, neither updates, nor deletes, nor any other type of read
   access than table scans, are allowed on this table (they are allowed after
   the phase's end).
   Any started table scan on this table:
   - must remember its position between two read operations, without influence
   from other scans/inserts;
   - must return rows inserted before and after it started (be catching up
   continuously) (however, when it reports EOF it is allowed to stop catching
   up and report EOF until closed).
   - must return rows in insertion order.
   - may be started from the first record (ha_rnd_init, ha_rnd_next) or from
   the record where the previous scan was ended (position(), ha_rnd_end,
   [...], ha_rnd_init, ha_rnd_pos(saved position), ha_rnd_next).
   - must return positions (handler::position()) which are stable if a write
   later occurs, so that a handler::rnd_pos() happening after the write finds
   the same record.

   Cursor re-positioning when MEMORY is converted to InnoDB
   ========================================================

   See create_ondisk_from_heap(). A requirement is that InnoDB is able to
   start a scan like this: rnd_init, rnd_pos(some PK value), rnd_next.

   @param thd   Thread handler
   @param tl    Table reference wanting the copy

   @returns New clone, or NULL if error
*/

TABLE *Common_table_expr::clone_tmp_table(THD *thd, TABLE_LIST *tl) {
#ifndef DBUG_OFF
  /*
    We're adding a clone; if another clone has been opened before, it was not
    aware of the new one, so perhaps the storage engine has not set up the
    necessary logic to share data among clones. Check that no clone is open:
  */
  Derived_refs_iterator it(tmp_tables[0]);
  while (TABLE *t = it.get_next())
    DBUG_ASSERT(!t->is_created() && !t->materialized);
#endif
  TABLE *first = tmp_tables[0]->table;
  // Allocate clone on the memory root of the TABLE_SHARE.
  TABLE *t = static_cast<TABLE *>(first->s->mem_root.Alloc(sizeof(TABLE)));
  if (!t) return nullptr; /* purecov: inspected */
  if (open_table_from_share(thd, first->s, tl->alias,
                            /*
                              Pass db_stat == 0 to delay opening of table in SE,
                              as table is not instantiated in SE yet.
                            */
                            0,
                            /* We need record[1] for this TABLE instance. */
                            EXTRA_RECORD |
                                /*
                                  Use DELAYED_OPEN to have its own record[0]
                                  (necessary because db_stat is 0).
                                  Otherwise it would be shared with 'first'
                                  and thus a write to tmp table would modify
                                  the row just read by readers.
                                */
                                DELAYED_OPEN,
                            0, t, false, nullptr))
    return nullptr; /* purecov: inspected */
  DBUG_ASSERT(t->s == first->s && t != first && t->file != first->file);
  t->s->increment_ref_count();

  // In case this clone is used to fill the materialized table:
  bitmap_set_all(t->write_set);
  t->reginfo.lock_type = TL_WRITE;
  t->copy_blobs = true;

  tl->table = t;
  t->set_pos_in_table_list(tl);

  t->set_not_started();

  if (tmp_tables.push_back(tl)) return nullptr; /* purecov: inspected */

  return t;
}

/**
   Replaces the recursive reference in query block 'sl' with a clone of
   the first tmp table.

   @param thd   Thread handler
   @param sl    Query block

   @returns true if error
*/
bool Common_table_expr::substitute_recursive_reference(THD *thd,
                                                       SELECT_LEX *sl) {
  TABLE_LIST *tl = sl->recursive_reference;
  DBUG_ASSERT(tl != nullptr && tl->table == nullptr);
  TABLE *t = clone_tmp_table(thd, tl);
  if (t == nullptr) return true; /* purecov: inspected */
  // Eliminate the dummy unit:
  tl->derived_unit()->exclude_tree(thd);
  tl->set_derived_unit(nullptr);
  tl->set_privileges(SELECT_ACL);
  return false;
}

/**
  Resolve a derived table or view reference, including recursively resolving
  contained subqueries.

  @param thd thread handle
  @param apply_semijoin Apply possible semi-join transforms if this is true

  @returns false if success, true if error
*/

bool TABLE_LIST::resolve_derived(THD *thd, bool apply_semijoin) {
  DBUG_TRACE;

  /*
    Helper class which takes care of restoration of members like
    THD::derived_tables_processing. These members are changed in this
    method scope for resolving derived tables.
  */
  class Context_handler {
   public:
    Context_handler(THD *thd)
        : m_thd(thd),
          m_deny_window_func_saved(thd->lex->m_deny_window_func),
          m_derived_tables_processing_saved(thd->derived_tables_processing) {
      /*
        Window functions are allowed; they're aggregated in the derived
        table's definition.
      */
      m_thd->lex->m_deny_window_func = 0;
      m_thd->derived_tables_processing = true;
    }

    ~Context_handler() {
      m_thd->lex->m_deny_window_func = m_deny_window_func_saved;
      m_thd->derived_tables_processing = m_derived_tables_processing_saved;
    }

   private:
    // Thread handle.
    THD *m_thd;

    // Saved state of THD::LEX::m_deny_window_func.
    nesting_map m_deny_window_func_saved;

    // Saved state of THD::derived_tables_processing.
    bool m_derived_tables_processing_saved;
  };

  if (!is_view_or_derived() || is_merged() || is_table_function()) return false;

  // This early return can be deleted after WL#6570.
  if (derived->is_prepared()) return false;

  // Dummy derived tables for recursive references disappear before this stage
  DBUG_ASSERT(this != select_lex->recursive_reference);

  if (is_derived() && derived->m_lateral_deps)
    select_lex->end_lateral_table = this;

  Context_handler ctx_handler(thd);

  if (derived->prepare_limit(thd, derived->global_parameters()))
    return true; /* purecov: inspected */

#ifndef DBUG_OFF  // CTEs, derived tables can have outer references
  if (is_view())  // but views cannot.
    for (SELECT_LEX *sl = derived->first_select(); sl; sl = sl->next_select()) {
      // Make sure there are no outer references
      DBUG_ASSERT(sl->context.outer_context == nullptr);
    }
#endif

  if (m_common_table_expr && m_common_table_expr->recursive &&
      !derived->is_recursive())  // in first resolution @todo delete in WL#6570
  {
    // Ensure it's UNION.
    if (!derived->is_union()) {
      my_error(ER_CTE_RECURSIVE_REQUIRES_UNION, MYF(0), alias);
      return true;
    }
    if (derived->global_parameters()->is_ordered()) {
      /*
        ORDER BY applied to the UNION causes the use of the union tmp
        table. The fake_select_lex would want to sort that table, which isn't
        going to work as the table is incomplete when fake_select_lex first
        reads it. Workaround: put ORDER BY in the top query.
        Another reason: allowing
        ORDER BY <condition using fulltext> would make the UNION tmp table be
        of MyISAM engine which recursive CTEs don't support.
        LIMIT is allowed and will stop the row generation after N rows.
        However, without ORDER BY the CTE's content is ordered in an
        unpredictable way, so LIMIT theoretically returns an unpredictable
        subset of rows. Users are on their own.
        Instead of LIMIT, users can have a counter column and use a WHERE
        on it, to control depth level, which sounds more intelligent than a
        limit.
      */
      my_error(ER_NOT_SUPPORTED_YET, MYF(0),
               "ORDER BY over UNION "
               "in recursive Common Table Expression");
      return true;
    }
    /*
      Should be:
      SELECT1 UNION [DISTINCT | ALL] ... SELECTN
      where SELECT1 is non-recursive, and all non-recursive SELECTs are before
      all recursive SELECTs.
      In SQL standard terms, the CTE must be "expandable" except that we allow
      it to have more than one recursive SELECT.
    */
    bool previous_is_recursive = false;
    SELECT_LEX *last_non_recursive = nullptr;
    for (SELECT_LEX *sl = derived->first_select(); sl; sl = sl->next_select()) {
      if (sl->is_recursive()) {
        if (sl->is_ordered() || sl->has_limit() || sl->is_distinct()) {
          /*
            On top of posing implementation problems, it looks meaningless to
            want to order/limit every iterative sub-result.
            SELECT DISTINCT, if all expressions are constant, is implemented
            as LIMIT in QEP_TAB::remove_duplicates(); do_select() starts with
            send_records=0 so loses track of rows which have been sent in
            previous iterations.
          */
          my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                   "ORDER BY / LIMIT / SELECT DISTINCT"
                   " in recursive query block of Common Table Expression");
          return true;
        }
        if (sl == derived->union_distinct && sl->next_select()) {
          /*
            Consider
              anchor UNION ALL rec1 UNION DISTINCT rec2 UNION ALL rec3:
            after execution of rec2 we must turn off the duplicate-checking
            index; it will thus not contain the keys of rows of rec3, so it
            becomes permanently unusable. The next iteration of rec1 or rec2
            may insert rows which are actually duplicates of those of rec3.
            So: if the last QB having DISTINCT to its left is recursive, and
            it is followed by another QB (necessarily connected with ALL),
            reject the query.
          */
          my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                   "recursive query blocks with"
                   " UNION DISTINCT then UNION ALL, in recursive "
                   "Common Table Expression");
          return true;
        }
      } else {
        if (previous_is_recursive) {
          my_error(ER_CTE_RECURSIVE_REQUIRES_NONRECURSIVE_FIRST, MYF(0), alias);
          return true;
        }
        last_non_recursive = sl;
      }
      previous_is_recursive = sl->is_recursive();
    }
    if (last_non_recursive == nullptr) {
      my_error(ER_CTE_RECURSIVE_REQUIRES_NONRECURSIVE_FIRST, MYF(0), alias);
      return true;
    }
    derived->first_recursive = last_non_recursive->next_select();
    DBUG_ASSERT(derived->is_recursive());
  }

  DEBUG_SYNC(thd, "derived_not_set");

  derived->derived_table = this;

  if (!(derived_result = new (thd->mem_root) Query_result_union()))
    return true; /* purecov: inspected */

  /// Give the unit to the result (the other fields are ignored).
  if (derived_result->prepare(thd, derived->types, derived_unit())) return true;

  /*
    Prepare the underlying query expression of the derived table.
    The SELECT_STRAIGHT_JOIN option prevents semi-join transformation.
  */
  if (derived->prepare(thd, derived_result,
                       !apply_semijoin ? SELECT_NO_SEMI_JOIN : 0, 0))
    return true;

  if (check_duplicate_names(m_derived_column_names, derived->types, false))
    return true;

  if (is_derived()) {
    // The underlying tables of a derived table are all readonly:
    for (SELECT_LEX *sl = derived->first_select(); sl; sl = sl->next_select())
      sl->set_tables_readonly();
    /*
      A derived table is transparent with respect to privilege checking.
      This setting means that privilege checks ignore the derived table
      and are done properly in underlying base tables and views.
      SELECT_ACL is used because derived tables cannot be used for update,
      delete or insert.
    */
    set_privileges(SELECT_ACL);

    if (derived->m_lateral_deps) {
      select_lex->end_lateral_table = nullptr;
      derived->m_lateral_deps &= ~PSEUDO_TABLE_BITS;
      if (derived->m_lateral_deps == 0) {
        /*
          Table doesn't depend on tables in the same FROM clause, so it can be
          evaluated once per execution of the parent query; having the map
          equal to 0 is like removing the LATERAL word.
        */
      } else {
        propagate_table_maps(0);
        if (check_right_lateral_join(this, derived->m_lateral_deps))
          return true;
      }
    }
  }

  return false;
}

/// Helper function for TABLE_LIST::setup_materialized_derived()
static void swap_column_names_of_unit_and_tmp_table(
    List<Item> &unit_items, const Create_col_name_list &tmp_table_col_names) {
  if (unit_items.elements != tmp_table_col_names.size())
    // check_duplicate_names() will find and report error
    return;
  List_iterator_fast<Item> li(unit_items);
  Item *item;
  uint fieldnr = 0;
  while ((item = li++)) {
    const char *s = item->item_name.ptr();
    size_t l = item->item_name.length();
    LEX_CSTRING &other_name =
        const_cast<LEX_CSTRING &>(tmp_table_col_names[fieldnr]);
    item->item_name.set(other_name.str, other_name.length);
    other_name.str = s;
    other_name.length = l;
    fieldnr++;
  }
}

/**
  Prepare a derived table or view for materialization.

  @param  thd   THD pointer

  @return false if successful, true if error
*/
bool TABLE_LIST::setup_materialized_derived(THD *thd)

{
  return setup_materialized_derived_tmp_table(thd) ||
         derived->check_materialized_derived_query_blocks(thd);
}

/**
  Sets up the tmp table to contain the derived table's rows.
  @param  thd   THD pointer
  @return false if successful, true if error
*/
bool TABLE_LIST::setup_materialized_derived_tmp_table(THD *thd)

{
  DBUG_TRACE;

  DBUG_ASSERT(is_view_or_derived() && !is_merged() && table == nullptr);

  DBUG_PRINT("info", ("algorithm: TEMPORARY TABLE"));

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_derived(trace, is_view() ? "view" : "derived");
  trace_derived.add_utf8_table(this)
      .add("select#", derived->first_select()->select_number)
      .add("materialized", true);

  set_uses_materialization();

  // From resolver POV, columns of this table are readonly
  set_readonly();

  if (m_common_table_expr && m_common_table_expr->tmp_tables.size() > 0) {
    trace_derived.add("reusing_tmp_table", true);
    table = m_common_table_expr->clone_tmp_table(thd, this);
    if (table == nullptr) return true; /* purecov: inspected */
    derived_result->table = table;
  }

  if (table == nullptr) {
    // Create the result table for the materialization
    ulonglong create_options =
        derived->first_select()->active_options() | TMP_TABLE_ALL_COLUMNS;

    if (m_derived_column_names) {
      /*
        Tmp table's columns will be created from derived->types (the SELECT
        list), names included.
        But the user asked that the tmp table's columns use other specified
        names. So, we replace the names of SELECT list items with specified
        column names, just for the duration of tmp table creation.
      */
      swap_column_names_of_unit_and_tmp_table(derived->types,
                                              *m_derived_column_names);
    }

    // If we're materializing directly into the result and we have a UNION
    // DISTINCT query, we're going to need a unique index for deduplication.
    // (If we're materializing into a temporary table instead, the deduplication
    // will happen on that table, and is not set here.) create_result_table()
    // will figure out whether it wants to create it as the primary key or just
    // a regular index.
    bool is_distinct = derived->can_materialize_directly_into_result() &&
                       derived->union_distinct != nullptr;

    bool rc = derived_result->create_result_table(
        thd, &derived->types, is_distinct, create_options, alias, false, false);

    if (m_derived_column_names)  // Restore names
      swap_column_names_of_unit_and_tmp_table(derived->types,
                                              *m_derived_column_names);

    if (rc) return true; /* purecov: inspected */

    table = derived_result->table;
    table->set_pos_in_table_list(this);
    if (m_common_table_expr && m_common_table_expr->tmp_tables.push_back(this))
      return true; /* purecov: inspected */
  }

  // Make table's name same as the underlying materialized table
  set_name_temporary();

  table->s->tmp_table =
      system_tmp_table ? SYSTEM_TMP_TABLE : NON_TRANSACTIONAL_TMP_TABLE;

  // Table is "nullable" if inner table of an outer_join
  if (is_inner_table_of_outer_join()) table->set_nullable();

  dep_tables |= derived->m_lateral_deps;

  return false;
}

/**
  Sets up query blocks belonging to the query expression of a materialized
  derived table.
  @param  thd_arg   THD pointer
  @return false if successful, true if error
*/

bool SELECT_LEX_UNIT::check_materialized_derived_query_blocks(THD *thd_arg) {
  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    // All underlying tables are read-only
    sl->set_tables_readonly();
    /*
      Derived tables/view are materialized prior to UPDATE, thus we can skip
      them from table uniqueness check
    */
    sl->propagate_unique_test_exclusion();

    /*
      SELECT privilege is needed for all materialized derived tables and views,
      and columns must be marked for read.
    */
    if (sl->check_view_privileges(thd_arg, SELECT_ACL, SELECT_ACL)) return true;

    // Set all selected fields to be read:
    // @todo Do not set fields that are not referenced from outer query
    List_iterator<Item> it(sl->all_fields);
    Item *item;
    Column_privilege_tracker tracker(thd_arg, SELECT_ACL);
    Mark_field mf(MARK_COLUMNS_READ);
    while ((item = it++)) {
      if (item->walk(&Item::check_column_privileges, enum_walk::PREFIX,
                     (uchar *)thd_arg))
        return true;
      item->walk(&Item::mark_field_in_map, enum_walk::POSTFIX, (uchar *)&mf);
    }
  }
  return false;
}

/**
  Prepare a table function for materialization.

  @param  thd   THD pointer

  @return false if successful, true if error
*/
bool TABLE_LIST::setup_table_function(THD *thd) {
  DBUG_TRACE;

  DBUG_ASSERT(is_table_function());

  DBUG_PRINT("info", ("algorithm: TEMPORARY TABLE"));

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_derived(trace, "table_function");
  const char *func_name;
  uint func_name_len;
  func_name = table_function->func_name();
  func_name_len = strlen(func_name);

  set_uses_materialization();

  /*
    A table function has name resolution context of query which owns FROM
    clause. So it automatically is LATERAL. This end_lateral_table is to
    make sure a table function won't access tables located after it in FROM
    clause.
  */
  select_lex->end_lateral_table = this;

  if (table_function->init()) return true;

  // Create the result table for the materialization
  if (table_function->create_result_table(0LL, alias))
    return true; /* purecov: inspected */
  table = table_function->table;
  table->set_pos_in_table_list(this);

  table->s->tmp_table = NON_TRANSACTIONAL_TMP_TABLE;

  // Table is "nullable" if inner table of an outer_join
  if (is_inner_table_of_outer_join()) table->set_nullable();

  const char *saved_where = thd->where;
  thd->where = "a table function argument";
  enum_mark_columns saved_mark = thd->mark_used_columns;
  thd->mark_used_columns = MARK_COLUMNS_READ;
  if (table_function->init_args()) {
    thd->mark_used_columns = saved_mark;
    return true;
  }
  thd->mark_used_columns = saved_mark;
  set_privileges(SELECT_ACL);
  /*
    Trace needs to be here as it'ss print the table, and columns have to be
    set up at the moment of printing.
  */
  trace_derived.add_utf8_table(this)
      .add_utf8("function_name", func_name, func_name_len)
      .add("materialized", true);

  select_lex->end_lateral_table = nullptr;

  propagate_table_maps(0);
  if (check_right_lateral_join(this, table_function->used_tables()))
    return true;

  thd->where = saved_where;

  return false;
}

/**
  Optimize the query expression representing a derived table/view.

  @note
  If optimizer finds out that the derived table/view is of the type
  "SELECT a_constant" this functions also materializes it.

  @param thd thread handle

  @returns false if success, true if error.
*/

bool TABLE_LIST::optimize_derived(THD *thd) {
  DBUG_TRACE;

  SELECT_LEX_UNIT *const unit = derived_unit();

  DBUG_ASSERT(unit && !unit->is_optimized());

  if (unit->optimize(thd, table) || thd->is_error()) return true;

  if (materializable_is_const() &&
      (create_materialized_table(thd) || materialize_derived(thd)))
    return true;

  return false;
}

/**
  Create result table for a materialized derived table/view/table function.

  @param thd     thread handle

  This function actually creates the result table for given 'derived'
  table/view, but it doesn't fill it.

  @returns false if success, true if error.
*/

bool TABLE_LIST::create_materialized_table(THD *thd) {
  DBUG_TRACE;

  // @todo: Be able to assert !table->is_created() as well
  DBUG_ASSERT((is_table_function() || derived_unit()) &&
              uses_materialization() && table);

  if (!table->is_created()) {
    Derived_refs_iterator it(this);
    while (TABLE *t = it.get_next())
      if (t->is_created()) {
        if (open_tmp_table(table)) return true; /* purecov: inspected */
        break;
      }
  }

  /*
    Don't create result table if:
    1) Table is already created, or
    2) Table is a constant one with all NULL values.
  */
  if (table->is_created() ||                          // 1
      (select_lex->join != nullptr &&                 // 2
       (select_lex->join->const_table_map & map())))  // 2
  {
    /*
      At this point, JT_CONST derived tables should be null rows. Otherwise
      they would have been materialized already.
    */
#ifndef DBUG_OFF
    if (table != nullptr) {
      QEP_TAB *tab = table->reginfo.qep_tab;
      DBUG_ASSERT(tab == nullptr || tab->type() != JT_CONST ||
                  table->has_null_row());
    }
#endif
    return false;
  }
  /* create tmp table */
  if (instantiate_tmp_table(thd, table)) return true; /* purecov: inspected */

  table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);

  return false;
}

/**
  Materialize derived table

  @param  thd	    Thread handle

  Derived table is resolved with temporary table. It is created based on the
  queries defined. After temporary table is materialized, if this is not
  EXPLAIN, then the entire unit / node is deleted. unit is deleted if UNION is
  used for derived table and node is deleted is it is a  simple SELECT.
  If you use this function, make sure it's not called at prepare.
  Due to evaluation of LIMIT clause it can not be used at prepared stage.

  @returns false if success, true if error.
*/

bool TABLE_LIST::materialize_derived(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(is_view_or_derived() && uses_materialization());
  DBUG_ASSERT(table && table->is_created() && !table->materialized);

  Derived_refs_iterator it(this);
  while (TABLE *t = it.get_next())
    if (t->materialized) {
      table->materialized = true;
      table->set_not_started();
      return false;
    }

  /*
    The with-recursive algorithm needs the table scan to return rows in
    insertion order.
    For MEMORY and Temptable it is true.
    For InnoDB: InnoDB's table scan returns rows in PK order. If the PK
    is (not) the autogenerated autoincrement InnoDB ROWID, PK order will (not)
    be the same as insertion order.
    So let's verify that the table has no MySQL-created PK.
  */
  SELECT_LEX_UNIT *const unit = derived_unit();
  if (unit->is_recursive()) {
    DBUG_ASSERT(table->s->primary_key == MAX_KEY);
  }

  if (table->hash_field) {
    table->file->ha_index_init(0, false);
  }

  // execute unit without cleaning up
  bool res = unit->execute(thd);

  if (table->hash_field) {
    table->file->ha_index_or_rnd_end();
  }

  if (!res) {
    /*
      Here we entirely fix both TABLE_LIST and list of SELECT's as
      there were no derived tables
    */
    if (derived_result->flush()) res = true; /* purecov: inspected */
  }

  table->materialized = true;

  // Mark the table as not started (default is just zero status),
  // or read_system() and read_const() will forget to read the row.
  table->set_not_started();

  return res;
}

/**
   Clean up the query expression for a materialized derived table
*/

bool TABLE_LIST::cleanup_derived(THD *thd) {
  DBUG_ASSERT(is_view_or_derived() && uses_materialization());
  return derived_unit()->cleanup(thd, false);
}
