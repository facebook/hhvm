/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @brief
  This file defines all numerical Items
*/

#include "sql/item_func.h"

#include <scope_guard.h>  // Scope_guard
#include <string.h>
#include <time.h>
#include <algorithm>
#include <atomic>
#include <cfloat>  // DBL_DIG
#include <cmath>   // std::log2
#include <iosfwd>
#include <limits>  // std::numeric_limits
#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>

#include "field.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_bit.h"      // my_count_bits
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_double2ulonglong.h"
#include "my_hostname.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "my_user.h"  // parse_user
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/plugin_audit.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_password_policy.h"
#include "mysql/service_thd_wait.h"
#include "mysql/status_var.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_password_strength
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"  // mysql_bin_log
#include "sql/check_stack.h"
#include "sql/current_thd.h"                 // current_thd
#include "sql/dd/info_schema/table_stats.h"  // dd::info_schema::Table_stati...
#include "sql/dd/info_schema/tablespace_stats.h"  // dd::info_schema::Tablesp...
#include "sql/dd/object_id.h"
#include "sql/dd/properties.h"  // dd::Properties
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/index.h"  // Index::enum_index_type
#include "sql/dd_sql_view.h"     // push_view_warning_or_error
#include "sql/dd_table_share.h"  // dd_get_old_field_type
#include "sql/debug_sync.h"      // DEBUG_SYNC
#include "sql/derror.h"          // ER_THD
#include "sql/error_handler.h"   // Internal_error_handler
#include "sql/item.h"            // Item_json
#include "sql/item_cmpfunc.h"    // get_datetime_value
#include "sql/item_json_func.h"  // get_json_wrapper
#include "sql/item_strfunc.h"    // Item_func_concat_ws
#include "sql/json_dom.h"        // Json_wrapper
#include "sql/key.h"
#include "sql/log_event.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"              // log_10 stage_user_sleep
#include "sql/parse_tree_helpers.h"  // PT_item_list
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/resourcegroups/resource_group.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/resourcegroups/resource_group_mgr.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_mi.h"   // Master_info
#include "sql/rpl_msr.h"  // channel_map
#include "sql/rpl_rli.h"  // Relay_log_info
#include "sql/sp.h"       // sp_setup_routine
#include "sql/sp_head.h"  // sp_name
#include "sql/sql_array.h"
#include "sql/sql_audit.h"  // audit_global_variable
#include "sql/sql_base.h"   // Internal_error_handler_holder
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_cmd.h"
#include "sql/sql_error.h"
#include "sql/sql_exchange.h"  // sql_exchange
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_load.h"       // Sql_cmd_load_table
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_parse.h"      // check_stack_overrun
#include "sql/sql_show.h"       // append_identifier
#include "sql/sql_time.h"       // TIME_from_longlong_packed
#include "sql/strfunc.h"        // find_type
#include "sql/system_variables.h"
#include "sql/thd_raii.h"
#include "sql/val_int_compare.h"  // Integer_value
#include "template_utils.h"       // pointer_cast
#include "thr_mutex.h"

using std::max;
using std::min;

static void free_user_var(user_var_entry *entry) { entry->destroy(); }

bool check_reserved_words(const char *name) {
  if (!my_strcasecmp(system_charset_info, name, "GLOBAL") ||
      !my_strcasecmp(system_charset_info, name, "LOCAL") ||
      !my_strcasecmp(system_charset_info, name, "SESSION"))
    return true;
  return false;
}

/**
  Evaluate a constant condition, represented by an Item tree

  @param      thd   Thread handler
  @param      cond  The constant condition to evaluate
  @param[out] value Returned value, either true or false

  @returns false if evaluation is successful, true otherwise
*/

bool eval_const_cond(THD *thd, Item *cond, bool *value) {
  // Function may be used both during resolving and during optimization:
  DBUG_ASSERT(cond->may_evaluate_const(thd));
  *value = cond->val_bool();
  return thd->is_error();
}

/**
   Test if the sum of arguments overflows the ulonglong range.
*/
static inline bool test_if_sum_overflows_ull(ulonglong arg1, ulonglong arg2) {
  return ULLONG_MAX - arg1 < arg2;
}

void Item_func::set_arguments(List<Item> &list, bool context_free) {
  allowed_arg_cols = 1;
  arg_count = list.elements;
  args = tmp_arg;  // If 2 arguments
  if (arg_count <= 2 ||
      (args = (Item **)(*THR_MALLOC)->Alloc(sizeof(Item *) * arg_count))) {
    List_iterator_fast<Item> li(list);
    Item *item;
    Item **save_args = args;

    while ((item = li++)) {
      *(save_args++) = item;
      if (!context_free) add_accum_properties(item);
    }
  } else
    arg_count = 0;  // OOM
  list.empty();     // Fields are used
}

Item_func::Item_func(const POS &pos, PT_item_list *opt_list)
    : super(pos), allowed_arg_cols(1) {
  if (opt_list == nullptr) {
    args = tmp_arg;
    arg_count = 0;
  } else
    set_arguments(opt_list->value, true);
}

Item_func::Item_func(THD *thd, Item_func *item)
    : Item_result_field(thd, item),
      null_on_null(item->null_on_null),
      allowed_arg_cols(item->allowed_arg_cols),
      used_tables_cache(item->used_tables_cache),
      not_null_tables_cache(item->not_null_tables_cache),
      arg_count(item->arg_count) {
  if (arg_count) {
    if (arg_count <= 2)
      args = tmp_arg;
    else {
      if (!(args = (Item **)thd->alloc(sizeof(Item *) * arg_count))) return;
    }
    memcpy((char *)args, (char *)item->args, sizeof(Item *) * arg_count);
  }
}

bool Item_func::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  const bool no_named_params = !may_have_named_parameters();
  for (size_t i = 0; i < arg_count; i++) {
    add_accum_properties(args[i]);
    if (args[i]->itemize(pc, &args[i])) return true;
    if (no_named_params && !args[i]->item_name.is_autogenerated()) {
      my_error(functype() == FUNC_SP ? ER_WRONG_PARAMETERS_TO_STORED_FCT
                                     : ER_WRONG_PARAMETERS_TO_NATIVE_FCT,
               MYF(0), func_name());
      return true;
    }
  }
  return false;
}

/*
  Resolve references to table column for a function and its argument

  SYNOPSIS:
  fix_fields()
  thd		Thread object

  DESCRIPTION
    Call fix_fields() for all arguments to the function.  The main intention
    is to allow all Item_field() objects to setup pointers to the table fields.

    Sets as a side effect the following class variables:
      maybe_null	Set if any argument may return NULL
      used_tables_cache Set to union of the tables used by arguments

      str_value.charset If this is a string function, set this to the
                        character set for the first argument.
                        If any argument is binary, this is set to binary

   If for any item any of the defaults are wrong, then this can
   be fixed in the resolve_type() function that is called after this one or
   by writing a specialized fix_fields() for the item.

  RETURN VALUES
  false	ok
  true	Got error.  Stored with my_error().
*/

bool Item_func::fix_fields(THD *thd, Item **) {
  DBUG_ASSERT(fixed == 0 || basic_const_item());

  Item **arg, **arg_end;
  uchar buff[STACK_BUFF_ALLOC];  // Max argument in function

  /*
    Semi-join flattening should only be performed for top-level
    predicates. Disable it for predicates that live under an
    Item_func.
  */
  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  used_tables_cache = get_initial_pseudo_tables();
  not_null_tables_cache = 0;

  /*
    Use stack limit of STACK_MIN_SIZE * 2 since
    on some platforms a recursive call to fix_fields
    requires more than STACK_MIN_SIZE bytes (e.g. for
    MIPS, it takes about 22kB to make one recursive
    call to Item_func::fix_fields())
  */
  if (check_stack_overrun(thd, STACK_MIN_SIZE * 2, buff))
    return true;    // Fatal error if flag is set!
  if (arg_count) {  // Print purify happy
    for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
      if (fix_func_arg(thd, arg)) return true;
    }
  }

  if (resolve_type(thd) || thd->is_error())  // Some impls still not error-safe
    return true;
  fixed = true;
  return false;
}

bool Item_func::fix_func_arg(THD *thd, Item **arg) {
  if ((!(*arg)->fixed && (*arg)->fix_fields(thd, arg)))
    return true; /* purecov: inspected */
  Item *item = *arg;

  if (allowed_arg_cols) {
    if (item->check_cols(allowed_arg_cols)) return true;
  } else {
    /*  we have to fetch allowed_arg_cols from first argument */
    DBUG_ASSERT(arg == args);  // it is first argument
    allowed_arg_cols = item->cols();
    DBUG_ASSERT(allowed_arg_cols);  // Can't be 0 any more
  }

  maybe_null |= item->maybe_null;
  used_tables_cache |= item->used_tables();
  if (null_on_null) not_null_tables_cache |= item->not_null_tables();
  add_accum_properties(item);

  return false;
}

void Item_func::fix_after_pullout(SELECT_LEX *parent_select,
                                  SELECT_LEX *removed_select) {
  if (const_item()) {
    /*
      Pulling out a const item changes nothing to it. Moreover, some items may
      have decided that they're const by some other logic than the generic
      one below, and we must preserve that decision.
    */
    return;
  }

  Item **arg, **arg_end;

  used_tables_cache = get_initial_pseudo_tables();
  not_null_tables_cache = 0;

  if (arg_count) {
    for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
      Item *const item = *arg;
      item->fix_after_pullout(parent_select, removed_select);
      used_tables_cache |= item->used_tables();
      if (null_on_null) not_null_tables_cache |= item->not_null_tables();
    }
  }
}

bool Item_func::walk(Item_processor processor, enum_walk walk,
                     uchar *argument) {
  if ((walk & enum_walk::PREFIX) && (this->*processor)(argument)) return true;

  Item **arg, **arg_end;
  for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
    if ((*arg)->walk(processor, walk, argument)) return true;
  }
  return (walk & enum_walk::POSTFIX) && (this->*processor)(argument);
}

void Item_func::traverse_cond(Cond_traverser traverser, void *argument,
                              traverse_order order) {
  if (arg_count) {
    Item **arg, **arg_end;

    switch (order) {
      case (PREFIX):
        (*traverser)(this, argument);
        for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
          (*arg)->traverse_cond(traverser, argument, order);
        }
        break;
      case (POSTFIX):
        for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
          (*arg)->traverse_cond(traverser, argument, order);
        }
        (*traverser)(this, argument);
    }
  } else
    (*traverser)(this, argument);
}

/**
  Transform an Item_func object with a transformer callback function.

    The function recursively applies the transform method to each
    argument of the Item_func node.
    If the call of the method for an argument item returns a new item
    the old item is substituted for a new one.
    After this the transformer is applied to the root node
    of the Item_func object.
*/

Item *Item_func::transform(Item_transformer transformer, uchar *argument) {
  if (arg_count) {
    Item **arg, **arg_end;
    for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
      Item *new_item = (*arg)->transform(transformer, argument);
      if (new_item == nullptr) return nullptr; /* purecov: inspected */

      /*
        THD::change_item_tree() should be called only if the tree was
        really transformed, i.e. when a new item has been created.
        Otherwise we'll be allocating a lot of unnecessary memory for
        change records at each execution.
      */
      if (*arg != new_item) current_thd->change_item_tree(arg, new_item);
    }
  }
  return (this->*transformer)(argument);
}

/**
  Compile Item_func object with a processor and a transformer
  callback functions.

    First the function applies the analyzer to the root node of
    the Item_func object. Then if the analyzer succeeeds (returns true)
    the function recursively applies the compile method to each argument
    of the Item_func node.
    If the call of the method for an argument item returns a new item
    the old item is substituted for a new one.
    After this the transformer is applied to the root node
    of the Item_func object.
*/

Item *Item_func::compile(Item_analyzer analyzer, uchar **arg_p,
                         Item_transformer transformer, uchar *arg_t) {
  if (!(this->*analyzer)(arg_p)) return this;
  if (arg_count) {
    Item **arg, **arg_end;
    for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
      /*
        The same parameter value of arg_p must be passed
        to analyze any argument of the condition formula.
      */
      uchar *arg_v = *arg_p;
      Item *new_item = (*arg)->compile(analyzer, &arg_v, transformer, arg_t);
      if (new_item == nullptr) return nullptr;
      if (*arg != new_item) current_thd->change_item_tree(arg, new_item);
    }
  }
  return (this->*transformer)(arg_t);
}

/**
  See comments in Item_cmp_func::split_sum_func()
*/

void Item_func::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                               List<Item> &fields) {
  Item **arg, **arg_end;
  for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++)
    (*arg)->split_sum_func2(thd, ref_item_array, fields, arg, true);
}

void Item_func::update_used_tables() {
  used_tables_cache = get_initial_pseudo_tables();
  not_null_tables_cache = 0;

  /*
    Rollup property not always derivable from arguments, so don't reset that,
    cf. "GROUP BY (a+b) WITH ROLLUP": the a and the b are never marked, cf. the
    logic in `resolve_rollup_item', `resolve_rollup_wfs' and
    `change_func_or_wf_group_ref', so "a+b" being a rollup expression can't be
    derived from a or b.
  */
  m_accum_properties &= PROP_ROLLUP_EXPR;

  for (uint i = 0; i < arg_count; i++) {
    args[i]->update_used_tables();
    used_tables_cache |= args[i]->used_tables();
    if (null_on_null) not_null_tables_cache |= args[i]->not_null_tables();
    add_accum_properties(args[i]);
  }
}

void Item_func::print(const THD *thd, String *str,
                      enum_query_type query_type) const {
  str->append(func_name());
  str->append('(');
  print_args(thd, str, 0, query_type);
  str->append(')');
}

void Item_func::print_args(const THD *thd, String *str, uint from,
                           enum_query_type query_type) const {
  for (uint i = from; i < arg_count; i++) {
    if (i != from) str->append(',');
    args[i]->print(thd, str, query_type);
  }
}

void Item_func::print_op(const THD *thd, String *str,
                         enum_query_type query_type) const {
  str->append('(');
  for (uint i = 0; i < arg_count - 1; i++) {
    args[i]->print(thd, str, query_type);
    str->append(' ');
    str->append(func_name());
    str->append(' ');
  }
  args[arg_count - 1]->print(thd, str, query_type);
  str->append(')');
}

/// @note Please keep in sync with Item_sum::eq().
bool Item_func::eq(const Item *item, bool binary_cmp) const {
  /* Assume we don't have rtti */
  if (this == item) return true;
  if (item->type() != FUNC_ITEM) return false;
  const Item_func *item_func = down_cast<const Item_func *>(item);
  Item_func::Functype func_type;
  if ((func_type = functype()) != item_func->functype() ||
      arg_count != item_func->arg_count ||
      (func_type != Item_func::FUNC_SP &&
       strcmp(func_name(), item_func->func_name()) != 0) ||
      (func_type == Item_func::FUNC_SP &&
       my_strcasecmp(system_charset_info, func_name(), item_func->func_name())))
    return false;
  for (uint i = 0; i < arg_count; i++)
    if (!args[i]->eq(item_func->args[i], binary_cmp)) return false;
  return true;
}

Field *Item_func::tmp_table_field(TABLE *table) {
  Field *field = nullptr;

  switch (result_type()) {
    case INT_RESULT:
      if (max_length > MY_INT32_NUM_DECIMAL_DIGITS)
        field = new (*THR_MALLOC) Field_longlong(
            max_length, maybe_null, item_name.ptr(), unsigned_flag);
      else
        field = new (*THR_MALLOC)
            Field_long(max_length, maybe_null, item_name.ptr(), unsigned_flag);
      break;
    case REAL_RESULT:
      if (this->data_type() == MYSQL_TYPE_FLOAT) {
        field = new (*THR_MALLOC)
            Field_float(max_char_length(), maybe_null, item_name.ptr(),
                        decimals, unsigned_flag);
      } else {
        field = new (*THR_MALLOC)
            Field_double(max_char_length(), maybe_null, item_name.ptr(),
                         decimals, unsigned_flag);
      }
      break;
    case STRING_RESULT:
      return make_string_field(table);
      break;
    case DECIMAL_RESULT:
      field = Field_new_decimal::create_from_item(this);
      break;
    case ROW_RESULT:
    default:
      // This case should never be chosen
      DBUG_ASSERT(0);
      field = nullptr;
      break;
  }
  if (field) field->init(table);
  return field;
}

my_decimal *Item_func::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed);
  longlong nr = val_int();
  if (null_value) return nullptr; /* purecov: inspected */
  int2my_decimal(E_DEC_FATAL_ERROR, nr, unsigned_flag, decimal_value);
  return decimal_value;
}

String *Item_real_func::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  double nr = val_real();
  if (null_value) return nullptr; /* purecov: inspected */
  str->set_real(nr, decimals, collation.collation);
  return str;
}

my_decimal *Item_real_func::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed);
  double nr = val_real();
  if (null_value) return nullptr; /* purecov: inspected */
  double2my_decimal(E_DEC_FATAL_ERROR, nr, decimal_value);
  return decimal_value;
}

void Item_func::fix_num_length_and_dec() {
  uint fl_length = 0;
  decimals = 0;
  for (uint i = 0; i < arg_count; i++) {
    decimals = max(decimals, args[i]->decimals);
    fl_length = max(fl_length, args[i]->max_length);
  }
  max_length = float_length(decimals);
  if (fl_length > max_length) {
    decimals = DECIMAL_NOT_SPECIFIED;
    max_length = float_length(DECIMAL_NOT_SPECIFIED);
  }
}

void Item_func_numhybrid::fix_num_length_and_dec() {}

void Item_func::signal_divide_by_null() {
  THD *thd = current_thd;
  if (thd->variables.sql_mode & MODE_ERROR_FOR_DIVISION_BY_ZERO)
    push_warning(thd, Sql_condition::SL_WARNING, ER_DIVISION_BY_ZERO,
                 ER_THD(thd, ER_DIVISION_BY_ZERO));
  null_value = true;
}

void Item_func::signal_invalid_argument_for_log() {
  THD *thd = current_thd;
  push_warning(thd, Sql_condition::SL_WARNING,
               ER_INVALID_ARGUMENT_FOR_LOGARITHM,
               ER_THD(thd, ER_INVALID_ARGUMENT_FOR_LOGARITHM));
  null_value = true;
}

Item *Item_func::get_tmp_table_item(THD *thd) {
  DBUG_TRACE;

  /*
    For items with aggregate functions, return the copy
    of the function.
    For constant items, return the same object as fields
    are not created in temp tables for them.
    For items with windowing functions, return the same
    object (temp table fields are not created for windowing
    functions if they are not evaluated at this stage).
    For items which need to store ROLLUP NULLs, we need
    the same object as we need to detect if ROLLUP NULL's
    need to be written for this item (in has_rollup_result).
  */
  if (!has_aggregation() && !const_item() && !has_wf() && !has_rollup_expr()) {
    Item *result = new Item_field(result_field);
    return result;
  }
  Item *result = copy_or_same(thd);
  return result;
}

const Item_field *Item_func::contributes_to_filter(
    table_map read_tables, table_map filter_for_table,
    const MY_BITMAP *fields_to_ignore) const {
  DBUG_ASSERT((read_tables & filter_for_table) == 0);
  /*
    Multiple equality (Item_equal) should not call this function
    because it would reject valid comparisons.
  */
  DBUG_ASSERT(functype() != MULT_EQUAL_FUNC);

  /*
    To contribute to filering effect, the condition must refer to
    exactly one unread table: the table filtering is currently
    calculated for.
  */
  if ((used_tables() & ~read_tables) != filter_for_table) return nullptr;

  /*
    Whether or not this Item_func has an operand that is a field in
    'filter_for_table' that is not in 'fields_to_ignore'.
  */
  Item_field *usable_field = nullptr;

  /*
    Whether or not this Item_func has an operand that can be used as
    available value. arg_count==1 for Items with implicit values like
    "field IS NULL".
  */
  bool found_comparable = (arg_count == 1);

  for (uint i = 0; i < arg_count; i++) {
    const Item::Type arg_type = args[i]->real_item()->type();

    if (arg_type == Item::SUBSELECT_ITEM) {
      if (args[i]->const_for_execution()) {
        // Constant subquery, i.e., not a dependent subquery.
        found_comparable = true;
        continue;
      }

      /*
        This is either "fld OP <dependent_subquery>" or "fld BETWEEN X
        and Y" where either X or Y is a dependent subquery. Filtering
        effect should not be calculated for this item because the cost
        of evaluating the dependent subquery is currently not
        calculated and its accompanying filtering effect is too
        uncertain. See WL#7384.
      */
      return nullptr;
    }  // ... if subquery.

    const table_map used_tabs = args[i]->used_tables();

    if (arg_type == Item::FIELD_ITEM && (used_tabs == filter_for_table)) {
      /*
        The qualifying table of args[i] is filter_for_table. args[i]
        may be a field or a reference to a field, e.g. through a
        view.
      */
      Item_field *fld = static_cast<Item_field *>(args[i]->real_item());

      /*
        Use args[i] as value if
        1) this field shall be ignored, or
        2) a usable field has already been found (meaning that
        this is "filter_for_table.colX OP filter_for_table.colY").
      */
      if (bitmap_is_set(fields_to_ignore, fld->field->field_index) ||  // 1)
          usable_field)                                                // 2)
      {
        found_comparable = true;
        continue;
      }

      /*
        This field shall contribute to filtering effect if a
        value is found for it
      */
      usable_field = fld;
    }  // if field.
    else {
      /*
        It's not a subquery. May be a function, a constant, an outer
        reference, a field of another table...

        Already checked that this predicate does not refer to tables
        later in the join sequence. Verify it:
      */
      DBUG_ASSERT(!(used_tabs & (~read_tables & ~filter_for_table)));
      found_comparable = true;
    }
  }
  return (found_comparable ? usable_field : nullptr);
}

/**
  Return new Item_field if given expression matches GC

  @see substitute_gc()

  @param func           Expression to be replaced
  @param fld            GCs field
  @param type           Result type to match with Field
  @param[out] found     If given, just return found field, without Item_field

  @returns
    item new Item_field for matched GC
    NULL otherwise
*/

Item_field *get_gc_for_expr(Item_func **func, Field *fld, Item_result type,
                            Field **found) {
  Item_func *expr = down_cast<Item_func *>(fld->gcol_info->expr_item);

  /*
    In the case where the generated column expression returns JSON and
    the predicate compares the values as strings, it is not safe to
    replace the expression with the generated column, since the
    indexed string values will be double-quoted. The generated column
    expression should use the JSON_UNQUOTE function to strip off the
    double-quotes in order to get a usable index for looking up
    strings. See also the comment below.
  */
  if (type == STRING_RESULT && expr->data_type() == MYSQL_TYPE_JSON)
    return nullptr;

  /*
    In order to match expressions against a functional index's expression,
    it's needed to skip CAST(.. AS .. ) and potentially COLLATE from the latter.
    This can't be joined with striping json_unquote below, since we might need
    to skip it too in expression like:
      CAST(JSON_UNQUOTE(<expr>) AS CHAR(X))
  */

  if (expr->functype() == Item_func::COLLATE_FUNC &&
      (*func)->functype() != Item_func::COLLATE_FUNC) {
    if (!expr->arguments()[0]->can_be_substituted_for_gc()) return nullptr;
    expr = down_cast<Item_func *>(expr->arguments()[0]);
  }

  if (expr->functype() == Item_func::TYPECAST_FUNC &&
      (*func)->functype() != Item_func::TYPECAST_FUNC) {
    if (!expr->arguments()[0]->can_be_substituted_for_gc()) return nullptr;
    expr = down_cast<Item_func *>(expr->arguments()[0]);
  }

  /*
    Skip unquoting function. This is needed to address JSON string
    comparison issue. All JSON_* functions return quoted strings. In
    order to create usable index, GC column expression has to include
    JSON_UNQUOTE function, e.g JSON_UNQUOTE(JSON_EXTRACT(..)).
    Hence, the unquoting function in column expression have to be
    skipped in order to correctly match GC expr to expr in
    WHERE condition.  The exception is if user has explicitly used
    JSON_UNQUOTE in WHERE condition.
  */
  if (!strcmp(expr->func_name(), "json_unquote") &&
      strcmp((*func)->func_name(), "json_unquote")) {
    if (!expr->arguments()[0]->can_be_substituted_for_gc()) return nullptr;
    expr = down_cast<Item_func *>(expr->arguments()[0]);
  }

  DBUG_ASSERT(expr->can_be_substituted_for_gc());

  // JSON implementation always uses binary collation
  bool bin_cmp = (expr->data_type() == MYSQL_TYPE_JSON);
  if (type == fld->result_type() && (*func)->eq(expr, bin_cmp)) {
    if (found) {
      // Temporary mark the field in order to check correct value conversion
      fld->table->mark_column_used(fld, MARK_COLUMNS_TEMP);
      *found = fld;
      return nullptr;
    }
    // Mark field for read
    fld->table->mark_column_used(fld, MARK_COLUMNS_READ);
    Item_field *field = new Item_field(fld);
    return field;
  }
  if (found) *found = nullptr;
  return nullptr;
}

/**
  Attempt to substitute an expression with an equivalent generated
  column in a predicate.

  @param expr      the expression that should be substituted
  @param value     if given, value will be coerced to GC field's type and
                   the result will substitute the original value. Used by
                   multi-valued index.
  @param gc_fields list of indexed generated columns to check for
                   equivalence with the expression
  @param type      the acceptable type of the generated column that
                   replaces the expression
  @param predicate the predicate in which the substitution is done

  @return true on error, false on success
*/
static bool substitute_gc_expression(Item_func **expr, Item **value,
                                     List<Field> *gc_fields, Item_result type,
                                     Item_func *predicate) {
  List_iterator<Field> li(*gc_fields);
  Item_field *item_field = nullptr;
  while (Field *field = li++) {
    // Check whether the field has usable keys.
    Key_map tkm = field->part_of_key;
    tkm.merge(field->part_of_prefixkey);  // Include prefix keys.
    tkm.intersect(field->table->keys_in_use_for_query);
    /*
      Don't substitute if:
      1) Key is disabled
      2) It's a multi-valued index's field and predicate isn't MEMBER OF
    */
    if (tkm.is_clear_all() ||                           // (1)
        (field->is_array() && predicate->functype() !=  // (2)
                                  Item_func::MEMBER_OF_FUNC))
      continue;
    // If the field is a hidden field used by a functional index, we require
    // that the collation of the field must match the collation of the
    // expression. If not, we might end up with the wrong result when using
    // the index (see bug#27337092). Ideally, this should be done for normal
    // generated columns as well, but that is delayed to a later fix since the
    // impact might be quite large.
    if (!(field->is_field_for_functional_index() &&
          field->match_collation_to_optimize_range() &&
          (*expr)->collation.collation != field->charset())) {
      item_field = get_gc_for_expr(expr, field, type);
      if (item_field != nullptr) break;
    }
  }

  if (item_field == nullptr) return false;

  // A matching expression is found. Substitute the expression with
  // the matching generated column.
  THD *thd = item_field->field->table->in_use;
  if (item_field->returns_array() && value) {
    Json_wrapper wr;
    String str_val, buf;
    Field_typed_array *afld = down_cast<Field_typed_array *>(item_field->field);

    Functional_index_error_handler functional_index_error_handler(afld, thd);

    if (get_json_atom_wrapper(value, 0, "MEMBER OF", &str_val, &buf, &wr,
                              nullptr, true))
      return true;

    auto to_wr = make_unique_destroy_only<Json_wrapper>(thd->mem_root);
    if (to_wr == nullptr) return true;

    // Don't substitute if value can't be coerced to field's type
    if (afld->coerce_json_value(&wr, /*no_error=*/true, to_wr.get()))
      return false;

    Item_json *jsn =
        new (thd->mem_root) Item_json(std::move(to_wr), predicate->item_name);
    if (jsn == nullptr || jsn->fix_fields(thd, nullptr)) return true;
    thd->change_item_tree(value, jsn);
  }
  thd->change_item_tree(pointer_cast<Item **>(expr), item_field);

  // Adjust the predicate.
  if (predicate->functype() == Item_func::IN_FUNC)
    down_cast<Item_func_in *>(predicate)->cleanup_arrays();
  return predicate->resolve_type(thd);
}

/**
  A helper function for Item_func::gc_subst_transformer, that tries to
  substitute the given JSON_CONTAINS or JSON_OVERLAPS function for one of GCs
  from the provided list. The function checks whether there's an index with
  matching expression and whether all scalars for lookup can be coerced to
  index's GC field without errors. If so, index's GC field substitutes the
  given function, args are replaced for array of coerced values in order to
  match GC's type. substitute_gc_expression() can't be used to these functions
  as it's tailored to handle regular single-valued indexes and doesn't ensure
  correct coercion of all values to lookup in multi-valued index.

  @param func     Function to replace
  @param vals     Args to replace
  @param vals_wr  Json_wrapper containing array of values for index lookup
  @param gc_fields List of generated fields to look the function's substitute in
*/

static void gc_subst_overlaps_contains(Item_func **func, Item **vals,
                                       Json_wrapper &vals_wr,
                                       List<Field> *gc_fields) {
  // Field to substitute function for. NULL when no matching index was found.
  Field *found = nullptr;
  DBUG_ASSERT(vals_wr.type() != enum_json_type::J_OBJECT &&
              vals_wr.type() != enum_json_type::J_ERROR);
  THD *thd = current_thd;
  // Vector of coerced keys
  Json_array_ptr coerced_keys = create_dom_ptr<Json_array>();

  // Find a field that matches the expression
  for (Field &fld : *gc_fields) {
    bool can_use_index = true;
    // Check whether field has usable keys
    Key_map tkm = fld.part_of_key;
    tkm.intersect(fld.table->keys_in_use_for_query);

    if (tkm.is_clear_all() || !fld.is_array()) continue;
    Functional_index_error_handler func_idx_err_hndl(&fld, thd);
    found = nullptr;

    get_gc_for_expr(func, &fld, fld.result_type(), &found);
    if (!found || !found->is_array()) continue;
    Field_typed_array *afld = down_cast<Field_typed_array *>(found);
    // Check that array's values can be coerced to found field's type
    uint len;
    if (vals_wr.type() == enum_json_type::J_ARRAY)
      len = vals_wr.length();
    else
      len = 1;
    coerced_keys->clear();
    for (uint i = 0; i < len; i++) {
      Json_wrapper elt = vals_wr[i];
      Json_wrapper res;
      if (afld->coerce_json_value(&elt, true, &res)) {
        can_use_index = false;
        found = nullptr;
        break;
      }
      coerced_keys->append_clone(res.to_dom(thd));
    }
    if (can_use_index) break;
  }
  if (!found) return;
  TABLE *table = found->table;
  Item_field *subs_item = new Item_field(found);
  if (!subs_item) return;
  auto res = make_unique_destroy_only<Json_wrapper>(thd->mem_root,
                                                    coerced_keys.release());
  if (res == nullptr) return;
  Item_json *array_arg =
      new (thd->mem_root) Item_json(std::move(res), (*func)->item_name);
  if (!array_arg || array_arg->fix_fields(thd, nullptr)) return;
  table->mark_column_used(found, MARK_COLUMNS_READ);
  table->in_use->change_item_tree(pointer_cast<Item **>(func), subs_item);
  table->in_use->change_item_tree(vals, array_arg);
}

/**
  Transformer function for GC substitution.

  @param arg  List of indexed GC field

  @return this item

  @details This function transforms the WHERE condition. It doesn't change
  'this' item but rather changes its arguments. It takes list of GC fields
  and checks whether arguments of 'this' item matches them and index over
  the GC field isn't disabled with hints. If so, it replaces
  the argument with newly created Item_field which uses the matched GC
  field. Following functions' arguments could be transformed:
  - EQ_FUNC, LT_FUNC, LE_FUNC, GE_FUNC, GT_FUNC
    - Left _or_ right argument if the opposite argument is a constant.
  - IN_FUNC, BETWEEN
    - Left argument if all other arguments are constant and of the same type.

  After transformation comparators are updated to take into account the new
  field.
*/

Item *Item_func::gc_subst_transformer(uchar *arg) {
  List<Field> *gc_fields = pointer_cast<List<Field> *>(arg);

  switch (functype()) {
    case EQ_FUNC:
    case LT_FUNC:
    case LE_FUNC:
    case GE_FUNC:
    case GT_FUNC: {
      Item_func **func = nullptr;
      Item *val = nullptr;

      // Check if we can substitute a function with a GC. The
      // predicate must be on the form <expr> OP <constant> or
      // <constant> OP <expr>.
      if (args[0]->can_be_substituted_for_gc() && args[1]->const_item()) {
        func = pointer_cast<Item_func **>(args);
        val = args[1];
      } else if (args[1]->can_be_substituted_for_gc() &&
                 args[0]->const_item()) {
        func = pointer_cast<Item_func **>(args + 1);
        val = args[0];
      } else {
        break;
      }

      if (substitute_gc_expression(func, nullptr, gc_fields, val->result_type(),
                                   this))
        return nullptr; /* purecov: inspected */
      break;
    }
    case BETWEEN:
    case IN_FUNC: {
      if (!args[0]->can_be_substituted_for_gc()) break;

      // Can only substitute if all the operands on the right-hand
      // side are constants of the same type.
      Item_result type = args[1]->result_type();
      if (!std::all_of(args + 1, args + arg_count,
                       [type](const Item *item_arg) {
                         return item_arg->const_item() &&
                                item_arg->result_type() == type;
                       })) {
        break;
      }
      Item **to_subst = args;
      if (substitute_gc_expression(pointer_cast<Item_func **>(to_subst),
                                   nullptr, gc_fields, type, this))
        return nullptr;
      break;
    }
    case MEMBER_OF_FUNC: {
      Item_result type = args[0]->result_type();
      /*
        Check whether MEMBER OF is applicable for optimization:
        1) 1st arg is a constant
        2) .. and it isn't NULL, as MEMBER OF can't be used to lookup NULLs
        3) 2nd arg can be substituted for a GC
        4) .. and it's of JSON type
      */
      if (args[0]->const_item() &&                    // 1
          !args[0]->is_null() &&                      // 2
          args[1]->can_be_substituted_for_gc() &&     // 3
          args[1]->data_type() == MYSQL_TYPE_JSON) {  // 4
        Item **to_subst = args + 1;
        if (substitute_gc_expression(pointer_cast<Item_func **>(to_subst), args,
                                     gc_fields, type, this))
          return nullptr;
      }
      break;
    }
    case JSON_CONTAINS: {
      Json_wrapper vals_wr;
      String str;
      /*
        Check whether JSON_CONTAINS is applicable for optimization:
        1) 1st arg is a local field
        2) 1st arg's type is JSON
        3) value to lookup is a constant
        4) value to lookup is a proper JSON doc
        5) value to lookup is an array or scalar
      */
      if (args[0]->type() != Item::FUNC_ITEM ||       // 1
          args[0]->data_type() != MYSQL_TYPE_JSON ||  // 2
          !args[1]->real_item()->const_item())        // 3
        break;
      if (get_json_wrapper(args, 1, &str, func_name(), &vals_wr) ||  // 4
          args[1]->null_value ||
          vals_wr.type() == enum_json_type::J_OBJECT)  // 5
        break;
      gc_subst_overlaps_contains(pointer_cast<Item_func **>(args), args + 1,
                                 vals_wr, gc_fields);
      break;
    }
    case JSON_OVERLAPS: {
      Item **func = nullptr;
      int vals = -1;
      Json_wrapper vals_wr;
      String str;

      /*
        Check whether JSON_OVERLAPS is applicable for optimization:
        1) One arg is a function and another is a const expr
        2) value to lookup is a proper JSON doc
        3) value to lookup is an array or scalar
      */

      if (args[0]->type() == Item::FUNC_ITEM && args[1]->const_item()) {  // 1
        func = args;
        vals = 1;
      } else if (args[1]->type() == Item::FUNC_ITEM &&
                 args[0]->const_item()) {  // 1
        func = args + 1;
        vals = 0;
      }
      if (!func) break;
      if (get_json_wrapper(args, vals, &str, func_name(), &vals_wr) ||  // 2
          args[vals]->null_value ||
          vals_wr.type() == enum_json_type::J_OBJECT)  // 3
        break;
      gc_subst_overlaps_contains(pointer_cast<Item_func **>(func), args + vals,
                                 vals_wr, gc_fields);
      break;
    }
    default:
      break;
  }
  return this;
}

void Item_func::replace_argument(THD *thd, Item **oldpp, Item *newp) {
  thd->change_item_tree(oldpp, newp);
}

double Item_int_func::val_real() {
  DBUG_ASSERT(fixed == 1);

  return unsigned_flag ? (double)((ulonglong)val_int()) : (double)val_int();
}

String *Item_int_func::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  longlong nr = val_int();
  if (null_value) return nullptr;
  str->set_int(nr, unsigned_flag, collation.collation);
  return str;
}

bool Item_func_connection_id::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

bool Item_func_connection_id::resolve_type(THD *thd) {
  if (Item_int_func::resolve_type(thd)) return true;
  unsigned_flag = true;
  return false;
}

bool Item_func_connection_id::fix_fields(THD *thd, Item **ref) {
  if (Item_int_func::fix_fields(thd, ref)) return true;
  thd->thread_specific_used = true;
  value = thd->variables.pseudo_thread_id;
  return false;
}

/**
  Check arguments to determine the data type for a numeric
  function of two arguments.
*/

void Item_num_op::set_numeric_type(void) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("name %s", func_name()));
  DBUG_ASSERT(arg_count == 2);
  Item_result r0 = args[0]->numeric_context_result_type();
  Item_result r1 = args[1]->numeric_context_result_type();

  DBUG_ASSERT(r0 != STRING_RESULT && r1 != STRING_RESULT);

  if (r0 == REAL_RESULT || r1 == REAL_RESULT) {
    /*
      Since DATE/TIME/DATETIME data types return INT_RESULT/DECIMAL_RESULT
      type codes, we should never get to here when both fields are temporal.
    */
    DBUG_ASSERT(!args[0]->is_temporal() || !args[1]->is_temporal());
    set_data_type(MYSQL_TYPE_DOUBLE);
    hybrid_type = REAL_RESULT;
    aggregate_float_properties(args, arg_count);
    max_length = float_length(decimals);
  } else if (r0 == DECIMAL_RESULT || r1 == DECIMAL_RESULT) {
    set_data_type(MYSQL_TYPE_NEWDECIMAL);
    hybrid_type = DECIMAL_RESULT;
    result_precision();
  } else {
    DBUG_ASSERT(r0 == INT_RESULT && r1 == INT_RESULT);
    set_data_type(MYSQL_TYPE_LONGLONG);
    decimals = 0;
    hybrid_type = INT_RESULT;
    result_precision();
  }
  DBUG_PRINT("info", ("Type: %s", (hybrid_type == REAL_RESULT
                                       ? "REAL_RESULT"
                                       : hybrid_type == DECIMAL_RESULT
                                             ? "DECIMAL_RESULT"
                                             : hybrid_type == INT_RESULT
                                                   ? "INT_RESULT"
                                                   : "--ILLEGAL!!!--")));
}

/**
  Set data type for a numeric function with one argument
  (can be also used by a numeric function with many arguments, if the result
  type depends only on the first argument)
*/

void Item_func_num1::set_numeric_type() {
  DBUG_TRACE;
  DBUG_PRINT("info", ("name %s", func_name()));
  switch (hybrid_type = args[0]->result_type()) {
    case INT_RESULT:
      set_data_type(MYSQL_TYPE_LONGLONG);
      unsigned_flag = args[0]->unsigned_flag;
      break;
    case STRING_RESULT:
    case REAL_RESULT:
      set_data_type(MYSQL_TYPE_DOUBLE);
      hybrid_type = REAL_RESULT;
      max_length = float_length(decimals);
      break;
    case DECIMAL_RESULT:
      set_data_type(MYSQL_TYPE_NEWDECIMAL);
      unsigned_flag = args[0]->unsigned_flag;
      break;
    default:
      DBUG_ASSERT(0);
  }
  DBUG_PRINT("info", ("Type: %s", (hybrid_type == REAL_RESULT
                                       ? "REAL_RESULT"
                                       : hybrid_type == DECIMAL_RESULT
                                             ? "DECIMAL_RESULT"
                                             : hybrid_type == INT_RESULT
                                                   ? "INT_RESULT"
                                                   : "--ILLEGAL!!!--")));
}

void Item_func_num1::fix_num_length_and_dec() {
  decimals = args[0]->decimals;
  max_length = args[0]->max_length;
}

/*
  Reject geometry arguments, should be called in resolve_type() for
  SQL functions/operators where geometries are not suitable as operands.
 */
bool reject_geometry_args(uint arg_count, Item **args, Item_result_field *me) {
  /*
    We want to make sure the operands are not GEOMETRY strings because
    it's meaningless for them to participate in arithmetic and/or numerical
    calculations.

    When a variable holds a MySQL Geometry byte string, it is regarded as a
    string rather than a MYSQL_TYPE_GEOMETRY, so here we can't catch an illegal
    variable argument which was assigned with a geometry.

    Item::data_type() requires the item not be of ROW_RESULT, since a row
    isn't a field.
  */
  for (uint i = 0; i < arg_count; i++) {
    if (args[i]->result_type() != ROW_RESULT &&
        args[i]->data_type() == MYSQL_TYPE_GEOMETRY) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), me->func_name());
      return true;
    }
  }

  return false;
}

/**
  Go through the arguments of a function and check if any of them are
  JSON. If a JSON argument is found, raise a warning saying that this
  operation is not supported yet. This function is used to notify
  users that they are comparing JSON values using a mechanism that has
  not yet been updated to use the JSON comparator. JSON values are
  typically handled as strings in that case.

  @param arg_count  the number of arguments
  @param args       the arguments to go through looking for JSON values
  @param msg        the message that explains what is not supported
*/
void unsupported_json_comparison(size_t arg_count, Item **args,
                                 const char *msg) {
  for (size_t i = 0; i < arg_count; ++i) {
    if (args[i]->result_type() == STRING_RESULT &&
        args[i]->data_type() == MYSQL_TYPE_JSON) {
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_NOT_SUPPORTED_YET,
                          ER_THD(current_thd, ER_NOT_SUPPORTED_YET), msg);
      break;
    }
  }
}

bool Item_func_numhybrid::resolve_type(THD *) {
  fix_num_length_and_dec();
  set_numeric_type();
  return reject_geometry_args(arg_count, args, this);
}

String *Item_func_numhybrid::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  switch (hybrid_type) {
    case DECIMAL_RESULT: {
      my_decimal decimal_value, *val;
      if (!(val = decimal_op(&decimal_value))) return nullptr;  // null is set
      my_decimal_round(E_DEC_FATAL_ERROR, val, decimals, false, val);
      str->set_charset(collation.collation);
      my_decimal2string(E_DEC_FATAL_ERROR, val, str);
      break;
    }
    case INT_RESULT: {
      longlong nr = int_op();
      if (null_value) return nullptr; /* purecov: inspected */
      str->set_int(nr, unsigned_flag, collation.collation);
      break;
    }
    case REAL_RESULT: {
      double nr = real_op();
      if (null_value) return nullptr; /* purecov: inspected */
      str->set_real(nr, decimals, collation.collation);
      break;
    }
    case STRING_RESULT:
      switch (data_type()) {
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          return val_string_from_datetime(str);
        case MYSQL_TYPE_DATE:
          return val_string_from_date(str);
        case MYSQL_TYPE_TIME:
          return val_string_from_time(str);
        default:
          break;
      }
      return str_op(&str_value);
    default:
      DBUG_ASSERT(0);
  }
  return str;
}

double Item_func_numhybrid::val_real() {
  DBUG_ASSERT(fixed == 1);
  switch (hybrid_type) {
    case DECIMAL_RESULT: {
      my_decimal decimal_value, *val;
      double result;
      if (!(val = decimal_op(&decimal_value))) return 0.0;  // null is set
      my_decimal2double(E_DEC_FATAL_ERROR, val, &result);
      return result;
    }
    case INT_RESULT: {
      longlong result = int_op();
      return unsigned_flag ? (double)((ulonglong)result) : (double)result;
    }
    case REAL_RESULT:
      return real_op();
    case STRING_RESULT: {
      switch (data_type()) {
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          return val_real_from_decimal();
        default:
          break;
      }
      const char *end_not_used;
      int err_not_used;
      String *res = str_op(&str_value);
      return (res ? my_strntod(res->charset(), res->ptr(), res->length(),
                               &end_not_used, &err_not_used)
                  : 0.0);
    }
    default:
      DBUG_ASSERT(0);
  }
  return 0.0;
}

longlong Item_func_numhybrid::val_int() {
  DBUG_ASSERT(fixed == 1);
  switch (hybrid_type) {
    case DECIMAL_RESULT: {
      my_decimal decimal_value, *val;
      if (!(val = decimal_op(&decimal_value))) return 0;  // null is set
      longlong result;
      my_decimal2int(E_DEC_FATAL_ERROR, val, unsigned_flag, &result);
      return result;
    }
    case INT_RESULT:
      return int_op();
    case REAL_RESULT: {
      return llrint_with_overflow_check(real_op());
    }
    case STRING_RESULT: {
      switch (data_type()) {
        case MYSQL_TYPE_DATE:
          return val_int_from_date();
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          return val_int_from_datetime();
        case MYSQL_TYPE_TIME:
          return val_int_from_time();
        default:
          break;
      }
      int err_not_used;
      String *res;
      if (!(res = str_op(&str_value))) return 0;

      const char *end = res->ptr() + res->length();
      const CHARSET_INFO *cs = res->charset();
      return (*(cs->cset->strtoll10))(cs, res->ptr(), &end, &err_not_used);
    }
    default:
      DBUG_ASSERT(0);
  }
  return 0;
}

my_decimal *Item_func_numhybrid::val_decimal(my_decimal *decimal_value) {
  my_decimal *val = decimal_value;
  DBUG_ASSERT(fixed == 1);
  switch (hybrid_type) {
    case DECIMAL_RESULT:
      val = decimal_op(decimal_value);
      break;
    case INT_RESULT: {
      longlong result = int_op();
      int2my_decimal(E_DEC_FATAL_ERROR, result, unsigned_flag, decimal_value);
      break;
    }
    case REAL_RESULT: {
      double result = real_op();
      double2my_decimal(E_DEC_FATAL_ERROR, result, decimal_value);
      break;
    }
    case STRING_RESULT: {
      switch (data_type()) {
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          return val_decimal_from_date(decimal_value);
        case MYSQL_TYPE_TIME:
          return val_decimal_from_time(decimal_value);
        default:
          break;
      }
      String *res;
      if (!(res = str_op(&str_value))) return nullptr;

      str2my_decimal(E_DEC_FATAL_ERROR, res->ptr(), res->length(),
                     res->charset(), decimal_value);
      break;
    }
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  }
  return val;
}

bool Item_func_numhybrid::get_date(MYSQL_TIME *ltime,
                                   my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed == 1);
  switch (data_type()) {
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      return date_op(ltime, fuzzydate);
    case MYSQL_TYPE_TIME:
      return get_date_from_time(ltime);
    default:
      return Item::get_date_from_non_temporal(ltime, fuzzydate);
  }
}

bool Item_func_numhybrid::get_time(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed == 1);
  switch (data_type()) {
    case MYSQL_TYPE_TIME:
      return time_op(ltime);
    case MYSQL_TYPE_DATE:
      return get_time_from_date(ltime);
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      return get_time_from_datetime(ltime);
    default:
      return Item::get_time_from_non_temporal(ltime);
  }
}

void Item_typecast_signed::print(const THD *thd, String *str,
                                 enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("cast("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" as signed)"));
}

bool Item_typecast_signed::resolve_type(THD *) {
  fix_char_length(
      min<uint32>(args[0]->max_char_length(), MY_INT64_NUM_DECIMAL_DIGITS));
  return reject_geometry_args(arg_count, args, this);
}

longlong Item_typecast_signed::val_int_from_str(int *error) {
  char buff[MAX_FIELD_WIDTH], *start;
  size_t length;
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  longlong value;
  const CHARSET_INFO *cs;

  /*
    For a string result, we must first get the string and then convert it
    to a longlong
  */

  if (!(res = args[0]->val_str(&tmp))) {
    null_value = true;
    *error = 0;
    return 0;
  }
  null_value = false;
  start = res->ptr();
  length = res->length();
  cs = res->charset();

  const char *end = start + length;
  value = cs->cset->strtoll10(cs, start, &end, error);
  if (*error > 0 || end != start + length) {
    ErrConvString err(res);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "INTEGER", err.ptr());
  }
  return value;
}

longlong Item_typecast_signed::val_int() {
  longlong value;
  int error;

  if (args[0]->cast_to_int_type() != STRING_RESULT || args[0]->is_temporal()) {
    value = args[0]->val_int();
    null_value = args[0]->null_value;
    return value;
  }

  value = val_int_from_str(&error);
  if (value < 0 && error == 0) {
    push_warning(current_thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                 "Cast to signed converted positive out-of-range integer to "
                 "it's negative complement");
  }
  return value;
}

void Item_typecast_unsigned::print(const THD *thd, String *str,
                                   enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("cast("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" as unsigned)"));
}

longlong Item_typecast_unsigned::val_int() {
  longlong value;
  int error;

  if (args[0]->cast_to_int_type() == DECIMAL_RESULT) {
    my_decimal tmp, *dec = args[0]->val_decimal(&tmp);
    if (!(null_value = args[0]->null_value))
      my_decimal2int(E_DEC_FATAL_ERROR, dec, !dec->sign(), &value);
    else
      value = 0;
    return value;
  } else if (args[0]->cast_to_int_type() != STRING_RESULT ||
             args[0]->is_temporal()) {
    value = args[0]->val_int();
    null_value = args[0]->null_value;
    return value;
  }

  value = val_int_from_str(&error);
  if (error < 0)
    push_warning(current_thd, Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                 "Cast to unsigned converted negative integer to it's "
                 "positive complement");
  return value;
}

String *Item_typecast_decimal::val_str(String *str) {
  my_decimal tmp_buf, *tmp = val_decimal(&tmp_buf);
  if (null_value) return nullptr;
  my_decimal2string(E_DEC_FATAL_ERROR, tmp, str);
  return str;
}

double Item_typecast_decimal::val_real() {
  my_decimal tmp_buf, *tmp = val_decimal(&tmp_buf);
  double res;
  if (null_value) return 0.0;
  my_decimal2double(E_DEC_FATAL_ERROR, tmp, &res);
  return res;
}

longlong Item_typecast_decimal::val_int() {
  my_decimal tmp_buf, *tmp = val_decimal(&tmp_buf);
  longlong res;
  if (null_value) return 0;
  my_decimal2int(E_DEC_FATAL_ERROR, tmp, unsigned_flag, &res);
  return res;
}

my_decimal *Item_typecast_decimal::val_decimal(my_decimal *dec) {
  my_decimal tmp_buf, *tmp = args[0]->val_decimal(&tmp_buf);
  bool sign;
  uint precision;

  if ((null_value = args[0]->null_value)) return nullptr;
  my_decimal_round(E_DEC_FATAL_ERROR, tmp, decimals, false, dec);
  sign = dec->sign();
  if (unsigned_flag) {
    if (sign) {
      my_decimal_set_zero(dec);
      goto err;
    }
  }
  precision =
      my_decimal_length_to_precision(max_length, decimals, unsigned_flag);
  if (precision - decimals < (uint)my_decimal_intg(dec)) {
    max_my_decimal(dec, precision, decimals);
    dec->sign(sign);
    goto err;
  }
  return dec;

err:
  push_warning_printf(
      current_thd, Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE,
      ER_THD(current_thd, ER_WARN_DATA_OUT_OF_RANGE), item_name.ptr(), 1L);
  return dec;
}

void Item_typecast_decimal::print(const THD *thd, String *str,
                                  enum_query_type query_type) const {
  uint precision =
      my_decimal_length_to_precision(max_length, decimals, unsigned_flag);
  str->append(STRING_WITH_LEN("cast("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" as decimal("));
  str->append_ulonglong(precision);
  str->append(',');
  str->append_ulonglong(decimals);
  str->append(')');
  str->append(')');
}

String *Item_typecast_real::val_str(String *str) {
  double res = val_real();
  if (null_value) return nullptr;

  str->set_real(res, decimals, collation.collation);
  return str;
}

double Item_typecast_real::val_real() {
  double res = args[0]->val_real();
  null_value = args[0]->null_value;
  if (null_value) return 0.0;
  if (data_type() == MYSQL_TYPE_FLOAT &&
      ((res > std::numeric_limits<float>::max()) ||
       res < std::numeric_limits<float>::lowest()))
    return raise_float_overflow();
  return check_float_overflow(res);
}

longlong Item_typecast_real::val_int() {
  double res = val_real();
  if (null_value) return 0;

  if (unsigned_flag) {
    if (res < 0 || res >= (double)ULLONG_MAX) {
      return raise_integer_overflow();
    } else
      return (longlong)double2ulonglong(res);
  } else {
    if (res <= (double)LLONG_MIN || res > (double)LLONG_MAX) {
      return raise_integer_overflow();
    } else
      return (longlong)res;
  }
}

bool Item_typecast_real::get_date(MYSQL_TIME *ltime,
                                  my_time_flags_t fuzzydate) {
  return my_double_to_datetime_with_warn(val_real(), ltime, fuzzydate);
}

bool Item_typecast_real::get_time(MYSQL_TIME *ltime) {
  return my_double_to_time_with_warn(val_real(), ltime);
}

my_decimal *Item_typecast_real::val_decimal(my_decimal *decimal_value) {
  double result = val_real();
  if (null_value) return nullptr;
  double2my_decimal(E_DEC_FATAL_ERROR, result, decimal_value);

  return decimal_value;
}

void Item_typecast_real::print(const THD *thd, String *str,
                               enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("cast("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" as "));
  str->append((data_type() == MYSQL_TYPE_FLOAT) ? "float)" : "double)");
}

double Item_func_plus::real_op() {
  double value = args[0]->val_real() + args[1]->val_real();
  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0.0;
  return check_float_overflow(value);
}

longlong Item_func_plus::int_op() {
  longlong val0 = args[0]->val_int();
  longlong val1 = args[1]->val_int();
  longlong res = val0 + val1;
  bool res_unsigned = false;

  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0;

  /*
    First check whether the result can be represented as a
    (bool unsigned_flag, longlong value) pair, then check if it is compatible
    with this Item's unsigned_flag by calling check_integer_overflow().
  */
  if (args[0]->unsigned_flag) {
    if (args[1]->unsigned_flag || val1 >= 0) {
      if (test_if_sum_overflows_ull((ulonglong)val0, (ulonglong)val1)) goto err;
      res_unsigned = true;
    } else {
      /* val1 is negative */
      if ((ulonglong)val0 > (ulonglong)LLONG_MAX) res_unsigned = true;
    }
  } else {
    if (args[1]->unsigned_flag) {
      if (val0 >= 0) {
        if (test_if_sum_overflows_ull((ulonglong)val0, (ulonglong)val1))
          goto err;
        res_unsigned = true;
      } else {
        if ((ulonglong)val1 > (ulonglong)LLONG_MAX) res_unsigned = true;
      }
    } else {
      if (val0 >= 0 && val1 >= 0)
        res_unsigned = true;
      else if (val0 < 0 && val1 < 0 && res >= 0)
        goto err;
    }
  }
  return check_integer_overflow(res, res_unsigned);

err:
  return raise_integer_overflow();
}

/**
  Calculate plus of two decimals.

  @param decimal_value	Buffer that can be used to store result

  @return Value of operation as a decimal
  @retval
    0  Value was NULL;  In this case null_value is set
*/

my_decimal *Item_func_plus::decimal_op(my_decimal *decimal_value) {
  my_decimal value1, *val1;
  my_decimal value2, *val2;
  val1 = args[0]->val_decimal(&value1);
  if ((null_value = args[0]->null_value)) return nullptr;
  val2 = args[1]->val_decimal(&value2);
  if (!(null_value =
            (args[1]->null_value || check_decimal_overflow(my_decimal_add(
                                        E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW,
                                        decimal_value, val1, val2)) > 3)))
    return decimal_value;
  return nullptr;
}

/**
  Set precision of results for additive operations (+ and -)
*/
void Item_func_additive_op::result_precision() {
  decimals = max(args[0]->decimals, args[1]->decimals);
  int arg1_int = args[0]->decimal_precision() - args[0]->decimals;
  int arg2_int = args[1]->decimal_precision() - args[1]->decimals;
  int precision = max(arg1_int, arg2_int) + 1 + decimals;

  /* Integer operations keep unsigned_flag if one of arguments is unsigned */
  if (result_type() == INT_RESULT)
    unsigned_flag = args[0]->unsigned_flag | args[1]->unsigned_flag;
  else
    unsigned_flag = args[0]->unsigned_flag & args[1]->unsigned_flag;
  max_length = my_decimal_precision_to_length_no_truncation(precision, decimals,
                                                            unsigned_flag);
}

/**
  The following function is here to allow the user to force
  subtraction of UNSIGNED BIGINT/DECIMAL to return negative values.
*/

bool Item_func_minus::resolve_type(THD *thd) {
  if (Item_num_op::resolve_type(thd)) return true;
  if (unsigned_flag && (thd->variables.sql_mode & MODE_NO_UNSIGNED_SUBTRACTION))
    unsigned_flag = false;
  return false;
}

double Item_func_minus::real_op() {
  double value = args[0]->val_real() - args[1]->val_real();
  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0.0;
  return check_float_overflow(value);
}

longlong Item_func_minus::int_op() {
  longlong val0 = args[0]->val_int();
  longlong val1 = args[1]->val_int();
  longlong res = val0 - val1;
  bool res_unsigned = false;

  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0;

  /*
    First check whether the result can be represented as a
    (bool unsigned_flag, longlong value) pair, then check if it is compatible
    with this Item's unsigned_flag by calling check_integer_overflow().
  */
  if (args[0]->unsigned_flag) {
    if (args[1]->unsigned_flag) {
      if ((ulonglong)val0 < (ulonglong)val1) {
        if (res >= 0) goto err;
      } else
        res_unsigned = true;
    } else {
      if (val1 >= 0) {
        if ((ulonglong)val0 > (ulonglong)val1) res_unsigned = true;
      } else {
        if (test_if_sum_overflows_ull((ulonglong)val0, (ulonglong)-val1))
          goto err;
        res_unsigned = true;
      }
    }
  } else {
    if (args[1]->unsigned_flag) {
      if ((ulonglong)(val0 - LLONG_MIN) < (ulonglong)val1) goto err;
    } else {
      if (val0 > 0 && val1 < 0)
        res_unsigned = true;
      else if (val0 < 0 && val1 > 0 && res >= 0)
        goto err;
    }
  }
  return check_integer_overflow(res, res_unsigned);

err:
  return raise_integer_overflow();
}

/**
  See Item_func_plus::decimal_op for comments.
*/

my_decimal *Item_func_minus::decimal_op(my_decimal *decimal_value) {
  my_decimal value1, *val1;
  my_decimal value2, *val2;

  val1 = args[0]->val_decimal(&value1);
  if ((null_value = args[0]->null_value)) return nullptr;

  val2 = args[1]->val_decimal(&value2);
  if ((null_value = args[1]->null_value)) return nullptr;

  if ((null_value = check_decimal_overflow(
                        my_decimal_sub(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW,
                                       decimal_value, val1, val2)) > 3)) {
    /*
      Do not return a NULL pointer, as the result may be used in subsequent
      arithmetic operations.
     */
    my_decimal_set_zero(decimal_value);
    return decimal_value;
  }
  /*
   Allow sign mismatch only if sql_mode includes MODE_NO_UNSIGNED_SUBTRACTION
   See Item_func_minus::resolve_type().
  */
  if (unsigned_flag && decimal_value->sign()) {
    /*
      Do not return a NULL pointer, as the result may be used in subsequent
      arithmetic operations.
     */
    my_decimal_set_zero(decimal_value);
    null_value = maybe_null;
    raise_decimal_overflow();
    return decimal_value;
  }
  return decimal_value;
}

double Item_func_mul::real_op() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real() * args[1]->val_real();
  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0.0;
  return check_float_overflow(value);
}

longlong Item_func_mul::int_op() {
  DBUG_ASSERT(fixed == 1);
  longlong a = args[0]->val_int();
  longlong b = args[1]->val_int();
  longlong res;
  ulonglong res0, res1;

  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0;

  if (a == 0 || b == 0) return 0;

  /*
    First check whether the result can be represented as a
    (bool unsigned_flag, longlong value) pair, then check if it is compatible
    with this Item's unsigned_flag by calling check_integer_overflow().

    Let a = a1 * 2^32 + a0 and b = b1 * 2^32 + b0. Then
    a * b = (a1 * 2^32 + a0) * (b1 * 2^32 + b0) = a1 * b1 * 2^64 +
            + (a1 * b0 + a0 * b1) * 2^32 + a0 * b0;
    We can determine if the above sum overflows the ulonglong range by
    sequentially checking the following conditions:
    1. If both a1 and b1 are non-zero.
    2. Otherwise, if (a1 * b0 + a0 * b1) is greater than ULONG_MAX.
    3. Otherwise, if (a1 * b0 + a0 * b1) * 2^32 + a0 * b0 is greater than
    ULLONG_MAX.

    Since we also have to take the unsigned_flag for a and b into account,
    it is easier to first work with absolute values and set the
    correct sign later.

    We handle INT_MIN64 == -9223372036854775808 specially first,
    to avoid UBSAN warnings.
  */
  const bool a_negative = (!args[0]->unsigned_flag && a < 0);
  const bool b_negative = (!args[1]->unsigned_flag && b < 0);

  const bool res_unsigned = (a_negative == b_negative);

  if (a_negative && a == INT_MIN64) {
    if (b == 1) return check_integer_overflow(a, res_unsigned);
    return raise_integer_overflow();
  }

  if (b_negative && b == INT_MIN64) {
    if (a == 1) return check_integer_overflow(b, res_unsigned);
    return raise_integer_overflow();
  }

  if (a_negative) {
    a = -a;
  }
  if (b_negative) {
    b = -b;
  }

  ulong a0 = 0xFFFFFFFFUL & a;
  ulong a1 = ((ulonglong)a) >> 32;
  ulong b0 = 0xFFFFFFFFUL & b;
  ulong b1 = ((ulonglong)b) >> 32;

  if (a1 && b1) goto err;

  res1 = (ulonglong)a1 * b0 + (ulonglong)a0 * b1;
  if (res1 > 0xFFFFFFFFUL) goto err;

  res1 = res1 << 32;
  res0 = (ulonglong)a0 * b0;

  if (test_if_sum_overflows_ull(res1, res0)) goto err;
  res = res1 + res0;

  if (a_negative != b_negative) {
    if ((ulonglong)res > (ulonglong)LLONG_MAX) goto err;
    res = -res;
  }

  return check_integer_overflow(res, res_unsigned);

err:
  return raise_integer_overflow();
}

/** See Item_func_plus::decimal_op for comments. */

my_decimal *Item_func_mul::decimal_op(my_decimal *decimal_value) {
  my_decimal value1, *val1;
  my_decimal value2, *val2;
  val1 = args[0]->val_decimal(&value1);
  if ((null_value = args[0]->null_value)) return nullptr;
  val2 = args[1]->val_decimal(&value2);
  if (!(null_value =
            (args[1]->null_value || (check_decimal_overflow(my_decimal_mul(
                                         E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW,
                                         decimal_value, val1, val2)) > 3))))
    return decimal_value;
  return nullptr;
}

void Item_func_mul::result_precision() {
  /* Integer operations keep unsigned_flag if one of arguments is unsigned */
  if (result_type() == INT_RESULT)
    unsigned_flag = args[0]->unsigned_flag | args[1]->unsigned_flag;
  else
    unsigned_flag = args[0]->unsigned_flag & args[1]->unsigned_flag;
  decimals = min(args[0]->decimals + args[1]->decimals, DECIMAL_MAX_SCALE);
  uint est_prec = args[0]->decimal_precision() + args[1]->decimal_precision();
  uint precision = min<uint>(est_prec, DECIMAL_MAX_PRECISION);
  max_length = my_decimal_precision_to_length_no_truncation(precision, decimals,
                                                            unsigned_flag);
}

double Item_func_div::real_op() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  double val2 = args[1]->val_real();
  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0.0;
  if (val2 == 0.0) {
    signal_divide_by_null();
    return 0.0;
  }
  return check_float_overflow(value / val2);
}

my_decimal *Item_func_div::decimal_op(my_decimal *decimal_value) {
  my_decimal value1, *val1;
  my_decimal value2, *val2;
  int err;

  val1 = args[0]->val_decimal(&value1);
  if ((null_value = args[0]->null_value)) return nullptr;
  val2 = args[1]->val_decimal(&value2);
  if ((null_value = args[1]->null_value)) return nullptr;
  if ((err = check_decimal_overflow(
           my_decimal_div(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW & ~E_DEC_DIV_ZERO,
                          decimal_value, val1, val2, prec_increment))) > 3) {
    if (err == E_DEC_DIV_ZERO) signal_divide_by_null();
    null_value = true;
    return nullptr;
  }
  return decimal_value;
}

void Item_func_div::result_precision() {
  uint precision = min<uint>(
      args[0]->decimal_precision() + args[1]->decimals + prec_increment,
      DECIMAL_MAX_PRECISION);

  if (result_type() == DECIMAL_RESULT) DBUG_ASSERT(precision > 0);

  /* Integer operations keep unsigned_flag if one of arguments is unsigned */
  if (result_type() == INT_RESULT)
    unsigned_flag = args[0]->unsigned_flag | args[1]->unsigned_flag;
  else
    unsigned_flag = args[0]->unsigned_flag & args[1]->unsigned_flag;
  decimals = min<uint>(args[0]->decimals + prec_increment, DECIMAL_MAX_SCALE);
  max_length = my_decimal_precision_to_length_no_truncation(precision, decimals,
                                                            unsigned_flag);
}

bool Item_func_div::resolve_type(THD *thd) {
  DBUG_TRACE;
  prec_increment = thd->variables.div_precincrement;
  if (Item_num_op::resolve_type(thd)) return true;

  switch (hybrid_type) {
    case REAL_RESULT: {
      decimals = max(args[0]->decimals, args[1]->decimals) + prec_increment;
      decimals = min(decimals, uint8(DECIMAL_NOT_SPECIFIED));
      uint tmp = float_length(decimals);
      if (decimals == DECIMAL_NOT_SPECIFIED)
        max_length = tmp;
      else {
        max_length = args[0]->max_length - args[0]->decimals + decimals;
        max_length = min(max_length, tmp);
      }
      break;
    }
    case INT_RESULT:
      set_data_type(MYSQL_TYPE_NEWDECIMAL);
      hybrid_type = DECIMAL_RESULT;
      DBUG_PRINT("info", ("Type changed: DECIMAL_RESULT"));
      result_precision();
      break;
    case DECIMAL_RESULT:
      result_precision();
      break;
    default:
      DBUG_ASSERT(0);
  }
  maybe_null = true;  // division by zero
  return false;
}

/* Integer division */
longlong Item_func_int_div::val_int() {
  DBUG_ASSERT(fixed == 1);

  /*
    Perform division using DECIMAL math if either of the operands has a
    non-integer type
  */
  if (args[0]->result_type() != INT_RESULT ||
      args[1]->result_type() != INT_RESULT) {
    my_decimal tmp;
    my_decimal *val0p = args[0]->val_decimal(&tmp);
    if ((null_value = args[0]->null_value)) return 0;
    my_decimal val0 = *val0p;

    my_decimal *val1p = args[1]->val_decimal(&tmp);
    if ((null_value = args[1]->null_value)) return 0;
    my_decimal val1 = *val1p;

    int err;
    if ((err = my_decimal_div(E_DEC_FATAL_ERROR & ~E_DEC_DIV_ZERO, &tmp, &val0,
                              &val1, 0)) > 3) {
      if (err == E_DEC_DIV_ZERO) signal_divide_by_null();
      return 0;
    }

    my_decimal truncated;
    const bool do_truncate = true;
    if (my_decimal_round(E_DEC_FATAL_ERROR, &tmp, 0, do_truncate, &truncated))
      DBUG_ASSERT(false);

    longlong res;
    if (my_decimal2int(E_DEC_FATAL_ERROR, &truncated, unsigned_flag, &res) &
        E_DEC_OVERFLOW)
      raise_integer_overflow();
    return res;
  }

  longlong val0 = args[0]->val_int();
  longlong val1 = args[1]->val_int();
  bool val0_negative, val1_negative, res_negative;
  ulonglong uval0, uval1, res;
  if ((null_value = (args[0]->null_value || args[1]->null_value))) return 0;
  if (val1 == 0) {
    signal_divide_by_null();
    return 0;
  }

  val0_negative = !args[0]->unsigned_flag && val0 < 0;
  val1_negative = !args[1]->unsigned_flag && val1 < 0;
  res_negative = val0_negative != val1_negative;
  uval0 = (ulonglong)(val0_negative && val0 != LLONG_MIN ? -val0 : val0);
  uval1 = (ulonglong)(val1_negative && val1 != LLONG_MIN ? -val1 : val1);
  res = uval0 / uval1;
  if (res_negative) {
    if (res > (ulonglong)LLONG_MAX) return raise_integer_overflow();
    res = (ulonglong)(-(longlong)res);
  }
  return check_integer_overflow(res, !res_negative);
}

bool Item_func_int_div::resolve_type(THD *) {
  Item_result argtype = args[0]->result_type();
  /* use precision ony for the data type it is applicable for and valid */
  uint32 char_length =
      args[0]->max_char_length() -
      (argtype == DECIMAL_RESULT || argtype == INT_RESULT ? args[0]->decimals
                                                          : 0);
  fix_char_length(char_length > MY_INT64_NUM_DECIMAL_DIGITS
                      ? MY_INT64_NUM_DECIMAL_DIGITS
                      : char_length);
  maybe_null = true;
  unsigned_flag = args[0]->unsigned_flag | args[1]->unsigned_flag;
  return reject_geometry_args(arg_count, args, this);
}

longlong Item_func_mod::int_op() {
  DBUG_ASSERT(fixed == 1);
  longlong val0 = args[0]->val_int();
  longlong val1 = args[1]->val_int();
  bool val0_negative, val1_negative;
  ulonglong uval0, uval1;
  ulonglong res;

  if ((null_value = args[0]->null_value || args[1]->null_value))
    return 0; /* purecov: inspected */
  if (val1 == 0) {
    signal_divide_by_null();
    return 0;
  }

  /*
    '%' is calculated by integer division internally. Since dividing
    LLONG_MIN by -1 generates SIGFPE, we calculate using unsigned values and
    then adjust the sign appropriately.
  */
  val0_negative = !args[0]->unsigned_flag && val0 < 0;
  val1_negative = !args[1]->unsigned_flag && val1 < 0;
  uval0 = (ulonglong)(val0_negative && val0 != LLONG_MIN ? -val0 : val0);
  uval1 = (ulonglong)(val1_negative && val1 != LLONG_MIN ? -val1 : val1);
  res = uval0 % uval1;
  return check_integer_overflow(val0_negative ? -res : res, !val0_negative);
}

double Item_func_mod::real_op() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  double val2 = args[1]->val_real();
  if ((null_value = args[0]->null_value || args[1]->null_value))
    return 0.0; /* purecov: inspected */
  if (val2 == 0.0) {
    signal_divide_by_null();
    return 0.0;
  }
  return fmod(value, val2);
}

my_decimal *Item_func_mod::decimal_op(my_decimal *decimal_value) {
  my_decimal value1, *val1;
  my_decimal value2, *val2;

  val1 = args[0]->val_decimal(&value1);
  if ((null_value = args[0]->null_value)) return nullptr;
  val2 = args[1]->val_decimal(&value2);
  if ((null_value = args[1]->null_value)) return nullptr;
  switch (my_decimal_mod(E_DEC_FATAL_ERROR & ~E_DEC_DIV_ZERO, decimal_value,
                         val1, val2)) {
    case E_DEC_TRUNCATED:
    case E_DEC_OK:
      return decimal_value;
    case E_DEC_DIV_ZERO:
      signal_divide_by_null();
      // Fall through.
    default:
      null_value = true;
      return nullptr;
  }
}

void Item_func_mod::result_precision() {
  decimals = max(args[0]->decimals, args[1]->decimals);
  max_length = max(args[0]->max_length, args[1]->max_length);
  // Increase max_length if we have: signed % unsigned(precision == scale)
  if (!args[0]->unsigned_flag && args[1]->unsigned_flag &&
      args[0]->max_length <= args[1]->max_length &&
      args[1]->decimals == args[1]->decimal_precision()) {
    max_length += 1;
  }
}

bool Item_func_mod::resolve_type(THD *thd) {
  if (Item_num_op::resolve_type(thd)) return true;
  maybe_null = true;
  unsigned_flag = args[0]->unsigned_flag;
  return false;
}

double Item_func_neg::real_op() {
  double value = args[0]->val_real();
  null_value = args[0]->null_value;
  return -value;
}

longlong Item_func_neg::int_op() {
  longlong value = args[0]->val_int();
  if ((null_value = args[0]->null_value)) return 0;
  if (args[0]->unsigned_flag && (ulonglong)value > (ulonglong)LLONG_MAX + 1ULL)
    return raise_integer_overflow();
  // For some platforms we need special handling of LLONG_MIN to
  // guarantee overflow.
  if (value == LLONG_MIN && !args[0]->unsigned_flag && !unsigned_flag)
    return raise_integer_overflow();
  // Avoid doing '-value' below, it is undefined.
  if (value == LLONG_MIN && args[0]->unsigned_flag && !unsigned_flag)
    return LLONG_MIN;
  return check_integer_overflow(-value, !args[0]->unsigned_flag && value < 0);
}

my_decimal *Item_func_neg::decimal_op(my_decimal *decimal_value) {
  my_decimal val, *value = args[0]->val_decimal(&val);
  if (!(null_value = args[0]->null_value)) {
    my_decimal2decimal(value, decimal_value);
    my_decimal_neg(decimal_value);
    return decimal_value;
  }
  return nullptr;
}

void Item_func_neg::fix_num_length_and_dec() {
  decimals = args[0]->decimals;
  /* 1 add because sign can appear */
  max_length = args[0]->max_length + 1;
}

bool Item_func_neg::resolve_type(THD *thd) {
  DBUG_TRACE;
  if (Item_func_num1::resolve_type(thd)) return true;
  /*
    If this is in integer context keep the context as integer if possible
    (This is how multiplication and other integer functions works)
    Use val() to get value as arg_type doesn't mean that item is
    Item_int or Item_real due to existence of Item_param.
  */
  if (hybrid_type == INT_RESULT && args[0]->const_item()) {
    longlong val = args[0]->val_int();
    if ((ulonglong)val >= (ulonglong)LLONG_MIN &&
        ((ulonglong)val != (ulonglong)LLONG_MIN ||
         args[0]->type() != INT_ITEM)) {
      /*
        Ensure that result is converted to DECIMAL, as longlong can't hold
        the negated number
      */
      set_data_type(MYSQL_TYPE_NEWDECIMAL);
      hybrid_type = DECIMAL_RESULT;
      DBUG_PRINT("info", ("Type changed: DECIMAL_RESULT"));
    }
  }
  unsigned_flag = false;
  return false;
}

double Item_func_abs::real_op() {
  double value = args[0]->val_real();
  null_value = args[0]->null_value;
  return fabs(value);
}

longlong Item_func_abs::int_op() {
  longlong value = args[0]->val_int();
  if ((null_value = args[0]->null_value)) return 0;
  if (unsigned_flag) return value;
  /* -LLONG_MIN = LLONG_MAX + 1 => outside of signed longlong range */
  if (value == LLONG_MIN) return raise_integer_overflow();
  return (value >= 0) ? value : -value;
}

my_decimal *Item_func_abs::decimal_op(my_decimal *decimal_value) {
  my_decimal val, *value = args[0]->val_decimal(&val);
  if (!(null_value = args[0]->null_value)) {
    my_decimal2decimal(value, decimal_value);
    if (decimal_value->sign()) my_decimal_neg(decimal_value);
    return decimal_value;
  }
  return nullptr;
}

bool Item_func_abs::resolve_type(THD *thd) {
  if (Item_func_num1::resolve_type(thd)) return true;
  unsigned_flag = args[0]->unsigned_flag;
  return false;
}

bool Item_dec_func::resolve_type(THD *) {
  decimals = DECIMAL_NOT_SPECIFIED;
  max_length = float_length(decimals);
  maybe_null = true;
  return reject_geometry_args(arg_count, args, this);
}

/** Gateway to natural LOG function. */
double Item_func_ln::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  if (value <= 0.0) {
    signal_invalid_argument_for_log();
    return 0.0;
  }
  return log(value);
}

/**
  Extended but so slower LOG function.

  We have to check if all values are > zero and first one is not one
  as these are the cases then result is not a number.
*/
double Item_func_log::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  if (value <= 0.0) {
    signal_invalid_argument_for_log();
    return 0.0;
  }
  if (arg_count == 2) {
    double value2 = args[1]->val_real();
    if ((null_value = args[1]->null_value)) return 0.0;
    if (value2 <= 0.0 || value == 1.0) {
      signal_invalid_argument_for_log();
      return 0.0;
    }
    return log(value2) / log(value);
  }
  return log(value);
}

double Item_func_log2::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();

  if ((null_value = args[0]->null_value)) return 0.0;
  if (value <= 0.0) {
    signal_invalid_argument_for_log();
    return 0.0;
  }
  return std::log2(value);
}

double Item_func_log10::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  if (value <= 0.0) {
    signal_invalid_argument_for_log();
    return 0.0;
  }
  return log10(value);
}

double Item_func_exp::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0; /* purecov: inspected */
  return check_float_overflow(exp(value));
}

double Item_func_sqrt::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = (args[0]->null_value || value < 0)))
    return 0.0; /* purecov: inspected */
  return sqrt(value);
}

double Item_func_pow::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  double val2 = args[1]->val_real();
  if ((null_value = (args[0]->null_value || args[1]->null_value)))
    return 0.0; /* purecov: inspected */
  const double pow_result = pow(value, val2);
  return check_float_overflow(pow_result);
}

// Trigonometric functions

double Item_func_acos::val_real() {
  DBUG_ASSERT(fixed == 1);
  /* One can use this to defer SELECT processing. */
  DEBUG_SYNC(current_thd, "before_acos_function");
  double value = args[0]->val_real();
  if ((null_value = (args[0]->null_value || (value < -1.0 || value > 1.0))))
    return 0.0;
  return acos(value);
}

double Item_func_asin::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = (args[0]->null_value || (value < -1.0 || value > 1.0))))
    return 0.0;
  return asin(value);
}

double Item_func_atan::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  if (arg_count == 2) {
    double val2 = args[1]->val_real();
    if ((null_value = args[1]->null_value)) return 0.0;
    return check_float_overflow(atan2(value, val2));
  }
  return atan(value);
}

double Item_func_cos::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  return cos(value);
}

double Item_func_sin::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  return sin(value);
}

double Item_func_tan::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  return check_float_overflow(tan(value));
}

double Item_func_cot::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  double val2 = tan(value);
  if (val2 == 0.0) {
    return raise_float_overflow();
  }
  return check_float_overflow(1.0 / val2);
}

// Bitwise functions

bool Item_func_bit::resolve_type(THD *) {
  if (bit_func_returns_binary(
          args[0],
          binary_result_requires_binary_second_arg() ? args[1] : nullptr)) {
    hybrid_type = STRING_RESULT;
    set_data_type_string(max<uint32>(args[0]->max_length,
                                     binary_result_requires_binary_second_arg()
                                         ? args[1]->max_length
                                         : 0U),
                         &my_charset_bin);
  } else {
    hybrid_type = INT_RESULT;
    set_data_type_longlong();
    unsigned_flag = true;
  }
  return reject_geometry_args(arg_count, args, this);
}

longlong Item_func_bit::val_int() {
  DBUG_ASSERT(fixed);
  if (hybrid_type == INT_RESULT)
    return int_op();
  else {
    String *res;
    if (!(res = str_op(&str_value))) return 0;

    int ovf_error;
    const char *from = res->ptr();
    size_t len = res->length();
    const char *end = from + len;
    return my_strtoll10(from, &end, &ovf_error);
  }
}

double Item_func_bit::val_real() {
  DBUG_ASSERT(fixed);
  if (hybrid_type == INT_RESULT)
    return static_cast<ulonglong>(int_op());
  else {
    String *res;
    if (!(res = str_op(&str_value))) return 0.0;

    int ovf_error;
    const char *from = res->ptr();
    size_t len = res->length();
    const char *end = from + len;
    return my_strtod(from, &end, &ovf_error);
  }
}

my_decimal *Item_func_bit::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed);
  if (hybrid_type == INT_RESULT)
    return val_decimal_from_int(decimal_value);
  else
    return val_decimal_from_string(decimal_value);
}

String *Item_func_bit::val_str(String *str) {
  DBUG_ASSERT(fixed);
  if (hybrid_type == INT_RESULT) {
    longlong nr = int_op();
    if (null_value) return nullptr;
    str->set_int(nr, unsigned_flag, collation.collation);
    return str;
  } else
    return str_op(str);
}

// Shift-functions, same as << and >> in C/C++

/**
  Template function that evaluates the bitwise shift operation over integer
  arguments.
  @tparam to_left True if left-shift, false if right-shift
*/
template <bool to_left>
longlong Item_func_shift::eval_int_op() {
  DBUG_ASSERT(fixed);
  ulonglong res = args[0]->val_uint();
  if (args[0]->null_value) return error_int();

  ulonglong shift = args[1]->val_uint();
  if (args[1]->null_value) return error_int();

  null_value = false;
  if (shift < sizeof(longlong) * 8)
    return to_left ? (res << shift) : (res >> shift);
  return 0;
}

/// Instantiations of the above
template longlong Item_func_shift::eval_int_op<true>();
template longlong Item_func_shift::eval_int_op<false>();

/**
  Template function that evaluates the bitwise shift operation over binary
  string arguments.
  @tparam to_left True if left-shift, false if right-shift
*/
template <bool to_left>
String *Item_func_shift::eval_str_op(String *) {
  DBUG_ASSERT(fixed);

  String tmp_str;
  String *arg = args[0]->val_str(&tmp_str);
  if (!arg || args[0]->null_value) return error_str();

  ssize_t arg_length = arg->length();
  size_t shift =
      min(args[1]->val_uint(), static_cast<ulonglong>(arg_length) * 8);
  if (args[1]->null_value) return error_str();

  if (tmp_value.alloc(arg->length())) return error_str();

  tmp_value.length(arg_length);
  tmp_value.set_charset(&my_charset_bin);
  /*
    Example with left-shift-by-21-bits:
    |........|........|........|........|
      byte i  byte i+1 byte i+2 byte i+3
    First (leftmost) bit has number 1.
    21 = 2*8 + 5.
    Bits of number 1-3 of byte 'i' receive bits 22-24 i.e. the last 3 bits of
    byte 'i+2'. So, take byte 'i+2', shift it left by 5 bits, that puts the
    last 3 bits of byte 'i+2' in bits 1-3, and 0s elsewhere.
    Bits of number 4-8 of byte 'i' receive bits 25-39 i.e. the first 5 bits of
    byte 'i+3'. So, take byte 'i+3', shift it right by 3 bits, that puts the
    first 5 bits of byte 'i+3' in bits 4-8, and 0s elsewhere.
    In total, do OR of both results.
  */
  size_t mod = shift % 8;
  size_t mod_complement = 8 - mod;
  ssize_t entire_bytes = shift / 8;

  const unsigned char *from_c = pointer_cast<const unsigned char *>(arg->ptr());
  unsigned char *to_c = pointer_cast<unsigned char *>(tmp_value.c_ptr_quick());

  if (to_left) {
    // Bytes of lower index are overwritten by bytes of higher index
    for (ssize_t i = 0; i < arg_length; i++)
      if (i + entire_bytes + 1 < arg_length)
        to_c[i] = (from_c[i + entire_bytes] << mod) |
                  (from_c[i + entire_bytes + 1] >> mod_complement);
      else if (i + entire_bytes + 1 == arg_length)
        to_c[i] = from_c[i + entire_bytes] << mod;
      else
        to_c[i] = 0;
  } else {
    // Bytes of higher index are overwritten by bytes of lower index
    for (ssize_t i = arg_length - 1; i >= 0; i--)
      if (i > entire_bytes)
        to_c[i] = (from_c[i - entire_bytes] >> mod) |
                  (from_c[i - entire_bytes - 1] << mod_complement);
      else if (i == entire_bytes)
        to_c[i] = from_c[i - entire_bytes] >> mod;
      else
        to_c[i] = 0;
  }

  null_value = false;
  return &tmp_value;
}

/// Instantiations of the above
template String *Item_func_shift::eval_str_op<true>(String *);
template String *Item_func_shift::eval_str_op<false>(String *);

// Bit negation ('~')

longlong Item_func_bit_neg::int_op() {
  DBUG_ASSERT(fixed);
  ulonglong res = (ulonglong)args[0]->val_int();
  if (args[0]->null_value) return error_int();
  null_value = false;
  return ~res;
}

String *Item_func_bit_neg::str_op(String *str) {
  DBUG_ASSERT(fixed);
  String *res = args[0]->val_str(str);
  if (args[0]->null_value || !res) return error_str();

  if (tmp_value.alloc(res->length())) return error_str();

  size_t arg_length = res->length();
  tmp_value.length(arg_length);
  tmp_value.set_charset(&my_charset_bin);
  const unsigned char *from_c = pointer_cast<const unsigned char *>(res->ptr());
  unsigned char *to_c = pointer_cast<unsigned char *>(tmp_value.c_ptr_quick());
  size_t i = 0;
  while (i + sizeof(longlong) <= arg_length) {
    int8store(&to_c[i], ~(uint8korr(&from_c[i])));
    i += sizeof(longlong);
  }
  while (i < arg_length) {
    to_c[i] = ~from_c[i];
    i++;
  }

  null_value = false;
  return &tmp_value;
}

/**
  Template function used to evaluate the bitwise operation over int arguments.

  @param int_func  The bitwise function.
*/
template <class Int_func>
longlong Item_func_bit_two_param::eval_int_op(Int_func int_func) {
  DBUG_ASSERT(fixed);
  ulonglong arg0 = args[0]->val_uint();
  if (args[0]->null_value) return error_int();
  ulonglong arg1 = args[1]->val_uint();
  if (args[1]->null_value) return error_int();
  null_value = false;
  return (longlong)int_func(arg0, arg1);
}

/// Instantiations of the above
template longlong Item_func_bit_two_param::eval_int_op<std::bit_or<ulonglong>>(
    std::bit_or<ulonglong>);
template longlong Item_func_bit_two_param::eval_int_op<std::bit_and<ulonglong>>(
    std::bit_and<ulonglong>);
template longlong Item_func_bit_two_param::eval_int_op<std::bit_xor<ulonglong>>(
    std::bit_xor<ulonglong>);

/**
  Template function that evaluates the bitwise operation over binary arguments.
  Checks that both arguments have same length and applies the bitwise operation

   @param char_func  The Bitwise function used to evaluate unsigned chars.
   @param int_func   The Bitwise function used to evaluate unsigned long longs.
*/
template <class Char_func, class Int_func>
String *Item_func_bit_two_param::eval_str_op(String *, Char_func char_func,
                                             Int_func int_func) {
  DBUG_ASSERT(fixed);
  String arg0_buff;
  String *s1 = args[0]->val_str(&arg0_buff);

  if (args[0]->null_value || !s1) return error_str();

  String arg1_buff;
  String *s2 = args[1]->val_str(&arg1_buff);

  if (args[1]->null_value || !s2) return error_str();

  size_t arg_length = s1->length();
  if (arg_length != s2->length()) {
    my_error(ER_INVALID_BITWISE_OPERANDS_SIZE, MYF(0), func_name());
    return error_str();
  }

  if (tmp_value.alloc(arg_length)) return error_str();

  tmp_value.length(arg_length);
  tmp_value.set_charset(&my_charset_bin);

  const uchar *s1_c_p = pointer_cast<const uchar *>(s1->ptr());
  const uchar *s2_c_p = pointer_cast<const uchar *>(s2->ptr());
  char *res = tmp_value.ptr();
  size_t i = 0;
  while (i + sizeof(longlong) <= arg_length) {
    int8store(&res[i], int_func(uint8korr(&s1_c_p[i]), uint8korr(&s2_c_p[i])));
    i += sizeof(longlong);
  }
  while (i < arg_length) {
    res[i] = char_func(s1_c_p[i], s2_c_p[i]);
    i++;
  }

  null_value = false;
  return &tmp_value;
}

/// Instantiations of the above
template String *
Item_func_bit_two_param::eval_str_op<std::bit_or<char>, std::bit_or<ulonglong>>(
    String *, std::bit_or<char>, std::bit_or<ulonglong>);
template String *Item_func_bit_two_param::eval_str_op<
    std::bit_and<char>, std::bit_and<ulonglong>>(String *, std::bit_and<char>,
                                                 std::bit_and<ulonglong>);
template String *Item_func_bit_two_param::eval_str_op<
    std::bit_xor<char>, std::bit_xor<ulonglong>>(String *, std::bit_xor<char>,
                                                 std::bit_xor<ulonglong>);

bool Item::bit_func_returns_binary(const Item *a, const Item *b) {
  /*
    Checks if the bitwise function should return binary data.
    The conditions to return true are the following:

    1. If there's only one argument(so b is nullptr),
    then a must be a [VAR]BINARY Item, different from the hex/bit/NULL literal.

    2. If there are two arguments, both should be [VAR]BINARY
    and at least one of them should be different from the hex/bit/NULL literal
  */
  // Check if a is [VAR]BINARY Item
  bool a_is_binary = a->result_type() == STRING_RESULT &&
                     a->collation.collation == &my_charset_bin;
  // Check if b is not null and is [VAR]BINARY Item
  bool b_is_binary = b && b->result_type() == STRING_RESULT &&
                     b->collation.collation == &my_charset_bin;

  return a_is_binary && (!b || b_is_binary) &&
         ((a->type() != Item::VARBIN_ITEM && a->type() != Item::NULL_ITEM) ||
          (b && b->type() != Item::VARBIN_ITEM &&
           b->type() != Item::NULL_ITEM));
}

// Conversion functions

bool Item_func_integer::resolve_type(THD *) {
  max_length =
      min(args[0]->max_length - args[0]->decimals + 1, float_length(decimals));
  return reject_geometry_args(arg_count, args, this);
}

bool Item_func_int_val::resolve_type(THD *) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("name %s", func_name()));

  if (reject_geometry_args(arg_count, args, this)) return true;

  switch (args[0]->result_type()) {
    case STRING_RESULT:
    case REAL_RESULT:
      set_data_type_double();
      hybrid_type = REAL_RESULT;
      break;
    case INT_RESULT:
      set_data_type_longlong();
      unsigned_flag = args[0]->unsigned_flag;
      hybrid_type = INT_RESULT;
      break;
    case DECIMAL_RESULT: {
      // For historical reasons, CEILING and FLOOR convert DECIMAL inputs into
      // BIGINT (granted that they are small enough to fit) while ROUND and
      // TRUNCATE don't. As items are not yet evaluated at this point,
      // assumptions must be made about when a conversion from DECIMAL_RESULT to
      // INT_RESULT can be safely achieved.
      //
      // During the rounding operation, we account for signedness by always
      // assuming that the argument DECIMAL is signed. Additionally, since we
      // call set_data_type_decimal with a scale of 0, we must increment the
      // precision here, as the rounding operation may cause an increase in
      // order of magnitude.
      int precision = args[0]->decimal_precision() - args[0]->decimals;
      if (args[0]->decimals != 0) ++precision;
      set_data_type_decimal(precision, 0);
      hybrid_type = DECIMAL_RESULT;

      // The max_length of the biggest INT_RESULT, BIGINT, is 20 regardless of
      // signedness, as a minus sign will be counted as one digit. A DECIMAL of
      // length 20 could be bigger than the max BIGINT value, thus requiring a
      // length < 20. DECIMAL_LONGLONG_DIGITS value is 22, which is presumably
      // the sum of 20 digits, a minus sign and a decimal point; requiring -2
      // when considering the conversion.
      if (max_length < (DECIMAL_LONGLONG_DIGITS - 2)) {
        set_data_type_longlong();
        hybrid_type = INT_RESULT;
      }

      break;
    }
    default:
      DBUG_ASSERT(0);
  }
  DBUG_PRINT("info", ("Type: %s", (hybrid_type == REAL_RESULT
                                       ? "REAL_RESULT"
                                       : hybrid_type == DECIMAL_RESULT
                                             ? "DECIMAL_RESULT"
                                             : hybrid_type == INT_RESULT
                                                   ? "INT_RESULT"
                                                   : "--ILLEGAL!!!--")));

  return false;
}

longlong Item_func_ceiling::int_op() {
  longlong result;
  switch (args[0]->result_type()) {
    case INT_RESULT:
      result = args[0]->val_int();
      null_value = args[0]->null_value;
      break;
    case DECIMAL_RESULT: {
      my_decimal dec_buf, *dec;
      if ((dec = Item_func_ceiling::decimal_op(&dec_buf)))
        my_decimal2int(E_DEC_FATAL_ERROR, dec, unsigned_flag, &result);
      else
        result = 0;
      break;
    }
    default:
      result = (longlong)Item_func_ceiling::real_op();
  };
  return result;
}

double Item_func_ceiling::real_op() {
  double value = args[0]->val_real();
  null_value = args[0]->null_value;
  return ceil(value);
}

my_decimal *Item_func_ceiling::decimal_op(my_decimal *decimal_value) {
  my_decimal val, *value = args[0]->val_decimal(&val);
  if (!(null_value =
            (args[0]->null_value ||
             my_decimal_ceiling(E_DEC_FATAL_ERROR, value, decimal_value) > 1)))
    return decimal_value;
  return nullptr;
}

longlong Item_func_floor::int_op() {
  longlong result;
  switch (args[0]->result_type()) {
    case INT_RESULT:
      result = args[0]->val_int();
      null_value = args[0]->null_value;
      break;
    case DECIMAL_RESULT: {
      my_decimal dec_buf, *dec;
      if ((dec = Item_func_floor::decimal_op(&dec_buf)))
        my_decimal2int(E_DEC_FATAL_ERROR, dec, unsigned_flag, &result);
      else
        result = 0;
      break;
    }
    default:
      result = (longlong)Item_func_floor::real_op();
  };
  return result;
}

double Item_func_floor::real_op() {
  double value = args[0]->val_real();
  null_value = args[0]->null_value;
  return floor(value);
}

my_decimal *Item_func_floor::decimal_op(my_decimal *decimal_value) {
  my_decimal val, *value = args[0]->val_decimal(&val);
  if (!(null_value =
            (args[0]->null_value ||
             my_decimal_floor(E_DEC_FATAL_ERROR, value, decimal_value) > 1)))
    return decimal_value;
  return nullptr;
}

bool Item_func_round::resolve_type(THD *) {
  int decimals_to_set;
  longlong val1;
  bool val1_unsigned;

  unsigned_flag = args[0]->unsigned_flag;
  if (reject_geometry_args(arg_count, args, this)) return true;

  if (!args[1]->const_item()) {
    decimals = args[0]->decimals;
    max_length = float_length(decimals);
    if (args[0]->result_type() == DECIMAL_RESULT) {
      max_length++;
      set_data_type(MYSQL_TYPE_NEWDECIMAL);
      hybrid_type = DECIMAL_RESULT;
    } else {
      set_data_type(MYSQL_TYPE_DOUBLE);
      hybrid_type = REAL_RESULT;
    }
    return false;
  }

  val1 = args[1]->val_int();
  if ((null_value = args[1]->is_null())) {
    // Set a data type - we do not provide excessive is_null() checks
    if (is_numeric_type(args[0]->data_type())) {
      set_data_type_from_item(args[0]);
      hybrid_type = args[0]->result_type();
    } else {
      set_data_type(MYSQL_TYPE_DOUBLE);
      hybrid_type = REAL_RESULT;
    }
    return false;
  }

  val1_unsigned = args[1]->unsigned_flag;
  if (val1 < 0)
    decimals_to_set = val1_unsigned ? INT_MAX : 0;
  else
    decimals_to_set = (val1 > INT_MAX) ? INT_MAX : (int)val1;

  if (args[0]->decimals == DECIMAL_NOT_SPECIFIED) {
    decimals = min(decimals_to_set, DECIMAL_NOT_SPECIFIED);
    max_length = float_length(decimals);
    set_data_type(MYSQL_TYPE_DOUBLE);
    hybrid_type = REAL_RESULT;
    return false;
  }

  switch (args[0]->result_type()) {
    case REAL_RESULT:
    case STRING_RESULT:
      set_data_type(MYSQL_TYPE_DOUBLE);
      hybrid_type = REAL_RESULT;
      decimals = min(decimals_to_set, DECIMAL_NOT_SPECIFIED);
      max_length = float_length(decimals);
      break;
    case INT_RESULT:
      if ((!decimals_to_set && truncate) ||
          (args[0]->decimal_precision() < DECIMAL_LONGLONG_DIGITS)) {
        bool length_can_increase = (!truncate && (val1 < 0) && !val1_unsigned);
        max_length = args[0]->max_length + length_can_increase;
        /* Here we can keep INT_RESULT */
        set_data_type(MYSQL_TYPE_LONGLONG);
        hybrid_type = INT_RESULT;
        break;
      }
      /* fall through */
    case DECIMAL_RESULT: {
      set_data_type(MYSQL_TYPE_NEWDECIMAL);
      hybrid_type = DECIMAL_RESULT;
      decimals_to_set = min(DECIMAL_MAX_SCALE, decimals_to_set);
      int decimals_delta = args[0]->decimals - decimals_to_set;
      int precision = args[0]->decimal_precision();
      int length_increase = ((decimals_delta <= 0) || truncate) ? 0 : 1;

      precision -= decimals_delta - length_increase;
      decimals = min(decimals_to_set, DECIMAL_MAX_SCALE);
      max_length = my_decimal_precision_to_length_no_truncation(
          precision, decimals, unsigned_flag);
      break;
    }
    default:
      DBUG_ASSERT(0); /* This result type isn't handled */
  }
  return false;
}

double my_double_round(double value, longlong dec, bool dec_unsigned,
                       bool truncate) {
  bool dec_negative = (dec < 0) && !dec_unsigned;
  int log_10_size = array_elements(log_10);  // 309
  if (dec_negative && dec <= -log_10_size) return 0.0;

  ulonglong abs_dec = dec_negative ? -dec : dec;

  double tmp = (abs_dec < array_elements(log_10) ? log_10[abs_dec]
                                                 : pow(10.0, (double)abs_dec));

  double value_mul_tmp = value * tmp;
  if (!dec_negative && !std::isfinite(value_mul_tmp)) return value;

  double value_div_tmp = value / tmp;
  if (truncate) {
    if (value >= 0.0)
      return dec < 0 ? floor(value_div_tmp) * tmp : floor(value_mul_tmp) / tmp;
    else
      return dec < 0 ? ceil(value_div_tmp) * tmp : ceil(value_mul_tmp) / tmp;
  }

  return dec < 0 ? rint(value_div_tmp) * tmp : rint(value_mul_tmp) / tmp;
}

double Item_func_round::real_op() {
  const double value = args[0]->val_real();
  const longlong decimal_places = args[1]->val_int();

  if (!(null_value = args[0]->null_value || args[1]->null_value))
    return my_double_round(value, decimal_places, args[1]->unsigned_flag,
                           truncate);

  return 0.0;
}

/*
  Rounds a given value to a power of 10 specified as the 'to' argument.
*/
static inline ulonglong my_unsigned_round(ulonglong value, ulonglong to,
                                          bool *round_overflow) {
  ulonglong tmp = value / to * to;
  if (value - tmp < (to >> 1)) {
    return tmp;
  } else {
    if (test_if_sum_overflows_ull(tmp, to)) {
      *round_overflow = true;
      return 0;
    }
    return tmp + to;
  }
}

longlong Item_func_round::int_op() {
  longlong value = args[0]->val_int();
  longlong dec = args[1]->val_int();
  decimals = 0;
  ulonglong abs_dec;
  if ((null_value = args[0]->null_value || args[1]->null_value)) return 0;
  if ((dec >= 0) || args[1]->unsigned_flag)
    return value;  // integer have not digits after point

  abs_dec = -static_cast<ulonglong>(dec);
  longlong tmp;

  if (abs_dec >= array_elements(log_10_int)) return 0;

  tmp = log_10_int[abs_dec];

  if (truncate)
    return (unsigned_flag) ? ((ulonglong)value / tmp) * tmp
                           : (value / tmp) * tmp;
  else if (unsigned_flag || value >= 0) {
    bool round_overflow = false;
    ulonglong rounded_value =
        my_unsigned_round(static_cast<ulonglong>(value), tmp, &round_overflow);
    if (!unsigned_flag && rounded_value > LLONG_MAX)
      return raise_integer_overflow();
    if (round_overflow) return raise_integer_overflow();
    return rounded_value;
  } else {
    // We round "towards nearest", so
    // -9223372036854775808 should round to
    // -9223372036854775810 which underflows, or
    // -9223372036854775800 which is OK, or
    // -9223372036854776000 which underflows, and so on ...
    if (value == LLONG_MIN) {
      switch (abs_dec) {
        case 0:
          return LLONG_MIN;
        case 1:
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
        case 9:
        case 10:
        case 14:
        case 19:
          return raise_integer_overflow();
        default:
          return (LLONG_MIN / tmp) * tmp;
      }
    }
    bool not_used = false;
    ulonglong rounded_value =
        my_unsigned_round(static_cast<ulonglong>(-value), tmp, &not_used);
    if (rounded_value > LLONG_MAX) return raise_integer_overflow();

    return -static_cast<longlong>(rounded_value);
  }
}

my_decimal *Item_func_round::decimal_op(my_decimal *decimal_value) {
  my_decimal val, *value = args[0]->val_decimal(&val);
  longlong dec = args[1]->val_int();
  if (dec >= 0 || args[1]->unsigned_flag)
    dec = min<ulonglong>(dec, decimals);
  else if (dec < INT_MIN)
    dec = INT_MIN;

  if (!(null_value = (args[0]->null_value || args[1]->null_value ||
                      my_decimal_round(E_DEC_FATAL_ERROR, value, (int)dec,
                                       truncate, decimal_value) > 1)))
    return decimal_value;
  return nullptr;
}

bool Item_func_rand::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  /*
    When RAND() is binlogged, the seed is binlogged too.  So the
    sequence of random numbers is the same on a replication slave as
    on the master.  However, if several RAND() values are inserted
    into a table, the order in which the rows are modified may differ
    between master and slave, because the order is undefined.  Hence,
    the statement is unsafe to log in statement format.
  */
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);

  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_RAND);
  return false;
}

void Item_func_rand::seed_random(Item *arg) {
  /*
    TODO: do not do reinit 'rand' for every execute of PS/SP if
    args[0] is a constant.
  */
  uint32 tmp = (uint32)arg->val_int();
  randominit(rand, (uint32)(tmp * 0x10001L + 55555555L),
             (uint32)(tmp * 0x10000001L));
}

bool Item_func_rand::resolve_type(THD *thd) {
  if (Item_real_func::resolve_type(thd)) return true;
  return reject_geometry_args(arg_count, args, this);
}

bool Item_func_rand::fix_fields(THD *thd, Item **ref) {
  if (Item_real_func::fix_fields(thd, ref)) return true;

  if (arg_count) {  // Only use argument once in query
    /*
      Allocate rand structure once: we must use thd->stmt_arena
      to create rand in proper mem_root if it's a prepared statement or
      stored procedure.

      No need to send a Rand log event if seed was given eg: RAND(seed),
      as it will be replicated in the query as such.
    */
    if (!rand &&
        !(rand = (struct rand_struct *)thd->stmt_arena->alloc(sizeof(*rand))))
      return true;
  } else {
    /*
      Save the seed only the first time RAND() is used in the query
      Once events are forwarded rather than recreated,
      the following can be skipped if inside the slave thread
    */
    if (!thd->rand_used) {
      thd->rand_used = true;
      thd->rand_saved_seed1 = thd->rand.seed1;
      thd->rand_saved_seed2 = thd->rand.seed2;
    }
    rand = &thd->rand;
  }
  return false;
}

double Item_func_rand::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (arg_count) {
    if (!args[0]->const_for_execution())
      seed_random(args[0]);
    else if (first_eval) {
      /*
        Constantness of args[0] may be set during JOIN::optimize(), if arg[0]
        is a field item of "constant" table. Thus, we have to evaluate
        seed_random() for constant arg there but not at the fix_fields method.
      */
      first_eval = false;
      seed_random(args[0]);
    }
  }
  return my_rnd(rand);
}

bool Item_func_sign::resolve_type(THD *thd) {
  if (Item_int_func::resolve_type(thd)) return true;
  return reject_geometry_args(arg_count, args, this);
}

longlong Item_func_sign::val_int() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  null_value = args[0]->null_value;
  return value < 0.0 ? -1 : (value > 0 ? 1 : 0);
}

bool Item_func_units::resolve_type(THD *) {
  decimals = DECIMAL_NOT_SPECIFIED;
  max_length = float_length(decimals);
  return reject_geometry_args(arg_count, args, this);
}

double Item_func_units::val_real() {
  DBUG_ASSERT(fixed == 1);
  double value = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0;
  return check_float_overflow(value * mul + add);
}

bool Item_func_min_max::resolve_type(THD *) {
  aggregate_type(make_array(args, arg_count));
  hybrid_type = Field::result_merge_type(data_type());
  if (hybrid_type == STRING_RESULT) {
    /*
      If one or more of the arguments have a temporal data type, temporal_item
      must be set for correct conversion from temporal values to various result
      types.
    */
    for (uint i = 0; i < arg_count; i++) {
      if (args[i]->is_temporal()) {
        /*
          If one of the arguments is DATETIME, overwrite any existing
          temporal_item since DATETIME contains both date and time and is the
          most general and detailed data type to which other temporal types can
          be converted without loss of information.
        */
        if (!temporal_item || args[i]->data_type() == MYSQL_TYPE_DATETIME)
          temporal_item = args[i];
      }
    }
    /*
      If one or more, but not all, of the arguments have a temporal data type,
      the data type of this item must be set temporarily to ensure that the
      various aggregate functions derive the correct properties.
    */
    enum_field_types tmp_data_type = (!is_temporal() && has_temporal_arg())
                                         ? temporal_item->data_type()
                                         : data_type();
    enum_field_types aggregated_data_type = data_type();
    set_data_type(tmp_data_type);
    if (aggregate_string_properties(func_name(), args, arg_count)) return true;
    set_data_type(aggregated_data_type);
  } else
    aggregate_num_type(hybrid_type, args, arg_count);

  /*
  LEAST and GREATEST convert JSON values to strings before they are
  compared, so their JSON nature is lost. Raise a warning to
  indicate to the users that the values are not compared using the
  JSON comparator, as they might expect. Also update the field type
  of the result to reflect that the result is a string.
*/
  unsupported_json_comparison(arg_count, args,
                              "comparison of JSON in the "
                              "LEAST and GREATEST operators");
  if (data_type() == MYSQL_TYPE_JSON) set_data_type(MYSQL_TYPE_VARCHAR);

  return reject_geometry_args(arg_count, args, this);
}

bool Item_func_min_max::cmp_datetimes(longlong *value) {
  THD *thd = current_thd;
  longlong res = 0;
  for (uint i = 0; i < arg_count; i++) {
    Item **arg = args + i;
    bool is_null;
    longlong tmp =
        get_datetime_value(thd, &arg, nullptr, temporal_item, &is_null);

    // Check if we need to stop (because of error or KILL)  and stop the loop
    if ((null_value = thd->is_error())) return true;
    if ((null_value = args[i]->null_value)) return true;
    if (i == 0 || (tmp < res) == m_is_least_func) res = tmp;
  }
  *value = res;
  return false;
}

bool Item_func_min_max::cmp_times(longlong *value) {
  longlong res = 0;
  for (uint i = 0; i < arg_count; i++) {
    longlong tmp = args[i]->val_time_temporal();
    if ((null_value = args[i]->null_value)) return true;
    if (i == 0 || (tmp < res) == m_is_least_func) res = tmp;
  }
  *value = res;
  return false;
}

String *Item_func_min_max::str_op(String *str) {
  DBUG_ASSERT(fixed);
  null_value = false;
  if (compare_as_dates()) {
    longlong result = 0;
    if (cmp_datetimes(&result)) return nullptr;

    /*
      If result is greater than 0, the winning argument was successfully
      converted to a time value and should be converted to a string
      formatted in accordance with the data type in temporal_item. Otherwise,
      the arguments should be compared based on their raw string value.
    */
    if (result > 0) {
      MYSQL_TIME ltime;
      enum_field_types field_type = temporal_item->data_type();
      TIME_from_longlong_packed(&ltime, field_type, result);
      null_value = my_TIME_to_str(&ltime, str, decimals);
      if (null_value) return nullptr;
      if (str->needs_conversion(collation.collation)) {
        uint errors = 0;
        StringBuffer<STRING_BUFFER_USUAL_SIZE * 2> convert_string(nullptr);
        bool copy_failed =
            convert_string.copy(str->ptr(), str->length(), str->charset(),
                                collation.collation, &errors);
        if (copy_failed || errors || str->copy(convert_string))
          return error_str();
      }
      return str;
    }
  }

  // Find the least/greatest argument based on string value.
  String *res = nullptr;
  bool res_in_str = false;
  for (uint i = 0; i < arg_count; i++) {
    /*
      Because val_str() may reallocate the underlying buffer of its String
      parameter, it is paramount the passed String argument do not share an
      underlying buffer with the currently stored result against which it will
      be compared to ensure that String comparison operates on two
      non-overlapping buffers.
    */
    String *val_buf = res_in_str ? &m_string_buf : str;
    DBUG_ASSERT(!res ||
                (res != val_buf && !res->uses_buffer_owned_by(val_buf)));
    String *val = args[i]->val_str(val_buf);
    if ((null_value = args[i]->null_value)) return nullptr;
    if (i == 0 ||
        (sortcmp(val, res, collation.collation) < 0) == m_is_least_func) {
      res = val;
      res_in_str = !res_in_str;
    }
  }
  res->set_charset(collation.collation);
  return res;
}

bool Item_func_min_max::date_op(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed == 1);
  longlong result = 0;
  if (cmp_datetimes(&result)) return true;
  TIME_from_longlong_packed(ltime, data_type(), result);
  int warnings;
  return check_date(*ltime, non_zero_date(*ltime), fuzzydate, &warnings);
}

bool Item_func_min_max::time_op(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed == 1);
  longlong result = 0;
  if (compare_as_dates()) {
    if (cmp_datetimes(&result)) return true;
    TIME_from_longlong_packed(ltime, data_type(), result);
    datetime_to_time(ltime);
    return false;
  }

  if (cmp_times(&result)) return true;
  TIME_from_longlong_time_packed(ltime, result);
  return false;
}

double Item_func_min_max::real_op() {
  DBUG_ASSERT(fixed == 1);
  null_value = false;
  if (compare_as_dates()) {
    longlong result = 0;
    if (cmp_datetimes(&result)) return 0;
    return double_from_datetime_packed(temporal_item->data_type(), result);
  }

  // Find the least/greatest argument based on double value.
  double result = 0;
  for (uint i = 0; i < arg_count; i++) {
    const double tmp = args[i]->val_real();
    if ((null_value = args[i]->null_value)) break;
    if (i == 0 || (tmp < result) == m_is_least_func) result = tmp;
  }
  return result;
}

longlong Item_func_min_max::int_op() {
  DBUG_ASSERT(fixed);
  null_value = false;
  longlong res = 0;
  if (compare_as_dates()) {
    if (cmp_datetimes(&res)) return 0;
    return longlong_from_datetime_packed(temporal_item->data_type(), res);
  }

  // Find the least/greatest argument based on integer value.
  for (uint i = 0; i < arg_count; i++) {
    const longlong val = args[i]->val_int();
    if ((null_value = args[i]->null_value)) return 0;
#ifndef DBUG_OFF
    Integer_value arg_val(val, args[i]->unsigned_flag);
    DBUG_ASSERT(!unsigned_flag || !arg_val.is_negative());
#endif
    const bool val_is_smaller = unsigned_flag ? static_cast<ulonglong>(val) <
                                                    static_cast<ulonglong>(res)
                                              : val < res;
    if (i == 0 || val_is_smaller == m_is_least_func) res = val;
  }
  return res;
}

my_decimal *Item_func_min_max::decimal_op(my_decimal *dec) {
  DBUG_ASSERT(fixed == 1);
  null_value = false;
  if (compare_as_dates()) {
    longlong result = 0;
    if (cmp_datetimes(&result)) return nullptr;
    return my_decimal_from_datetime_packed(dec, temporal_item->data_type(),
                                           result);
  }

  // Find the least/greatest argument based on decimal value.
  my_decimal tmp_buf, *res = args[0]->val_decimal(dec);
  for (uint i = 0; i < arg_count; i++) {
    my_decimal *tmp = args[i]->val_decimal(res == dec ? &tmp_buf : dec);
    if ((null_value = args[i]->null_value)) return nullptr;
    if (i == 0 || (my_decimal_cmp(tmp, res) < 0) == m_is_least_func) res = tmp;
  }
  //  Result must me copied from temporary buffer to remain valid after return.
  if (res == &tmp_buf) {
    my_decimal2decimal(res, dec);
    res = dec;
  }
  return res;
}

double Item_func_min_max::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (has_temporal_arg() && data_type() == MYSQL_TYPE_VARCHAR)
    return real_op();  // For correct conversion from temporal value to string.
  return Item_func_numhybrid::val_real();
}

longlong Item_func_min_max::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (has_temporal_arg() && data_type() == MYSQL_TYPE_VARCHAR)
    return int_op();  // For correct conversion from temporal value to int.
  return Item_func_numhybrid::val_int();
}

my_decimal *Item_func_min_max::val_decimal(my_decimal *dec) {
  DBUG_ASSERT(fixed == 1);
  if (has_temporal_arg() && data_type() == MYSQL_TYPE_VARCHAR)
    return decimal_op(
        dec);  // For correct conversion from temporal value to dec
  return Item_func_numhybrid::val_decimal(dec);
}

double Item_func_rollup_const::val_real() {
  DBUG_ASSERT(fixed == 1);
  double res = args[0]->val_real();
  if ((null_value = args[0]->null_value)) return 0.0;
  return res;
}

longlong Item_func_rollup_const::val_int() {
  DBUG_ASSERT(fixed == 1);
  longlong res = args[0]->val_int();
  if ((null_value = args[0]->null_value)) return 0;
  return res;
}

String *Item_func_rollup_const::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  if ((null_value = args[0]->null_value)) return nullptr;
  return res;
}

my_decimal *Item_func_rollup_const::val_decimal(my_decimal *dec) {
  DBUG_ASSERT(fixed == 1);
  my_decimal *res = args[0]->val_decimal(dec);
  if ((null_value = args[0]->null_value)) return nullptr;
  return res;
}

bool Item_func_rollup_const::val_json(Json_wrapper *result) {
  DBUG_ASSERT(fixed == 1);
  bool res = args[0]->val_json(result);
  null_value = args[0]->null_value;
  return res;
}

longlong Item_func_length::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  if (!res) {
    null_value = true;
    return 0; /* purecov: inspected */
  }
  null_value = false;
  return (longlong)res->length();
}

longlong Item_func_char_length::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  if (!res) {
    null_value = true;
    return 0; /* purecov: inspected */
  }
  null_value = false;
  return (longlong)res->numchars();
}

longlong Item_func_coercibility::val_int() {
  DBUG_ASSERT(fixed == 1);
  null_value = false;
  return (longlong)args[0]->collation.derivation;
}

bool Item_func_locate::resolve_type(THD *) {
  max_length = MY_INT32_NUM_DECIMAL_DIGITS;
  return agg_arg_charsets_for_comparison(cmp_collation, args, 2);
}

longlong Item_func_locate::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *a = args[0]->val_str(&value1);
  String *b = args[1]->val_str(&value2);
  if (!a || !b) {
    null_value = true;
    return 0; /* purecov: inspected */
  }
  null_value = false;
  /* must be longlong to avoid truncation */
  longlong start = 0;
  longlong start0 = 0;
  my_match_t match;

  if (arg_count == 3) {
    const longlong tmp = args[2]->val_int();
    if ((null_value = args[2]->null_value) || tmp <= 0) return 0;
    start0 = start = tmp - 1;

    if (start > static_cast<longlong>(a->length())) return 0;

    /* start is now sufficiently valid to pass to charpos function */
    start = a->charpos((int)start);

    if (start + b->length() > a->length()) return 0;
  }

  if (!b->length())  // Found empty string at start
    return start + 1;

  if (!cmp_collation.collation->coll->strstr(
          cmp_collation.collation, a->ptr() + start,
          (uint)(a->length() - start), b->ptr(), b->length(), &match, 1))
    return 0;
  return (longlong)match.mb_len + start0 + 1;
}

void Item_func_locate::print(const THD *thd, String *str,
                             enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("locate("));
  args[1]->print(thd, str, query_type);
  str->append(',');
  args[0]->print(thd, str, query_type);
  if (arg_count == 3) {
    str->append(',');
    args[2]->print(thd, str, query_type);
  }
  str->append(')');
}

longlong Item_func_validate_password_strength::val_int() {
  char buff[STRING_BUFFER_USUAL_SIZE];
  String value(buff, sizeof(buff), system_charset_info);
  String *field = args[0]->val_str(&value);
  if ((null_value = args[0]->null_value) || field->length() == 0) return 0;
  return (my_calculate_password_strength(field->ptr(), field->length()));
}

longlong Item_func_field::val_int() {
  DBUG_ASSERT(fixed == 1);

  if (cmp_type == STRING_RESULT) {
    String *field;
    if (!(field = args[0]->val_str(&value))) return 0;
    for (uint i = 1; i < arg_count; i++) {
      String *tmp_value = args[i]->val_str(&tmp);
      if (tmp_value && !sortcmp(field, tmp_value, cmp_collation.collation))
        return (longlong)(i);
    }
  } else if (cmp_type == INT_RESULT) {
    longlong val = args[0]->val_int();
    if (args[0]->null_value) return 0;
    for (uint i = 1; i < arg_count; i++) {
      if (val == args[i]->val_int() && !args[i]->null_value)
        return (longlong)(i);
    }
  } else if (cmp_type == DECIMAL_RESULT) {
    my_decimal dec_arg_buf, *dec_arg, dec_buf,
        *dec = args[0]->val_decimal(&dec_buf);
    if (args[0]->null_value) return 0;
    for (uint i = 1; i < arg_count; i++) {
      dec_arg = args[i]->val_decimal(&dec_arg_buf);
      if (!args[i]->null_value && !my_decimal_cmp(dec_arg, dec))
        return (longlong)(i);
    }
  } else {
    double val = args[0]->val_real();
    if (args[0]->null_value) return 0;
    for (uint i = 1; i < arg_count; i++) {
      if (val == args[i]->val_real() && !args[i]->null_value)
        return (longlong)(i);
    }
  }
  return 0;
}

bool Item_func_field::resolve_type(THD *) {
  maybe_null = false;
  max_length = 3;
  cmp_type = args[0]->result_type();
  for (uint i = 1; i < arg_count; i++)
    cmp_type = item_cmp_type(cmp_type, args[i]->result_type());
  if (cmp_type == STRING_RESULT)
    return agg_arg_charsets_for_comparison(cmp_collation, args, arg_count);
  return false;
}

longlong Item_func_ascii::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  if (!res) {
    null_value = true;
    return 0;
  }
  null_value = false;
  return (longlong)(res->length() ? (uchar)(*res)[0] : (uchar)0);
}

longlong Item_func_ord::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  if (!res) {
    null_value = true;
    return 0;
  }
  null_value = false;
  if (!res->length()) return 0;
  if (use_mb(res->charset())) {
    const char *str = res->ptr();
    uint32 n = 0, l = my_ismbchar(res->charset(), str, str + res->length());
    if (!l) return (longlong)((uchar)*str);
    while (l--) n = (n << 8) | (uint32)((uchar)*str++);
    return (longlong)n;
  }
  return (longlong)((uchar)(*res)[0]);
}

/* Search after a string in a string of strings separated by ',' */
/* Returns number of found type >= 1 or 0 if not found */
/* This optimizes searching in enums to bit testing! */

bool Item_func_find_in_set::resolve_type(THD *) {
  max_length = 3;  // 1-999

  if (args[0]->const_item() && args[1]->type() == FIELD_ITEM) {
    Field *field = ((Item_field *)args[1])->field;
    if (field->real_type() == MYSQL_TYPE_SET) {
      String *find = args[0]->val_str(&value);
      if (find) {
        // find is not NULL pointer so args[0] is not a null-value
        DBUG_ASSERT(!args[0]->null_value);
        enum_value = find_type(((Field_enum *)field)->typelib, find->ptr(),
                               find->length(), false);
        enum_bit = 0;
        if (enum_value) enum_bit = 1LL << (enum_value - 1);
      }
    }
  }
  return agg_arg_charsets_for_comparison(cmp_collation, args, 2);
}

static const char separator = ',';

longlong Item_func_find_in_set::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (enum_value) {
    // enum_value is set iff args[0]->const_item() in resolve_type().
    DBUG_ASSERT(args[0]->const_item());

    ulonglong tmp = (ulonglong)args[1]->val_int();
    null_value = args[1]->null_value;
    /*
      No need to check args[0]->null_value since enum_value is set iff
      args[0] is a non-null const item. Note: no DBUG_ASSERT on
      args[0]->null_value here because args[0] may have been replaced
      by an Item_cache on which val_int() has not been called. See
      BUG#11766317
    */
    if (!null_value) {
      if (tmp & enum_bit) return enum_value;
    }
    return 0L;
  }

  String *find = args[0]->val_str(&value);
  String *buffer = args[1]->val_str(&value2);
  if (!find || !buffer) {
    null_value = true;
    return 0; /* purecov: inspected */
  }
  null_value = false;

  if (buffer->length() >= find->length()) {
    my_wc_t wc = 0;
    const CHARSET_INFO *cs = cmp_collation.collation;
    const char *str_begin = buffer->ptr();
    const char *str_end = buffer->ptr();
    const char *real_end = str_end + buffer->length();
    const uchar *find_str = (const uchar *)find->ptr();
    size_t find_str_len = find->length();
    int position = 0;
    while (true) {
      int symbol_len;
      if ((symbol_len =
               cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(str_end),
                               pointer_cast<const uchar *>(real_end))) > 0) {
        const char *substr_end = str_end + symbol_len;
        bool is_last_item = (substr_end == real_end);
        bool is_separator = (wc == (my_wc_t)separator);
        if (is_separator || is_last_item) {
          position++;
          if (is_last_item && !is_separator) str_end = substr_end;
          if (!my_strnncoll(cs, (const uchar *)str_begin,
                            (uint)(str_end - str_begin), find_str,
                            find_str_len))
            return (longlong)position;
          else
            str_begin = substr_end;
        }
        str_end = substr_end;
      } else if (str_end - str_begin == 0 && find_str_len == 0 &&
                 wc == (my_wc_t)separator)
        return (longlong)++position;
      else
        return 0LL;
    }
  }
  return 0;
}

longlong Item_func_bit_count::val_int() {
  DBUG_ASSERT(fixed);
  if (bit_func_returns_binary(args[0], nullptr)) {
    String *s = args[0]->val_str(&str_value);
    if (args[0]->null_value || !s) return error_int();

    const char *val = s->ptr();

    longlong len = 0;
    size_t i = 0;
    size_t arg_length = s->length();
    while (i + sizeof(longlong) <= arg_length) {
      len += my_count_bits(uint8korr(&val[i]));
      i += sizeof(longlong);
    }
    while (i < arg_length) {
      len += _my_bits_nbits[(uchar)val[i]];
      i++;
    }

    null_value = false;
    return len;
  }

  ulonglong value = (ulonglong)args[0]->val_int();
  if (args[0]->null_value) return error_int(); /* purecov: inspected */

  null_value = false;
  return (longlong)my_count_bits(value);
}

/****************************************************************************
** Functions to handle dynamic loadable functions
****************************************************************************/

udf_handler::udf_handler(udf_func *udf_arg)
    : u_d(udf_arg),
      buffers(nullptr),
      error(0),
      is_null(0),
      initialized(false),
      m_args_extension(),
      m_return_value_extension(&my_charset_bin, result_type()),
      not_original(false) {}

udf_handler::~udf_handler() {
  /* Everything should be properly cleaned up by this moment. */
  DBUG_ASSERT(not_original || !(initialized || buffers));
}

void udf_handler::cleanup() {
  if (!not_original) {
    if (initialized) {
      if (u_d->func_deinit != nullptr) {
        Udf_func_deinit deinit = u_d->func_deinit;
        (*deinit)(&initid);
      }
      DEBUG_SYNC(current_thd, "udf_handler_cleanup_sync");
      free_udf(u_d);
      initialized = false;
    }
    if (buffers)  // Because of bug in ecc
      delete[] buffers;
    buffers = nullptr;
  }
}

void udf_handler::clear() {
  is_null = 0;
  Udf_func_clear func = u_d->func_clear;
  func(&initid, &is_null, &error);
}

void udf_handler::add(bool *null_value) {
  if (get_arguments()) {
    *null_value = 1;
    return;
  }
  Udf_func_add func = u_d->func_add;
  func(&initid, &f_args, &is_null, &error);
  *null_value = (bool)(is_null || error);
}

bool udf_handler::fix_fields(THD *thd, Item_result_field *func, uint arg_count,
                             Item **arguments) {
  uchar buff[STACK_BUFF_ALLOC];  // Max argument in function
  DBUG_TRACE;

  if (check_stack_overrun(thd, STACK_MIN_SIZE, buff))
    return true;  // Fatal error flag is set!

  udf_func *tmp_udf = find_udf(u_d->name.str, (uint)u_d->name.length, true);

  if (!tmp_udf) {
    my_error(ER_CANT_FIND_UDF, MYF(0), u_d->name.str);
    return true;
  }
  u_d = tmp_udf;
  args = arguments;

  /*
    RAII wrapper to free the memory allocated in case of any failure while
    initializing the UDF
  */
  class cleanup_guard {
   public:
    cleanup_guard(udf_func *udf_func) : m_udf_func(udf_func) {
      DBUG_ASSERT(udf_func);
    }
    ~cleanup_guard() {
      if (m_udf_func) free_udf(m_udf_func);
    }
    void defer() { m_udf_func = nullptr; }

   private:
    udf_func *m_udf_func;
  } udf_fun_guard(u_d);

  /* Fix all arguments */
  func->maybe_null = false;
  used_tables_cache = 0;

  if ((f_args.arg_count = arg_count)) {
    if (!(f_args.arg_type =
              (Item_result *)(*THR_MALLOC)
                  ->Alloc(f_args.arg_count * sizeof(Item_result)))) {
      return true;
    }
    uint i;
    Item **arg, **arg_end;
    for (i = 0, arg = arguments, arg_end = arguments + arg_count;
         arg != arg_end; arg++, i++) {
      if (!(*arg)->fixed && (*arg)->fix_fields(thd, arg)) {
        return true;
      }
      // we can't assign 'item' before, because fix_fields() can change arg
      Item *item = *arg;
      if (item->check_cols(1)) {
        return true;
      }
      /*
        TODO: We should think about this. It is not always
        right way just to set an UDF result to return my_charset_bin
        if one argument has binary sorting order.
        The result collation should be calculated according to arguments
        derivations in some cases and should not in other cases.
        Moreover, some arguments can represent a numeric input
        which doesn't effect the result character set and collation.
        There is no a general rule for UDF. Everything depends on
        the particular user defined function.
      */
      if (item->collation.collation->state & MY_CS_BINSORT)
        func->collation.set(&my_charset_bin);
      func->maybe_null |= item->maybe_null;
      func->add_accum_properties(item);
      used_tables_cache |= item->used_tables();
      f_args.arg_type[i] = item->result_type();
    }
    // TODO: why all following memory is not allocated with 1 call of sql_alloc?
    if (!(buffers = new (std::nothrow) String[arg_count]) ||
        !(f_args.args =
              (char **)(*THR_MALLOC)->Alloc(arg_count * sizeof(char *))) ||
        !(f_args.lengths =
              (ulong *)(*THR_MALLOC)->Alloc(arg_count * sizeof(long))) ||
        !(f_args.maybe_null =
              (char *)(*THR_MALLOC)->Alloc(arg_count * sizeof(char))) ||
        !(num_buffer = (char *)(*THR_MALLOC)
                           ->Alloc(arg_count * ALIGN_SIZE(sizeof(double)))) ||
        !(f_args.attributes =
              (char **)(*THR_MALLOC)->Alloc(arg_count * sizeof(char *))) ||
        !(f_args.attribute_lengths =
              (ulong *)(*THR_MALLOC)->Alloc(arg_count * sizeof(long))) ||
        !(m_args_extension.charset_info =
              (const CHARSET_INFO **)(*THR_MALLOC)
                  ->Alloc(f_args.arg_count * sizeof(CHARSET_INFO *)))) {
      return true;
    }
  }

  if (func->resolve_type(thd)) return true;

  initid.max_length = func->max_length;
  initid.maybe_null = func->maybe_null;
  initid.const_item = used_tables_cache == 0;
  initid.decimals = func->decimals;
  initid.ptr = nullptr;
  initid.extension = &m_return_value_extension;

  if (u_d->func_init) {
    char init_msg_buff[MYSQL_ERRMSG_SIZE];
    *init_msg_buff = '\0';
    char *to = num_buffer;
    f_args.extension = &m_args_extension;
    for (uint i = 0; i < arg_count; i++) {
      /*
       For a constant argument i, args->args[i] points to the argument value.
       For non-constant, args->args[i] is NULL.
      */
      f_args.args[i] = nullptr; /* Non-const unless updated below. */

      f_args.lengths[i] = arguments[i]->max_length;
      f_args.maybe_null[i] = arguments[i]->maybe_null;
      f_args.attributes[i] = const_cast<char *>(arguments[i]->item_name.ptr());
      f_args.attribute_lengths[i] = arguments[i]->item_name.length();
      m_args_extension.charset_info[i] = arguments[i]->collation.collation;

      if (arguments[i]->may_evaluate_const(thd)) {
        switch (arguments[i]->result_type()) {
          case STRING_RESULT:
          case DECIMAL_RESULT: {
            get_string(i);
            break;
          }
          case INT_RESULT:
            *((longlong *)to) = arguments[i]->val_int();
            if (arguments[i]->null_value) continue;
            f_args.args[i] = to;
            to += ALIGN_SIZE(sizeof(longlong));
            break;
          case REAL_RESULT:
            *((double *)to) = arguments[i]->val_real();
            if (arguments[i]->null_value) continue;
            f_args.args[i] = to;
            to += ALIGN_SIZE(sizeof(double));
            break;
          case ROW_RESULT:
          default:
            // This case should never be chosen
            DBUG_ASSERT(0);
            break;
        }
      }
    }
    Udf_func_init init = u_d->func_init;
    if ((error = (uchar)init(&initid, &f_args, init_msg_buff))) {
      my_error(ER_CANT_INITIALIZE_UDF, MYF(0), u_d->name.str, init_msg_buff);
      return true;
    }
    func->max_length = min<uint32>(initid.max_length, MAX_BLOB_WIDTH);
    func->maybe_null = initid.maybe_null;
    if (!initid.const_item && used_tables_cache == 0)
      used_tables_cache = RAND_TABLE_BIT;
    func->decimals = min<uint>(initid.decimals, DECIMAL_NOT_SPECIFIED);
    /*
      For UDFs of type string, override character set and collation from
      return value extension specification.
    */
    if (result_type() == STRING_RESULT)
      func->set_data_type_string(func->max_length,
                                 m_return_value_extension.charset_info);
  }
  initialized = true;
  /*
    UDF initialization complete so leave the freeing up resources to
    cleanup method.
  */
  udf_fun_guard.defer();
  return false;
}

bool udf_handler::get_arguments() {
  if (error) return true;  // Got an error earlier
  char *to = num_buffer;
  for (uint i = 0; i < f_args.arg_count; i++) {
    f_args.args[i] = nullptr;
    switch (f_args.arg_type[i]) {
      case STRING_RESULT:
        if (get_and_convert_string(i)) return true;
        break;
      case DECIMAL_RESULT:
        get_string(i);
        break;
      case INT_RESULT:
        *((longlong *)to) = args[i]->val_int();
        if (!args[i]->null_value) {
          f_args.args[i] = to;
          to += ALIGN_SIZE(sizeof(longlong));
        }
        break;
      case REAL_RESULT:
        *((double *)to) = args[i]->val_real();
        if (!args[i]->null_value) {
          f_args.args[i] = to;
          to += ALIGN_SIZE(sizeof(double));
        }
        break;
      case ROW_RESULT:
      default:
        // This case should never be chosen
        DBUG_ASSERT(0);
        break;
    }
  }
  return false;
}

double udf_handler::val_real(bool *null_value) {
  is_null = 0;
  if (get_arguments()) {
    *null_value = 1;
    return 0.0;
  }
  Udf_func_double func = (Udf_func_double)u_d->func;
  double tmp = func(&initid, &f_args, &is_null, &error);
  if (is_null || error) {
    *null_value = 1;
    return 0.0;
  }
  *null_value = 0;
  return tmp;
}
longlong udf_handler::val_int(bool *null_value) {
  is_null = 0;
  if (get_arguments()) {
    *null_value = 1;
    return 0LL;
  }
  Udf_func_longlong func = (Udf_func_longlong)u_d->func;
  longlong tmp = func(&initid, &f_args, &is_null, &error);
  if (is_null || error) {
    *null_value = 1;
    return 0LL;
  }
  *null_value = 0;
  return tmp;
}

/**
  @return
    (String*)NULL in case of NULL values
*/
String *udf_handler::val_str(String *str, String *save_str) {
  uchar is_null_tmp = 0;
  ulong res_length;
  DBUG_TRACE;

  if (get_arguments()) return nullptr;
  Udf_func_string func = reinterpret_cast<Udf_func_string>(u_d->func);

  if ((res_length = str->alloced_length()) <
      MAX_FIELD_WIDTH) {  // This happens VERY seldom
    if (str->alloc(MAX_FIELD_WIDTH)) {
      error = 1;
      return nullptr;
    }
  }
  char *res =
      func(&initid, &f_args, str->ptr(), &res_length, &is_null_tmp, &error);
  DBUG_PRINT("info", ("udf func returned, res_length: %lu", res_length));
  if (is_null_tmp || !res || error)  // The !res is for safety
  {
    DBUG_PRINT("info", ("Null or error"));
    return nullptr;
  }

  String *res_str = result_string(res, res_length, str, save_str);
  DBUG_PRINT("exit", ("res_str: %s", res_str->ptr()));
  return res_str;
}

/*
  For the moment, UDF functions are returning DECIMAL values as strings
*/

my_decimal *udf_handler::val_decimal(bool *null_value, my_decimal *dec_buf) {
  char buf[DECIMAL_MAX_STR_LENGTH + 1];
  ulong res_length = DECIMAL_MAX_STR_LENGTH;

  if (get_arguments()) {
    *null_value = true;
    return nullptr;
  }
  Udf_func_string func = reinterpret_cast<Udf_func_string>(u_d->func);

  char *res = func(&initid, &f_args, buf, &res_length, &is_null, &error);
  if (is_null || error) {
    *null_value = true;
    return nullptr;
  }
  const char *end = res + res_length;
  str2my_decimal(E_DEC_FATAL_ERROR, res, dec_buf, &end);
  return dec_buf;
}

/**
  Process the result string returned by the udf() method. The charset
  info might have changed therefore updates the same String. If user
  used the input String as result string then return the same.

  @param [in] res the result string returned by the udf() method.
  @param [in] res_length  length of the string
  @param [out] str The input result string passed to the udf() method
  @param [out] save_str Keeps copy of the returned String

  @returns A pointer to either the str or save_str that points
            to final result String
*/
String *udf_handler::result_string(const char *res, size_t res_length,
                                   String *str, String *save_str) {
  const auto *charset = m_return_value_extension.charset_info;
  String *res_str = nullptr;
  if (res == str->ptr()) {
    res_str = str;
    res_str->length(res_length);
    res_str->set_charset(charset);
  } else {
    res_str = save_str;
    res_str->set(res, res_length, charset);
  }
  return res_str;
}

/**
  Get the details of the input String arguments.

  @param [in] index of the argument to be looked in the arguments array
*/
void udf_handler::get_string(uint index) {
  String *res = args[index]->val_str(&buffers[index]);
  if (!args[index]->null_value) {
    f_args.args[index] = res->ptr();
    f_args.lengths[index] = res->length();
  } else {
    f_args.lengths[index] = 0;
  }
}

/**
  Get the details of the input String argument.
  If the charset of the input argument is not same as specified by the
  user then convert the String.

  @param [in] index of the argument to be looked in the arguments array

  @retval false Able to fetch the argument details
  @retval true  Otherwise
*/
bool udf_handler::get_and_convert_string(uint index) {
  String *res = args[index]->val_str(&buffers[index]);

  /* m_args_extension.charset_info[index] is a legitimate charset */
  if (res && res->charset() != m_args_extension.charset_info[index]) {
    String temp;
    uint dummy;
    if (temp.copy(res->ptr(), res->length(), res->charset(),
                  m_args_extension.charset_info[index], &dummy)) {
      return true;
    }
    *res = std::move(temp);
  }
  if (!args[index]->null_value) {
    f_args.args[index] = res->c_ptr_safe();
    f_args.lengths[index] = res->length();
  } else {
    f_args.lengths[index] = 0;
  }
  return false;
}

bool Item_udf_func::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_UDF);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

void Item_udf_func::cleanup() {
  udf.cleanup();
  Item_func::cleanup();
}

void Item_udf_func::print(const THD *thd, String *str,
                          enum_query_type query_type) const {
  str->append(func_name());
  str->append('(');
  for (uint i = 0; i < arg_count; i++) {
    if (i != 0) str->append(',');
    args[i]->print_item_w_name(thd, str, query_type);
  }
  str->append(')');
}

double Item_func_udf_float::val_real() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  DBUG_PRINT("info", ("result_type: %d  arg_count: %d", args[0]->result_type(),
                      arg_count));
  return udf.val_real(&null_value);
}

String *Item_func_udf_float::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  double nr = val_real();
  if (null_value) return nullptr; /* purecov: inspected */
  str->set_real(nr, decimals, &my_charset_bin);
  return str;
}

longlong Item_func_udf_int::val_int() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  return udf.val_int(&null_value);
}

String *Item_func_udf_int::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  longlong nr = val_int();
  if (null_value) return nullptr;
  str->set_int(nr, unsigned_flag, &my_charset_bin);
  return str;
}

longlong Item_func_udf_decimal::val_int() {
  my_decimal dec_buf, *dec = udf.val_decimal(&null_value, &dec_buf);
  longlong result;
  if (null_value) return 0;
  my_decimal2int(E_DEC_FATAL_ERROR, dec, unsigned_flag, &result);
  return result;
}

double Item_func_udf_decimal::val_real() {
  my_decimal dec_buf, *dec = udf.val_decimal(&null_value, &dec_buf);
  double result;
  if (null_value) return 0.0;
  my_decimal2double(E_DEC_FATAL_ERROR, dec, &result);
  return result;
}

my_decimal *Item_func_udf_decimal::val_decimal(my_decimal *dec_buf) {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  DBUG_PRINT("info", ("result_type: %d  arg_count: %d", args[0]->result_type(),
                      arg_count));

  return udf.val_decimal(&null_value, dec_buf);
}

String *Item_func_udf_decimal::val_str(String *str) {
  my_decimal dec_buf, *dec = udf.val_decimal(&null_value, &dec_buf);
  if (null_value) return nullptr;
  if (str->length() < DECIMAL_MAX_STR_LENGTH)
    str->length(DECIMAL_MAX_STR_LENGTH);
  my_decimal_round(E_DEC_FATAL_ERROR, dec, decimals, false, &dec_buf);
  my_decimal2string(E_DEC_FATAL_ERROR, &dec_buf, str);
  return str;
}

bool Item_func_udf_decimal::resolve_type(THD *) {
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  fix_num_length_and_dec();
  return false;
}

/* Default max_length is max argument length */

bool Item_func_udf_str::resolve_type(THD *) {
  uint result_length = 0;
  for (uint i = 0; i < arg_count; i++)
    result_length = max(result_length, args[i]->max_length);
  // If the UDF has an init function, this may be overridden later.
  set_data_type_string(result_length, &my_charset_bin);
  return false;
}

String *Item_func_udf_str::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = udf.val_str(str, &str_value);
  null_value = !res;
  return res;
}

bool Item_master_pos_wait::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

/**
  Wait until we are at or past the given position in the master binlog
  on the slave.
*/

longlong Item_master_pos_wait::val_int() {
  DBUG_ASSERT(fixed == 1);
  THD *thd = current_thd;
  String *log_name = args[0]->val_str(&value);
  int event_count = 0;

  null_value = false;
  if (thd->slave_thread || !log_name || !log_name->length()) {
    null_value = true;
    return 0;
  }
  Master_info *mi;
  longlong pos = (ulong)args[1]->val_int();
  double timeout = (arg_count >= 3) ? args[2]->val_real() : 0;
  if (timeout < 0) {
    if (thd->is_strict_sql_mode()) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "MASTER_POS_WAIT.");
    } else {
      thd->really_audit_instrumented_event =
          thd->variables.audit_instrumented_event;
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_WRONG_ARGUMENTS,
                          ER_THD(thd, ER_WRONG_ARGUMENTS), "MASTER_POS_WAIT.");
      thd->really_audit_instrumented_event = 0;
      null_value = true;
    }
    return 0;
  }

  channel_map.rdlock();

  if (arg_count == 4) {
    String *channel_str;
    if (!(channel_str = args[3]->val_str(&value))) {
      null_value = true;
      return 0;
    }

    mi = channel_map.get_mi(channel_str->ptr());

  } else {
    if (channel_map.get_num_instances() > 1) {
      mi = nullptr;
      my_error(ER_SLAVE_MULTIPLE_CHANNELS_CMD, MYF(0));
    } else
      mi = channel_map.get_default_channel_mi();
  }

  if (mi != nullptr) mi->inc_reference();

  channel_map.unlock();

  if (mi == nullptr || (event_count = mi->rli->wait_for_pos(thd, log_name, pos,
                                                            timeout)) == -2) {
    null_value = true;
    event_count = 0;
  }

  if (mi != nullptr) mi->dec_reference();
  return event_count;
}

bool Item_wait_for_executed_gtid_set::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  /*
    It is unsafe because the return value depends on timing. If the timeout
    happens, the return value is different from the one in which the function
    returns with success.
  */
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

/**
  Wait until the given gtid_set is found in the executed gtid_set independent
  of the slave threads.
*/
longlong Item_wait_for_executed_gtid_set::val_int() {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);
  THD *thd = current_thd;
  String *gtid_text = args[0]->val_str(&value);

  null_value = false;

  if (gtid_text == nullptr) {
    my_error(ER_MALFORMED_GTID_SET_SPECIFICATION, MYF(0), "NULL");
    return 0;
  }

  // Waiting for a GTID in a slave thread could cause the slave to
  // hang/deadlock.
  if (thd->slave_thread) {
    null_value = true;
    return 0;
  }

  Gtid_set wait_for_gtid_set(global_sid_map, nullptr);

  global_sid_lock->rdlock();
  if (get_gtid_mode(GTID_MODE_LOCK_SID) == GTID_MODE_OFF) {
    global_sid_lock->unlock();
    my_error(ER_GTID_MODE_OFF, MYF(0), "use WAIT_FOR_EXECUTED_GTID_SET");
    null_value = true;
    return 0;
  }

  if (wait_for_gtid_set.add_gtid_text(gtid_text->c_ptr_safe()) !=
      RETURN_STATUS_OK) {
    global_sid_lock->unlock();
    // Error has already been generated.
    return 1;
  }

  // Cannot wait for a GTID that the thread owns since that would
  // immediately deadlock.
  if (thd->owned_gtid.sidno > 0 &&
      wait_for_gtid_set.contains_gtid(thd->owned_gtid)) {
    char buf[Gtid::MAX_TEXT_LENGTH + 1];
    thd->owned_gtid.to_string(global_sid_map, buf);
    global_sid_lock->unlock();
    my_error(ER_CANT_WAIT_FOR_EXECUTED_GTID_SET_WHILE_OWNING_A_GTID, MYF(0),
             buf);
    return 0;
  }

  gtid_state->begin_gtid_wait(GTID_MODE_LOCK_SID);

  double timeout = (arg_count == 2) ? args[1]->val_real() : 0;
  if (timeout < 0) {
    if (thd->is_strict_sql_mode()) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "WAIT_FOR_EXECUTED_GTID_SET.");
    } else {
      thd->really_audit_instrumented_event =
          thd->variables.audit_instrumented_event;
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_WRONG_ARGUMENTS,
                          ER_THD(thd, ER_WRONG_ARGUMENTS),
                          "WAIT_FOR_EXECUTED_GTID_SET.");
      thd->really_audit_instrumented_event = 0;
      null_value = true;
    }
    gtid_state->end_gtid_wait();
    global_sid_lock->unlock();
    return 0;
  }

  bool result = gtid_state->wait_for_gtid_set(thd, &wait_for_gtid_set, timeout);
  global_sid_lock->unlock();
  gtid_state->end_gtid_wait();

  return result;
}

Item_master_gtid_set_wait::Item_master_gtid_set_wait(const POS &pos, Item *a)
    : Item_int_func(pos, a) {
  push_deprecated_warn(current_thd, "WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS",
                       "WAIT_FOR_EXECUTED_GTID_SET");
}

Item_master_gtid_set_wait::Item_master_gtid_set_wait(const POS &pos, Item *a,
                                                     Item *b)
    : Item_int_func(pos, a, b) {
  push_deprecated_warn(current_thd, "WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS",
                       "WAIT_FOR_EXECUTED_GTID_SET");
}

Item_master_gtid_set_wait::Item_master_gtid_set_wait(const POS &pos, Item *a,
                                                     Item *b, Item *c)
    : Item_int_func(pos, a, b, c) {
  push_deprecated_warn(current_thd, "WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS",
                       "WAIT_FOR_EXECUTED_GTID_SET");
}

bool Item_master_gtid_set_wait::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

longlong Item_master_gtid_set_wait::val_int() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  int event_count = 0;

  null_value = false;

  String *gtid = args[0]->val_str(&value);
  THD *thd = current_thd;
  Master_info *mi = nullptr;
  double timeout = (arg_count >= 2) ? args[1]->val_real() : 0;
  if (timeout < 0) {
    if (thd->is_strict_sql_mode()) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0),
               "WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS.");
    } else {
      thd->really_audit_instrumented_event =
          thd->variables.audit_instrumented_event;
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_WRONG_ARGUMENTS,
                          ER_THD(thd, ER_WRONG_ARGUMENTS),
                          "WAIT_UNTIL_SQL_THREAD_AFTER_GTIDS.");
      thd->really_audit_instrumented_event = 0;
      null_value = true;
    }
    return 0;
  }

  if (thd->slave_thread || !gtid) {
    null_value = true;
    return 0;
  }

  channel_map.rdlock();

  /* If replication channel is mentioned */
  if (arg_count == 3) {
    String *channel_str;
    if (!(channel_str = args[2]->val_str(&value))) {
      channel_map.unlock();
      null_value = true;
      return 0;
    }
    mi = channel_map.get_mi(channel_str->ptr());
  } else {
    if (channel_map.get_num_instances() > 1) {
      channel_map.unlock();
      mi = nullptr;
      my_error(ER_SLAVE_MULTIPLE_CHANNELS_CMD, MYF(0));
      return 0;
    } else
      mi = channel_map.get_default_channel_mi();
  }

  if (get_gtid_mode(GTID_MODE_LOCK_CHANNEL_MAP) == GTID_MODE_OFF) {
    null_value = true;
    channel_map.unlock();
    return 0;
  }
  gtid_state->begin_gtid_wait(GTID_MODE_LOCK_CHANNEL_MAP);

  if (mi) mi->inc_reference();

  channel_map.unlock();

  if (mi && mi->rli) {
    event_count = mi->rli->wait_for_gtid_set(thd, gtid, timeout);
    if (event_count == -2) {
      null_value = true;
      event_count = 0;
    }
  } else
    /*
      Replication has not been set up, we should return NULL;
     */
    null_value = true;

  if (mi != nullptr) mi->dec_reference();

  gtid_state->end_gtid_wait();

  return event_count;
}

/**
  Return 1 if both arguments are Gtid_sets and the first is a subset
  of the second.  Generate an error if any of the arguments is not a
  Gtid_set.
*/
longlong Item_func_gtid_subset::val_int() {
  DBUG_TRACE;
  if (args[0]->null_value || args[1]->null_value) {
    null_value = true;
    return 0;
  }
  String *string1, *string2;
  const char *charp1, *charp2;
  int ret = 1;
  enum_return_status status;
  // get strings without lock
  if ((string1 = args[0]->val_str(&buf1)) != nullptr &&
      (charp1 = string1->c_ptr_safe()) != nullptr &&
      (string2 = args[1]->val_str(&buf2)) != nullptr &&
      (charp2 = string2->c_ptr_safe()) != nullptr) {
    Sid_map sid_map(nullptr /*no rwlock*/);
    // compute sets while holding locks
    const Gtid_set sub_set(&sid_map, charp1, &status);
    if (status == RETURN_STATUS_OK) {
      const Gtid_set super_set(&sid_map, charp2, &status);
      if (status == RETURN_STATUS_OK)
        ret = sub_set.is_subset(&super_set) ? 1 : 0;
    }
  }
  return ret;
}

/**
  Enables a session to wait on a condition until a timeout or a network
  disconnect occurs.

  @remark The connection is polled every m_interrupt_interval nanoseconds.
*/

class Interruptible_wait {
  THD *m_thd;
  struct timespec m_abs_timeout;
  static const ulonglong m_interrupt_interval;

 public:
  Interruptible_wait(THD *thd) : m_thd(thd) {}

  ~Interruptible_wait() {}

 public:
  /**
    Set the absolute timeout.

    @param timeout The amount of time in nanoseconds to wait
  */
  void set_timeout(ulonglong timeout) {
    /*
      Calculate the absolute system time at the start so it can
      be controlled in slices. It relies on the fact that once
      the absolute time passes, the timed wait call will fail
      automatically with a timeout error.
    */
    set_timespec_nsec(&m_abs_timeout, timeout);
  }

  /** The timed wait. */
  int wait(mysql_cond_t *, mysql_mutex_t *);
};

/** Time to wait before polling the connection status. */
const ulonglong Interruptible_wait::m_interrupt_interval = 5 * 1000000000ULL;

/**
  Wait for a given condition to be signaled.

  @param cond   The condition variable to wait on.
  @param mutex  The associated mutex.

  @remark The absolute timeout is preserved across calls.

  @retval return value from mysql_cond_timedwait
*/

int Interruptible_wait::wait(mysql_cond_t *cond, mysql_mutex_t *mutex) {
  int error;
  struct timespec timeout;

  while (true) {
    /* Wait for a fixed interval. */
    set_timespec_nsec(&timeout, m_interrupt_interval);

    /* But only if not past the absolute timeout. */
    if (cmp_timespec(&timeout, &m_abs_timeout) > 0) timeout = m_abs_timeout;

    error = mysql_cond_timedwait(cond, mutex, &timeout);
    if (is_timeout(error)) {
      /* Return error if timed out or connection is broken. */
      if (!cmp_timespec(&timeout, &m_abs_timeout) || !m_thd->is_connected())
        break;
    }
    /* Otherwise, propagate status to the caller. */
    else
      break;
  }

  return error;
}

/*
  User-level locks implementation.
*/

/**
  For locks with EXPLICIT duration, MDL returns a new ticket
  every time a lock is granted. This allows to implement recursive
  locks without extra allocation or additional data structures, such
  as below. However, if there are too many tickets in the same
  MDL_context, MDL_context::find_ticket() is getting too slow,
  since it's using a linear search.
  This is why a separate structure is allocated for a user
  level lock held by connection, and before requesting a new lock from MDL,
  GET_LOCK() checks thd->ull_hash if such lock is already granted,
  and if so, simply increments a reference counter.
*/

struct User_level_lock {
  MDL_ticket *ticket;
  uint refs;
};

/**
  Release all user level locks for this THD.
*/

void mysql_ull_cleanup(THD *thd) {
  DBUG_TRACE;

  for (const auto &key_and_value : thd->ull_hash) {
    User_level_lock *ull = key_and_value.second;
    thd->mdl_context.release_lock(ull->ticket);
    my_free(ull);
  }

  thd->ull_hash.clear();
}

/**
  Set explicit duration for metadata locks corresponding to
  user level locks to protect them from being released at the end
  of transaction.
*/

void mysql_ull_set_explicit_lock_duration(THD *thd) {
  DBUG_TRACE;

  for (const auto &key_and_value : thd->ull_hash) {
    User_level_lock *ull = key_and_value.second;
    thd->mdl_context.set_lock_duration(ull->ticket, MDL_EXPLICIT);
  }
}

/**
  When MDL detects a lock wait timeout, it pushes an error into the statement
  diagnostics area. For GET_LOCK(), lock wait timeout is not an error, but a
  special return value (0). NULL is returned in case of error. Capture and
  suppress lock wait timeout.
  We also convert ER_LOCK_DEADLOCK error to ER_USER_LOCK_DEADLOCK error.
  The former means that implicit rollback of transaction has occurred
  which doesn't (and should not) happen when we get deadlock while waiting
  for user-level lock.
*/

class User_level_lock_wait_error_handler : public Internal_error_handler {
 public:
  User_level_lock_wait_error_handler() : m_lock_wait_timeout(false) {}

  bool got_timeout() const { return m_lock_wait_timeout; }

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_LOCK_WAIT_TIMEOUT) {
      m_lock_wait_timeout = true;
      return true;
    } else if (sql_errno == ER_LOCK_DEADLOCK) {
      my_error(ER_USER_LOCK_DEADLOCK, MYF(0));
      return true;
    }

    return false;
  }

 private:
  bool m_lock_wait_timeout;
};

class MDL_lock_get_owner_thread_id_visitor : public MDL_context_visitor {
 public:
  MDL_lock_get_owner_thread_id_visitor() : m_owner_id(0) {}

  void visit_context(const MDL_context *ctx) {
    m_owner_id = ctx->get_owner()->get_thd()->thread_id();
  }

  my_thread_id get_owner_id() const { return m_owner_id; }

 private:
  my_thread_id m_owner_id;
};

/**
  Helper function which checks if user-level lock name is acceptable
  and converts it to system charset (utf8). Error is emitted if name
  is not acceptable. Name is also lowercased to ensure that user-level
  lock names are treated in case-insensitive fashion even though MDL
  subsystem which used by implementation does binary comparison of keys.

  @param buff      Buffer for lowercased name in system charset of
                   NAME_LEN + 1 bytes length.
  @param org_name  Original string passed as name parameter to
                   user-level lock function.

  @return True in case of error, false on success.
*/

static bool check_and_convert_ull_name(char *buff, const String *org_name) {
  if (!org_name || !org_name->length()) {
    my_error(ER_USER_LOCK_WRONG_NAME, MYF(0), (org_name ? "" : "NULL"));
    return true;
  }

  const char *well_formed_error_pos;
  const char *cannot_convert_error_pos;
  const char *from_end_pos;
  size_t bytes_copied;

  bytes_copied = well_formed_copy_nchars(
      system_charset_info, buff, NAME_LEN, org_name->charset(), org_name->ptr(),
      org_name->length(), NAME_CHAR_LEN, &well_formed_error_pos,
      &cannot_convert_error_pos, &from_end_pos);

  if (well_formed_error_pos || cannot_convert_error_pos ||
      from_end_pos < org_name->ptr() + org_name->length()) {
    ErrConvString err(org_name);
    my_error(ER_USER_LOCK_WRONG_NAME, MYF(0), err.ptr());
    return true;
  }

  buff[bytes_copied] = '\0';

  my_casedn_str(system_charset_info, buff);

  return false;
}

bool Item_func_get_lock::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/**
  Get a user level lock.

  @note Sets null_value to true on error.

  @note This means that SQL-function GET_LOCK() returns:
        1    - if lock was acquired.
        0    - if lock was not acquired due to timeout.
        NULL - in case of error such as bad lock name, deadlock,
               thread being killed (also error is emitted).

  @retval
    1    : Got lock
  @retval
    0    : Timeout, error.
*/

longlong Item_func_get_lock::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  ulonglong timeout = args[1]->val_int();
  char name[NAME_LEN + 1];
  THD *thd = current_thd;
  DBUG_TRACE;

  null_value = true;
  /*
    In slave thread no need to get locks, everything is serialized. Anyway
    there is no way to make GET_LOCK() work on slave like it did on master
    (i.e. make it return exactly the same value) because we don't have the
    same other concurrent threads environment. No matter what we return here,
    it's not guaranteed to be same as on master. So we always return 1.
  */
  if (thd->slave_thread) {
    null_value = false;
    return 1;
  }

  if (check_and_convert_ull_name(name, res)) return 0;

  DBUG_PRINT("info", ("lock %s, thd=%lu", name, (ulong)thd->real_id));

  /*
    Convert too big and negative timeout values to INT_MAX32.
    This gives robust, "infinite" wait on all platforms.
  */
  if (timeout > INT_MAX32) timeout = INT_MAX32;

  MDL_request ull_request;
  MDL_REQUEST_INIT(&ull_request, MDL_key::USER_LEVEL_LOCK, "", name,
                   MDL_EXCLUSIVE, MDL_EXPLICIT);
  std::string ull_key(pointer_cast<const char *>(ull_request.key.ptr()),
                      ull_request.key.length());

  const auto it = thd->ull_hash.find(ull_key);
  if (it != thd->ull_hash.end()) {
    /* Recursive lock. */
    it->second->refs++;
    null_value = false;
    return 1;
  }

  User_level_lock_wait_error_handler error_handler;

  thd->push_internal_handler(&error_handler);
  bool error = thd->mdl_context.acquire_lock_nsec(
      &ull_request, static_cast<ulonglong>(timeout) * 1000000000ULL);
  (void)thd->pop_internal_handler();

  if (error) {
    /*
      Return 0 in case of timeout and NULL in case of deadlock/other
      errors. In the latter case error (e.g. ER_USER_LOCK_DEADLOCK)
      will be reported as well.
    */
    if (error_handler.got_timeout()) null_value = false;
    return 0;
  }

  User_level_lock *ull = nullptr;
  ull = reinterpret_cast<User_level_lock *>(
      my_malloc(key_memory_User_level_lock, sizeof(User_level_lock), MYF(0)));

  if (ull == nullptr) {
    thd->mdl_context.release_lock(ull_request.ticket);
    return 0;
  }

  ull->ticket = ull_request.ticket;
  ull->refs = 1;

  thd->ull_hash.emplace(ull_key, ull);
  null_value = false;

  return 1;
}

bool Item_func_release_lock::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/**
  Release a user level lock.

  @note Sets null_value to true on error/if no connection holds such lock.

  @note This means that SQL-function RELEASE_LOCK() returns:
        1    - if lock was held by this connection and was released.
        0    - if lock was held by some other connection (and was not released).
        NULL - if name of lock is bad or if it was not held by any connection
               (in the former case also error will be emitted),

  @return
    - 1 if lock released
    - 0 if lock wasn't held/error.
*/

longlong Item_func_release_lock::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  char name[NAME_LEN + 1];
  THD *thd = current_thd;
  DBUG_TRACE;

  null_value = true;

  if (check_and_convert_ull_name(name, res)) return 0;

  DBUG_PRINT("info", ("lock %s", name));

  MDL_key ull_key;
  ull_key.mdl_key_init(MDL_key::USER_LEVEL_LOCK, "", name);

  const auto it = thd->ull_hash.find(
      std::string(pointer_cast<const char *>(ull_key.ptr()), ull_key.length()));
  if (it == thd->ull_hash.end()) {
    /*
      When RELEASE_LOCK() is called for lock which is not owned by the
      connection it should return 0 or NULL depending on whether lock
      is owned by any other connection or not.
    */
    MDL_lock_get_owner_thread_id_visitor get_owner_visitor;

    if (thd->mdl_context.find_lock_owner(&ull_key, &get_owner_visitor))
      return 0;

    null_value = get_owner_visitor.get_owner_id() == 0;

    return 0;
  }
  User_level_lock *ull = it->second;

  null_value = false;
  if (--ull->refs == 0) {
    thd->ull_hash.erase(it);
    thd->mdl_context.release_lock(ull->ticket);
    my_free(ull);
  }
  return 1;
}

bool Item_func_release_all_locks::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/**
  Release all user level lock held by connection.

  @return Number of locks released including recursive lock count.
*/

longlong Item_func_release_all_locks::val_int() {
  DBUG_ASSERT(fixed == 1);
  THD *thd = current_thd;
  uint result = 0;
  DBUG_TRACE;

  for (const auto &key_and_value : thd->ull_hash) {
    User_level_lock *ull = key_and_value.second;
    thd->mdl_context.release_lock(ull->ticket);
    result += ull->refs;
    my_free(ull);
  }
  thd->ull_hash.clear();

  return result;
}

bool Item_func_is_free_lock::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/**
  Check if user level lock is free.

  @note Sets null_value=true on error.

  @note As result SQL-function IS_FREE_LOCK() returns:
        1    - if lock is free,
        0    - if lock is in use
        NULL - if lock name is bad or OOM (also error is emitted).

  @retval
    1		Available
  @retval
    0		Already taken, or error
*/

longlong Item_func_is_free_lock::val_int() {
  DBUG_ASSERT(fixed == 1);
  value.length(0);
  String *res = args[0]->val_str(&value);
  char name[NAME_LEN + 1];
  THD *thd = current_thd;

  null_value = true;

  if (check_and_convert_ull_name(name, res)) return 0;

  MDL_key ull_key;
  ull_key.mdl_key_init(MDL_key::USER_LEVEL_LOCK, "", name);

  MDL_lock_get_owner_thread_id_visitor get_owner_visitor;

  if (thd->mdl_context.find_lock_owner(&ull_key, &get_owner_visitor)) return 0;

  null_value = false;
  return (get_owner_visitor.get_owner_id() == 0);
}

bool Item_func_is_used_lock::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/**
  Check if user level lock is used and return connection id of owner.

  @note Sets null_value=true if lock is free/on error.

  @note SQL-function IS_USED_LOCK() returns:
        #    - connection id of lock owner if lock is acquired.
        NULL - if lock is free or on error (in the latter case
               also error is emitted).

  @return Connection id of lock owner, 0 if lock is free/on error.
*/

longlong Item_func_is_used_lock::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  char name[NAME_LEN + 1];
  THD *thd = current_thd;

  null_value = true;

  if (check_and_convert_ull_name(name, res)) return 0;

  MDL_key ull_key;
  ull_key.mdl_key_init(MDL_key::USER_LEVEL_LOCK, "", name);

  MDL_lock_get_owner_thread_id_visitor get_owner_visitor;

  if (thd->mdl_context.find_lock_owner(&ull_key, &get_owner_visitor)) return 0;

  my_thread_id thread_id = get_owner_visitor.get_owner_id();
  if (thread_id == 0) return 0;

  null_value = false;
  return thread_id;
}

bool Item_func_last_insert_id::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->safe_to_cache_query = false;
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

longlong Item_func_last_insert_id::val_int() {
  THD *thd = current_thd;
  DBUG_ASSERT(fixed == 1);
  if (arg_count) {
    longlong value = args[0]->val_int();
    null_value = args[0]->null_value;
    /*
      LAST_INSERT_ID(X) must affect the client's mysql_insert_id() as
      documented in the manual. We don't want to touch
      first_successful_insert_id_in_cur_stmt because it would make
      LAST_INSERT_ID(X) take precedence over an generated auto_increment
      value for this row.
    */
    thd->arg_of_last_insert_id_function = true;
    thd->first_successful_insert_id_in_prev_stmt = value;
    return value;
  }
  return static_cast<longlong>(
      thd->read_first_successful_insert_id_in_prev_stmt());
}

bool Item_func_benchmark::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/* This function is just used to test speed of different functions */

longlong Item_func_benchmark::val_int() {
  DBUG_ASSERT(fixed == 1);
  char buff[MAX_FIELD_WIDTH];
  String tmp(buff, sizeof(buff), &my_charset_bin);
  my_decimal tmp_decimal;
  THD *thd = current_thd;
  ulonglong loop_count;

  loop_count = (ulonglong)args[0]->val_int();

  if (args[0]->null_value ||
      (!args[0]->unsigned_flag && (((longlong)loop_count) < 0))) {
    if (!args[0]->null_value) {
      char errbuff[22];
      llstr(((longlong)loop_count), errbuff);
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_WRONG_VALUE_FOR_TYPE,
                          ER_THD(current_thd, ER_WRONG_VALUE_FOR_TYPE), "count",
                          errbuff, "benchmark");
    }

    null_value = true;
    return 0;
  }

  null_value = false;
  for (ulonglong loop = 0; loop < loop_count && !thd->killed; loop++) {
    switch (args[1]->result_type()) {
      case REAL_RESULT:
        (void)args[1]->val_real();
        break;
      case INT_RESULT:
        (void)args[1]->val_int();
        break;
      case STRING_RESULT:
        (void)args[1]->val_str(&tmp);
        break;
      case DECIMAL_RESULT:
        (void)args[1]->val_decimal(&tmp_decimal);
        break;
      case ROW_RESULT:
      default:
        // This case should never be chosen
        DBUG_ASSERT(0);
        return 0;
    }
  }
  return 0;
}

void Item_func_benchmark::print(const THD *thd, String *str,
                                enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("benchmark("));
  args[0]->print(thd, str, query_type);
  str->append(',');
  args[1]->print(thd, str, query_type);
  str->append(')');
}

/**
  Lock which is used to implement interruptible wait for SLEEP() function.
*/

mysql_mutex_t LOCK_item_func_sleep;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_item_func_sleep;

static PSI_mutex_info item_func_sleep_mutexes[] = {
    {&key_LOCK_item_func_sleep, "LOCK_item_func_sleep", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static void init_item_func_sleep_psi_keys() {
  int count;

  count = static_cast<int>(array_elements(item_func_sleep_mutexes));
  mysql_mutex_register("sql", item_func_sleep_mutexes, count);
}
#endif

static bool item_func_sleep_inited = false;

void item_func_sleep_init() {
#ifdef HAVE_PSI_INTERFACE
  init_item_func_sleep_psi_keys();
#endif

  mysql_mutex_init(key_LOCK_item_func_sleep, &LOCK_item_func_sleep,
                   MY_MUTEX_INIT_SLOW);
  item_func_sleep_inited = true;
}

void item_func_sleep_free() {
  if (item_func_sleep_inited) {
    item_func_sleep_inited = false;
    mysql_mutex_destroy(&LOCK_item_func_sleep);
  }
}

bool Item_func_sleep::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

/** This function is just used to create tests with time gaps. */

longlong Item_func_sleep::val_int() {
  THD *thd = current_thd;
  Interruptible_wait timed_cond(thd);
  mysql_cond_t cond;
  double timeout;
  int error;

  DBUG_ASSERT(fixed == 1);

  timeout = args[0]->val_real();

  /*
    Report error or warning depending on the value of SQL_MODE.
    If SQL is STRICT then report error, else report warning and continue
    execution.
  */

  if (args[0]->null_value || timeout < 0) {
    if (!thd->lex->is_ignore() && thd->is_strict_sql_mode()) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "sleep.");
      return 0;
    } else
      thd->really_audit_instrumented_event =
          thd->variables.audit_instrumented_event;
    push_warning_printf(thd, Sql_condition::SL_WARNING, ER_WRONG_ARGUMENTS,
                        ER_THD(thd, ER_WRONG_ARGUMENTS), "sleep.");
    thd->really_audit_instrumented_event = 0;
  }
  /*
    On 64-bit OSX mysql_cond_timedwait() waits forever
    if passed abstime time has already been exceeded by
    the system time.
    When given a very short timeout (< 10 mcs) just return
    immediately.
    We assume that the lines between this test and the call
    to mysql_cond_timedwait() will be executed in less than 0.00001 sec.
  */
  if (timeout < 0.00001) return 0;

  timed_cond.set_timeout((ulonglong)(timeout * 1000000000.0));

  mysql_cond_init(key_item_func_sleep_cond, &cond);
  mysql_mutex_lock(&LOCK_item_func_sleep);

  thd->ENTER_COND(&cond, &LOCK_item_func_sleep, &stage_user_sleep, nullptr);

  error = 0;
  thd_wait_begin(thd, THD_WAIT_SLEEP);
  while (!thd->killed) {
    error = timed_cond.wait(&cond, &LOCK_item_func_sleep);
    if (is_timeout(error)) break;
    error = 0;
  }
  mysql_mutex_unlock(&LOCK_item_func_sleep);
  thd->EXIT_COND(nullptr);

  thd_wait_end(thd);

  mysql_cond_destroy(&cond);

  return (error == 0);  // Return 1 killed
}

/*
  @param cs  character set; IF we are creating the user_var_entry,
             we give it this character set.
*/
static user_var_entry *get_variable(THD *thd, const Name_string &name,
                                    const CHARSET_INFO *cs) {
  const std::string key(name.ptr(), name.length());

  /* Protects thd->user_vars. */
  mysql_mutex_assert_owner(&thd->LOCK_thd_data);

  user_var_entry *entry = find_or_nullptr(thd->user_vars, key);
  if (entry == nullptr && cs != nullptr) {
    entry = user_var_entry::create(thd, name, cs);
    if (entry == nullptr) return nullptr;
    thd->user_vars.emplace(
        key, unique_ptr_with_deleter<user_var_entry>(entry, &free_user_var));
  }
  return entry;
}

void Item_func_set_user_var::cleanup() {
  Item_func::cleanup();
  entry = nullptr;
}

bool Item_func_set_user_var::set_entry(THD *thd, bool create_if_not_exists) {
  if (entry && thd->thread_id() == entry_thread_id) {
  }  // update entry->update_query_id for PS
  else {
    const CHARSET_INFO *cs =
        create_if_not_exists
            ? (args[0]->collation.derivation == DERIVATION_NUMERIC
                   ? default_charset()
                   : args[0]->collation.collation)
            : nullptr;

    /* Protects thd->user_vars. */
    mysql_mutex_lock(&thd->LOCK_thd_data);
    entry = get_variable(thd, name, cs);
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    if (entry == nullptr) {
      entry_thread_id = 0;
      return true;
    }
    entry_thread_id = thd->thread_id();
  }
  /*
    Remember the last query which updated it, this way a query can later know
    if this variable is a constant item in the query (it is if update_query_id
    is different from query_id).

    If this object has delayed setting of non-constness, we delay this
    until Item_func_set-user_var::save_item_result().
  */
  if (!delayed_non_constness) entry->update_query_id = thd->query_id;
  return false;
}

/*
  When a user variable is updated (in a SET command or a query like
  SELECT @a:= ).
*/

bool Item_func_set_user_var::fix_fields(THD *thd, Item **ref) {
  DBUG_ASSERT(fixed == 0);
  // fix_fields will call Item_func_set_user_var::resolve_type()
  if (Item_func::fix_fields(thd, ref) || set_entry(thd, true)) return true;

  null_item = (args[0]->type() == NULL_ITEM);

  cached_result_type = args[0]->result_type();
  return false;
}

bool Item_func_set_user_var::resolve_type(THD *) {
  maybe_null = args[0]->maybe_null;
  decimals = args[0]->decimals;
  collation.set(DERIVATION_IMPLICIT);
  /*
     this sets the character set of the item immediately; rules for the
     character set of the variable ("entry" object) are different: if "entry"
     did not exist previously, set_entry () has created it and has set its
     character set; but if it existed previously, it keeps its previous
     character set, which may change only when we are sure that the assignment
     is to be executed, i.e. in user_var_entry::store ().
  */
  if (args[0]->collation.derivation == DERIVATION_NUMERIC)
    collation.collation = default_charset();
  else
    collation.collation = args[0]->collation.collation;

  enum_field_types data_type = Item::type_for_variable(args[0]->data_type());
  switch (data_type) {
    case MYSQL_TYPE_LONGLONG:
      set_data_type_longlong();
      max_length =
          args[0]->max_length;  // Preserves "length" of integer constants
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      set_data_type_decimal(args[0]->decimal_precision(), args[0]->decimals);
      break;
    case MYSQL_TYPE_DOUBLE:
      set_data_type_double();
      break;
    case MYSQL_TYPE_VARCHAR:
      set_data_type_string(args[0]->max_char_length());
      break;
    case MYSQL_TYPE_NULL:
    default:
      DBUG_ASSERT(false);
      set_data_type(MYSQL_TYPE_NULL);
      break;
  }

  unsigned_flag = args[0]->unsigned_flag;

  return false;
}

// static
user_var_entry *user_var_entry::create(THD *thd, const Name_string &name,
                                       const CHARSET_INFO *cs) {
  if (check_column_name(name.ptr())) {
    my_error(ER_ILLEGAL_USER_VAR, MYF(0), name.ptr());
    return nullptr;
  }

  user_var_entry *entry;
  size_t size =
      ALIGN_SIZE(sizeof(user_var_entry)) + (name.length() + 1) + extra_size;
  if (!(entry = (user_var_entry *)my_malloc(key_memory_user_var_entry, size,
                                            MYF(MY_WME | ME_FATALERROR))))
    return nullptr;
  entry->init(thd, name, cs);
  return entry;
}

bool user_var_entry::mem_realloc(size_t length) {
  if (length <= extra_size) {
    /* Enough space to store value in value struct */
    free_value();
    m_ptr = internal_buffer_ptr();
  } else {
    /* Allocate an external buffer */
    if (m_length != length) {
      if (m_ptr == internal_buffer_ptr()) m_ptr = nullptr;
      if (!(m_ptr = (char *)my_realloc(
                key_memory_user_var_entry_value, m_ptr, length,
                MYF(MY_ALLOW_ZERO_PTR | MY_WME | ME_FATALERROR))))
        return true;
    }
  }
  return false;
}

void user_var_entry::init(THD *thd, const Simple_cstring &name,
                          const CHARSET_INFO *cs) {
  DBUG_ASSERT(thd != nullptr);
  m_owner = thd;
  copy_name(name);
  reset_value();
  update_query_id = 0;
  collation.set(cs, DERIVATION_IMPLICIT, 0);
  unsigned_flag = false;
  /*
    If we are here, we were called from a SET or a query which sets a
    variable. Imagine it is this:
    INSERT INTO t SELECT @a:=10, @a:=@a+1.
    Then when we have a Item_func_get_user_var (because of the @a+1) so we
    think we have to write the value of @a to the binlog. But before that,
    we have a Item_func_set_user_var to create @a (@a:=10), in this we mark
    the variable as "already logged" (line below) so that it won't be logged
    by Item_func_get_user_var (because that's not necessary).
  */
  used_query_id = thd->query_id;
  m_type = STRING_RESULT;
}

/**
  Set value to user variable.
  @param from           pointer to buffer with new value
  @param length         length of new value
  @param type           type of new value

  @retval  false   on success
  @retval  true    on allocation error

*/
bool user_var_entry::store(const void *from, size_t length, Item_result type) {
  assert_locked();

  // Store strings with end \0
  if (mem_realloc(length + (type == STRING_RESULT))) return true;
  if (type == STRING_RESULT) m_ptr[length] = 0;  // Store end \0

  // Avoid memcpy of a my_decimal object, use copy CTOR instead.
  if (type == DECIMAL_RESULT) {
    DBUG_ASSERT(length == sizeof(my_decimal));
    const my_decimal *dec = static_cast<const my_decimal *>(from);
    dec->sanity_check();
    new (m_ptr) my_decimal(*dec);
  } else
    memcpy(m_ptr, from, length);

  m_length = length;
  m_type = type;
  return false;
}

void user_var_entry::assert_locked() const {
  mysql_mutex_assert_owner(&m_owner->LOCK_thd_data);
}

/**
  Set value to user variable.

  @param ptr            pointer to buffer with new value
  @param length         length of new value
  @param type           type of new value
  @param cs             charset info for new value
  @param dv             derivation for new value
  @param unsigned_arg   indiates if a value of type INT_RESULT is unsigned

  @note Sets error and fatal error if allocation fails.

  @retval
    false   success
  @retval
    true    failure
*/

bool user_var_entry::store(const void *ptr, size_t length, Item_result type,
                           const CHARSET_INFO *cs, Derivation dv,
                           bool unsigned_arg) {
  assert_locked();

  if (store(ptr, length, type)) return true;
  collation.set(cs, dv);
  unsigned_flag = unsigned_arg;
  return false;
}

void user_var_entry::lock() {
  DBUG_ASSERT(m_owner != nullptr);
  mysql_mutex_lock(&m_owner->LOCK_thd_data);
}

void user_var_entry::unlock() {
  DBUG_ASSERT(m_owner != nullptr);
  mysql_mutex_unlock(&m_owner->LOCK_thd_data);
}

bool Item_func_set_user_var::update_hash(const void *ptr, uint length,
                                         Item_result res_type,
                                         const CHARSET_INFO *cs, Derivation dv,
                                         bool unsigned_arg) {
  entry->lock();

  /*
    If we set a variable explicitely to NULL then keep the old
    result type of the variable
  */
  // args[0]->null_value could be outdated
  if (args[0]->type() == Item::FIELD_ITEM)
    null_value = ((Item_field *)args[0])->field->is_null();
  else
    null_value = args[0]->null_value;

  if (ptr == nullptr) {
    DBUG_ASSERT(length == 0);
    null_value = true;
  }

  if (null_value && null_item)
    res_type = entry->type();  // Don't change type of item

  if (null_value)
    entry->set_null_value(res_type);
  else if (entry->store(ptr, length, res_type, cs, dv, unsigned_arg)) {
    entry->unlock();
    null_value = true;
    return true;
  }
  entry->unlock();
  return false;
}

/** Get the value of a variable as a double. */

double user_var_entry::val_real(bool *null_value) const {
  if ((*null_value = (m_ptr == nullptr))) return 0.0;

  switch (m_type) {
    case REAL_RESULT:
      return *(double *)m_ptr;
    case INT_RESULT:
      return (double)*(longlong *)m_ptr;
    case DECIMAL_RESULT: {
      double result;
      my_decimal2double(E_DEC_FATAL_ERROR, (my_decimal *)m_ptr, &result);
      return result;
    }
    case STRING_RESULT:
      return my_atof(m_ptr);  // This is null terminated
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(false);  // Impossible
      break;
  }
  return 0.0;  // Impossible
}

/** Get the value of a variable as an integer. */

longlong user_var_entry::val_int(bool *null_value) const {
  if ((*null_value = (m_ptr == nullptr))) return 0LL;

  switch (m_type) {
    case REAL_RESULT:
      return (longlong) * (double *)m_ptr;
    case INT_RESULT:
      return *(longlong *)m_ptr;
    case DECIMAL_RESULT: {
      longlong result;
      my_decimal2int(E_DEC_FATAL_ERROR, (my_decimal *)m_ptr, false, &result);
      return result;
    }
    case STRING_RESULT: {
      int error;
      return my_strtoll10(m_ptr, nullptr,
                          &error);  // String is null terminated
    }
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(false);  // Impossible
      break;
  }
  return 0LL;  // Impossible
}

/** Get the value of a variable as a string. */

String *user_var_entry::val_str(bool *null_value, String *str,
                                uint decimals) const {
  if ((*null_value = (m_ptr == nullptr))) return (String *)nullptr;

  switch (m_type) {
    case REAL_RESULT:
      str->set_real(*(double *)m_ptr, decimals, collation.collation);
      break;
    case INT_RESULT:
      if (!unsigned_flag)
        str->set(*(longlong *)m_ptr, collation.collation);
      else
        str->set(*(ulonglong *)m_ptr, collation.collation);
      break;
    case DECIMAL_RESULT:
      str_set_decimal(E_DEC_FATAL_ERROR, pointer_cast<my_decimal *>(m_ptr), str,
                      collation.collation);
      break;
    case STRING_RESULT:
      if (str->copy(m_ptr, m_length, collation.collation))
        str = nullptr;  // EOM error
      break;
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(false);  // Impossible
      break;
  }
  return (str);
}

/** Get the value of a variable as a decimal. */

my_decimal *user_var_entry::val_decimal(bool *null_value,
                                        my_decimal *val) const {
  if ((*null_value = (m_ptr == nullptr))) return nullptr;

  switch (m_type) {
    case REAL_RESULT:
      double2my_decimal(E_DEC_FATAL_ERROR, *(double *)m_ptr, val);
      break;
    case INT_RESULT:
      int2my_decimal(E_DEC_FATAL_ERROR, *(longlong *)m_ptr, false, val);
      break;
    case DECIMAL_RESULT:
      my_decimal2decimal((my_decimal *)m_ptr, val);
      break;
    case STRING_RESULT:
      str2my_decimal(E_DEC_FATAL_ERROR, m_ptr, m_length, collation.collation,
                     val);
      break;
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(false);  // Impossible
      break;
  }
  return (val);
}

/**
  This functions is invoked on SET \@variable or
  \@variable:= expression.

  Evaluate (and check expression), store results.

  @note
    For now it always return OK. All problem with value evaluating
    will be caught by thd->is_error() check in sql_set_variables().

  @retval
    false OK.
*/

bool Item_func_set_user_var::check(bool use_result_field) {
  DBUG_TRACE;
  if (use_result_field && !result_field) use_result_field = false;

  switch (cached_result_type) {
    case REAL_RESULT: {
      save_result.vreal =
          use_result_field ? result_field->val_real() : args[0]->val_real();
      break;
    }
    case INT_RESULT: {
      save_result.vint =
          use_result_field ? result_field->val_int() : args[0]->val_int();
      unsigned_flag = use_result_field
                          ? ((Field_num *)result_field)->unsigned_flag
                          : args[0]->unsigned_flag;
      break;
    }
    case STRING_RESULT: {
      save_result.vstr = use_result_field ? result_field->val_str(&value)
                                          : args[0]->val_str(&value);
      break;
    }
    case DECIMAL_RESULT: {
      save_result.vdec = use_result_field
                             ? result_field->val_decimal(&decimal_buff)
                             : args[0]->val_decimal(&decimal_buff);
      break;
    }
    case ROW_RESULT:
    default:
      // This case should never be chosen
      DBUG_ASSERT(0);
      break;
  }
  return false;
}

/**
  @brief Evaluate and store item's result.
  This function is invoked on "SELECT ... INTO @var ...".

  @param    item    An item to get value from.
*/

void Item_func_set_user_var::save_item_result(Item *item) {
  DBUG_TRACE;

  switch (cached_result_type) {
    case REAL_RESULT:
      save_result.vreal = item->val_real();
      break;
    case INT_RESULT:
      save_result.vint = item->val_int();
      unsigned_flag = item->unsigned_flag;
      break;
    case STRING_RESULT:
      save_result.vstr = item->val_str(&value);
      break;
    case DECIMAL_RESULT:
      save_result.vdec = item->val_decimal(&decimal_buff);
      break;
    case ROW_RESULT:
    default:
      // Should never happen
      DBUG_ASSERT(0);
      break;
  }
  /*
    Set the ID of the query that last updated this variable. This is
    usually set by Item_func_set_user_var::set_entry(), but if this
    item has delayed setting of non-constness, we must do it now.
   */
  if (delayed_non_constness) entry->update_query_id = current_thd->query_id;
}

/**
  This functions is invoked on
  SET \@variable or \@variable:= expression.

  @note
    We have to store the expression as such in the variable, independent of
    the value method used by the user

  @retval
    0	OK
  @retval
    1	EOM Error

*/

bool Item_func_set_user_var::update() {
  bool res = false;
  DBUG_TRACE;

  switch (cached_result_type) {
    case REAL_RESULT: {
      res = update_hash(&save_result.vreal, sizeof(save_result.vreal),
                        REAL_RESULT, default_charset(), DERIVATION_IMPLICIT,
                        false);
      break;
    }
    case INT_RESULT: {
      res = update_hash(&save_result.vint, sizeof(save_result.vint), INT_RESULT,
                        default_charset(), DERIVATION_IMPLICIT, unsigned_flag);
      break;
    }
    case STRING_RESULT: {
      if (!save_result.vstr)  // Null value
        res = update_hash(nullptr, 0, STRING_RESULT, &my_charset_bin,
                          DERIVATION_IMPLICIT, false);
      else
        res = update_hash(save_result.vstr->ptr(), save_result.vstr->length(),
                          STRING_RESULT, save_result.vstr->charset(),
                          DERIVATION_IMPLICIT, false);
      break;
    }
    case DECIMAL_RESULT: {
      if (!save_result.vdec)  // Null value
        res = update_hash(nullptr, 0, DECIMAL_RESULT, &my_charset_bin,
                          DERIVATION_IMPLICIT, false);
      else
        res = update_hash(save_result.vdec, sizeof(my_decimal), DECIMAL_RESULT,
                          default_charset(), DERIVATION_IMPLICIT, false);
      break;
    }
    case ROW_RESULT:
    default:
      // This case should never be chosen
      DBUG_ASSERT(0);
      break;
  }
  return res;
}

double Item_func_set_user_var::val_real() {
  DBUG_ASSERT(fixed == 1);
  check(false);
  update();  // Store expression
  return entry->val_real(&null_value);
}

longlong Item_func_set_user_var::val_int() {
  DBUG_ASSERT(fixed == 1);
  check(false);
  update();  // Store expression
  return entry->val_int(&null_value);
}

String *Item_func_set_user_var::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  check(false);
  update();  // Store expression
  return entry->val_str(&null_value, str, decimals);
}

my_decimal *Item_func_set_user_var::val_decimal(my_decimal *val) {
  DBUG_ASSERT(fixed == 1);
  check(false);
  update();  // Store expression
  return entry->val_decimal(&null_value, val);
}

// just the assignment, for use in "SET @a:=5" type self-prints
void Item_func_set_user_var::print_assignment(
    const THD *thd, String *str, enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("@"));
  str->append(name);
  str->append(STRING_WITH_LEN(":="));
  args[0]->print(thd, str, query_type);
}

// parenthesize assignment for use in "EXPLAIN EXTENDED SELECT (@e:=80)+5"
void Item_func_set_user_var::print(const THD *thd, String *str,
                                   enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("("));
  print_assignment(thd, str, query_type);
  str->append(STRING_WITH_LEN(")"));
}

bool Item_func_set_user_var::send(Protocol *protocol, String *str_arg) {
  if (result_field) {
    check(true);
    update();
    /*
      TODO This func have to be changed to avoid sending data as a field.
    */
    return protocol->store_field(result_field);
  }
  return Item::send(protocol, str_arg);
}

void Item_func_set_user_var::make_field(Send_field *tmp_field) {
  if (result_field) {
    result_field->make_send_field(tmp_field);
    DBUG_ASSERT(tmp_field->table_name != nullptr);
    if (Item::item_name.is_set())
      tmp_field->col_name = Item::item_name.ptr();  // Use user supplied name
  } else
    Item::make_field(tmp_field);
}

/*
  Save the value of a user variable into a field

  SYNOPSIS
    save_in_field()
      field           target field to save the value to
      no_conversion   flag indicating whether conversions are allowed

  DESCRIPTION
    Save the function value into a field and update the user variable
    accordingly. If a result field is defined and the target field doesn't
    coincide with it then the value from the result field will be used as
    the new value of the user variable.

    The reason to have this method rather than simply using the result
    field in the val_xxx() methods is that the value from the result field
    not always can be used when the result field is defined.
    Let's consider the following cases:
    1) when filling a tmp table the result field is defined but the value of it
    is undefined because it has to be produced yet. Thus we can't use it.
    2) on execution of an INSERT ... SELECT statement the save_in_field()
    function will be called to fill the data in the new record. If the SELECT
    part uses a tmp table then the result field is defined and should be
    used in order to get the correct result.

    The difference between the SET_USER_VAR function and regular functions
    like CONCAT is that the Item_func objects for the regular functions are
    replaced by Item_field objects after the values of these functions have
    been stored in a tmp table. Yet an object of the Item_field class cannot
    be used to update a user variable.
    Due to this we have to handle the result field in a special way here and
    in the Item_func_set_user_var::send() function.

  RETURN VALUES
    TYPE_OK            Ok
    Everything else    Error
*/

type_conversion_status Item_func_set_user_var::save_in_field(
    Field *field, bool no_conversions, bool can_use_result_field) {
  bool use_result_field =
      (!can_use_result_field ? 0 : (result_field && result_field != field));
  type_conversion_status error;

  /* Update the value of the user variable */
  check(use_result_field);
  update();

  if (result_type() == STRING_RESULT ||
      (result_type() == REAL_RESULT && field->result_type() == STRING_RESULT)) {
    String *result;
    const CHARSET_INFO *cs = collation.collation;
    char buff[MAX_FIELD_WIDTH];  // Alloc buffer for small columns
    str_value.set_quick(buff, sizeof(buff), cs);
    result = entry->val_str(&null_value, &str_value, decimals);

    if (null_value) {
      str_value.set_quick(nullptr, 0, cs);
      return set_field_to_null_with_conversions(field, no_conversions);
    }

    /* NOTE: If null_value == false, "result" must be not NULL.  */

    field->set_notnull();
    error = field->store(result->ptr(), result->length(), cs);
    str_value.set_quick(nullptr, 0, cs);
  } else if (result_type() == REAL_RESULT) {
    double nr = entry->val_real(&null_value);
    if (null_value) return set_field_to_null(field);
    field->set_notnull();
    error = field->store(nr);
  } else if (result_type() == DECIMAL_RESULT) {
    my_decimal decimal_value;
    my_decimal *val = entry->val_decimal(&null_value, &decimal_value);
    if (null_value) return set_field_to_null(field);
    field->set_notnull();
    error = field->store_decimal(val);
  } else {
    longlong nr = entry->val_int(&null_value);
    if (null_value)
      return set_field_to_null_with_conversions(field, no_conversions);
    field->set_notnull();
    error = field->store(nr, unsigned_flag);
  }
  return error;
}

String *Item_func_get_user_var::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  if (!var_entry) return error_str();  // No such variable
  String *res = var_entry->val_str(&null_value, str, decimals);
  if (res && !my_charset_same(res->charset(), collation.collation)) {
    String tmpstr;
    uint error;
    if (tmpstr.copy(res->ptr(), res->length(), res->charset(),
                    collation.collation, &error) ||
        error > 0) {
      char tmp[32];
      convert_to_printable(tmp, sizeof(tmp), res->ptr(), res->length(),
                           res->charset(), 6);
      my_error(ER_INVALID_CHARACTER_STRING, MYF(0), collation.collation->csname,
               tmp);
      return error_str();
    }
    if (str->copy(tmpstr)) return error_str();
    return str;
  }
  return res;
}

double Item_func_get_user_var::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (!var_entry) return 0.0;  // No such variable
  return (var_entry->val_real(&null_value));
}

my_decimal *Item_func_get_user_var::val_decimal(my_decimal *dec) {
  DBUG_ASSERT(fixed == 1);
  if (!var_entry) return nullptr;
  return var_entry->val_decimal(&null_value, dec);
}

longlong Item_func_get_user_var::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (!var_entry) return 0LL;  // No such variable
  return (var_entry->val_int(&null_value));
}

/**
  Get variable by name and, if necessary, put the record of variable
  use into the binary log.

  When a user variable is invoked from an update query (INSERT, UPDATE etc),
  stores this variable and its value in thd->user_var_events, so that it can be
  written to the binlog (will be written just before the query is written, see
  log.cc).

  @param      thd         Current session.
  @param      sql_command The command the variable participates in.
  @param      name        Variable name
  @param[out] out_entry  variable structure or NULL. The pointer is set
                         regardless of whether function succeeded or not.

  @retval
    0  OK
  @retval
    1  Failed to put appropriate record into binary log

*/

static int get_var_with_binlog(THD *thd, enum_sql_command sql_command,
                               Name_string &name, user_var_entry **out_entry) {
  Binlog_user_var_event *user_var_event;
  user_var_entry *var_entry;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&thd->LOCK_thd_data);
  var_entry = get_variable(thd, name, nullptr);
  mysql_mutex_unlock(&thd->LOCK_thd_data);

  /*
    Any reference to user-defined variable which is done from stored
    function or trigger affects their execution and the execution of the
    calling statement. We must log all such variables even if they are
    not involved in table-updating statements.
  */
  if (!(opt_bin_log && (is_update_query(sql_command) || thd->in_sub_stmt))) {
    *out_entry = var_entry;
    return 0;
  }

  if (!var_entry) {
    /*
      If the variable does not exist, it's NULL, but we want to create it so
      that it gets into the binlog (if it didn't, the slave could be
      influenced by a variable of the same name previously set by another
      thread).
      We create it like if it had been explicitly set with SET before.
      The 'new' mimics what sql_yacc.yy does when 'SET @a=10;'.
      sql_set_variables() is what is called from 'case SQLCOM_SET_OPTION'
      in dispatch_command()). Instead of building a one-element list to pass to
      sql_set_variables(), we could instead manually call check() and update();
      this would save memory and time; but calling sql_set_variables() makes
      one unique place to maintain (sql_set_variables()).

      Manipulation with lex is necessary since free_underlaid_joins
      is going to release memory belonging to the main query.
    */

    List<set_var_base> tmp_var_list;
    LEX *sav_lex = thd->lex, lex_tmp;
    thd->lex = &lex_tmp;
    lex_start(thd);
    tmp_var_list.push_back(new (thd->mem_root) set_var_user(
        new Item_func_set_user_var(name, new Item_null(), false)));
    /* Create the variable */
    if (sql_set_variables(thd, &tmp_var_list, false)) {
      thd->lex = sav_lex;
      goto err;
    }
    thd->lex = sav_lex;
    mysql_mutex_lock(&thd->LOCK_thd_data);
    var_entry = get_variable(thd, name, nullptr);
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    if (var_entry == nullptr) goto err;
  } else if (var_entry->used_query_id == thd->query_id ||
             mysql_bin_log.is_query_in_union(thd, var_entry->used_query_id)) {
    /*
       If this variable was already stored in user_var_events by this query
       (because it's used in more than one place in the query), don't store
       it.
    */
    *out_entry = var_entry;
    return 0;
  }

  size_t size;
  /*
    First we need to store value of var_entry, when the next situation
    appears:
    > set @a:=1;
    > insert into t1 values (@a), (@a:=@a+1), (@a:=@a+1);
    We have to write to binlog value @a= 1.

    We allocate the user_var_event on user_var_events_alloc pool, not on
    the this-statement-execution pool because in SPs user_var_event objects
    may need to be valid after current [SP] statement execution pool is
    destroyed.
  */
  size = ALIGN_SIZE(sizeof(Binlog_user_var_event)) + var_entry->length();
  if (!(user_var_event =
            (Binlog_user_var_event *)thd->user_var_events_alloc->Alloc(size)))
    goto err;

  user_var_event->value =
      (char *)user_var_event + ALIGN_SIZE(sizeof(Binlog_user_var_event));
  user_var_event->user_var_event = var_entry;
  user_var_event->type = var_entry->type();
  user_var_event->charset_number = var_entry->collation.collation->number;
  user_var_event->unsigned_flag = var_entry->unsigned_flag;
  if (!var_entry->ptr()) {
    /* NULL value*/
    user_var_event->length = 0;
    user_var_event->value = nullptr;
  } else {
    // Avoid memcpy of a my_decimal object, use copy CTOR instead.
    user_var_event->length = var_entry->length();
    if (user_var_event->type == DECIMAL_RESULT) {
      DBUG_ASSERT(var_entry->length() == sizeof(my_decimal));
      const my_decimal *dec = static_cast<const my_decimal *>(
          static_cast<const void *>(var_entry->ptr()));
      dec->sanity_check();
      new (user_var_event->value) my_decimal(*dec);
    } else
      memcpy(user_var_event->value, var_entry->ptr(), var_entry->length());
  }
  /* Mark that this variable has been used by this query */
  var_entry->used_query_id = thd->query_id;
  if (thd->user_var_events.push_back(user_var_event)) goto err;

  *out_entry = var_entry;
  return 0;

err:
  *out_entry = var_entry;
  return 1;
}

bool Item_func_get_user_var::resolve_type(THD *thd) {
  maybe_null = true;
  decimals = DECIMAL_NOT_SPECIFIED;
  max_length = MAX_BLOB_WIDTH;

  used_tables_cache =
      thd->lex->locate_var_assignment(name) ? RAND_TABLE_BIT : 0;

  if (get_var_with_binlog(thd, thd->lex->sql_command, name, &var_entry))
    return true;

  /*
    If the variable didn't exist it has been created as a STRING-type.
    'var_entry' is NULL only if there occurred an error during the call to
    get_var_with_binlog.
  */
  if (var_entry) {
    m_cached_result_type = var_entry->type();
    unsigned_flag = var_entry->unsigned_flag;
    max_length = var_entry->length();

    collation.set(var_entry->collation);
    switch (m_cached_result_type) {
      case REAL_RESULT:
        set_data_type(MYSQL_TYPE_DOUBLE);
        max_length = DBL_DIG + 8;
        break;
      case INT_RESULT:
        set_data_type(MYSQL_TYPE_LONGLONG);
        max_length = MAX_BIGINT_WIDTH;
        decimals = 0;
        break;
      case STRING_RESULT:
        set_data_type_string(uint32(MAX_BLOB_WIDTH - 1));
        DBUG_ASSERT(data_type() != MYSQL_TYPE_VAR_STRING);
        if (data_type() == MYSQL_TYPE_VAR_STRING)
          set_data_type(MYSQL_TYPE_VARCHAR);
        break;
      case DECIMAL_RESULT:
        set_data_type(MYSQL_TYPE_NEWDECIMAL);
        max_length = DECIMAL_MAX_STR_LENGTH;
        decimals = DECIMAL_MAX_SCALE;
        break;
      case ROW_RESULT:  // Keep compiler happy
      default:
        DBUG_ASSERT(0);
        break;
    }
  } else {
    collation.set(&my_charset_bin, DERIVATION_IMPLICIT);
    null_value = true;
    m_cached_result_type = STRING_RESULT;
    set_data_type_string(uint32(MAX_BLOB_WIDTH));
    DBUG_ASSERT(data_type() != MYSQL_TYPE_VAR_STRING);
    if (data_type() == MYSQL_TYPE_VAR_STRING) set_data_type(MYSQL_TYPE_VARCHAR);
  }

  return false;
}

enum Item_result Item_func_get_user_var::result_type() const {
  return m_cached_result_type;
}

void Item_func_get_user_var::print(const THD *thd, String *str,
                                   enum_query_type) const {
  str->append(STRING_WITH_LEN("(@"));
  append_identifier(thd, str, name.ptr(), name.length());
  str->append(')');
}

bool Item_func_get_user_var::eq(const Item *item, bool) const {
  /* Assume we don't have rtti */
  if (this == item) return true;  // Same item is same.
  /* Check if other type is also a get_user_var() object */
  if (item->type() != FUNC_ITEM ||
      down_cast<const Item_func *>(item)->functype() != functype())
    return false;
  const Item_func_get_user_var *other =
      down_cast<const Item_func_get_user_var *>(item);
  return name.eq_bin(other->name);
}

bool Item_func_get_user_var::set_value(THD *thd, sp_rcontext * /*ctx*/,
                                       Item **it) {
  Item_func_set_user_var *suv = new Item_func_set_user_var(name, *it, false);
  /*
    Item_func_set_user_var is not fixed after construction, call
    fix_fields().
  */
  return (!suv || suv->fix_fields(thd, it) || suv->check(false) ||
          suv->update());
}

bool Item_user_var_as_out_param::fix_fields(THD *thd, Item **ref) {
  DBUG_ASSERT(fixed == 0);

  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_LOAD);
  auto exchange_cs =
      down_cast<Sql_cmd_load_table *>(thd->lex->m_sql_cmd)->m_exchange.cs;
  /*
    Let us set the same collation which is used for loading
    of fields in LOAD DATA INFILE.
    (Since Item_user_var_as_out_param is used only there).
  */
  const CHARSET_INFO *cs =
      exchange_cs ? exchange_cs : thd->variables.collation_database;

  if (Item::fix_fields(thd, ref)) return true;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&thd->LOCK_thd_data);
  entry = get_variable(thd, name, cs);
  if (entry != nullptr) {
    entry->set_type(STRING_RESULT);
    entry->update_query_id = thd->query_id;
  }
  mysql_mutex_unlock(&thd->LOCK_thd_data);

  if (entry == nullptr) return true;

  return false;
}

void Item_user_var_as_out_param::set_null_value(const CHARSET_INFO *) {
  entry->lock();
  entry->set_null_value(STRING_RESULT);
  entry->unlock();
}

void Item_user_var_as_out_param::set_value(const char *str, size_t length,
                                           const CHARSET_INFO *cs) {
  entry->lock();
  entry->store(str, length, STRING_RESULT, cs, DERIVATION_IMPLICIT,
               false /* unsigned_arg */);
  entry->unlock();
}

double Item_user_var_as_out_param::val_real() {
  DBUG_ASSERT(0);
  return 0.0;
}

longlong Item_user_var_as_out_param::val_int() {
  DBUG_ASSERT(0);
  return 0;
}

String *Item_user_var_as_out_param::val_str(String *) {
  DBUG_ASSERT(0);
  return nullptr;
}

my_decimal *Item_user_var_as_out_param::val_decimal(my_decimal *) {
  DBUG_ASSERT(0);
  return nullptr;
}

void Item_user_var_as_out_param::print(const THD *thd, String *str,
                                       enum_query_type) const {
  str->append('@');
  append_identifier(thd, str, name.ptr(), name.length());
}

Item_func_get_system_var::Item_func_get_system_var(sys_var *var_arg,
                                                   enum_var_type var_type_arg,
                                                   LEX_STRING *component_arg,
                                                   const char *name_arg,
                                                   size_t name_len_arg)
    : var(var_arg),
      var_type(var_type_arg),
      orig_var_type(var_type_arg),
      component(*component_arg),
      cache_present(0) {
  /* copy() will allocate the name */
  item_name.copy(name_arg, (uint)name_len_arg);
}

bool Item_func_get_system_var::is_written_to_binlog() {
  return var->is_written_to_binlog(var_type);
}

bool Item_func_get_system_var::resolve_type(THD *thd) {
  const char *cptr;
  maybe_null = true;

  if (!var->check_scope(var_type)) {
    if (var_type != OPT_DEFAULT) {
      my_error(ER_INCORRECT_GLOBAL_LOCAL_VAR, MYF(0), var->name.str,
               var_type == OPT_GLOBAL ? "SESSION" : "GLOBAL");
      return true;
    }
    /* As there was no local variable, return the global value */
    var_type = OPT_GLOBAL;
  }

  switch (var->show_type()) {
    case SHOW_LONG:
    case SHOW_INT:
    case SHOW_HA_ROWS:
    case SHOW_LONGLONG:
      collation.set_numeric();
      set_data_type(MYSQL_TYPE_LONGLONG);
      max_length = MY_INT64_NUM_DECIMAL_DIGITS;
      unsigned_flag = true;
      break;
    case SHOW_SIGNED_INT:
    case SHOW_SIGNED_LONG:
    case SHOW_SIGNED_LONGLONG:
      collation.set_numeric();
      set_data_type(MYSQL_TYPE_LONGLONG);
      max_length = MY_INT64_NUM_DECIMAL_DIGITS;
      unsigned_flag = false;
      break;
    case SHOW_CHAR:
    case SHOW_CHAR_PTR:
      set_data_type(MYSQL_TYPE_VARCHAR);
      mysql_mutex_lock(&LOCK_global_system_variables);
      cptr = var->show_type() == SHOW_CHAR
                 ? pointer_cast<const char *>(
                       var->value_ptr(thd, var_type, &component))
                 : *pointer_cast<const char *const *>(
                       var->value_ptr(thd, var_type, &component));
      if (cptr)
        max_length = system_charset_info->cset->numchars(
            system_charset_info, cptr, cptr + strlen(cptr));
      mysql_mutex_unlock(&LOCK_global_system_variables);
      collation.set(system_charset_info, DERIVATION_SYSCONST);
      max_length *= system_charset_info->mbmaxlen;
      decimals = DECIMAL_NOT_SPECIFIED;
      break;
    case SHOW_LEX_STRING: {
      set_data_type(MYSQL_TYPE_VARCHAR);
      mysql_mutex_lock(&LOCK_global_system_variables);
      const LEX_STRING *ls = pointer_cast<const LEX_STRING *>(
          var->value_ptr(thd, var_type, &component));
      max_length = system_charset_info->cset->numchars(
          system_charset_info, ls->str, ls->str + ls->length);
      mysql_mutex_unlock(&LOCK_global_system_variables);
      collation.set(system_charset_info, DERIVATION_SYSCONST);
      max_length *= system_charset_info->mbmaxlen;
      decimals = DECIMAL_NOT_SPECIFIED;
    } break;
    case SHOW_BOOL:
    case SHOW_MY_BOOL:
      collation.set_numeric();
      set_data_type(MYSQL_TYPE_LONGLONG);
      unsigned_flag = false;
      max_length = 1;
      break;
    case SHOW_TIMER:
    case SHOW_DOUBLE:
      collation.set_numeric();
      set_data_type(MYSQL_TYPE_DOUBLE);
      unsigned_flag = false;
      decimals = 6;
      max_length = DBL_DIG + 6;
      break;
    default:
      my_error(ER_VAR_CANT_BE_READ, MYF(0), var->name.str);
      return true;
  }
  return false;
}

void Item_func_get_system_var::print(const THD *, String *str,
                                     enum_query_type) const {
  str->append(item_name);
}

enum Item_result Item_func_get_system_var::result_type() const {
  switch (var->show_type()) {
    case SHOW_BOOL:
    case SHOW_MY_BOOL:
    case SHOW_INT:
    case SHOW_LONG:
    case SHOW_LONGLONG:
    case SHOW_SIGNED_INT:
    case SHOW_SIGNED_LONG:
    case SHOW_SIGNED_LONGLONG:
    case SHOW_HA_ROWS:
      return INT_RESULT;
    case SHOW_CHAR:
    case SHOW_CHAR_PTR:
    case SHOW_LEX_STRING:
      return STRING_RESULT;
    case SHOW_TIMER:
    case SHOW_DOUBLE:
      return REAL_RESULT;
    default:
      my_error(ER_VAR_CANT_BE_READ, MYF(0), var->name.str);
      return STRING_RESULT;  // keep the compiler happy
  }
}

Audit_global_variable_get_event::Audit_global_variable_get_event(
    THD *thd, Item_func_get_system_var *item, uchar cache_type)
    : m_thd(thd), m_item(item), m_val_type(cache_type) {
  // Variable is of GLOBAL scope.
  bool is_global_var =
      (m_item->var_type == OPT_GLOBAL && m_item->var->check_scope(OPT_GLOBAL));

  // Event is already audited for the same query.
  bool event_is_audited =
      m_item->cache_present != 0 && m_item->used_query_id == m_thd->query_id;

  m_audit_event = (is_global_var && !event_is_audited);
}

Audit_global_variable_get_event::~Audit_global_variable_get_event() {
  /*
    While converting value to string, integer or real type, if the value is
    cached for the types other then m_val_type for intermediate type
    conversions then event is already notified.
  */
  bool event_already_notified = (m_item->cache_present & (~m_val_type));

  if (m_audit_event && !event_already_notified) {
    String str;
    String *outStr = nullptr;

    if (!m_item->cached_null_value || !m_thd->is_error()) {
      outStr = &str;

      DBUG_ASSERT(m_item->cache_present != 0 &&
                  m_item->used_query_id == m_thd->query_id);

      if (m_item->cache_present & GET_SYS_VAR_CACHE_STRING)
        outStr = &m_item->cached_strval;
      else if (m_item->cache_present & GET_SYS_VAR_CACHE_LONG)
        str.set(m_item->cached_llval, m_item->collation.collation);
      else if (m_item->cache_present & GET_SYS_VAR_CACHE_DOUBLE)
        str.set_real(m_item->cached_dval, m_item->decimals,
                     m_item->collation.collation);
    }

    mysql_audit_notify(m_thd, AUDIT_EVENT(MYSQL_AUDIT_GLOBAL_VARIABLE_GET),
                       m_item->var->name.str, outStr ? outStr->ptr() : nullptr,
                       outStr ? outStr->length() : 0);
  }
}

template <typename T>
longlong Item_func_get_system_var::get_sys_var_safe(THD *thd) {
  T value;
  {
    MUTEX_LOCK(lock, &LOCK_global_system_variables);
    value = *pointer_cast<const T *>(var->value_ptr(thd, var_type, &component));
  }
  cache_present |= GET_SYS_VAR_CACHE_LONG;
  used_query_id = thd->query_id;
  cached_llval = null_value ? 0LL : static_cast<longlong>(value);
  cached_null_value = null_value;
  return cached_llval;
}

longlong Item_func_get_system_var::val_int() {
  THD *thd = current_thd;
  Audit_global_variable_get_event audit_sys_var(thd, this,
                                                GET_SYS_VAR_CACHE_LONG);
  DBUG_ASSERT(fixed);

  if (cache_present && thd->query_id == used_query_id) {
    if (cache_present & GET_SYS_VAR_CACHE_LONG) {
      null_value = cached_null_value;
      return cached_llval;
    } else if (cache_present & GET_SYS_VAR_CACHE_DOUBLE) {
      null_value = cached_null_value;
      cached_llval = (longlong)cached_dval;
      cache_present |= GET_SYS_VAR_CACHE_LONG;
      return cached_llval;
    } else if (cache_present & GET_SYS_VAR_CACHE_STRING) {
      null_value = cached_null_value;
      if (!null_value)
        cached_llval = longlong_from_string_with_check(
            cached_strval.charset(), cached_strval.c_ptr(),
            cached_strval.c_ptr() + cached_strval.length());
      else
        cached_llval = 0;
      cache_present |= GET_SYS_VAR_CACHE_LONG;
      return cached_llval;
    }
  }

  switch (var->show_type()) {
    case SHOW_INT:
      return get_sys_var_safe<uint>(thd);
    case SHOW_LONG:
      return get_sys_var_safe<ulong>(thd);
    case SHOW_LONGLONG:
      return get_sys_var_safe<ulonglong>(thd);
    case SHOW_SIGNED_INT:
      return get_sys_var_safe<int>(thd);
    case SHOW_SIGNED_LONG:
      return get_sys_var_safe<long>(thd);
    case SHOW_SIGNED_LONGLONG:
      return get_sys_var_safe<longlong>(thd);
    case SHOW_HA_ROWS:
      return get_sys_var_safe<ha_rows>(thd);
    case SHOW_BOOL:
      return get_sys_var_safe<bool>(thd);
    case SHOW_MY_BOOL:
      return get_sys_var_safe<bool>(thd);
    case SHOW_TIMER:
    case SHOW_DOUBLE: {
      double dval = val_real();

      used_query_id = thd->query_id;
      cached_llval = (longlong)dval;
      cache_present |= GET_SYS_VAR_CACHE_LONG;
      return cached_llval;
    }
    case SHOW_CHAR:
    case SHOW_CHAR_PTR:
    case SHOW_LEX_STRING: {
      String *str_val = val_str(nullptr);
      // Treat empty strings as NULL, like val_real() does.
      if (str_val && str_val->length())
        cached_llval = longlong_from_string_with_check(
            system_charset_info, str_val->c_ptr(),
            str_val->c_ptr() + str_val->length());
      else {
        null_value = true;
        cached_llval = 0;
      }

      cache_present |= GET_SYS_VAR_CACHE_LONG;
      return cached_llval;
    }

    default:
      my_error(ER_VAR_CANT_BE_READ, MYF(0), var->name.str);
      return 0;  // keep the compiler happy
  }
}

String *Item_func_get_system_var::val_str(String *str) {
  THD *thd = current_thd;
  Audit_global_variable_get_event audit_sys_var(thd, this,
                                                GET_SYS_VAR_CACHE_STRING);
  DBUG_ASSERT(fixed);

  if (cache_present && thd->query_id == used_query_id) {
    if (cache_present & GET_SYS_VAR_CACHE_STRING) {
      null_value = cached_null_value;
      return null_value ? nullptr : &cached_strval;
    } else if (cache_present & GET_SYS_VAR_CACHE_LONG) {
      null_value = cached_null_value;
      if (!null_value) cached_strval.set(cached_llval, collation.collation);
      cache_present |= GET_SYS_VAR_CACHE_STRING;
      return null_value ? nullptr : &cached_strval;
    } else if (cache_present & GET_SYS_VAR_CACHE_DOUBLE) {
      null_value = cached_null_value;
      if (!null_value)
        cached_strval.set_real(cached_dval, decimals, collation.collation);
      cache_present |= GET_SYS_VAR_CACHE_STRING;
      return null_value ? nullptr : &cached_strval;
    }
  }

  str = &cached_strval;
  null_value = false;
  switch (var->show_type()) {
    case SHOW_CHAR:
    case SHOW_CHAR_PTR:
    case SHOW_LEX_STRING: {
      mysql_mutex_lock(&LOCK_global_system_variables);
      const char *cptr = var->show_type() == SHOW_CHAR
                             ? pointer_cast<const char *>(
                                   var->value_ptr(thd, var_type, &component))
                             : *pointer_cast<const char *const *>(
                                   var->value_ptr(thd, var_type, &component));
      if (cptr) {
        size_t len = var->show_type() == SHOW_LEX_STRING
                         ? (pointer_cast<const LEX_STRING *>(
                                var->value_ptr(thd, var_type, &component)))
                               ->length
                         : strlen(cptr);
        if (str->copy(cptr, len, collation.collation)) {
          null_value = true;
          str = nullptr;
        }
      } else {
        null_value = true;
        str = nullptr;
      }
      mysql_mutex_unlock(&LOCK_global_system_variables);
      break;
    }

    case SHOW_INT:
    case SHOW_LONG:
    case SHOW_LONGLONG:
    case SHOW_SIGNED_INT:
    case SHOW_SIGNED_LONG:
    case SHOW_SIGNED_LONGLONG:
    case SHOW_HA_ROWS:
    case SHOW_BOOL:
    case SHOW_MY_BOOL:
      if (unsigned_flag)
        str->set((ulonglong)val_int(), collation.collation);
      else
        str->set(val_int(), collation.collation);
      break;
    case SHOW_TIMER:
    case SHOW_DOUBLE:
      str->set_real(val_real(), decimals, collation.collation);
      break;

    default:
      my_error(ER_VAR_CANT_BE_READ, MYF(0), var->name.str);
      str = error_str();
      break;
  }

  cache_present |= GET_SYS_VAR_CACHE_STRING;
  used_query_id = thd->query_id;
  cached_null_value = null_value;
  return str;
}

double Item_func_get_system_var::val_real() {
  THD *thd = current_thd;
  Audit_global_variable_get_event audit_sys_var(thd, this,
                                                GET_SYS_VAR_CACHE_DOUBLE);
  DBUG_ASSERT(fixed);

  if (cache_present && thd->query_id == used_query_id) {
    if (cache_present & GET_SYS_VAR_CACHE_DOUBLE) {
      null_value = cached_null_value;
      return cached_dval;
    } else if (cache_present & GET_SYS_VAR_CACHE_LONG) {
      null_value = cached_null_value;
      cached_dval = (double)cached_llval;
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      return cached_dval;
    } else if (cache_present & GET_SYS_VAR_CACHE_STRING) {
      null_value = cached_null_value;
      if (!null_value)
        cached_dval = double_from_string_with_check(
            cached_strval.charset(), cached_strval.c_ptr(),
            cached_strval.c_ptr() + cached_strval.length());
      else
        cached_dval = 0;
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      return cached_dval;
    }
  }

  switch (var->show_type()) {
    case SHOW_TIMER:
      cached_dval = my_timer_to_seconds((ulonglong)(val_int()));
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      used_query_id = thd->query_id;
      cached_null_value = null_value;
      return cached_dval;
    case SHOW_DOUBLE:
      mysql_mutex_lock(&LOCK_global_system_variables);
      cached_dval = *pointer_cast<const double *>(
          var->value_ptr(thd, var_type, &component));
      mysql_mutex_unlock(&LOCK_global_system_variables);
      used_query_id = thd->query_id;
      cached_null_value = null_value;
      if (null_value) cached_dval = 0;
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      return cached_dval;
    case SHOW_CHAR:
    case SHOW_LEX_STRING:
    case SHOW_CHAR_PTR: {
      mysql_mutex_lock(&LOCK_global_system_variables);
      const char *cptr = var->show_type() == SHOW_CHAR
                             ? pointer_cast<const char *>(
                                   var->value_ptr(thd, var_type, &component))
                             : *pointer_cast<const char *const *>(
                                   var->value_ptr(thd, var_type, &component));
      // Treat empty strings as NULL, like val_int() does.
      if (cptr && *cptr)
        cached_dval = double_from_string_with_check(system_charset_info, cptr,
                                                    cptr + strlen(cptr));
      else {
        null_value = true;
        cached_dval = 0;
      }
      mysql_mutex_unlock(&LOCK_global_system_variables);
      used_query_id = thd->query_id;
      cached_null_value = null_value;
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      return cached_dval;
    }
    case SHOW_INT:
    case SHOW_LONG:
    case SHOW_LONGLONG:
    case SHOW_SIGNED_INT:
    case SHOW_SIGNED_LONG:
    case SHOW_SIGNED_LONGLONG:
    case SHOW_HA_ROWS:
    case SHOW_BOOL:
    case SHOW_MY_BOOL:
      cached_dval = (double)val_int();
      cache_present |= GET_SYS_VAR_CACHE_DOUBLE;
      used_query_id = thd->query_id;
      cached_null_value = null_value;
      return cached_dval;
    default:
      my_error(ER_VAR_CANT_BE_READ, MYF(0), var->name.str);
      return 0;
  }
}

bool Item_func_get_system_var::eq(const Item *item, bool) const {
  /* Assume we don't have rtti */
  if (this == item) return true;  // Same item is same.
  /* Check if other type is also a get_user_var() object */
  if (item->type() != FUNC_ITEM ||
      down_cast<const Item_func *>(item)->functype() != functype())
    return false;
  const Item_func_get_system_var *other =
      down_cast<const Item_func_get_system_var *>(item);
  return (var == other->var && var_type == other->var_type);
}

void Item_func_get_system_var::cleanup() {
  Item_func::cleanup();
  cache_present = 0;
  var_type = orig_var_type;
  cached_strval.mem_free();
}

bool Item_func_match::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res) || against->itemize(pc, &against)) return true;
  add_accum_properties(against);

  pc->select->add_ftfunc_to_list(this);
  pc->thd->lex->set_using_match();

  switch (pc->select->parsing_place) {
    case CTX_WHERE:
    case CTX_ON:
      used_in_where_only = true;
      break;
    default:
      used_in_where_only = false;
  }

  return false;
}

/**
  Initialize searching within full-text index.

  @param thd    Thread handler

  @returns false if success, true if error
*/

bool Item_func_match::init_search(THD *thd) {
  DBUG_TRACE;

  /*
    We will skip execution if the item is not fixed
    with fix_field
  */
  if (!fixed) return false;

  TABLE *const table = table_ref->table;
  /* Check if init_search() has been called before */
  if (ft_handler && !master) {
    /*
      We should reset ft_handler (necessary in case of re-execution of
      subquery), as it is cleaned up when initializing the
      SortBufferIndirectIterator / SortFileIndirectIterator.
      TODO: Those iterators should not clean up ft_handler.
    */
    if (join_key) table->file->ft_handler = ft_handler;
    return false;
  }

  if (key == NO_SUCH_KEY) {
    List<Item> fields;
    if (fields.push_back(new Item_string(" ", 1, cmp_collation.collation)))
      return true;
    for (uint i = 0; i < arg_count; i++) fields.push_back(args[i]);
    concat_ws = new Item_func_concat_ws(fields);
    if (concat_ws == nullptr) return true;
    /*
      Above function used only to get value and do not need fix_fields for it:
      Item_string - basic constant
      fields - fix_fields() was already called for this arguments
      Item_func_concat_ws - do not need fix_fields() to produce value
    */
    concat_ws->quick_fix_field();
  }

  if (master) {
    if (master->init_search(thd)) return true;

    ft_handler = master->ft_handler;
    return false;
  }

  String *ft_tmp = nullptr;

  // MATCH ... AGAINST (NULL) is meaningless, but possible
  if (!(ft_tmp = key_item()->val_str(&value))) {
    ft_tmp = &value;
    value.set("", 0, cmp_collation.collation);
  }

  if (ft_tmp->charset() != cmp_collation.collation) {
    uint dummy_errors;
    search_value.copy(ft_tmp->ptr(), ft_tmp->length(), ft_tmp->charset(),
                      cmp_collation.collation, &dummy_errors);
    ft_tmp = &search_value;
  }

  if (!table->is_created()) {
    my_error(ER_NO_FT_MATERIALIZED_SUBQUERY, MYF(0));
    return true;
  }

  DBUG_ASSERT(master == nullptr);
  ft_handler = table->file->ft_init_ext_with_hints(key, ft_tmp, get_hints());
  if (thd->is_error()) return true;

  if (join_key) table->file->ft_handler = ft_handler;

  return false;
}

float Item_func_match::get_filtering_effect(THD *, table_map filter_for_table,
                                            table_map read_tables,
                                            const MY_BITMAP *fields_to_ignore,
                                            double rows_in_table) {
  const Item_field *fld =
      contributes_to_filter(read_tables, filter_for_table, fields_to_ignore);
  if (!fld) return COND_FILTER_ALLPASS;

  /*
    MATCH () ... AGAINST" is similar to "LIKE '...'" which has the
    same selectivity as "col BETWEEN ...".
  */
  return fld->get_cond_filter_default_probability(rows_in_table,
                                                  COND_FILTER_BETWEEN);
}

/**
   Add field into table read set.

   @param field field to be added to the table read set.
*/
static void update_table_read_set(const Field *field) {
  TABLE *table = field->table;

  if (!bitmap_test_and_set(table->read_set, field->field_index))
    table->covering_keys.intersect(field->part_of_key);
}

bool Item_func_match::fix_fields(THD *thd, Item **ref) {
  DBUG_ASSERT(fixed == 0);
  DBUG_ASSERT(arg_count > 0);
  Item *item = nullptr;  // Safe as arg_count is > 1

  maybe_null = true;
  join_key = false;

  /*
    const_item is assumed in quite a bit of places, so it would be difficult
    to remove;  If it would ever to be removed, this should include
    modifications to find_best and auto_close as complement to auto_init code
    above.
  */
  enum_mark_columns save_mark_used_columns = thd->mark_used_columns;
  /*
    Since different engines require different columns for FTS index lookup
    we prevent updating of table read_set in argument's ::fix_fields().
  */
  thd->mark_used_columns = MARK_COLUMNS_NONE;
  if (Item_func::fix_fields(thd, ref) || fix_func_arg(thd, &against) ||
      !against->const_for_execution()) {
    thd->mark_used_columns = save_mark_used_columns;
    my_error(ER_WRONG_ARGUMENTS, MYF(0), "AGAINST");
    return true;
  }
  thd->mark_used_columns = save_mark_used_columns;

  bool allows_multi_table_search = true;
  for (uint i = 0; i < arg_count; i++) {
    item = args[i] = args[i]->real_item();
    if (item->type() != Item::FIELD_ITEM ||
        /* Cannot use FTS index with outer table field */
        (item->used_tables() & OUTER_REF_TABLE_BIT)) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "MATCH");
      return true;
    }
    allows_multi_table_search &= allows_search_on_non_indexed_columns(
        ((Item_field *)item)->field->table);
  }

  /*
    Check that all columns come from the same table.
    We've already checked that columns in MATCH are fields so
    INNER_TABLE_BIT can only appear from AGAINST argument.
  */
  if ((used_tables_cache & ~INNER_TABLE_BIT) != item->used_tables())
    key = NO_SUCH_KEY;

  if (key == NO_SUCH_KEY && !allows_multi_table_search) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), "MATCH");
    return true;
  }
  table_ref = ((Item_field *)item)->table_ref;

  /*
    Here we make an assumption that if the engine supports
    fulltext extension(HA_CAN_FULLTEXT_EXT flag) then table
    can have FTS_DOC_ID column. Atm this is the only way
    to distinguish MyISAM and InnoDB engines.
    Generally table_ref should be available, but in case of
    a generated column's generation expression it's not. Thus
    we use field's table, at this moment it's already available.
  */
  TABLE *const table =
      table_ref ? table_ref->table : ((Item_field *)item)->field->table;

  if (!(table->file->ha_table_flags() & HA_CAN_FULLTEXT)) {
    my_error(ER_TABLE_CANT_HANDLE_FT, MYF(0));
    return true;
  }

  if ((table->file->ha_table_flags() & HA_CAN_FULLTEXT_EXT)) {
    Field *doc_id_field = table->fts_doc_id_field;
    /*
      Update read set with FTS_DOC_ID column so that indexes that have
      FTS_DOC_ID part can be considered as a covering index.
    */
    if (doc_id_field)
      update_table_read_set(doc_id_field);
    else {
      /* read_set needs to be updated for MATCH arguments */
      for (uint i = 0; i < arg_count; i++)
        update_table_read_set(((Item_field *)args[i])->field);
      /*
        Prevent index only accces by non-FTS index if table does not have
        FTS_DOC_ID column, find_relevance does not work properly without
        FTS_DOC_ID value. Decision for FTS index about index only access
        is made later by JOIN::fts_index_access() function.
      */
      table->covering_keys.clear_all();
    }

  } else {
    /*
      Since read_set is not updated for MATCH arguments
      it's necessary to update it here for MyISAM.
    */
    for (uint i = 0; i < arg_count; i++)
      update_table_read_set(((Item_field *)args[i])->field);
  }

  table->fulltext_searched = true;

  if (!master) {
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    hints = new (thd->mem_root) Ft_hints(flags);
    if (!hints) {
      my_error(ER_TABLE_CANT_HANDLE_FT, MYF(0));
      return true;
    }
  }
  return agg_item_collations_for_comparison(cmp_collation, func_name(), args,
                                            arg_count, 0);
}

bool Item_func_match::fix_index(const THD *thd) {
  Item_field *item;
  TABLE *table;
  uint ft_to_key[MAX_KEY], ft_cnt[MAX_KEY], fts = 0, keynr;
  uint max_cnt = 0, mkeys = 0, i;

  if (!table_ref) goto err;

  /*
    We will skip execution if the item is not fixed
    with fix_field
  */
  if (!fixed) {
    if (allows_search_on_non_indexed_columns(table_ref->table))
      key = NO_SUCH_KEY;

    return false;
  }
  if (key == NO_SUCH_KEY) return false;

  table = table_ref->table;
  for (keynr = 0; keynr < table->s->keys; keynr++) {
    if ((table->key_info[keynr].flags & HA_FULLTEXT) &&
        (flags & FT_BOOL ? table->keys_in_use_for_query.is_set(keynr)
                         : table->s->usable_indexes(thd).is_set(keynr)))

    {
      ft_to_key[fts] = keynr;
      ft_cnt[fts] = 0;
      fts++;
    }
  }

  if (!fts) goto err;

  for (i = 0; i < arg_count; i++) {
    item = (Item_field *)(args[i]->real_item());
    for (keynr = 0; keynr < fts; keynr++) {
      KEY *ft_key = &table->key_info[ft_to_key[keynr]];
      uint key_parts = ft_key->user_defined_key_parts;

      for (uint part = 0; part < key_parts; part++) {
        if (item->field->eq(ft_key->key_part[part].field)) ft_cnt[keynr]++;
      }
    }
  }

  for (keynr = 0; keynr < fts; keynr++) {
    if (ft_cnt[keynr] > max_cnt) {
      mkeys = 0;
      max_cnt = ft_cnt[mkeys] = ft_cnt[keynr];
      ft_to_key[mkeys] = ft_to_key[keynr];
      continue;
    }
    if (max_cnt && ft_cnt[keynr] == max_cnt) {
      mkeys++;
      ft_cnt[mkeys] = ft_cnt[keynr];
      ft_to_key[mkeys] = ft_to_key[keynr];
      continue;
    }
  }

  for (keynr = 0; keynr <= mkeys; keynr++) {
    // partial keys doesn't work
    if (max_cnt < arg_count ||
        max_cnt < table->key_info[ft_to_key[keynr]].user_defined_key_parts)
      continue;

    key = ft_to_key[keynr];

    return false;
  }

err:
  if (table_ref != nullptr &&
      allows_search_on_non_indexed_columns(table_ref->table)) {
    key = NO_SUCH_KEY;
    return false;
  }
  my_error(ER_FT_MATCHING_KEY_NOT_FOUND, MYF(0));
  return true;
}

bool Item_func_match::eq(const Item *item, bool binary_cmp) const {
  /* We ignore FT_SORTED flag when checking for equality since result is
     equvialent regardless of sorting */
  DBUG_ASSERT(item->type() != FUNC_ITEM ||
              down_cast<const Item_func *>(item)->functype() != MATCH_FUNC);
  if (item->type() != FUNC_ITEM ||
      down_cast<const Item_func *>(item)->functype() != FT_FUNC ||
      (flags | FT_SORTED) !=
          (down_cast<const Item_func_match *>(item)->flags | FT_SORTED))
    return false;

  const Item_func_match *ifm = down_cast<const Item_func_match *>(item);

  if (key == ifm->key && table_ref == ifm->table_ref &&
      key_item()->eq(ifm->key_item(), binary_cmp))
    return true;

  return false;
}

double Item_func_match::val_real() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  if (ft_handler == nullptr) return -1.0;

  TABLE *const table = table_ref->table;
  if (key != NO_SUCH_KEY && table->has_null_row())  // NULL row from outer join
    return 0.0;

  if (get_master()->join_key) {
    if (table->file->ft_handler)
      return ft_handler->please->get_relevance(ft_handler);
    get_master()->join_key = false;
  }

  if (key == NO_SUCH_KEY) {
    String *a = concat_ws->val_str(&value);
    if ((null_value = (a == nullptr)) || !a->length()) return 0;
    return ft_handler->please->find_relevance(ft_handler, (uchar *)a->ptr(),
                                              a->length());
  }
  return ft_handler->please->find_relevance(ft_handler, table->record[0], 0);
}

void Item_func_match::print(const THD *thd, String *str,
                            enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("(match "));
  print_args(thd, str, 0, query_type);
  str->append(STRING_WITH_LEN(" against ("));
  against->print(thd, str, query_type);
  if (flags & FT_BOOL)
    str->append(STRING_WITH_LEN(" in boolean mode"));
  else if (flags & FT_EXPAND)
    str->append(STRING_WITH_LEN(" with query expansion"));
  str->append(STRING_WITH_LEN("))"));
}

/**
  Function sets FT hints(LIMIT, flags) depending on
  various join conditions.

  @param join     Pointer to JOIN object.
  @param ft_flag  FT flag value.
  @param ft_limit Limit value.
  @param no_cond  true if MATCH is not used in WHERE condition.
*/

void Item_func_match::set_hints(JOIN *join, uint ft_flag, ha_rows ft_limit,
                                bool no_cond) {
  DBUG_ASSERT(!master);

  if (!join)  // used for count() optimization
  {
    hints->set_hint_flag(ft_flag);
    return;
  }

  /* skip hints setting if there are aggregates(except of FT_NO_RANKING) */
  if (join->implicit_grouping || join->group_list || join->select_distinct) {
    /* 'No ranking' is possibe even if aggregates are present */
    if ((ft_flag & FT_NO_RANKING)) hints->set_hint_flag(FT_NO_RANKING);
    return;
  }

  hints->set_hint_flag(ft_flag);

  /**
    Only one table is used, there is no aggregates,
    WHERE condition is a single MATCH expression
    (WHERE MATCH(..) or WHERE MATCH(..) [>=,>] value) or
    there is no WHERE condition.
  */
  if (join->primary_tables == 1 && (no_cond || is_simple_expression()))
    hints->set_hint_limit(ft_limit);
}

/***************************************************************************
  System variables
****************************************************************************/

/**
  Return value of an system variable base[.name] as a constant item.

  @param pc                     Current parse context
  @param var_type               global / session
  @param name                   Name of base or system variable
  @param component              Component.

  @note
    If component.str = 0 then the variable name is in 'name'

  @return
    - 0  : error
    - #  : constant item
*/

Item *get_system_var(Parse_context *pc, enum_var_type var_type, LEX_STRING name,
                     LEX_STRING component) {
  THD *thd = pc->thd;
  sys_var *var;
  LEX_STRING *base_name, *component_name;

  if (component.str) {
    base_name = &component;
    component_name = &name;
  } else {
    base_name = &name;
    component_name = &component;  // Empty string
  }

  if (!(var = find_sys_var(thd, base_name->str, base_name->length)))
    return nullptr;
  if (component.str) {
    if (!var->is_struct()) {
      my_error(ER_VARIABLE_IS_NOT_STRUCT, MYF(0), base_name->str);
      return nullptr;
    }
  }
  thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);

  component_name->length =
      min(component_name->length, size_t(MAX_SYS_VAR_LENGTH));

  var->do_deprecated_warning(thd);

  return new Item_func_get_system_var(var, var_type, component_name, nullptr,
                                      0);
}

bool Item_func_row_count::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  LEX *lex = pc->thd->lex;
  lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  lex->safe_to_cache_query = false;
  return false;
}

longlong Item_func_row_count::val_int() {
  DBUG_ASSERT(fixed == 1);
  THD *thd = current_thd;

  return thd->get_row_count_func();
}

Item_func_sp::Item_func_sp(const POS &pos, const LEX_STRING &db_name,
                           const LEX_STRING &fn_name, bool use_explicit_name,
                           PT_item_list *opt_list)
    : Item_func(pos, opt_list), m_sp(nullptr), sp_result_field(nullptr) {
  /*
    Set to false here, which is the default according to SQL standard.
    RETURNS NULL ON NULL INPUT can be implemented by modifying this member.
  */
  null_on_null = false;
  maybe_null = true;
  set_stored_program();
  THD *thd = current_thd;
  m_name = new (thd->mem_root)
      sp_name(to_lex_cstring(db_name), fn_name, use_explicit_name);
}

bool Item_func_sp::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  if (m_name == nullptr) return true;  // OOM

  THD *thd = pc->thd;
  LEX *lex = thd->lex;

  context = lex->current_context();
  lex->safe_to_cache_query = false;

  if (m_name->m_db.str == nullptr) {
    if (thd->lex->copy_db_to(&m_name->m_db.str, &m_name->m_db.length)) {
      my_error(ER_NO_DB_ERROR, MYF(0));
      return true;
    }
  }

  m_name->init_qname(thd);
  sp_add_own_used_routine(lex, thd, Sroutine_hash_entry::FUNCTION, m_name);

  return false;
}

void Item_func_sp::cleanup() {
  if (sp_result_field) {
    destroy(sp_result_field);
    sp_result_field = nullptr;
  }
  m_sp = nullptr;
  Item_func::cleanup();
  set_stored_program();
}

const char *Item_func_sp::func_name() const {
  const THD *thd = current_thd;
  /* Calculate length to avoid reallocation of string for sure */
  size_t len =
      (((m_name->m_explicit_name ? m_name->m_db.length : 0) +
        m_name->m_name.length) *
           2 +                              // characters*quoting
       2 +                                  // ` and `
       (m_name->m_explicit_name ? 3 : 0) +  // '`', '`' and '.' for the db
       1 +                                  // end of string
       ALIGN_SIZE(1));                      // to avoid String reallocation
  String qname((char *)thd->mem_root->Alloc(len), len, system_charset_info);

  qname.length(0);
  if (m_name->m_explicit_name) {
    append_identifier(thd, &qname, m_name->m_db.str, m_name->m_db.length);
    qname.append('.');
  }
  append_identifier(thd, &qname, m_name->m_name.str, m_name->m_name.length);
  return qname.ptr();
}

table_map Item_func_sp::get_initial_pseudo_tables() const {
  /*
    INNER_TABLE_BIT prevents function from being evaluated in preparation phase.
    @todo - make this dependent on READS SQL or MODIFIES SQL.
            Due to bug#26422182, a function cannot be executed before tables
            are locked, even though it accesses no tables.
  */
  return m_sp->m_chistics->detistic ? INNER_TABLE_BIT : RAND_TABLE_BIT;
}

static void my_missing_function_error(const LEX_STRING &token,
                                      const char *func_name) {
  if (token.length && is_lex_native_function(&token))
    my_error(ER_FUNC_INEXISTENT_NAME_COLLISION, MYF(0), func_name);
  else
    my_error(ER_SP_DOES_NOT_EXIST, MYF(0), "FUNCTION", func_name);
}

/**
  @brief Initialize the result field by creating a temporary dummy table
    and assign it to a newly created field object. Meta data used to
    create the field is fetched from the sp_head belonging to the stored
    procedure found in the stored procedure function cache.

  @note This function should be called from fix_fields to init the result
    field. It is some what related to Item_field.

  @see Item_field

  @param thd A pointer to the session and thread context.

  @return Function return error status.
  @retval true is returned on an error
  @retval false is returned on success.
*/

bool Item_func_sp::init_result_field(THD *thd) {
  LEX_CSTRING empty_name = {STRING_WITH_LEN("")};
  DBUG_TRACE;

  DBUG_ASSERT(m_sp == nullptr);
  DBUG_ASSERT(sp_result_field == nullptr);

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);
  if (!(m_sp = sp_setup_routine(thd, enum_sp_type::FUNCTION, m_name,
                                &thd->sp_func_cache))) {
    my_missing_function_error(m_name->m_name, m_name->m_qname.str);
    return true;
  }

  /*
     A Field need to be attached to a Table.
     Below we "create" a dummy table by initializing
     the needed pointers.
   */
  TABLE *dummy_table = new (thd->mem_root) TABLE;
  if (dummy_table == nullptr) return true;
  TABLE_SHARE *share = new (thd->mem_root) TABLE_SHARE;
  if (share == nullptr) return true;

  dummy_table->s = share;
  dummy_table->alias = "";
  if (maybe_null) dummy_table->set_nullable();
  dummy_table->in_use = thd;
  dummy_table->copy_blobs = true;
  share->table_cache_key = empty_name;
  share->db = empty_name;
  share->table_name = empty_name;

  sp_result_field =
      m_sp->create_result_field(thd, max_length, item_name.ptr(), dummy_table);
  return sp_result_field == nullptr;
}

/**
  @brief Initialize local members with values from the Field interface.

  @note called from Item::fix_fields.
*/

bool Item_func_sp::resolve_type(THD *) {
  DBUG_TRACE;

  DBUG_ASSERT(sp_result_field);
  set_data_type(sp_result_field->type());
  decimals = sp_result_field->decimals();
  max_length = sp_result_field->field_length;
  collation.set(sp_result_field->charset());
  maybe_null = true;
  unsigned_flag = (sp_result_field->flags & UNSIGNED_FLAG);

  return false;
}

bool Item_func_sp::val_json(Json_wrapper *result) {
  if (sp_result_field->type() == MYSQL_TYPE_JSON) {
    if (execute()) return true;

    if (null_value) return false;

    Field_json *json_value = down_cast<Field_json *>(sp_result_field);
    return json_value->val_json(result);
  }

  /* purecov: begin deadcode */
  DBUG_ASSERT(false);
  my_error(ER_INVALID_CAST_TO_JSON, MYF(0));
  return error_json();
  /* purecov: end */
}

/**
  @brief Execute function & store value in field.

  @return Function returns error status.
  @retval false on success.
  @retval true if an error occurred.
*/

bool Item_func_sp::execute() {
  THD *thd = current_thd;

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);
  /* Execute function and store the return value in the field. */

  if (execute_impl(thd)) {
    null_value = true;
    if (thd->killed) thd->send_kill_message();
    return true;
  }

  /* Check that the field (the value) is not NULL. */
  null_value = sp_result_field->is_null();

  return false;
}

/**
   @brief Execute function and store the return value in the field.

   @note This function was intended to be the concrete implementation of
    the interface function execute. This was never realized.

   @return The error state.
   @retval false on success
   @retval true if an error occurred.
*/
bool Item_func_sp::execute_impl(THD *thd) {
  bool err_status = true;
  Sub_statement_state statement_state;
  Security_context *save_security_ctx = thd->security_context();
  enum enum_sp_data_access access =
      (m_sp->m_chistics->daccess == SP_DEFAULT_ACCESS)
          ? SP_DEFAULT_ACCESS_MAPPING
          : m_sp->m_chistics->daccess;

  DBUG_TRACE;

  if (context->security_ctx) {
    /* Set view definer security context */
    thd->set_security_context(context->security_ctx);
  }
  if (sp_check_access(thd)) goto error;

  /*
    Throw an error if a non-deterministic function is called while
    statement-based replication (SBR) is active.
  */

  if (!m_sp->m_chistics->detistic && !trust_function_creators &&
      (access == SP_CONTAINS_SQL || access == SP_MODIFIES_SQL_DATA) &&
      (mysql_bin_log.is_open() &&
       thd->variables.binlog_format == BINLOG_FORMAT_STMT)) {
    my_error(ER_BINLOG_UNSAFE_ROUTINE, MYF(0));
    goto error;
  }

  /*
    The 'function call' top statement can not distinguish if its sub
    statements (function) have 'CREATE/DROP TEMPORARY TABLE' or not
    before executing its sub statements, It is too late to set the
    binlog format to row in mixed mode when executing the 'CREATE/DROP
    TEMPORARY TABLE' in sub statement, because the binlog format is not
    consistent before and after 'CREATE/DROP TEMPORARY TABLE'. Which
    implies that we have to write the 'function call' top statement
    into binlog if the function contains 'CREATE/DROP TEMPORARY TABLE'
    in mixed mode. Because of that constrain we have to write the
    'function call' top statement into binlog if the function contains
    the DMLs on temporary table in mixed mode, another reason is that
    the DMLs on temporary table might be in the same function as
    'CREATE/DROP TEMPORARY TABLE'. Which requires to set binlog format
    to statement if the function contains DML statement(s) on temporary
    table in mixed mode.
  */
  if (thd->variables.binlog_format == BINLOG_FORMAT_MIXED &&
      (thd->lex->stmt_accessed_table(LEX::STMT_READS_TEMP_TRANS_TABLE) ||
       thd->lex->stmt_accessed_table(LEX::STMT_READS_TEMP_NON_TRANS_TABLE)))
    thd->clear_current_stmt_binlog_format_row();

  /*
    Disable the binlogging if this is not a SELECT statement. If this is a
    SELECT, leave binlogging on, so execute_function() code writes the
    function call into binlog.
  */
  thd->reset_sub_statement_state(&statement_state, SUB_STMT_FUNCTION);
  err_status = m_sp->execute_function(thd, args, arg_count, sp_result_field);
  thd->restore_sub_statement_state(&statement_state);

error:
  thd->set_security_context(save_security_ctx);

  return err_status;
}

void Item_func_sp::make_field(Send_field *tmp_field) {
  DBUG_TRACE;
  DBUG_ASSERT(sp_result_field);
  sp_result_field->make_send_field(tmp_field);
  if (item_name.is_set()) tmp_field->col_name = item_name.ptr();
}

Item_result Item_func_sp::result_type() const {
  DBUG_TRACE;
  DBUG_PRINT("info", ("m_sp = %p", (void *)m_sp));
  DBUG_ASSERT(sp_result_field);
  return sp_result_field->result_type();
}

bool Item_func_found_rows::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  push_warning(current_thd, Sql_condition::SL_WARNING,
               ER_WARN_DEPRECATED_SYNTAX,
               ER_THD(current_thd, ER_WARN_DEPRECATED_FOUND_ROWS));
  return false;
}

longlong Item_func_found_rows::val_int() {
  DBUG_ASSERT(fixed == 1);
  return current_thd->found_rows();
}

Field *Item_func_sp::tmp_table_field(TABLE *) {
  DBUG_TRACE;

  DBUG_ASSERT(sp_result_field);
  return sp_result_field;
}

/**
  @brief Checks if requested access to function can be granted to user.
    If function isn't found yet, it searches function first.
    If function can't be found or user don't have requested access
    error is raised.

  @param thd thread handler

  @return Indication if the access was granted or not.
  @retval false Access is granted.
  @retval true Requested access can't be granted or function doesn't exists.

*/

bool Item_func_sp::sp_check_access(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(m_sp);
  if (check_routine_access(thd, EXECUTE_ACL, m_sp->m_db.str, m_sp->m_name.str,
                           false, false))
    return true;

  return false;
}

bool Item_func_sp::fix_fields(THD *thd, Item **ref) {
  bool res;
  Security_context *save_security_ctx = thd->security_context();

  DBUG_TRACE;
  DBUG_ASSERT(fixed == 0);

  /*
    Checking privileges to execute the function while creating view and
    executing the function of select.
   */
  if (!thd->lex->is_view_context_analysis() ||
      (thd->lex->sql_command == SQLCOM_CREATE_VIEW)) {
    if (context->security_ctx) {
      /* Set view definer security context */
      thd->set_security_context(context->security_ctx);
    }

    /*
      Check whether user has execute privilege or not
     */

    Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
        thd, context->view_error_handler, context->view_error_handler_arg);

    res = check_routine_access(thd, EXECUTE_ACL, m_name->m_db.str,
                               m_name->m_name.str, false, false);
    thd->set_security_context(save_security_ctx);

    if (res) {
      return res;
    }
  }

  /*
    We must call init_result_field before Item_func::fix_fields()
    to make m_sp and result_field members available to resolve_type(),
    which is called from Item_func::fix_fields().
  */
  res = init_result_field(thd);

  if (res) return res;

  res = Item_func::fix_fields(thd, ref);
  if (res) return res;

  if (thd->lex->is_view_context_analysis()) {
    /*
      Here we check privileges of the stored routine only during view
      creation, in order to validate the view.  A runtime check is
      perfomed in Item_func_sp::execute(), and this method is not
      called during context analysis.  Notice, that during view
      creation we do not infer into stored routine bodies and do not
      check privileges of its statements, which would probably be a
      good idea especially if the view has SQL SECURITY DEFINER and
      the used stored procedure has SQL SECURITY DEFINER.
    */
    res = sp_check_access(thd);
    /*
      Try to set and restore the security context to see whether it's valid
    */
    Security_context *save_security_context;
    res = m_sp->set_security_ctx(thd, &save_security_context);
    if (!res)
      m_sp->m_security_ctx.restore_security_context(thd, save_security_context);
  }

  return res;
}

void Item_func_sp::update_used_tables() {
  Item_func::update_used_tables();

  /* This is reset by Item_func::update_used_tables(). */
  set_stored_program();
}

void Item_func_sp::fix_after_pullout(SELECT_LEX *parent_select,
                                     SELECT_LEX *removed_select) {
  Item_func::fix_after_pullout(parent_select, removed_select);
}

/*
  uuid_short handling.

  The short uuid is defined as a longlong that contains the following bytes:

  Bytes  Comment
  1      Server_id & 255
  4      Startup time of server in seconds
  3      Incrementor

  This means that an uuid is guaranteed to be unique
  even in a replication environment if the following holds:

  - The last byte of the server id is unique
  - If you between two shutdown of the server don't get more than
    an average of 2^24 = 16M calls to uuid_short() per second.
*/

ulonglong uuid_value;

void uuid_short_init() {
  uuid_value =
      ((((ulonglong)server_id) << 56) + (((ulonglong)server_start_time) << 24));
}

bool Item_func_uuid_short::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

longlong Item_func_uuid_short::val_int() {
  ulonglong val;
  mysql_mutex_lock(&LOCK_uuid_generator);
  val = uuid_value++;
  mysql_mutex_unlock(&LOCK_uuid_generator);
  return (longlong)val;
}

bool Item_func_version::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  return false;
}

/**
  Check if schema and table are hidden by NDB engine.

  @param    thd           Thread handle.
  @param    schema_name   Schema name.
  @param    table_name    Table name.

  @retval   true          If schema and table are hidden by NDB.
  @retval   false         If schema and table are not hidden by NDB.
*/

static inline bool is_hidden_by_ndb(THD *thd, String *schema_name,
                                    String *table_name) {
  if (!strncmp(schema_name->ptr(), "ndb", 3)) {
    List<LEX_STRING> list;

    // Check if schema is of ndb and if it is hidden by it.
    LEX_STRING sch_name = schema_name->lex_string();
    list.push_back(&sch_name);
    ha_find_files(thd, nullptr, nullptr, nullptr, true, &list);
    if (list.elements == 0) {
      // Schema is hidden by ndb engine.
      return true;
    }

    // Check if table is hidden by ndb.
    if (table_name != nullptr) {
      list.empty();
      LEX_STRING tbl_name = table_name->lex_string();
      list.push_back(&tbl_name);
      ha_find_files(thd, schema_name->ptr(), nullptr, nullptr, false, &list);
      if (list.elements == 0) {
        // Table is hidden by ndb engine.
        return true;
      }
    }
  }

  return false;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    In order for INFORMATION_SCHEMA to skip listing database for which
    the user does not have rights, the following internal functions are used.

  Syntax:
    int CAN_ACCCESS_DATABASE(schema_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/

longlong Item_func_can_access_database::val_int() {
  DBUG_TRACE;

  // Read schema_name
  String schema_name;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  if (schema_name_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();

  // Check if schema is hidden.
  THD *thd = current_thd;
  if (is_hidden_by_ndb(thd, schema_name_ptr, nullptr)) return 0;

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr())) return 1;

  // Check access
  Security_context *sctx = thd->security_context();
  if (!(sctx->master_access(schema_name_ptr->ptr()) &
            (DB_OP_ACLS | SHOW_DB_ACL) ||
        acl_get(thd, sctx->host().str, sctx->ip().str, sctx->priv_user().str,
                schema_name_ptr->ptr(), false) ||
        !check_grant_db(thd, schema_name_ptr->ptr()))) {
    return 0;
  }

  return 1;
}

static bool check_table_and_trigger_access(Item **args, bool check_trigger_acl,
                                           bool *null_value) {
  DBUG_TRACE;

  // Read schema_name, table_name
  String schema_name;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String table_name;
  String *table_name_ptr = args[1]->val_str(&table_name);
  if (schema_name_ptr == nullptr || table_name_ptr == nullptr) {
    *null_value = true;
    return false;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();
  table_name_ptr->c_ptr_safe();

  // Check if table is hidden.
  THD *thd = current_thd;
  if (is_hidden_by_ndb(thd, schema_name_ptr, table_name_ptr)) return false;

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr())) return true;

  // Check access
  ulong db_access = 0;
  if (check_access(thd, SELECT_ACL, schema_name_ptr->ptr(), &db_access, nullptr,
                   false, true))
    return false;

  TABLE_LIST table_list;
  table_list.db = schema_name_ptr->ptr();
  table_list.db_length = schema_name_ptr->length();
  if (lower_case_table_names == 2) {
    my_casedn_str(files_charset_info, table_name_ptr->ptr());
  }
  table_list.table_name = table_name_ptr->ptr();
  table_list.table_name_length = table_name_ptr->length();
  table_list.grant.privilege = db_access;

  if (check_trigger_acl == false) {
    if (db_access & TABLE_OP_ACLS) return true;

    // Check table access
    if (check_grant(thd, TABLE_OP_ACLS, &table_list, true, 1, true))
      return false;
  } else  // Trigger check.
  {
    // Check trigger access
    if (check_trigger_acl &&
        check_table_access(thd, TRIGGER_ACL, &table_list, false, 1, true))
      return false;
  }

  return true;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from new DD using system views.
    In order for INFORMATION_SCHEMA to skip listing table for which
    the user does not have rights, the following UDF's is used.

  Syntax:
    int CAN_ACCCESS_TABLE(schema_name, table_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/
longlong Item_func_can_access_table::val_int() {
  DBUG_TRACE;

  if (check_table_and_trigger_access(args, false, &null_value)) return 1;

  return 0;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from new DD using system views. In
    order for INFORMATION_SCHEMA to skip listing table for which the user
    does not have rights on triggers, the following UDF's is used.

  Syntax:
    int CAN_ACCCESS_TRIGGER(schema_name, table_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/
longlong Item_func_can_access_trigger::val_int() {
  DBUG_TRACE;

  if (check_table_and_trigger_access(args, true, &null_value)) return 1;

  return 0;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views. In
    order for INFORMATION_SCHEMA to skip listing routine for which the user
    does not have rights, the following UDF's is used.

  Syntax:
    int CAN_ACCESS_ROUTINE(schema_name, name, type, user, definer,
                           check_full_access);

  @returns,
    1 - If current user has access.
    0 - If not.
*/
longlong Item_func_can_access_routine::val_int() {
  DBUG_TRACE;

  // Read schema_name, table_name
  String schema_name;
  String routine_name;
  String type;
  String definer;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *routine_name_ptr = args[1]->val_str(&routine_name);
  String *type_ptr = args[2]->val_str(&type);
  String *definer_ptr = args[3]->val_str(&definer);
  bool check_full_access = args[4]->val_int();
  if (schema_name_ptr == nullptr || routine_name_ptr == nullptr ||
      type_ptr == nullptr || definer_ptr == nullptr || args[4]->null_value) {
    null_value = true;
    return 0;
  }

  // Make strings safe.
  schema_name_ptr->c_ptr_safe();
  routine_name_ptr->c_ptr_safe();
  type_ptr->c_ptr_safe();
  definer_ptr->c_ptr_safe();

  bool is_procedure = (strcmp(type_ptr->ptr(), "PROCEDURE") == 0);

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr()) ||
      !my_strcasecmp(system_charset_info, schema_name_ptr->ptr(), "sys"))
    return 1;

  /*
    Check if user has full access to the routine properties (i.e including
    stored routine code), or partial access (i.e to view its other properties).
  */

  char user_name_holder[USERNAME_LENGTH + 1];
  LEX_STRING user_name = {user_name_holder, USERNAME_LENGTH};

  char host_name_holder[HOSTNAME_LENGTH + 1];
  LEX_STRING host_name = {host_name_holder, HOSTNAME_LENGTH};

  parse_user(definer_ptr->ptr(), definer_ptr->length(), user_name.str,
             &user_name.length, host_name.str, &host_name.length);

  THD *thd = current_thd;
  bool full_access = has_full_view_routine_access(thd, schema_name_ptr->ptr(),
                                                  user_name.str, host_name.str);

  if (check_full_access) {
    return full_access ? 1 : 0;
  } else if (!full_access && !has_partial_view_routine_access(
                                 thd, schema_name_ptr->ptr(),
                                 routine_name_ptr->ptr(), is_procedure)) {
    return 0;
  }

  return 1;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    In order for INFORMATION_SCHEMA to skip listing event for which
    the user does not have rights, the following internal functions are used.

  Syntax:
    int CAN_ACCCESS_EVENT(schema_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/

longlong Item_func_can_access_event::val_int() {
  DBUG_TRACE;

  // Read schema_name
  String schema_name;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  if (schema_name_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();

  // Check if schema is hidden.
  THD *thd = current_thd;
  if (is_hidden_by_ndb(thd, schema_name_ptr, nullptr)) return 0;

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr())) return 1;

  // Check access
  if (check_access(thd, EVENT_ACL, schema_name_ptr->ptr(), nullptr, nullptr,
                   false, true)) {
    return 0;
  }

  return 1;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    In order for INFORMATION_SCHEMA to skip listing resource groups for which
    the user does not have rights, the following internal functions are used.

  Syntax:
    int CAN_ACCCESS_RESOURCE_GROUP(resource_group_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/

longlong Item_func_can_access_resource_group::val_int() {
  DBUG_TRACE;

  auto mgr_ptr = resourcegroups::Resource_group_mgr::instance();
  if (!mgr_ptr->resource_group_support()) {
    null_value = true;
    return false;
  }

  // Read resource group name.
  String res_grp_name;
  String *res_grp_name_ptr = args[0]->val_str(&res_grp_name);

  if (res_grp_name_ptr == nullptr) {
    null_value = true;
    return false;
  }

  // Make sure we have safe string to access.
  res_grp_name_ptr->c_ptr_safe();

  MDL_ticket *ticket = nullptr;
  if (mgr_ptr->acquire_shared_mdl_for_resource_group(
          current_thd, res_grp_name_ptr->c_ptr(), MDL_EXPLICIT, &ticket, false))
    return false;

  auto res_grp_ptr = mgr_ptr->get_resource_group(res_grp_name_ptr->c_ptr());
  longlong result = true;
  if (res_grp_ptr != nullptr) {
    Security_context *sctx = current_thd->security_context();
    if (res_grp_ptr->type() == resourcegroups::Type::SYSTEM_RESOURCE_GROUP) {
      if (!(sctx->check_access(SUPER_ACL) ||
            sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN"))
                .first))
        result = false;
    } else {
      if (!(sctx->check_access(SUPER_ACL) ||
            sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN"))
                .first ||
            sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_USER"))
                .first))
        result = false;
    }
  }
  mgr_ptr->release_shared_mdl_for_resource_group(current_thd, ticket);
  return res_grp_ptr != nullptr ? result : false;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    In order for INFORMATION_SCHEMA to skip listing column for which
    the user does not have rights, the following UDF's is used.

  Syntax:
    int CAN_ACCCESS_COLUMN(schema_name,
                           table_name,
                           field_name);

  @returns,
    1 - If current user has access.
    0 - If not.
*/
longlong Item_func_can_access_column::val_int() {
  DBUG_TRACE;

  // Read schema_name, table_name
  String schema_name;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String table_name;
  String *table_name_ptr = args[1]->val_str(&table_name);
  if (schema_name_ptr == nullptr || table_name_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();
  table_name_ptr->c_ptr_safe();

  // Check if table is hidden.
  THD *thd = current_thd;
  if (is_hidden_by_ndb(thd, schema_name_ptr, table_name_ptr)) return 0;

  // Read column_name.
  String column_name;
  String *column_name_ptr = args[2]->val_str(&column_name);
  if (column_name_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  column_name_ptr->c_ptr_safe();

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr())) return 1;

  // Check access
  GRANT_INFO grant_info;

  if (check_access(thd, SELECT_ACL, schema_name_ptr->ptr(),
                   &grant_info.privilege, nullptr, false, true))
    return 0;

  uint col_access =
      get_column_grant(thd, &grant_info, schema_name_ptr->ptr(),
                       table_name_ptr->ptr(), column_name_ptr->ptr()) &
      COL_ACLS;
  if (!col_access) {
    return 0;
  }

  return 1;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    In order for INFORMATION_SCHEMA to skip listing view definition
    for the user without rights, the following UDF's is used.

  Syntax:
    int CAN_ACCESS_VIEW(schema_name, view_name, definer, options);

  @returns,
    1 - If current user has access.
    0 - If not.
*/
longlong Item_func_can_access_view::val_int() {
  DBUG_TRACE;

  // Read schema_name, table_name
  String schema_name;
  String table_name;
  String definer;
  String options;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *table_name_ptr = args[1]->val_str(&table_name);
  String *definer_ptr = args[2]->val_str(&definer);
  String *options_ptr = args[3]->val_str(&options);
  if (schema_name_ptr == nullptr || table_name_ptr == nullptr ||
      definer_ptr == nullptr || options_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Make strings safe.
  schema_name_ptr->c_ptr_safe();
  table_name_ptr->c_ptr_safe();
  definer_ptr->c_ptr_safe();
  options_ptr->c_ptr_safe();

  // Skip INFORMATION_SCHEMA database
  if (is_infoschema_db(schema_name_ptr->ptr()) ||
      !my_strcasecmp(system_charset_info, schema_name_ptr->ptr(), "sys"))
    return 1;

  // Check if view is valid. If view is invalid then push invalid view
  // warning.
  bool is_view_valid = true;
  std::unique_ptr<dd::Properties> view_options(
      dd::Properties::parse_properties(options_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!view_options.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           options_ptr->c_ptr_safe());
    DBUG_ASSERT(false);
    return 0;
  }

  if (view_options->get("view_valid", &is_view_valid)) return 0;

  // Show warning/error if view is invalid.
  THD *thd = current_thd;
  const String db_str(schema_name_ptr->c_ptr_safe(), system_charset_info);
  const String name_str(table_name_ptr->c_ptr_safe(), system_charset_info);
  if (!is_view_valid &&
      !thd->lex->m_IS_table_stats.check_error_for_key(db_str, name_str)) {
    std::string err_message = push_view_warning_or_error(
        current_thd, schema_name_ptr->ptr(), table_name_ptr->ptr());

    /*
      Cache the error message, so that we do not show the same error multiple
      times.
     */
    thd->lex->m_IS_table_stats.store_error_message(db_str, name_str, nullptr,
                                                   err_message.c_str());
  }

  //
  // Check if definer user/host has access.
  //

  Security_context *sctx = thd->security_context();

  // NOTE: this is a copy/paste from sp_head::set_definer().

  char user_name_holder[USERNAME_LENGTH + 1];
  LEX_STRING user_name = {user_name_holder, USERNAME_LENGTH};

  char host_name_holder[HOSTNAME_LENGTH + 1];
  LEX_STRING host_name = {host_name_holder, HOSTNAME_LENGTH};

  parse_user(definer_ptr->ptr(), definer_ptr->length(), user_name.str,
             &user_name.length, host_name.str, &host_name.length);

  std::string definer_user(user_name.str, user_name.length);
  std::string definer_host(host_name.str, host_name.length);

  if (!strcmp(definer_user.c_str(), sctx->priv_user().str) &&
      !my_strcasecmp(system_charset_info, definer_host.c_str(),
                     sctx->priv_host().str))
    return 1;

  //
  // Check for ACL's
  //

  if ((thd->col_access & (SHOW_VIEW_ACL | SELECT_ACL)) ==
      (SHOW_VIEW_ACL | SELECT_ACL))
    return 1;

  TABLE_LIST table_list;
  uint view_access;
  table_list.db = schema_name_ptr->ptr();
  table_list.table_name = table_name_ptr->ptr();
  table_list.grant.privilege = thd->col_access;
  view_access = get_table_grant(thd, &table_list);
  if ((view_access & (SHOW_VIEW_ACL | SELECT_ACL)) ==
      (SHOW_VIEW_ACL | SELECT_ACL))
    return 1;

  return 0;
}

/**
  Skip hidden tables, columns, indexes and index elements.
  Do not skip them, when SHOW EXTENDED command are run.

  Syntax:
    longlong  IS_VISIBLE_DD_OBJECT(table_type, is_object_hidden);

  @returns,
    1 - If dd object is visible
    0 - If not visible.
*/
longlong Item_func_is_visible_dd_object::val_int() {
  DBUG_TRACE;

  DBUG_ASSERT(arg_count == 1 || arg_count == 2);
  DBUG_ASSERT(args[0]->null_value == false);

  if (args[0]->null_value || (arg_count == 2 && args[1]->null_value)) {
    null_value = true;
    return false;
  }

  null_value = false;
  THD *thd = current_thd;

  auto table_type =
      static_cast<dd::Abstract_table::enum_hidden_type>(args[0]->val_int());

  bool show_table = (table_type == dd::Abstract_table::HT_VISIBLE);

  // Make I_S.TABLES show the hidden system view 'show_statistics' for
  // testing purpose.
  DBUG_EXECUTE_IF("fetch_system_view_definition", { return 1; });

  if (thd->lex->m_extended_show)
    show_table =
        show_table || (table_type == dd::Abstract_table::HT_HIDDEN_DDL);

  if (arg_count == 1 || show_table == false) return (show_table ? 1 : 0);

  bool show_non_table_objects;
  if (thd->lex->m_extended_show)
    show_non_table_objects = true;
  else
    show_non_table_objects = (args[1]->val_bool() == false);

  return show_non_table_objects ? 1 : 0;
}

/**
  Get table statistics from dd::info_schema::get_table_statistics.

  @param      args       List of parameters in following order,

                         - Schema_name
                         - Table_name
                         - Engine_name
                         - se_private_id
                         - Hidden_table
                         - Tablespace_se_private_data
                         - Table_se_private_data (Used if stype is AUTO_INC)
                         - Partition name (optional argument).

  @param      arg_count  Number of arguments in 'args'

  @param      stype      Type of statistics that is requested

  @param[out] null_value Marked true indicating NULL, if there is no value.

  @returns ulonglong representing the statistics requested.
*/

static ulonglong get_table_statistics(
    Item **args, uint arg_count, dd::info_schema::enum_table_stats_type stype,
    bool *null_value) {
  DBUG_TRACE;
  *null_value = false;

  // Reads arguments
  String schema_name;
  String table_name;
  String engine_name;
  String ts_se_private_data;
  String tbl_se_private_data;
  String partition_name;
  String *partition_name_ptr = nullptr;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *table_name_ptr = args[1]->val_str(&table_name);
  String *engine_name_ptr = args[2]->val_str(&engine_name);
  bool skip_hidden_table = args[4]->val_int();
  String *ts_se_private_data_ptr = args[5]->val_str(&ts_se_private_data);
  ulonglong stat_data = args[6]->val_uint();
  ulonglong cached_timestamp = args[7]->val_uint();

  String *tbl_se_private_data_ptr = nullptr;

  /*
    The same native function used by I_S.TABLES is used by I_S.PARTITIONS.
    We invoke native function with partition name only with I_S.PARTITIONS
    as a last argument. So, we check for argument count below, before
    reading partition name.
  */
  if (stype == dd::info_schema::enum_table_stats_type::AUTO_INCREMENT) {
    tbl_se_private_data_ptr = args[8]->val_str(&tbl_se_private_data);
    if (arg_count == 10) partition_name_ptr = args[9]->val_str(&partition_name);
  } else if (arg_count == 9)
    partition_name_ptr = args[8]->val_str(&partition_name);

  if (schema_name_ptr == nullptr || table_name_ptr == nullptr ||
      engine_name_ptr == nullptr || skip_hidden_table) {
    *null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();
  table_name_ptr->c_ptr_safe();
  engine_name_ptr->c_ptr_safe();

  // Do not read dynamic stats for I_S tables.
  if (is_infoschema_db(schema_name_ptr->ptr())) return 0;

  // Read the statistic value from cache.
  THD *thd = current_thd;
  dd::Object_id se_private_id = (dd::Object_id)args[3]->val_uint();
  ulonglong result = thd->lex->m_IS_table_stats.read_stat(
      thd, *schema_name_ptr, *table_name_ptr, *engine_name_ptr,
      (partition_name_ptr ? partition_name_ptr->c_ptr_safe() : nullptr),
      se_private_id,
      (ts_se_private_data_ptr ? ts_se_private_data_ptr->c_ptr_safe() : nullptr),
      (tbl_se_private_data_ptr ? tbl_se_private_data_ptr->c_ptr_safe()
                               : nullptr),
      stat_data, cached_timestamp, stype);

  return result;
}

/**
  Get index size by key prefix passed.

  @param      args       List of parameters in following order,

                         - Schema_name
                         - Table_name
                         - Index_name
                         - prefix_key

  @param     function_name  Name of function to be used in error messages.


  @param[out] index_size stores index size for prefix key passed.

  @returns true if success or false otherwise
*/
static bool get_index_size_by_prefix(Item **args, const char *function_name,
                                     ulonglong *index_size) {
  DBUG_TRACE;

  bool result = true;
  // Read arguments
  String schema_name;
  String table_name;
  String index_name;

  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *table_name_ptr = args[1]->val_str(&table_name);
  String *index_name_ptr = args[2]->val_str(&index_name);

  // Validate args
  // All args should be non null and last args should be numeric
  if (schema_name_ptr == nullptr || table_name_ptr == nullptr ||
      index_name_ptr == nullptr) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), function_name);
    return false;
  }
  if (!is_integer_type(args[3]->data_type())) {
    auto msg = "GET_INDEX_SIZE_BY_PREFIX: 4th argument expected to be numeric";
    my_error(ER_WRONG_ARGUMENTS, MYF(0), msg);
    return false;
  }

  // Prepare temporary lex
  THD *thd = current_thd;
  LEX temp_lex, *lex;
  LEX *old_lex = thd->lex;
  thd->lex = lex = &temp_lex;
  lex_start(thd);
  LEX_CSTRING db_name_lex_cstr, table_name_lex_cstr;
  db_name_lex_cstr.str = schema_name_ptr->c_ptr_safe();
  db_name_lex_cstr.length = schema_name_ptr->length();
  table_name_lex_cstr.str = table_name_ptr->c_ptr_safe();
  table_name_lex_cstr.length = table_name_ptr->length();

  if (make_table_list(thd, lex->select_lex, db_name_lex_cstr,
                      table_name_lex_cstr)) {
    result = false;
    goto end;
  }

  TABLE_LIST *table_list;
  table_list = lex->select_lex->table_list.first;
  table_list->required_type = dd::enum_table_type::BASE_TABLE;
  if (open_tables_for_query(thd, table_list, 0)) {
    result = false;
    goto end;
  }

  {
    TABLE *table = table_list->table;
    uint key_index = 0;

    // Search for key with the index name.
    while (key_index < table->s->keys) {
      if (!my_strcasecmp(system_charset_info,
                         (table->key_info + key_index)->name,
                         index_name_ptr->c_ptr_safe()))
        break;

      key_index++;
    }

    if (key_index == table->s->keys) {
      // index is not found
      result = false;
      my_error(ER_NO_SUCH_INDEX, MYF(0), table_name_ptr->c_ptr_safe());
      goto end;
    } else {
      uchar key_buf[MAX_KEY_LENGTH + MAX_FIELD_WIDTH];
      KEY *index_info = table->key_info + key_index;
      auto field = index_info->key_part[0].field;
      if (!is_integer_type(field->type())) {
        result = false;
        auto msg =
            "GET_INDEX_SIZE_BY_PREFIX: Index keypart expected to be numeric";
        my_error(ER_WRONG_ARGUMENTS, MYF(0), msg);
        goto end;
      }
      auto old_map = tmp_use_all_columns(table, table->write_set);
      auto grd = create_scope_guard(
          [&]() { tmp_restore_column_map(table->write_set, old_map); });
      auto field_length = index_info->key_part[0].store_length;
      if (args[3]->save_in_field(field, false) != TYPE_OK) {
        result = false;
        my_error(ER_WRONG_ARGUMENTS, MYF(0), function_name);
        goto end;
      }
      key_copy(key_buf, table->record[0], index_info, field_length);
      key_range min_range;
      key_range max_range;
      min_range.key = key_buf;
      min_range.flag = HA_READ_KEY_EXACT;
      max_range.key = key_buf;
      max_range.flag = HA_READ_AFTER_KEY;
      min_range.length = max_range.length = field_length;
      min_range.keypart_map = max_range.keypart_map = 1;
      *index_size =
          table->file->records_size_in_range(key_index, &min_range, &max_range);
    }
  }
end:
  lex->unit->cleanup(thd, true);
  lex_end(thd->lex);
  thd->lex = old_lex;
  return result;
}

longlong Item_func_internal_table_rows::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::TABLE_ROWS,
      &null_value);

  if (null_value == false && result == (ulonglong)-1) null_value = true;

  return result;
}

longlong Item_func_get_index_size_by_prefix::val_int() {
  DBUG_TRACE;
  ulonglong return_value;
  bool result = get_index_size_by_prefix(args, func_name(), &return_value);
  return result ? return_value : 0;
}

longlong Item_func_internal_avg_row_length::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count,
      dd::info_schema::enum_table_stats_type::TABLE_AVG_ROW_LENGTH,
      &null_value);
  return result;
}

longlong Item_func_internal_data_length::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::DATA_LENGTH,
      &null_value);
  return result;
}

longlong Item_func_internal_max_data_length::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::MAX_DATA_LENGTH,
      &null_value);
  return result;
}

longlong Item_func_internal_index_length::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::INDEX_LENGTH,
      &null_value);
  return result;
}

longlong Item_func_internal_data_free::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::DATA_FREE,
      &null_value);

  if (null_value == false && result == (ulonglong)-1) null_value = true;

  return result;
}

longlong Item_func_internal_auto_increment::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::AUTO_INCREMENT,
      &null_value);

  if (null_value == false && result < (ulonglong)1) null_value = true;

  return result;
}

longlong Item_func_internal_checksum::val_int() {
  DBUG_TRACE;

  ulonglong result = get_table_statistics(
      args, arg_count, dd::info_schema::enum_table_stats_type::CHECKSUM,
      &null_value);

  if (null_value == false && result == 0) null_value = true;

  return result;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    INFORMATION_SCHEMA.STATISTICS.COMMENT is used to indicate if the indexes are
    disabled by ALTER TABLE ... DISABLE KEYS. This property of table is stored
    in mysql.tables.options as 'keys_disabled=0/1/'. This internal function
    returns value of option 'keys_disabled' for a given table.

  Syntax:
    int INTERNAL_KEYS_DISABLED(table_options);

  @returns,
    1 - If keys are disabled.
    0 - If not.
*/
longlong Item_func_internal_keys_disabled::val_int() {
  DBUG_TRACE;

  // Read options.
  String options;
  String *options_ptr = args[0]->val_str(&options);
  if (options_ptr == nullptr) return 0;

  // Read table option from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(options_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           options_ptr->c_ptr_safe());
    DBUG_ASSERT(false);
    return 0;
  }

  // Read keys_disabled sub type.
  uint keys_disabled = 0;
  p->get("keys_disabled", &keys_disabled);

  return keys_disabled;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    INFORMATION_SCHEMA.STATISTICS.CARDINALITY reads data from SE.

  Syntax:
    int INTERNAL_INDEX_COLUMN_CARDINALITY(
          schema_name,
          table_name,
          index_name,
          column_name,
          index_ordinal_position,
          column_ordinal_position,
          engine,
          se_private_id,
          is_hidden,
          stat_cardinality,
          cached_timestamp);

  @returns Cardinatily. Or sets null_value to true if cardinality is -1.
*/
longlong Item_func_internal_index_column_cardinality::val_int() {
  DBUG_TRACE;
  null_value = false;

  // Read arguments
  String schema_name;
  String table_name;
  String index_name;
  String column_name;
  String engine_name;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *table_name_ptr = args[1]->val_str(&table_name);
  String *index_name_ptr = args[2]->val_str(&index_name);
  String *column_name_ptr = args[3]->val_str(&column_name);
  uint index_ordinal_position = args[4]->val_uint();
  uint column_ordinal_position = args[5]->val_uint();
  String *engine_name_ptr = args[6]->val_str(&engine_name);
  dd::Object_id se_private_id = (dd::Object_id)args[7]->val_uint();
  bool hidden_index = args[8]->val_int();
  ulonglong stat_cardinality = args[9]->val_uint();
  ulonglong cached_timestamp = args[10]->val_uint();

  // stat_cardinality and cached_timestamp from mysql.index_stats can be null
  // when stat is fetched for 1st time without executing ANALYZE command.
  if (schema_name_ptr == nullptr || table_name_ptr == nullptr ||
      index_name_ptr == nullptr || engine_name_ptr == nullptr ||
      column_name_ptr == nullptr || args[4]->null_value ||
      args[5]->null_value || args[8]->null_value || hidden_index) {
    null_value = true;
    return 0;
  }

  // Make sure we have safe string to access.
  schema_name_ptr->c_ptr_safe();
  table_name_ptr->c_ptr_safe();
  index_name_ptr->c_ptr_safe();
  column_name_ptr->c_ptr_safe();
  engine_name_ptr->c_ptr_safe();

  ulonglong result = 0;
  THD *thd = current_thd;
  result = thd->lex->m_IS_table_stats.read_stat(
      thd, *schema_name_ptr, *table_name_ptr, *index_name_ptr, nullptr,
      *column_name_ptr, index_ordinal_position - 1, column_ordinal_position - 1,
      *engine_name_ptr, se_private_id, nullptr, nullptr, stat_cardinality,
      cached_timestamp,
      dd::info_schema::enum_table_stats_type::INDEX_COLUMN_CARDINALITY);

  if (result == (ulonglong)-1) null_value = true;

  return result;
}

/**
  Retrieve tablespace statistics from SE

  @param      thd        The current thread.

  @param      args       List of parameters in following order,

                         - Tablespace_name
                         - Engine_name
                         - Tablespace_se_private_data

  @param[out] null_value Marked true indicating NULL, if there is no value.
*/

void retrieve_tablespace_statistics(THD *thd, Item **args, bool *null_value) {
  DBUG_TRACE;
  *null_value = false;

  // Reads arguments
  String tablespace_name;
  String *tablespace_name_ptr = args[0]->val_str(&tablespace_name);
  String file_name;
  String *file_name_ptr = args[1]->val_str(&file_name);
  String engine_name;
  String *engine_name_ptr = args[2]->val_str(&engine_name);
  String ts_se_private_data;
  String *ts_se_private_data_ptr = args[3]->val_str(&ts_se_private_data);

  if (tablespace_name_ptr == nullptr || file_name_ptr == nullptr ||
      engine_name_ptr == nullptr) {
    *null_value = true;
    return;
  }

  // Make sure we have safe string to access.
  tablespace_name_ptr->c_ptr_safe();
  file_name_ptr->c_ptr_safe();
  engine_name_ptr->c_ptr_safe();

  // Read the statistic value from cache.
  if (thd->lex->m_IS_tablespace_stats.read_stat(
          thd, *tablespace_name_ptr, *file_name_ptr, *engine_name_ptr,
          (ts_se_private_data_ptr ? ts_se_private_data_ptr->c_ptr_safe()
                                  : nullptr)))
    *null_value = true;
}

longlong Item_func_internal_tablespace_id::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_ID, &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_logfile_group_number::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_LOGFILE_GROUP_NUMBER,
        &result);
    if (result == (ulonglong)-1) null_value = true;

    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_free_extents::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_FREE_EXTENTS, &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_total_extents::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_TOTAL_EXTENTS, &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_extent_size::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_EXTENT_SIZE, &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_initial_size::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_INITIAL_SIZE, &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_maximum_size::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_MAXIMUM_SIZE, &result);
    if (result == (ulonglong)-1) null_value = true;

    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_autoextend_size::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_AUTOEXTEND_SIZE,
        &result);
    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_version::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_VERSION, &result);
    if (result == (ulonglong)-1) null_value = true;

    return result;
  }

  return result;
}

longlong Item_func_internal_tablespace_data_free::val_int() {
  DBUG_TRACE;
  ulonglong result = -1;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_DATA_FREE, &result);
    return result;
  }

  return result;
}

Item_func_version::Item_func_version(const POS &pos)
    : Item_static_string_func(pos, NAME_STRING("version()"), server_version,
                              strlen(server_version), system_charset_info,
                              DERIVATION_SYSCONST) {}

/*
    Syntax:
      string get_dd_char_length()
*/
longlong Item_func_internal_dd_char_length::val_int() {
  DBUG_TRACE;
  null_value = false;

  dd::enum_column_types col_type = (dd::enum_column_types)args[0]->val_int();
  uint field_length = args[1]->val_int();
  String cs_name;
  String *cs_name_ptr = args[2]->val_str(&cs_name);
  uint flag = args[3]->val_int();

  // Stop if we found a NULL argument.
  if (args[0]->null_value || args[1]->null_value || cs_name_ptr == nullptr ||
      args[3]->null_value) {
    null_value = true;
    return 0;
  }

  // Read character set.
  CHARSET_INFO *cs = get_charset_by_name(cs_name_ptr->c_ptr_safe(), MYF(0));
  if (!cs) {
    null_value = true;
    return 0;
  }

  // Check data types for getting info
  enum_field_types field_type = dd_get_old_field_type(col_type);
  bool blob_flag = is_blob(field_type);
  if (!blob_flag && field_type != MYSQL_TYPE_ENUM &&
      field_type != MYSQL_TYPE_SET &&
      field_type != MYSQL_TYPE_VARCHAR &&  // For varbinary type
      field_type != MYSQL_TYPE_STRING)     // For binary type
  {
    null_value = true;
    return 0;
  }

  std::ostringstream oss("");
  switch (field_type) {
    case MYSQL_TYPE_BLOB:
      field_length = 65535;
      break;
    case MYSQL_TYPE_TINY_BLOB:
      field_length = 255;
      break;
    case MYSQL_TYPE_MEDIUM_BLOB:
      field_length = 16777215;
      break;
    case MYSQL_TYPE_LONG_BLOB:
      field_length = 4294967295;
      break;
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_STRING:
      break;
    default:
      break;
  }

  if (!flag && field_length) {
    if (blob_flag)
      return field_length / cs->mbminlen;
    else
      return field_length / cs->mbmaxlen;
  } else if (flag && field_length) {
    return field_length;
  }

  return 0;
}

longlong Item_func_internal_get_view_warning_or_error::val_int() {
  DBUG_TRACE;

  String schema_name;
  String table_name;
  String table_type;
  String *schema_name_ptr = args[0]->val_str(&schema_name);
  String *table_name_ptr = args[1]->val_str(&table_name);
  String *table_type_ptr = args[2]->val_str(&table_type);

  if (table_type_ptr == nullptr || schema_name_ptr == nullptr ||
      table_name_ptr == nullptr) {
    return 0;
  }

  String options;
  String *options_ptr = args[3]->val_str(&options);
  if (strcmp(table_type_ptr->c_ptr_safe(), "VIEW") == 0 &&
      options_ptr != nullptr) {
    bool is_view_valid = true;
    std::unique_ptr<dd::Properties> view_options(
        dd::Properties::parse_properties(options_ptr->c_ptr_safe()));

    // Warn if the property string is corrupt.
    if (!view_options.get()) {
      LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
             options_ptr->c_ptr_safe());
      DBUG_ASSERT(false);
      return 0;
    }

    // Return 0 if get_bool() or push_view_warning_or_error() fails
    if (view_options->get("view_valid", &is_view_valid)) return 0;

    if (is_view_valid == false) {
      push_view_warning_or_error(current_thd, schema_name_ptr->c_ptr_safe(),
                                 table_name_ptr->c_ptr_safe());
      return 0;
    }
  }

  return 1;
}

/**
  @brief
    INFORMATION_SCHEMA picks metadata from DD using system views.
    INFORMATION_SCHEMA.STATISTICS.SUB_PART represents index sub part length.
    This internal function is used to get index sub part length.

  Syntax:
    int GET_DD_INDEX_SUB_PART_LENGTH(
          index_column_usage_length,
          column_type,
          column_length,
          column_collation_id,
          index_type);

  @returns Index sub part length.
*/
longlong Item_func_get_dd_index_sub_part_length::val_int() {
  DBUG_TRACE;
  null_value = true;

  // Read arguments
  uint key_part_length = args[0]->val_int();
  dd::enum_column_types col_type =
      static_cast<dd::enum_column_types>(args[1]->val_int());
  uint column_length = args[2]->val_int();
  uint csid = args[3]->val_int();
  dd::Index::enum_index_type idx_type =
      static_cast<dd::Index::enum_index_type>(args[4]->val_int());
  if (args[0]->null_value || args[1]->null_value || args[2]->null_value ||
      args[3]->null_value || args[4]->null_value)
    return 0;

  // Read server col_type and check if we have key part.
  enum_field_types field_type = dd_get_old_field_type(col_type);
  if (!Field::type_can_have_key_part(field_type)) return 0;

  // Calculate the key length for the column. Note that we pass inn dummy values
  // for "decimals", "is_unsigned" and "elements" since none of those arguments
  // will affect the key length for any of the data types that can have a prefix
  // index (see Field::type_can_have_key_part above).
  uint32 column_key_length =
      calc_key_length(field_type, column_length, 0, false, 0);

  // Read column charset id from args[3]
  const CHARSET_INFO *column_charset = &my_charset_latin1;
  if (csid) {
    column_charset = get_charset(csid, MYF(0));
    DBUG_ASSERT(column_charset);
  }

  if ((idx_type != dd::Index::IT_FULLTEXT) &&
      (key_part_length != column_key_length)) {
    longlong sub_part_length = key_part_length / column_charset->mbmaxlen;
    null_value = false;
    return sub_part_length;
  }

  return 0;
}

/**
  @brief
   Internal function used by INFORMATION_SCHEMA implementation to check
   if a role is a mandatory role.

  Syntax:
    int INTERNAL_IS_MANDATORY_ROLE(role_user, role_host);

  @returns,
    1 - If the role is mandatory.
    0 - If not.
*/

longlong Item_func_internal_is_mandatory_role::val_int() {
  DBUG_TRACE;

  // Read schema_name
  String role_name;
  String *role_name_ptr = args[0]->val_str(&role_name);
  String role_host;
  String *role_host_ptr = args[1]->val_str(&role_host);
  if (role_name_ptr == nullptr || role_host_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  // Create Auth_id for ID being searched.
  LEX_CSTRING lex_user;
  lex_user.str = role_name_ptr->c_ptr_safe();
  lex_user.length = role_name_ptr->length();

  LEX_CSTRING lex_host;
  lex_host.str = role_host_ptr->c_ptr_safe();
  lex_host.length = role_host_ptr->length();

  bool is_mandatory{false};
  if (is_mandatory_role(lex_user, lex_host, &is_mandatory)) {
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING,
        ER_FAILED_TO_DETERMINE_IF_ROLE_IS_MANDATORY,
        ER_THD(current_thd, ER_FAILED_TO_DETERMINE_IF_ROLE_IS_MANDATORY),
        lex_user.str, lex_host.str);
  }

  return is_mandatory;
}

/**
  @brief
   Internal function used by INFORMATION_SCHEMA implementation to check
   if a role enabled.

  Syntax:
    int INTERNAL_IS_ENABLED_ROLE(role_user, role_host);

  @returns,
    1 - If the role is enabled.
    0 - If not.
*/

longlong Item_func_internal_is_enabled_role::val_int() {
  DBUG_TRACE;
  THD *thd = current_thd;

  // Read schema_name
  String role_name;
  String *role_name_ptr = args[0]->val_str(&role_name);
  String role_host;
  String *role_host_ptr = args[1]->val_str(&role_host);
  if (role_name_ptr == nullptr || role_host_ptr == nullptr) {
    null_value = true;
    return 0;
  }

  if (thd->m_main_security_ctx.get_active_roles()->size() == 0) return 0;

  // Create Auth_id for ID being searched.
  LEX_CSTRING lex_user;
  lex_user.str = role_name_ptr->c_ptr_safe();
  lex_user.length = role_name_ptr->length();

  LEX_CSTRING lex_host;
  lex_host.str = role_host_ptr->c_ptr_safe();
  lex_host.length = role_host_ptr->length();

  // Match the ID and return true if found.
  for (auto &rid : *thd->m_main_security_ctx.get_active_roles()) {
    if (rid == std::make_pair(lex_user, lex_host)) return 1;
  }

  return 0;
}

bool Item_func::ensure_multi_equality_fields_are_available_walker(uchar *arg) {
  const table_map reachable_tables = *pointer_cast<table_map *>(arg);
  for (uint i = 0; i < arg_count; ++i) {
    if (args[i]->type() == FIELD_ITEM) {
      args[i] =
          FindEqualField(down_cast<Item_field *>(args[i]), reachable_tables);
    }
  }
  return false;
}
