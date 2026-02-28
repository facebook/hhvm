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

#include "sql/parse_tree_hints.h"

#include <stddef.h>
#include <string.h>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/item_subselect.h"
#include "sql/mysqld.h"  // table_alias_charset
#include "sql/query_options.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/resourcegroups/resource_group_mgr.h"
#include "sql/set_var.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"

extern struct st_opt_hint_info opt_hint_info[];

/**
  Returns pointer to Opt_hints_global object,
  create Opt_hints object if not exist.

  @param pc   pointer to Parse_context object

  @return  pointer to Opt_hints object,
           NULL if failed to create the object
*/

static Opt_hints_global *get_global_hints(Parse_context *pc) {
  LEX *lex = pc->thd->lex;

  if (!lex->opt_hints_global)
    lex->opt_hints_global =
        new (pc->thd->mem_root) Opt_hints_global(pc->thd->mem_root);
  if (lex->opt_hints_global) lex->opt_hints_global->set_resolved();
  return lex->opt_hints_global;
}

/**
  Returns pointer to Opt_hints_qb object
  for query block given by parse context,
  create Opt_hints_qb object if not exist.

  @param pc      pointer to Parse_context object
  @param select  pointer to SELECT_LEX object

  @return  pointer to Opt_hints_qb object,
           NULL if failed to create the object
*/

static Opt_hints_qb *get_qb_hints(Parse_context *pc, SELECT_LEX *select) {
  if (select->opt_hints_qb) return select->opt_hints_qb;

  Opt_hints_global *global_hints = get_global_hints(pc);
  if (global_hints == nullptr) return nullptr;

  Opt_hints_qb *qb = new (pc->thd->mem_root)
      Opt_hints_qb(global_hints, pc->thd->mem_root, select->select_number);
  if (qb) {
    global_hints->register_child(qb);
    select->opt_hints_qb = qb;
    qb->set_resolved();
  }
  return qb;
}

/**
  Find existing Opt_hints_qb object, print warning
  if the query block is not found.

  @param pc          pointer to Parse_context object
  @param qb_name     query block name
  @param hint        processed hint

  @return  pointer to Opt_hints_table object if found,
           NULL otherwise
*/

static Opt_hints_qb *find_qb_hints(Parse_context *pc,
                                   const LEX_CSTRING *qb_name, PT_hint *hint) {
  if (qb_name->length == 0)  // no QB NAME is used
    return pc->select->opt_hints_qb;

  Opt_hints_qb *qb =
      static_cast<Opt_hints_qb *>(pc->thd->lex->opt_hints_global->find_by_name(
          qb_name, system_charset_info));

  //  Find query block using system name. Used for proper parsing of view body.
  if (!qb) {
    LEX *lex = pc->thd->lex;
    for (SELECT_LEX *select = lex->all_selects_list; select != nullptr;
         select = select->next_select_in_list()) {
      LEX_CSTRING sys_name;  // System QB name
      char buff[32];         // Buffer to hold sys name
      sys_name.str = buff;
      sys_name.length = snprintf(buff, sizeof(buff), "%s%x", "select#",
                                 select->select_number);

      if (!cmp_lex_string(&sys_name, qb_name, system_charset_info)) {
        qb = get_qb_hints(pc, select);
        break;
      }
    }
  }

  if (qb == nullptr)
    hint->print_warn(pc->thd, ER_WARN_UNKNOWN_QB_NAME, qb_name, nullptr,
                     nullptr, nullptr);

  return qb;
}

/**
  Returns pointer to Opt_hints_table object,
  create Opt_hints_table object if not exist.

  @param pc          pointer to Parse_context object
  @param table_name  pointer to Hint_param_table object
  @param qb          pointer to Opt_hints_qb object

  @return   pointer to Opt_hints_table object,
            NULL if failed to create the object
*/

static Opt_hints_table *get_table_hints(Parse_context *pc,
                                        Hint_param_table *table_name,
                                        Opt_hints_qb *qb) {
  Opt_hints_table *tab = static_cast<Opt_hints_table *>(
      qb->find_by_name(&table_name->table, table_alias_charset));
  if (!tab) {
    tab = new (pc->thd->mem_root)
        Opt_hints_table(&table_name->table, qb, pc->thd->mem_root);
    qb->register_child(tab);
  }

  return tab;
}

void PT_hint::print_warn(THD *thd, uint err_code,
                         const LEX_CSTRING *qb_name_arg,
                         LEX_CSTRING *table_name_arg, LEX_CSTRING *key_name_arg,
                         PT_hint *hint) const {
  String str;

  /* Append hint name */
  if (!state) str.append(STRING_WITH_LEN("NO_"));
  str.append(opt_hint_info[hint_type].hint_name);

  /* ER_WARN_UNKNOWN_QB_NAME with two arguments */
  if (err_code == ER_WARN_UNKNOWN_QB_NAME) {
    String qb_name_str;
    append_identifier(thd, &qb_name_str, qb_name_arg->str, qb_name_arg->length);
    push_warning_printf(thd, Sql_condition::SL_WARNING, err_code,
                        ER_THD(thd, ER_WARN_UNKNOWN_QB_NAME),
                        qb_name_str.c_ptr_safe(), str.c_ptr_safe());
    return;
  }

  /* ER_WARN_CONFLICTING_HINT with one argument */
  str.append('(');

  /* Append table name */
  if (table_name_arg && table_name_arg->length > 0)
    append_identifier(thd, &str, table_name_arg->str, table_name_arg->length);

  /* Append QB name */
  if (qb_name_arg && qb_name_arg->length > 0) {
    str.append(STRING_WITH_LEN("@"));
    append_identifier(thd, &str, qb_name_arg->str, qb_name_arg->length);
  }

  /* Append key name */
  if (key_name_arg && key_name_arg->length > 0) {
    str.append(' ');
    append_identifier(thd, &str, key_name_arg->str, key_name_arg->length);
  }

  /* Append additional hint arguments if they exist */
  if (hint) {
    if (qb_name_arg || table_name_arg || key_name_arg) str.append(' ');

    hint->append_args(thd, &str);
  }

  str.append(')');

  push_warning_printf(thd, Sql_condition::SL_WARNING, err_code,
                      ER_THD_NONCONST(thd, err_code), str.c_ptr_safe());
}

bool PT_qb_level_hint::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  Opt_hints_qb *qb = find_qb_hints(pc, &qb_name, this);
  if (qb == nullptr) return false;  // TODO: Should this generate a warning?

  bool no_warn = false;   // If true, do not print a warning
  bool conflict = false;  // true if this hint conflicts with a previous hint
  switch (type()) {
    case SEMIJOIN_HINT_ENUM:
      if (qb->subquery_hint)
        conflict = true;
      else if (!qb->semijoin_hint)
        qb->semijoin_hint = this;
      break;
    case SUBQUERY_HINT_ENUM:
      if (qb->semijoin_hint)
        conflict = true;
      else if (!qb->subquery_hint)
        qb->subquery_hint = this;
      break;
    case JOIN_PREFIX_HINT_ENUM:
    case JOIN_SUFFIX_HINT_ENUM:
      if (qb->get_switch(type()) || qb->get_switch(JOIN_FIXED_ORDER_HINT_ENUM))
        conflict = true;
      else
        qb->register_join_order_hint(this);
      break;
    case JOIN_ORDER_HINT_ENUM:
      if (qb->get_switch(JOIN_FIXED_ORDER_HINT_ENUM))
        conflict = true;
      else {
        /*
          Don't print 'conflicting hint' warning since
          it could be several JOIN_ORDER hints at the
          same time.
        */
        no_warn = true;
        qb->register_join_order_hint(this);
      }
      break;
    case JOIN_FIXED_ORDER_HINT_ENUM:
      if (qb->get_switch(JOIN_PREFIX_HINT_ENUM) ||
          qb->get_switch(JOIN_SUFFIX_HINT_ENUM) ||
          qb->get_switch(JOIN_ORDER_HINT_ENUM))
        conflict = true;
      else
        pc->select->add_base_options(SELECT_STRAIGHT_JOIN);
      break;
    default:
      DBUG_ASSERT(0);
  }

  if (conflict ||
      // Set hint or detect if hint has been set before
      (qb->set_switch(switch_on(), type(), false) && !no_warn))
    print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, &qb_name, nullptr, nullptr,
               this);

  return false;
}

void PT_qb_level_hint::append_args(const THD *thd, String *str) const {
  switch (type()) {
    case SEMIJOIN_HINT_ENUM: {
      int count = 0;
      if (args & OPTIMIZER_SWITCH_FIRSTMATCH) {
        str->append(STRING_WITH_LEN(" FIRSTMATCH"));
        ++count;
      }
      if (args & OPTIMIZER_SWITCH_LOOSE_SCAN) {
        if (count++ > 0) str->append(STRING_WITH_LEN(","));
        str->append(STRING_WITH_LEN(" LOOSESCAN"));
      }
      if (args & OPTIMIZER_SWITCH_MATERIALIZATION) {
        if (count++ > 0) str->append(STRING_WITH_LEN(","));
        str->append(STRING_WITH_LEN(" MATERIALIZATION"));
      }
      if (args & OPTIMIZER_SWITCH_DUPSWEEDOUT) {
        if (count++ > 0) str->append(STRING_WITH_LEN(","));
        str->append(STRING_WITH_LEN(" DUPSWEEDOUT"));
      }
      break;
    }
    case SUBQUERY_HINT_ENUM:
      switch (static_cast<SubqueryExecMethod>(args)) {
        case SubqueryExecMethod::EXEC_MATERIALIZATION:
          str->append(STRING_WITH_LEN(" MATERIALIZATION"));
          break;
        case SubqueryExecMethod::EXEC_EXISTS:
          str->append(STRING_WITH_LEN(" INTOEXISTS"));
          break;
        default:  // Exactly one of above strategies should always be specified
          DBUG_ASSERT(false);
      }
      break;
    case JOIN_PREFIX_HINT_ENUM:
    case JOIN_SUFFIX_HINT_ENUM:
    case JOIN_ORDER_HINT_ENUM:
      for (uint i = 0; i < table_list.size(); i++) {
        const Hint_param_table *table_name = &table_list.at(i);
        if (i != 0) str->append(',');
        append_table_name(thd, str, &table_name->opt_query_block,
                          &table_name->table);
      }
      break;
    case JOIN_FIXED_ORDER_HINT_ENUM:
      break;
    default:
      DBUG_ASSERT(false);
  }
}

bool PT_hint_list::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  if (!get_qb_hints(pc, pc->select)) return true;

  for (PT_hint **h = hints.begin(), **end = hints.end(); h < end; h++) {
    if (*h != nullptr) {
      if (pc->thd->lex->sql_command == SQLCOM_CREATE_VIEW &&
          !(*h)->supports_view())
        continue;
      if ((*h)->contextualize(pc)) return true;
    }
  }
  return false;
}

bool PT_table_level_hint::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  if (table_list.empty())  // Query block level hint
  {
    Opt_hints_qb *qb = find_qb_hints(pc, &qb_name, this);
    if (qb == nullptr) return false;

    if (qb->set_switch(switch_on(), type(), false))
      print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, &qb_name, nullptr, nullptr,
                 this);
    return false;
  }

  for (uint i = 0; i < table_list.size(); i++) {
    Hint_param_table *table_name = &table_list.at(i);
    /*
      If qb name exists then syntax '@qb_name table_name..' is used and
      we should use qb_name for finding query block. Otherwise syntax
      'table_name@qb_name' is used, so use table_name->opt_query_block.
    */
    const LEX_CSTRING *qb_name_str =
        qb_name.length > 0 ? &qb_name : &table_name->opt_query_block;

    Opt_hints_qb *qb = find_qb_hints(pc, qb_name_str, this);
    if (qb == nullptr) return false;

    Opt_hints_table *tab = get_table_hints(pc, table_name, qb);
    if (!tab) return true;

    if (tab->set_switch(switch_on(), type(), true))
      print_warn(pc->thd, ER_WARN_CONFLICTING_HINT,
                 &table_name->opt_query_block, &table_name->table, nullptr,
                 this);
  }

  return false;
}

void PT_key_level_hint::append_args(const THD *thd, String *str) const {
  if (is_compound_hint(type())) {
    for (uint i = 0; i < key_list.size(); i++) {
      const LEX_CSTRING *key_name = &key_list.at(i);
      str->append(STRING_WITH_LEN(" "));
      append_identifier(thd, str, key_name->str, key_name->length);
      if (i < key_list.size() - 1) {
        str->append(STRING_WITH_LEN(","));
      }
    }
  }
}

bool PT_key_level_hint::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  Opt_hints_qb *qb = find_qb_hints(pc, &table_name.opt_query_block, this);
  if (qb == nullptr) return false;

  Opt_hints_table *tab = get_table_hints(pc, &table_name, qb);
  if (!tab) return true;

  bool is_conflicting = false;
  if (key_list.empty())  // Table level hint
  {
    if ((is_compound_hint(type()) &&
         tab->get_compound_key_hint(type())->is_hint_conflicting(tab, NULL)) ||
        tab->set_switch(switch_on(), type(), false)) {
      print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, &table_name.opt_query_block,
                 &table_name.table, nullptr, this);
      return false;
    }
  }

  if (type() == INDEX_MERGE_HINT_ENUM && key_list.size() == 1 && switch_on()) {
    print_warn(pc->thd, ER_WARN_INVALID_HINT, &table_name.opt_query_block,
               &table_name.table, nullptr, this);
    return false;
  }

  Mem_root_array<Opt_hints_key *> key_hints(pc->thd->mem_root);
  for (size_t i = 0; i < key_list.size(); i++) {
    LEX_CSTRING *key_name = &key_list.at(i);
    Opt_hints_key *key =
        (Opt_hints_key *)tab->find_by_name(key_name, system_charset_info);

    if (!key) {
      key = new (pc->thd->mem_root)
          Opt_hints_key(key_name, tab, pc->thd->mem_root);
      tab->register_child(key);
    }

    bool is_specified = tab->is_specified(type()) || key->is_specified(type());
    if (is_specified && !is_compound_hint(type())) {
      print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, &table_name.opt_query_block,
                 &table_name.table, key_name, this);
      continue;
    }

    if (is_specified ||
        (is_compound_hint(type()) &&
         tab->get_compound_key_hint(type())->is_hint_conflicting(tab, key))) {
      is_conflicting = true;
      print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, &table_name.opt_query_block,
                 &table_name.table, NULL, this);
      break;
    }
    key_hints.push_back(key);
  }

  if (!is_conflicting) {
    for (size_t i = 0; i < key_hints.size(); i++) {
      Opt_hints_key *key = key_hints.at(i);
      key->set_switch(switch_on(), type(), true);
    }
    if (is_compound_hint(type())) {
      tab->get_compound_key_hint(type())->set_pt_hint(this);
      (void)tab->set_switch(switch_on(), type(), false);
    }
  }

  return false;
}

bool PT_hint_qb_name::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  Opt_hints_qb *qb = pc->select->opt_hints_qb;

  DBUG_ASSERT(qb);

  if (qb->get_name() ||                         // QB name is already set
      qb->get_parent()->find_by_name(&qb_name,  // Name is already used
                                     system_charset_info)) {
    print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, nullptr, nullptr, nullptr,
               this);
    return false;
  }

  qb->set_name(&qb_name);
  return false;
}

bool PT_hint_max_execution_time::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  if (pc->thd->lex->sql_command != SQLCOM_SELECT ||  // not a SELECT statement
      pc->thd->lex->sphead ||                        // or in a SP/trigger/event
      pc->select != pc->thd->lex->select_lex)        // or in a subquery
  {
    push_warning(pc->thd, Sql_condition::SL_WARNING,
                 ER_WARN_UNSUPPORTED_MAX_EXECUTION_TIME,
                 ER_THD(pc->thd, ER_WARN_UNSUPPORTED_MAX_EXECUTION_TIME));
    return false;
  }

  Opt_hints_global *global_hint = get_global_hints(pc);
  if (global_hint->is_specified(type())) {
    // Hint duplication: /*+ MAX_EXECUTION_TIME ... MAX_EXECUTION_TIME */
    print_warn(pc->thd, ER_WARN_CONFLICTING_HINT, nullptr, nullptr, nullptr,
               this);
    return false;
  }

  pc->thd->lex->max_execution_time = milliseconds;
  global_hint->set_switch(switch_on(), type(), false);
  global_hint->max_exec_time = this;
  return false;
}

bool PT_hint_sys_var::contextualize(Parse_context *pc) {
  if (!sys_var_value) {
    // No warning here, warning is issued by parser.
    return false;
  }

  sys_var *sys_var = find_sys_var_ex(pc->thd, sys_var_name.str,
                                     sys_var_name.length, true, false);
  if (!sys_var) {
    String str;
    str.append(STRING_WITH_LEN("'"));
    str.append(sys_var_name.str, sys_var_name.length);
    str.append(STRING_WITH_LEN("'"));
    push_warning_printf(
        pc->thd, Sql_condition::SL_WARNING, ER_UNRESOLVED_HINT_NAME,
        ER_THD(pc->thd, ER_UNRESOLVED_HINT_NAME), str.c_ptr_safe(), "SET_VAR");
    return false;
  }

  if (!sys_var->is_hint_updateable()) {
    String str;
    str.append(STRING_WITH_LEN("'"));
    str.append(sys_var_name.str, sys_var_name.length);
    str.append(STRING_WITH_LEN("'"));
    push_warning_printf(
        pc->thd, Sql_condition::SL_WARNING, ER_NOT_HINT_UPDATABLE_VARIABLE,
        ER_THD(pc->thd, ER_NOT_HINT_UPDATABLE_VARIABLE), str.c_ptr_safe());
    return false;
  }

  Opt_hints_global *global_hint = get_global_hints(pc);
  if (!global_hint) return true;
  if (!global_hint->sys_var_hint)
    global_hint->sys_var_hint =
        new (pc->thd->mem_root) Sys_var_hint(pc->thd->mem_root);
  if (!global_hint->sys_var_hint) return true;

  return global_hint->sys_var_hint->add_var(pc->thd, sys_var, sys_var_value);
}

bool PT_hint_resource_group::contextualize(Parse_context *pc) {
  if (super::contextualize(pc)) return true;

  auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();
  if (!res_grp_mgr->resource_group_support()) {
    pc->thd->resource_group_ctx()->m_warn = WARN_RESOURCE_GROUP_UNSUPPORTED;
    return false;
  }

  if (pc->thd->lex->sphead || pc->select != pc->thd->lex->select_lex) {
    pc->thd->resource_group_ctx()->m_warn =
        WARN_RESOURCE_GROUP_UNSUPPORTED_HINT;
    return false;
  }

  memcpy(pc->thd->resource_group_ctx()->m_switch_resource_group_str,
         m_resource_group_name.str, m_resource_group_name.length);
  pc->thd->resource_group_ctx()
      ->m_switch_resource_group_str[m_resource_group_name.length] = '\0';
  return false;
}
