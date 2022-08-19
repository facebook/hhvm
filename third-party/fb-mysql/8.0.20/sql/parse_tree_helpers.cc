/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/parse_tree_helpers.h"

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/mysql_lex_string.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/derror.h"
#include "sql/handler.h"
#include "sql/mysqld.h"
#include "sql/parse_tree_nodes.h"
#include "sql/resourcegroups/platform/thread_attrs_api.h"
#include "sql/resourcegroups/resource_group_mgr.h"  // Resource_group_mgr
#include "sql/sp_head.h"
#include "sql/sp_instr.h"
#include "sql/sp_pcontext.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_plugin_ref.h"
#include "sql/system_variables.h"
#include "sql/trigger_def.h"
#include "sql_string.h"

/**
  Create an object to represent a SP variable in the Item-hierarchy.

  @param thd              The current thread.
  @param name             The SP variable name.
  @param spv              The SP variable (optional).
  @param query_start_ptr  Start of the SQL-statement query string (optional).
  @param start            Start position of the SP variable name in the query.
  @param end              End position of the SP variable name in the query.

  @remark If spv is not specified, the name is used to search for the
          variable in the parse-time context. If the variable does not
          exist, a error is set and NULL is returned to the caller.

  @return An Item_splocal object representing the SP variable, or NULL on error.
*/
Item_splocal *create_item_for_sp_var(THD *thd, LEX_CSTRING name,
                                     sp_variable *spv,
                                     const char *query_start_ptr,
                                     const char *start, const char *end) {
  LEX *lex = thd->lex;
  size_t spv_pos_in_query = 0;
  size_t spv_len_in_query = 0;
  sp_pcontext *pctx = lex->get_sp_current_parsing_ctx();

  /* If necessary, look for the variable. */
  if (pctx && !spv) spv = pctx->find_variable(name.str, name.length, false);

  if (!spv) {
    my_error(ER_SP_UNDECLARED_VAR, MYF(0), name.str);
    return nullptr;
  }

  DBUG_ASSERT(pctx && spv);

  if (lex->reparse_common_table_expr_at != 0) {
    /*
      This variable doesn't exist in the original query: shouldn't be
      substituted for logging.
    */
    query_start_ptr = nullptr;
  }

  if (query_start_ptr) {
    /* Position and length of the SP variable name in the query. */
    spv_pos_in_query = start - query_start_ptr;
    spv_len_in_query = end - start;
  }

  Item_splocal *item = new (thd->mem_root) Item_splocal(
      name, spv->offset, spv->type, spv_pos_in_query, spv_len_in_query);

#ifndef DBUG_OFF
  if (item) item->m_sp = lex->sphead;
#endif

  return item;
}

bool find_sys_var_null_base(THD *thd, struct sys_var_with_base *tmp) {
  tmp->var = find_sys_var(thd, tmp->base_name.str, tmp->base_name.length);

  if (tmp->var == nullptr)
    my_error(ER_UNKNOWN_SYSTEM_VARIABLE, MYF(0), tmp->base_name.str);
  else
    tmp->base_name = NULL_CSTR;

  return thd->is_error();
}

/**
  Helper action for a SET statement.
  Used to push a system variable into the assignment list.

  @param thd      the current thread
  @param var_with_base  the system variable with base name
  @param var_type the scope of the variable
  @param val      the value being assigned to the variable

  @return true if error, false otherwise.
*/

bool set_system_variable(THD *thd, struct sys_var_with_base *var_with_base,
                         enum enum_var_type var_type, Item *val) {
  LEX *lex = thd->lex;
  sp_head *sp = lex->sphead;
  sp_pcontext *pctx = lex->get_sp_current_parsing_ctx();

  /* No AUTOCOMMIT from a stored function or trigger. */
  if (pctx && var_with_base->var == Sys_autocommit_ptr)
    sp->m_flags |= sp_head::HAS_SET_AUTOCOMMIT_STMT;

  if (lex->uses_stored_routines() &&
      ((var_with_base->var == Sys_gtid_next_ptr
#ifdef HAVE_GTID_NEXT_LIST
        || var_with_base->var == Sys_gtid_next_list_ptr
#endif
        ) ||
       Sys_gtid_purged_ptr == var_with_base->var)) {
    my_error(ER_SET_STATEMENT_CANNOT_INVOKE_FUNCTION, MYF(0),
             var_with_base->var->name.str);
    return true;
  }

  if (val && val->type() == Item::FIELD_ITEM &&
      ((Item_field *)val)->table_name) {
    my_error(ER_WRONG_TYPE_FOR_VAR, MYF(0), var_with_base->var->name.str);
    return true;
  }

  set_var *var = new (thd->mem_root)
      set_var(var_type, var_with_base->var, var_with_base->base_name, val);
  if (var == nullptr) return true;

  var->thd_id = lex->thread_id_opt;
  return lex->var_list.push_back(var);
}

/**
  Make a new string allocated on THD's mem-root.

  @param thd        thread handler.
  @param start_ptr  start of the new string.
  @param end_ptr    end of the new string.

  @return LEX_CSTRING object, containing a pointer to a newly
  constructed/allocated string, and its length. The pointer is NULL
  in case of out-of-memory error.
*/
LEX_CSTRING make_string(THD *thd, const char *start_ptr, const char *end_ptr) {
  size_t length = end_ptr - start_ptr;
  return {strmake_root(thd->mem_root, start_ptr, length), length};
}

/**
  Helper action for a SET statement.
  Used to SET a field of NEW row.

  @param pc                 the parse context
  @param trigger_field_name the NEW-row field name
  @param expr_item          the value expression being assigned
  @param expr_query         the value expression query

  @return error status (true if error, false otherwise).
*/

bool set_trigger_new_row(Parse_context *pc, LEX_CSTRING trigger_field_name,
                         Item *expr_item, LEX_CSTRING expr_query) {
  THD *thd = pc->thd;
  LEX *lex = thd->lex;
  sp_head *sp = lex->sphead;

  DBUG_ASSERT(expr_item);
  DBUG_ASSERT(sp->m_trg_chistics.action_time == TRG_ACTION_BEFORE &&
              (sp->m_trg_chistics.event == TRG_EVENT_INSERT ||
               sp->m_trg_chistics.event == TRG_EVENT_UPDATE));

  Item_trigger_field *trg_fld = new (pc->mem_root) Item_trigger_field(
      POS(), TRG_NEW_ROW, trigger_field_name.str, UPDATE_ACL, false);

  if (trg_fld == nullptr || trg_fld->itemize(pc, (Item **)&trg_fld))
    return true;
  DBUG_ASSERT(trg_fld->type() == Item::TRIGGER_FIELD_ITEM);

  sp_instr_set_trigger_field *i = new (pc->mem_root)
      sp_instr_set_trigger_field(sp->instructions(), lex, trigger_field_name,
                                 trg_fld, expr_item, expr_query);

  if (!i) return true;

  /*
    Let us add this item to list of all Item_trigger_field
    objects in trigger.
  */
  sp->m_cur_instr_trig_field_items.link_in_list(trg_fld,
                                                &trg_fld->next_trg_field);

  return sp->add_instr(thd, i);
}

void sp_create_assignment_lex(THD *thd, const char *option_ptr) {
  sp_head *sp = thd->lex->sphead;

  /*
    We can come here in the following cases:

      1. it's a regular SET statement outside stored programs
        (thd->lex->sphead is NULL);

      2. we're parsing a stored program normally (loading from mysql.proc, ...);

      3. we're re-parsing SET-statement with a user variable after meta-data
        change. It's guaranteed, that:
        - this SET-statement deals with a user/system variable (otherwise, it
          would be a different SP-instruction, and we would parse an
    expression);
        - this SET-statement has a single user/system variable assignment
          (that's how we generate sp_instr_stmt-instructions for
    SET-statements). So, in this case, even if thd->lex->sphead is set, we
    should not process further.
  */

  if (!sp ||             // case #1
      sp->is_invoked())  // case #3
  {
    return;
  }

  LEX *old_lex = thd->lex;
  sp->reset_lex(thd);
  LEX *const lex = thd->lex;

  /* Set new LEX as if we at start of set rule. */
  lex->sql_command = SQLCOM_SET_OPTION;
  lex->var_list.empty();
  lex->autocommit = false;

  /*
    It's a SET statement within SP. It will be either translated
    into one or more sp_instr_stmt instructions, or it will be
    sp_instr_set / sp_instr_set_trigger_field instructions.
    In any case, position of SP-variable can not be determined
    reliably. So, we set the start pointer of the current statement
    to NULL.
  */
  sp->m_parser_data.set_current_stmt_start_ptr(nullptr);
  sp->m_parser_data.set_option_start_ptr(option_ptr);

  /* Inherit from outer lex. */
  lex->option_type = old_lex->option_type;
}

/**
  Create a SP instruction for a SET assignment.

  @see sp_create_assignment_lex

  @param thd           Thread context
  @param expr_end_ptr  Option-value-expression end pointer

  @return false if success, true otherwise.
*/

bool sp_create_assignment_instr(THD *thd, const char *expr_end_ptr) {
  LEX *lex = thd->lex;
  sp_head *sp = lex->sphead;

  /*
    We can come here in the following cases:

      1. it's a regular SET statement outside stored programs
        (lex->sphead is NULL);

      2. we're parsing a stored program normally (loading from mysql.proc, ...);

      3. we're re-parsing SET-statement with a user variable after meta-data
        change. It's guaranteed, that:
        - this SET-statement deals with a user/system variable (otherwise, it
          would be a different SP-instruction, and we would parse an
    expression);
        - this SET-statement has a single user/system variable assignment
          (that's how we generate sp_instr_stmt-instructions for
    SET-statements). So, in this case, even if lex->sphead is set, we should not
    process further.
  */

  if (!sp ||             // case #1
      sp->is_invoked())  // case #3
  {
    return false;
  }

  if (!lex->var_list.is_empty()) {
    /* Extract expression string. */

    const char *expr_start_ptr = sp->m_parser_data.get_option_start_ptr();

    LEX_CSTRING expr{expr_start_ptr,
                     static_cast<size_t>(expr_end_ptr - expr_start_ptr)};

    /* Construct SET-statement query. */

    LEX_CSTRING set_stmt_query;

    set_stmt_query.length = expr.length + 3;
    char *c = static_cast<char *>(thd->alloc(set_stmt_query.length + 1));

    if (!c) return true;

    strmake(strmake(c, "SET", 3), expr.str, expr.length);
    set_stmt_query.str = c;

    /*
      We have assignment to user or system variable or option setting, so we
      should construct sp_instr_stmt for it.
    */

    sp_instr_stmt *i = new (thd->mem_root)
        sp_instr_stmt(sp->instructions(), lex, set_stmt_query);

    if (!i || sp->add_instr(thd, i)) return true;
  }

  /* Remember option_type of the currently parsed LEX. */
  const enum_var_type inner_option_type = lex->option_type;
  const ulong thd_id_opt = lex->thread_id_opt;

  if (sp->restore_lex(thd)) return true;

  /* Copy option_type to outer lex in case it has changed. */
  thd->lex->option_type = inner_option_type;
  thd->lex->thread_id_opt = thd_id_opt;

  return false;
}

/**
  Resolve engine by its name

  @param        thd            Thread handler.
  @param        name           Engine's name.
  @param        is_temp_table  True if temporary table.
  @param        strict         Force error if engine is unknown(*).
  @param[out]   ret            Engine object or NULL(**).

  @returns true if error is reported(**), otherwise false.

  @note *) NO_ENGINE_SUBSTITUTION sql_mode overrides the @c strict parameter.
  @note **) If @c strict if false and engine is unknown, the function outputs
            a warning, sets @c ret to NULL and returns false (success).
*/
bool resolve_engine(THD *thd, const LEX_CSTRING &name, bool is_temp_table,
                    bool strict, handlerton **ret) {
  plugin_ref plugin = ha_resolve_by_name(thd, &name, is_temp_table);
  if (plugin) {
    *ret = plugin_data<handlerton *>(plugin);
    return false;
  }

  if (strict || !is_engine_substitution_allowed(thd)) {
    my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), name.str);
    return true;
  }
  push_warning_printf(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_STORAGE_ENGINE,
                      ER_THD(thd, ER_UNKNOWN_STORAGE_ENGINE), name.str);
  *ret = nullptr;
  return false;
}

/**
  This helper function is responsible for aggregating grants from parser tokens
  to containers and masks which can be used during semantic analysis.

  @param thd The thread handler
  @param privs A list of parser tokens representing roles or privileges.
  @return Error state
    @retval true An error occurred
    @retval false Success
*/

bool apply_privileges(
    THD *thd, const Mem_root_array<class PT_role_or_privilege *> &privs) {
  LEX *const lex = thd->lex;

  for (PT_role_or_privilege *p : privs) {
    Privilege *privilege = p->get_privilege(thd);
    if (privilege == nullptr) return true;

    if (privilege->type == Privilege::DYNAMIC) {
      // We can push a reference to the PT object since it will have the same
      // life time as our dynamic_privileges list.
      LEX_CSTRING *grant =
          static_cast<LEX_CSTRING *>(thd->alloc(sizeof(LEX_CSTRING)));
      grant->str = static_cast<Dynamic_privilege *>(privilege)->ident.str;
      grant->length = static_cast<Dynamic_privilege *>(privilege)->ident.length;
      char *s = static_cast<Dynamic_privilege *>(privilege)->ident.str;
      char *s_end =
          s + static_cast<Dynamic_privilege *>(privilege)->ident.length;
      while (s != s_end) {
        *s = my_toupper(system_charset_info, *s);
        ++s;
      }
      lex->dynamic_privileges.push_back(grant);
    } else {
      auto grant = static_cast<Static_privilege *>(privilege)->grant;
      auto columns = static_cast<Static_privilege *>(privilege)->columns;

      if (columns == nullptr)
        lex->grant |= grant;
      else {
        for (auto &c : *columns) {
          auto new_str =
              new (thd->mem_root) String(c.str, c.length, system_charset_info);
          if (new_str == nullptr) return true;
          List_iterator<LEX_COLUMN> iter(lex->columns);
          class LEX_COLUMN *point;
          while ((point = iter++)) {
            if (!my_strcasecmp(system_charset_info, point->column.ptr(),
                               new_str->ptr()))
              break;
          }
          lex->grant_tot_col |= grant;
          if (point)
            point->rights |= grant;
          else {
            LEX_COLUMN *col = new (thd->mem_root) LEX_COLUMN(*new_str, grant);
            if (col == nullptr) return true;
            lex->columns.push_back(col);
          }
        }
      }
    }
  }  // end for
  return false;
}

bool validate_vcpu_range(const resourcegroups::Range &range) {
  auto vcpus = resourcegroups::Resource_group_mgr::instance()->num_vcpus();
  for (resourcegroups::platform::cpu_id_t cpu : {range.m_start, range.m_end}) {
    if (cpu >= vcpus) {
      my_error(ER_INVALID_VCPU_ID, MYF(0), cpu);
      return true;
    }
  }
  return false;
}

bool validate_resource_group_priority(THD *thd, int *priority,
                                      const LEX_CSTRING &name,
                                      const resourcegroups::Type &type) {
  auto mgr_ptr = resourcegroups::Resource_group_mgr::instance();
  if (mgr_ptr->thread_priority_available()) {
    int min = resourcegroups::platform::min_thread_priority_value();
    int max = resourcegroups::platform::max_thread_priority_value();

    if (type == resourcegroups::Type::USER_RESOURCE_GROUP)
      min = 0;
    else
      max = 0;

    if (*priority < min || *priority > max) {
      my_error(ER_INVALID_THREAD_PRIORITY, MYF(0), *priority,
               mgr_ptr->resource_group_type_str(type), name.str, min, max);
      return true;
    }
  } else if (*priority != 0) {
    push_warning_printf(thd, Sql_condition::SL_WARNING, ER_ATTRIBUTE_IGNORED,
                        ER_THD(thd, ER_ATTRIBUTE_IGNORED), "thread_priority",
                        "using default value");
    *priority = 0;
  }
  return false;
}

bool check_resource_group_support() {
  auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();
  if (!res_grp_mgr->resource_group_support()) {
    my_error(ER_FEATURE_UNSUPPORTED, MYF(0), "Resource Groups",
             res_grp_mgr->unsupport_reason());
    return true;
  }
  return false;
}

bool check_resource_group_name_len(
    const LEX_CSTRING &name, Sql_condition::enum_severity_level severity) {
  if (name.length <= NAME_CHAR_LEN) {
    return false;
  }
  if (severity == Sql_condition::SL_ERROR) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), name.str);
  } else {
    push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                        ER_TOO_LONG_IDENT,
                        ER_THD(current_thd, ER_TOO_LONG_IDENT), name.str);
  }
  return true;
}
