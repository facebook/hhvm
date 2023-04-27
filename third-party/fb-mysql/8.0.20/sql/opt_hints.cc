/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/opt_hints.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_table_map.h"
#include "mysql/udf_registration_types.h"
#include "mysqld_error.h"
#include "sql/derror.h"  // ER_THD
#include "sql/error_handler.h"
#include "sql/item.h"
#include "sql/item_subselect.h"
#include "sql/key.h"
#include "sql/mysqld.h"  // table_alias_charset
#include "sql/nested_join.h"
#include "sql/parse_tree_hints.h"
#include "sql/set_var.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"      // Sql_condition
#include "sql/sql_optimizer.h"  // JOIN class
#include "sql/sql_select.h"
#include "sql/table.h"

struct MEM_ROOT;

/**
  Information about hints. Sould be
  synchronized with opt_hints_enum enum.

  Note: Hint name depends on hint state. 'NO_' prefix is added
  if appropriate hint state bit(see Opt_hints_map::hints) is not
  set. Depending on 'switch_state_arg' argument in 'parse tree
  object' constructors(see parse_tree_hints.[h,cc]) implementor
  can control wishful form of the hint name.
*/

struct st_opt_hint_info opt_hint_info[] = {
    {"BKA", true, true, false},
    {"BNL", true, true, false},
    {"ICP", true, true, false},
    {"MRR", true, true, false},
    {"NO_RANGE_OPTIMIZATION", true, true, false},
    {"MAX_EXECUTION_TIME", false, false, false},
    {"QB_NAME", false, false, false},
    {"SEMIJOIN", false, false, false},
    {"SUBQUERY", false, false, false},
    {"MERGE", true, true, false},
    {"JOIN_PREFIX", false, false, true},
    {"JOIN_SUFFIX", false, false, true},
    {"JOIN_ORDER", false, false, true},
    {"JOIN_FIXED_ORDER", false, true, false},
    {"INDEX_MERGE", false, false, false},
    {"RESOURCE_GROUP", false, false, false},
    {"SKIP_SCAN", false, false, false},
    {"GROUP_BY_LIS", false, false, false},
    {"HASH_JOIN", true, true, false},
    {"INDEX", false, false, false},
    {"JOIN_INDEX", false, false, false},
    {"GROUP_INDEX", false, false, false},
    {"ORDER_INDEX", false, false, false},
    {nullptr, false, false, false}};

/**
  Prefix for system generated query block name.
  Used in information warning in EXPLAIN oputput.
*/

const LEX_CSTRING sys_qb_prefix = {"select#", 7};

/*
  Compare LEX_CSTRING objects.

  @param s     Pointer to LEX_CSTRING
  @param t     Pointer to LEX_CSTRING
  @param cs    Pointer to character set

  @return  0 if strings are equal
           1 if s is greater
          -1 if t is greater
*/

int cmp_lex_string(const LEX_CSTRING *s, const LEX_CSTRING *t,
                   const CHARSET_INFO *cs) {
  return cs->coll->strnncollsp(cs, pointer_cast<const uchar *>(s->str),
                               s->length, pointer_cast<const uchar *>(t->str),
                               t->length);
}

bool Opt_hints::get_switch(opt_hints_enum type_arg) const {
  if (is_specified(type_arg)) return hints_map.switch_on(type_arg);

  if (opt_hint_info[type_arg].check_upper_lvl)
    return parent->get_switch(type_arg);

  return false;
}

Opt_hints *Opt_hints::find_by_name(const LEX_CSTRING *name_arg,
                                   const CHARSET_INFO *cs) const {
  for (uint i = 0; i < child_array.size(); i++) {
    const LEX_CSTRING *name = child_array[i]->get_print_name();
    if (!cmp_lex_string(name, name_arg, cs)) return child_array[i];
  }
  return nullptr;
}

void Opt_hints::print(const THD *thd, String *str, enum_query_type query_type) {
  for (uint i = 0; i < MAX_HINT_ENUM; i++) {
    if (opt_hint_info[i].irregular_hint) continue;
    opt_hints_enum hint = static_cast<opt_hints_enum>(i);
    /*
       If printing a normalized query, also unresolved hints will be printed.
       (This is needed by query rewrite plugins which request
       normalized form before resolving has been performed.)
    */
    if (is_specified(hint) && !ignore_print(hint) &&
        (is_resolved(hint) || query_type == QT_NORMALIZED_FORMAT)) {
      append_hint_type(str, hint);
      str->append(STRING_WITH_LEN("("));
      append_name(thd, str);
      if (!opt_hint_info[i].switch_hint)
        get_complex_hints(hint)->append_args(thd, str);
      str->append(STRING_WITH_LEN(") "));
    }
  }

  print_irregular_hints(thd, str);

  for (uint i = 0; i < child_array.size(); i++)
    child_array[i]->print(thd, str, query_type);
}

void Opt_hints::append_hint_type(String *str, opt_hints_enum type) {
  const char *hint_name = opt_hint_info[type].hint_name;
  if (!hints_map.switch_on(type)) str->append(STRING_WITH_LEN("NO_"));
  str->append(hint_name);
}

void Opt_hints::print_warn_unresolved(THD *thd) {
  String hint_name_str, hint_type_str;
  append_name(thd, &hint_name_str);

  for (uint i = 0; i < MAX_HINT_ENUM; i++) {
    if (is_specified(static_cast<opt_hints_enum>(i))) {
      hint_type_str.length(0);
      append_hint_type(&hint_type_str, static_cast<opt_hints_enum>(i));
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_UNRESOLVED_HINT_NAME,
          ER_THD(thd, ER_UNRESOLVED_HINT_NAME), hint_name_str.c_ptr_safe(),
          hint_type_str.c_ptr_safe());
      get_parent()->set_unresolved(static_cast<opt_hints_enum>(i));
    }
  }
}

void Opt_hints::check_unresolved(THD *thd) {
  if (!is_resolved(MAX_HINT_ENUM)) print_warn_unresolved(thd);

  if (!is_all_resolved()) {
    for (uint i = 0; i < child_array.size(); i++)
      child_array[i]->check_unresolved(thd);
  }
}

PT_hint *Opt_hints_global::get_complex_hints(opt_hints_enum type) {
  if (type == MAX_EXEC_TIME_HINT_ENUM) return max_exec_time;

  DBUG_ASSERT(0);
  return nullptr;
}

void Opt_hints_global::print_irregular_hints(const THD *thd, String *str) {
  if (sys_var_hint) sys_var_hint->print(thd, str);
}

Opt_hints_qb::Opt_hints_qb(Opt_hints *opt_hints_arg, MEM_ROOT *mem_root_arg,
                           uint select_number_arg)
    : Opt_hints(nullptr, opt_hints_arg, mem_root_arg),
      select_number(select_number_arg),
      subquery_hint(nullptr),
      semijoin_hint(nullptr),
      join_order_hints(mem_root_arg),
      join_order_hints_ignored(0) {
  sys_name.str = buff;
  sys_name.length =
      snprintf(buff, sizeof(buff), "%s%x", sys_qb_prefix.str, select_number);
}

PT_hint *Opt_hints_qb::get_complex_hints(opt_hints_enum type) {
  if (type == SEMIJOIN_HINT_ENUM) return semijoin_hint;

  if (type == SUBQUERY_HINT_ENUM) return subquery_hint;

  DBUG_ASSERT(0);
  return nullptr;
}

Opt_hints_table *Opt_hints_qb::adjust_table_hints(TABLE_LIST *tr) {
  const LEX_CSTRING str = {tr->alias, strlen(tr->alias)};
  Opt_hints_table *tab =
      static_cast<Opt_hints_table *>(find_by_name(&str, table_alias_charset));

  tr->opt_hints_qb = this;

  if (!tab)  // Tables not found
    return nullptr;

  tab->adjust_key_hints(tr);
  return tab;
}

bool Opt_hints_qb::semijoin_enabled(THD *thd) const {
  if (subquery_hint)  // SUBQUERY hint disables semi-join
    return false;

  if (semijoin_hint) {
    // SEMIJOIN hint will always force semijoin regardless of optimizer_switch
    if (semijoin_hint->switch_on()) return true;

    // NO_SEMIJOIN hint.  If strategy list is empty, do not use SEMIJOIN
    if (semijoin_hint->get_args() == 0) return false;

    // Fall through: NO_SEMIJOIN w/ strategies neither turns SEMIJOIN off nor on
  }

  return thd->optimizer_switch_flag(OPTIMIZER_SWITCH_SEMIJOIN);
}

uint Opt_hints_qb::sj_enabled_strategies(uint opt_switches) const {
  // Hints override switches
  if (semijoin_hint) {
    const uint strategies = semijoin_hint->get_args();
    if (semijoin_hint->switch_on())  // SEMIJOIN hint
      return (strategies == 0) ? opt_switches : strategies;

    // NO_SEMIJOIN hint. Hints and optimizer_switch both affect strategies
    return ~strategies & opt_switches;
  }

  return opt_switches;
}

SubqueryExecMethod Opt_hints_qb::subquery_strategy() const {
  if (subquery_hint)
    return static_cast<SubqueryExecMethod>(subquery_hint->get_args());

  return SubqueryExecMethod::EXEC_UNSPECIFIED;
}

void Opt_hints_qb::print_irregular_hints(const THD *thd, String *str) {
  /* Print join order hints */
  for (uint i = 0; i < join_order_hints.size(); i++) {
    if (join_order_hints_ignored & (1ULL << i)) continue;
    const PT_qb_level_hint *hint = join_order_hints[i];
    str->append(opt_hint_info[hint->type()].hint_name);
    str->append(STRING_WITH_LEN("("));
    append_name(thd, str);
    str->append(STRING_WITH_LEN(" "));
    hint->append_args(thd, str);
    str->append(STRING_WITH_LEN(") "));
  }
}

/**
  Print warning about unresolved table for join order hints.

  @param thd pointer to THD object
  @param type hint type
  @param hint_table table name
*/

static void print_join_order_warn(THD *thd, opt_hints_enum type,
                                  const Hint_param_table *hint_table) {
  String hint_name_str, hint_type_str;
  hint_type_str.append(opt_hint_info[type].hint_name);
  append_table_name(thd, &hint_name_str, &hint_table->opt_query_block,
                    &hint_table->table);
  push_warning_printf(thd, Sql_condition::SL_WARNING, ER_UNRESOLVED_HINT_NAME,
                      ER_THD(thd, ER_UNRESOLVED_HINT_NAME),
                      hint_name_str.c_ptr_safe(), hint_type_str.c_ptr_safe());
}

/**
  Function compares hint table name and TABLE_LIST table name.
  Query block name is taken into account.

  @param hint_table         hint table name
  @param table              pointer to TABLE_LIST object

  @return false if table names are equal, true otherwise.
*/

static bool compare_table_name(const Hint_param_table *hint_table,
                               const TABLE_LIST *table) {
  const LEX_CSTRING *hint_qb_name = &hint_table->opt_query_block;
  const LEX_CSTRING *hint_table_name = &hint_table->table;

  const LEX_CSTRING *table_qb_name =
      table->opt_hints_qb ? table->opt_hints_qb->get_name() : nullptr;
  const LEX_CSTRING table_name = {table->alias, strlen(table->alias)};

  if (table_qb_name && table_qb_name->length > 0 && hint_qb_name->length > 0) {
    if (cmp_lex_string(hint_qb_name, table_qb_name, system_charset_info))
      return true;
  }

  if (cmp_lex_string(hint_table_name, &table_name, system_charset_info))
    return true;

  return false;
}

/**
  Function returns dependencies used for updating table dependencies
  depending on hint type.

  @param type          hint type
  @param hint_tab_map  hint table map
  @param table_map     table map

  @return table dependencies.
*/

static table_map get_other_dep(opt_hints_enum type, table_map hint_tab_map,
                               table_map table_map) {
  switch (type) {
    case JOIN_PREFIX_HINT_ENUM:
      if (hint_tab_map & table_map)  // Hint table: No additional dependencies
        return 0;
      else  // Other tables: depend on all hint tables
        return hint_tab_map;
    case JOIN_SUFFIX_HINT_ENUM:
      if (hint_tab_map & table_map)  // Hint table: depends on all other tables
        return ~hint_tab_map;
      else
        return 0;
    case JOIN_ORDER_HINT_ENUM:
      return 0;  // No additional dependencies
    default:
      DBUG_ASSERT(0);
      break;
  }
  return 0;
}

/**
  Auxiluary class is used to save/restore table dependencies.
*/

class Join_order_hint_handler {
  JOIN *join;
  table_map *orig_dep_array;  ///< Original table dependencies

 public:
  Join_order_hint_handler(JOIN *join_arg)
      : join(join_arg), orig_dep_array(nullptr) {}

  /**
    Allocates and initializes orig_dep_array.

    @return true if orig_dep_array is allocated, false otherwise.
  */
  bool init() {
    orig_dep_array =
        (table_map *)join->thd->alloc(sizeof(table_map) * join->tables);

    if (orig_dep_array == nullptr) return true;

    for (uint i = 0; i < join->tables; i++) {
      JOIN_TAB *tab = &join->join_tab[i];
      orig_dep_array[i] = tab->dependent;
    }
    return false;
  }

  void no_restore_deps() { orig_dep_array = nullptr; }

  /**
    Restore original dependencies if necessary.
  */

  ~Join_order_hint_handler() {
    if (orig_dep_array == nullptr) return;

    for (uint i = 0; i < join->tables; i++) {
      JOIN_TAB *tab = &join->join_tab[i];
      tab->dependent = orig_dep_array[i];
    }
  }
};

/**
  Function updates dependencies for nested joins. If table
  specified in the hint belongs to nested join, we need
  to update dependencies of all tables of the nested join
  with the same dependency as for the hint table. It is also
  necessary to update all tables of the nested joins this table
  is part of.

  @param join             pointer to JOIN object
  @param hint_tab         pointer to JOIN_TAB object
  @param hint_tab_map     map of the tables, specified in the hint
*/

static void update_nested_join_deps(JOIN *join, const JOIN_TAB *hint_tab,
                                    table_map hint_tab_map) {
  const TABLE_LIST *table = hint_tab->table_ref;
  if (table->embedding) {
    for (uint i = 0; i < join->tables; i++) {
      JOIN_TAB *tab = &join->join_tab[i];
      if (tab->table_ref->embedding) {
        const NESTED_JOIN *const nested_join =
            tab->table_ref->embedding->nested_join;
        if (hint_tab->embedding_map & nested_join->nj_map)
          tab->dependent |= (hint_tab_map & ~nested_join->used_tables);
      }
    }
  }
}

/**
  Function resolves hint tables, checks and sets table dependencies
  according to the hint. If the hint is ignored due to circular table
  dependencies, orginal dependencies are restored.

  @param join             pointer to JOIN object
  @param hint_table_list  hint table list
  @param type             hint type

  @return false if hint is applied, true otherwise.
*/

static bool set_join_hint_deps(JOIN *join,
                               const Hint_param_table_list *hint_table_list,
                               opt_hints_enum type) {
  /*
    Make a copy of the original table dependencies.
    If an error occurs when applying the hint dependencies,
    the original dependencies will be restored by the destructor for this
    object.
  */
  Join_order_hint_handler hint_handler(join);
  // Map of the tables, specified in the hint
  table_map hint_tab_map = 0;

  if (hint_handler.init()) return true;

  for (const Hint_param_table *hint_table = hint_table_list->begin();
       hint_table < hint_table_list->end(); hint_table++) {
    bool hint_table_found = false;
    for (uint i = 0; i < join->tables; i++) {
      const TABLE_LIST *table = join->join_tab[i].table_ref;
      if (!compare_table_name(hint_table, table)) {
        hint_table_found = true;
        /*
          Const tables are excluded from the process of dependency setting
          since they are always first in the table order. Note that it
          does not prevent the hint from being applied to the non-const
          tables of the hint.
        */
        if (join->const_table_map & table->map()) break;

        JOIN_TAB *tab = &join->join_tab[i];
        // Hint tables are always dependent on preceeding tables
        tab->dependent |= hint_tab_map;
        update_nested_join_deps(join, tab, hint_tab_map);
        hint_tab_map |= tab->table_ref->map();
        break;
      }
    }

    if (!hint_table_found) {
      print_join_order_warn(join->thd, type, hint_table);
      return true;
    }
  }

  // Add dependencies that are related to non-hint tables
  for (uint i = 0; i < join->tables; i++) {
    JOIN_TAB *tab = &join->join_tab[i];
    table_map dependent_tables =
        get_other_dep(type, hint_tab_map, tab->table_ref->map());
    update_nested_join_deps(join, tab, dependent_tables);
    tab->dependent |= dependent_tables;
  }

  if (join->propagate_dependencies()) return true;

  hint_handler.no_restore_deps();
  return false;
}

/**
  Function applies join order hints.

  @param join pointer to JOIN object
*/

void Opt_hints_qb::apply_join_order_hints(JOIN *join) {
  for (uint hint_idx = 0; hint_idx < join_order_hints.size(); hint_idx++) {
    PT_qb_level_hint *hint = join_order_hints[hint_idx];
    Hint_param_table_list *hint_table_list = hint->get_table_list();
    if (set_join_hint_deps(join, hint_table_list, hint->type()))
      //  Skip hint printing in EXPLAIN message.
      join_order_hints_ignored |= 1ULL << hint_idx;
  }
}

void Opt_hints_table::adjust_key_hints(TABLE_LIST *tr) {
  set_resolved();
  if (child_array_ptr()->size() == 0)  // No key level hints
  {
    get_parent()->incr_resolved_children();
    return;
  }

  /*
    Make sure that adjustement is done only once.
    Table has already been processed if keyinfo_array is not empty.
  */
  if (keyinfo_array.size()) return;

  if (tr->is_view_or_derived())
    return;  // Names of keys are not known for derived tables

  TABLE *table = tr->table;
  keyinfo_array.resize(table->s->keys, nullptr);

  for (Opt_hints **hint = child_array_ptr()->begin();
       hint < child_array_ptr()->end(); ++hint) {
    KEY *key_info = table->key_info;
    for (uint j = 0; j < table->s->keys; j++, key_info++) {
      const LEX_CSTRING key_name = {key_info->name, strlen(key_info->name)};
      if (!cmp_lex_string((*hint)->get_name(), &key_name,
                          system_charset_info)) {
        (*hint)->set_resolved();
        keyinfo_array[j] = static_cast<Opt_hints_key *>(*hint);
        incr_resolved_children();
        set_compound_key_hint_map(*hint, j);
      }
    }
  }

  /*
   Do not increase number of resolved tables
   if there are unresolved key objects. It's
   important for check_unresolved() function.
  */
  if (is_all_resolved()) get_parent()->incr_resolved_children();
}

bool is_compound_hint(opt_hints_enum type_arg) {
  return (
      type_arg == INDEX_MERGE_HINT_ENUM || type_arg == SKIP_SCAN_HINT_ENUM ||
      type_arg == INDEX_HINT_ENUM || type_arg == JOIN_INDEX_HINT_ENUM ||
      type_arg == GROUP_INDEX_HINT_ENUM || type_arg == ORDER_INDEX_HINT_ENUM ||
      type_arg == GROUP_BY_LIS_HINT_ENUM);
}

PT_hint *Opt_hints_table::get_complex_hints(opt_hints_enum type) {
  DBUG_ASSERT(is_compound_hint(type));
  return get_compound_key_hint(type)->get_pt_hint();
}

bool Opt_hints_table::is_hint_conflicting(Opt_hints_key *key_hint,
                                          opt_hints_enum type) {
  if ((key_hint == NULL) && is_specified(type)) return true;
  return (key_hint && key_hint->is_specified(type));
}

/**
  Function updates key_to_use key map depending on index hint state.

  @param keys_to_use            key to use
  @param available_keys_to_use  available keys to use
  @param type_arg               hint type
*/

void Opt_hints_table::update_index_hint_map(Key_map *keys_to_use,
                                            Key_map *available_keys_to_use,
                                            opt_hints_enum type_arg) {
  // Check if hint is resolved.
  if (is_resolved(type_arg)) {
    Key_map *keys_specified_in_hint =
        get_compound_key_hint(type_arg)->get_key_map();
    if (get_switch(type_arg)) {
      // If the hint is on and no keys are specified in the hint,
      // then set "keys_to_use" to all the available keys.
      if (keys_specified_in_hint->is_clear_all())
        keys_to_use->merge(*available_keys_to_use);
      // If hint is on and there are keys specified in the hint, then add
      // the specified keys to "keys_to_use" taking care of the disabled keys
      // (available_keys_to_use).
      else {
        keys_to_use->merge(*keys_specified_in_hint);
        keys_to_use->intersect(*available_keys_to_use);
      }
    } else {
      // If hint is off and there are no keys specified in the hint, then
      // we clear "keys_to_use".
      if (keys_specified_in_hint->is_clear_all()) keys_to_use->clear_all();
      // If hint is off and some keys are specified in the hint, then remove
      // the specified keys from "keys_to_use.
      else
        keys_to_use->subtract(*keys_specified_in_hint);
    }
  }
}

/**
  Function updates keys_in_use_for_query, keys_in_use_for_group_by,
  keys_in_use_for_order_by depending on INDEX, JOIN_INDEX, GROUP_INDEX,
  ORDER_INDEX hints.

  @param thd            pointer to THD object
  @param tbl            pointer to TABLE object

  @return false if no index hint is specified, true otherwise.
*/

bool Opt_hints_table::update_index_hint_maps(THD *thd, TABLE *tbl) {
  if (!is_resolved(INDEX_HINT_ENUM) && !is_resolved(JOIN_INDEX_HINT_ENUM) &&
      !is_resolved(GROUP_INDEX_HINT_ENUM) &&
      !is_resolved(ORDER_INDEX_HINT_ENUM))
    return false;  // No index hint is specified

  Key_map usable_index_map(tbl->s->usable_indexes(thd));
  tbl->keys_in_use_for_query = tbl->keys_in_use_for_group_by =
      tbl->keys_in_use_for_order_by = usable_index_map;

  bool force_index = is_force_index_hint(INDEX_HINT_ENUM);
  tbl->force_index = (force_index || is_force_index_hint(JOIN_INDEX_HINT_ENUM));
  tbl->force_index_group =
      (force_index || is_force_index_hint(GROUP_INDEX_HINT_ENUM));
  tbl->force_index_order =
      (force_index || is_force_index_hint(ORDER_INDEX_HINT_ENUM));

  if (tbl->force_index || tbl->force_index_group || tbl->force_index_order) {
    tbl->keys_in_use_for_query.clear_all();
    tbl->keys_in_use_for_group_by.clear_all();
    tbl->keys_in_use_for_order_by.clear_all();
  }

  update_index_hint_map(&tbl->keys_in_use_for_query, &usable_index_map,
                        INDEX_HINT_ENUM);
  update_index_hint_map(&tbl->keys_in_use_for_group_by, &usable_index_map,
                        INDEX_HINT_ENUM);
  update_index_hint_map(&tbl->keys_in_use_for_order_by, &usable_index_map,
                        INDEX_HINT_ENUM);
  update_index_hint_map(&tbl->keys_in_use_for_query, &usable_index_map,
                        JOIN_INDEX_HINT_ENUM);
  update_index_hint_map(&tbl->keys_in_use_for_group_by, &usable_index_map,
                        GROUP_INDEX_HINT_ENUM);
  update_index_hint_map(&tbl->keys_in_use_for_order_by, &usable_index_map,
                        ORDER_INDEX_HINT_ENUM);
  /* Make sure "covering_keys" does not include indexes disabled with a hint */
  Key_map covering_keys(tbl->keys_in_use_for_query);
  covering_keys.merge(tbl->keys_in_use_for_group_by);
  covering_keys.merge(tbl->keys_in_use_for_order_by);
  tbl->covering_keys.intersect(covering_keys);
  return true;
}

/**
  Function checks if INDEX hint is conflicting with
  already specified JOIN_INDEX, GROUP_INDEX, ORDER_INDEX
  hints.

  @param table_hint         pointer to table hint
  @param key_hint           pointer to key hint

  @return false if no conflict, true otherwise.
*/

bool Glob_index_key_hint::is_hint_conflicting(Opt_hints_table *table_hint,
                                              Opt_hints_key *key_hint) {
  return (table_hint->is_hint_conflicting(key_hint, JOIN_INDEX_HINT_ENUM) ||
          table_hint->is_hint_conflicting(key_hint, GROUP_INDEX_HINT_ENUM) ||
          table_hint->is_hint_conflicting(key_hint, ORDER_INDEX_HINT_ENUM));
}

/**
  Function checks if JOIN_INDEX|GROUP_INDEX|ORDER_INDEX
  hint is conflicting with already specified INDEX hint.

  @param table_hint         pointer to table hint
  @param key_hint           pointer to key hint

  @return false if no conflict, true otherwise.
*/

bool Index_key_hint::is_hint_conflicting(Opt_hints_table *table_hint,
                                         Opt_hints_key *key_hint) {
  return table_hint->is_hint_conflicting(key_hint, INDEX_HINT_ENUM);
}

/**
  Function prints hint using the info from set_var variable.

  @param thd            Thread handle
  @param str            Pointer to string object
  @param var            Pointer to set_var object
*/

static void print_hint_from_var(const THD *thd, String *str, set_var *var) {
  str->append(STRING_WITH_LEN("SET_VAR("));
  var->print_short(thd, str);
  str->append(STRING_WITH_LEN(") "));
}

/**
  Function prints hint as it is specified.

  @param str            Pointer to string object
  @param sys_var_name   Variable name
  @param sys_var_value  Variable value
*/

static void print_hint_specified(String *str, LEX_CSTRING *sys_var_name,
                                 Item *sys_var_value) {
  str->append(STRING_WITH_LEN("SET_VAR("));
  str->append(sys_var_name->str, sys_var_name->length);
  str->append(STRING_WITH_LEN("="));
  char buff[STRING_BUFFER_USUAL_SIZE];
  String str_buff(buff, sizeof(buff), system_charset_info), *str_res;
  str_res = sys_var_value->val_str(&str_buff);
  if (sys_var_value->result_type() == STRING_RESULT) {
    str->append(STRING_WITH_LEN("'"));
    str->append(str_res->ptr(), str_res->length());
    str->append(STRING_WITH_LEN("'"));
  } else if (sys_var_value->result_type() == INT_RESULT)
    str->append(str_res->ptr(), str_res->length());
  str->append(STRING_WITH_LEN(") "));
}

bool Sys_var_hint::add_var(THD *thd, sys_var *sys_var, Item *sys_var_value) {
  for (uint i = 0; i < var_list.size(); i++) {
    const Hint_set_var *hint_var = var_list[i];
    set_var *var = hint_var->var;
    /*
      Issue a warning if system variable is already present in hint list.
    */
    if (!cmp_lex_string(&var->var->name, &sys_var->name, system_charset_info)) {
      String str;
      print_hint_specified(&str, &var->var->name, sys_var_value);
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WARN_CONFLICTING_HINT,
          ER_THD(thd, ER_WARN_CONFLICTING_HINT), str.c_ptr_safe());
      return false;
    }
  }

  set_var *var = new (thd->mem_root)
      set_var(OPT_SESSION, sys_var, sys_var->name, sys_var_value);
  if (!var) return true;

  Hint_set_var *hint_var = new (thd->mem_root) Hint_set_var(var);
  if (!hint_var) return true;

  return var_list.push_back(hint_var);
}

void Sys_var_hint::update_vars(THD *thd) {
  // Skip SET_VAR hint applying on the slave.
  if (thd->slave_thread) return;

  Set_var_error_handler error_handler(false);
  for (uint i = 0; i < var_list.size(); i++) {
    thd->push_internal_handler(&error_handler);
    Hint_set_var *hint_var = var_list[i];
    set_var *var = hint_var->var;
    if (!var->resolve(thd) && !var->check(thd)) {
      Item *save_value = var->var->copy_value(thd);
      if (!var->update(thd)) hint_var->save_value = save_value;
    }
    thd->pop_internal_handler();
    error_handler.reset_state();
  }
}

void Sys_var_hint::restore_vars(THD *thd) {
  Set_var_error_handler error_handler(true);
  thd->push_internal_handler(&error_handler);
  for (uint i = 0; i < var_list.size(); i++) {
    Hint_set_var *hint_var = var_list[i];
    set_var *var = hint_var->var;
    if (hint_var->save_value) {
      /* Restore original vaule for update */
      std::swap(var->value, hint_var->save_value);
      /*
        There should be no error since original value is restored.
      */
#ifndef DBUG_OFF
      DBUG_ASSERT(!var->check(thd));
      DBUG_ASSERT(!var->update(thd));
#else
      (void)var->check(thd);
      (void)var->update(thd);
#endif
      /* Restore hint vaule for further executions */
      std::swap(var->value, hint_var->save_value);
    }
  }
  thd->pop_internal_handler();
}

void Sys_var_hint::print(const THD *thd, String *str) {
  for (uint i = 0; i < var_list.size(); i++) {
    Hint_set_var *hint_var = var_list[i];
    if (hint_var->save_value) print_hint_from_var(thd, str, hint_var->var);
  }
}

/**
  Function returns hint value depending on
  the specfied hint level. If hint is specified
  on current level, current level hint value is
  returned, otherwise parent level hint is checked.

  @param hint              Pointer to the hint object
  @param parent_hint       Pointer to the parent hint object,
                           should never be NULL
  @param type_arg          hint type
  @param [out] ret_val     hint value depending on
                           what hint level is used

  @return true if hint is specified, false otherwise
*/

static bool get_hint_state(Opt_hints *hint, Opt_hints *parent_hint,
                           opt_hints_enum type_arg, bool *ret_val) {
  DBUG_ASSERT(parent_hint);

  if (opt_hint_info[type_arg].switch_hint) {
    if (hint && hint->is_specified(type_arg)) {
      *ret_val = hint->get_switch(type_arg);
      return true;
    } else if (opt_hint_info[type_arg].check_upper_lvl &&
               parent_hint->is_specified(type_arg)) {
      *ret_val = parent_hint->get_switch(type_arg);
      return true;
    }
  } else {
    if (hint && hint->is_specified(type_arg) &&
        /*
          This check is necessary because function idx_merge_key_enabled()
          can operate only with resolved hint. For unresolved hint function,
          idx_merge_key_enabled() always returns 'true' to emulate absence
          of the hint.
        */
        hint->is_resolved(type_arg)) {
      *ret_val = hint->get_switch(type_arg);
      return true;
    }
  }
  return false;
}

bool hint_key_state(const THD *thd, const TABLE_LIST *table, uint keyno,
                    opt_hints_enum type_arg, uint optimizer_switch) {
  Opt_hints_table *table_hints = table->opt_hints_table;

  /* Parent should always be initialized */
  if (table_hints && keyno != MAX_KEY) {
    Opt_hints_key *key_hints = table_hints->keyinfo_array.size() > 0
                                   ? table_hints->keyinfo_array[keyno]
                                   : NULL;
    bool ret_val = false;
    if (get_hint_state(key_hints, table_hints, type_arg, &ret_val))
      return ret_val;
  }

  return thd->optimizer_switch_flag(optimizer_switch);
}

bool hint_table_state(const THD *thd, const TABLE_LIST *table_list,
                      opt_hints_enum type_arg, uint optimizer_switch) {
  if (table_list->opt_hints_qb) {
    bool ret_val = false;
    if (get_hint_state(table_list->opt_hints_table, table_list->opt_hints_qb,
                       type_arg, &ret_val))
      return ret_val;
  }

  return thd->optimizer_switch_flag(optimizer_switch);
}

void append_table_name(const THD *thd, String *str, const LEX_CSTRING *qb_name,
                       const LEX_CSTRING *table_name) {
  /* Append table name */
  append_identifier(thd, str, table_name->str, table_name->length);

  /* Append QB name */
  if (qb_name && qb_name->length > 0) {
    str->append(STRING_WITH_LEN("@"));
    append_identifier(thd, str, qb_name->str, qb_name->length);
  }
}

bool compound_hint_key_enabled(const TABLE *table, uint keyno,
                               opt_hints_enum type_arg) {
  Opt_hints_table *table_hints = table->pos_in_table_list->opt_hints_table;

  if (table_hints && table_hints->is_resolved(type_arg)) {
    if (table_hints->get_compound_key_hint(type_arg)->is_key_map_clear_all())
      return table_hints->get_compound_key_hint(type_arg)
          ->get_pt_hint()
          ->switch_on();

    return table_hints->get_compound_key_hint(type_arg)->is_set_key_map(
               keyno) == table_hints->get_compound_key_hint(type_arg)
                             ->get_pt_hint()
                             ->switch_on();
  }
  return true;
}

bool idx_merge_hint_state(const TABLE *table, bool *use_cheapest_index_merge) {
  bool force_index_merge = hint_table_state(
      table->in_use, table->pos_in_table_list, INDEX_MERGE_HINT_ENUM, 0);
  if (force_index_merge) {
    DBUG_ASSERT(table->pos_in_table_list->opt_hints_table);
    Opt_hints_table *table_hints = table->pos_in_table_list->opt_hints_table;
    /*
      If INDEX_MERGE hint is used without only specified index,
      cheapest index merge should be used.
    */
    *use_cheapest_index_merge = table_hints->index_merge.is_key_map_clear_all();
  }

  return force_index_merge;
}
