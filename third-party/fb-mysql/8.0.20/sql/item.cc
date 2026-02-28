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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/item.h"

#include "my_config.h"

#include <stdio.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <stddef.h>
#include <algorithm>
#include <utility>

#include "decimal.h"
#include "float.h"
#include "limits.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "mysql.h"  // IS_NUM
#include "mysql_time.h"
#include "sql/aggregate_check.h"  // Distinct_check
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // get_column_grant
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/derror.h"         // ER_THD
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/gis/srid.h"
#include "sql/item_cmpfunc.h"    // COND_EQUAL
#include "sql/item_create.h"     // create_temporal_literal
#include "sql/item_func.h"       // item_func_sleep_init
#include "sql/item_json_func.h"  // json_value
#include "sql/item_row.h"
#include "sql/item_strfunc.h"  // Item_func_conv_charset
#include "sql/item_subselect.h"
#include "sql/item_sum.h"  // Item_sum
#include "sql/json_dom.h"  // Json_wrapper
#include "sql/key.h"
#include "sql/log_event.h"  // append_query_string
#include "sql/mysqld.h"     // lower_case_table_names files_charset_info
#include "sql/protocol.h"
#include "sql/query_options.h"
#include "sql/select_lex_visitor.h"
#include "sql/sp.h"           // sp_map_item_type
#include "sql/sp_rcontext.h"  // sp_rcontext
#include "sql/sql_base.h"     // view_ref_found
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_show.h"  // append_identifier
#include "sql/sql_time.h"  // Date_time_format
#include "sql/sql_view.h"  // VIEW_ANY_ACL
#include "sql/system_variables.h"
#include "sql/thd_raii.h"
#include "template_utils.h"
#include "typelib.h"
#include "unsafe_string_append.h"

using std::max;
using std::min;
using std::string;

const String my_null_string("NULL", 4, default_charset_info);

/**
  Alias from select list can be referenced only from ORDER BY (SQL Standard) or
  from HAVING, GROUP BY and a subquery in the select list (MySQL extension).

  We don't allow it be referenced from the SELECT list, with one exception:
  it's accepted if nested in a subquery, which is inconsistent but necessary
  as our users have shown to rely on this workaround.
*/
static inline bool select_alias_referencable(enum_parsing_context place) {
  return (place == CTX_SELECT_LIST || place == CTX_GROUP_BY ||
          place == CTX_HAVING || place == CTX_ORDER_BY);
}

/*****************************************************************************
** Item functions
*****************************************************************************/

/**
  Init all special items.
*/

void item_init(void) {
  item_func_sleep_init();
  uuid_short_init();
}

Item::Item()
    : next_free(nullptr),
      str_value(),
      collation(&my_charset_bin, DERIVATION_COERCIBLE),
      item_name(),
      orig_name(),
      max_length(0),
      marker(MARKER_NONE),
      cmp_context(INVALID_RESULT),
      is_parser_item(false),
      runtime_item(false),
      is_expensive_cache(-1),
      m_data_type(244),  // One less than JSON (245)
      fixed(false),
      decimals(0),
      maybe_null(false),
      null_value(false),
      unsigned_flag(false),
      m_is_window_function(false),
      derived_used(false),
      m_accum_properties(0) {
#ifndef DBUG_OFF
  contextualized = true;
#endif  // DBUG_OFF

  // Put item into global list so that we can free all items at end
  current_thd->add_item(this);
}

Item::Item(THD *thd, const Item *item)
    : next_free(nullptr),
      str_value(item->str_value),
      collation(item->collation),
      item_name(item->item_name),
      orig_name(item->orig_name),
      max_length(item->max_length),
      marker(MARKER_NONE),
      cmp_context(item->cmp_context),
      is_parser_item(false),
      runtime_item(false),
      is_expensive_cache(-1),
      m_data_type(item->data_type()),
      fixed(item->fixed),
      decimals(item->decimals),
      maybe_null(item->maybe_null),
      null_value(item->null_value),
      unsigned_flag(item->unsigned_flag),
      m_is_window_function(item->m_is_window_function),
      derived_used(item->derived_used),
      m_accum_properties(item->m_accum_properties) {
#ifndef DBUG_OFF
  DBUG_ASSERT(item->contextualized);
  contextualized = true;
#endif  // DBUG_OFF

  // Add item to global list
  thd->add_item(this);
}

Item::Item(const POS &)
    : next_free(nullptr),
      str_value(),
      collation(&my_charset_bin, DERIVATION_COERCIBLE),
      item_name(),
      orig_name(),
      max_length(0),
      marker(MARKER_NONE),
      cmp_context(INVALID_RESULT),
      is_parser_item(true),
      runtime_item(false),
      is_expensive_cache(-1),
      m_data_type(244),
      fixed(false),
      decimals(0),
      maybe_null(false),
      null_value(false),
      unsigned_flag(false),
      m_is_window_function(false),
      derived_used(false),
      m_accum_properties(0) {}

/**
  @todo
    Make this functions class dependent
*/

bool Item::val_bool() {
  switch (result_type()) {
    case INT_RESULT:
      return val_int() != 0;
    case DECIMAL_RESULT: {
      my_decimal decimal_value;
      my_decimal *val = val_decimal(&decimal_value);
      if (val) return !my_decimal_is_zero(val);
      return false;
    }
    case REAL_RESULT:
    case STRING_RESULT:
      return val_real() != 0.0;
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
      return false;  // Wrong (but safe)
  }
}

/*
  For the items which don't have its own fast val_str_ascii()
  implementation we provide a generic slower version,
  which converts from the Item character set to ASCII.
  For better performance conversion happens only in
  case of a "tricky" Item character set (e.g. UCS2).
  Normally conversion does not happen.
*/
String *Item::val_str_ascii(String *str) {
  DBUG_ASSERT(str != &str_value);

  uint errors;
  String *res = val_str(&str_value);
  if (!res) return nullptr;

  if (!(res->charset()->state & MY_CS_NONASCII))
    str = res;
  else {
    if ((null_value = str->copy(res->ptr(), res->length(), collation.collation,
                                &my_charset_latin1, &errors)))
      return nullptr;
  }
  return str;
}

String *Item::val_string_from_real(String *str) {
  double nr = val_real();
  if (null_value) return nullptr; /* purecov: inspected */
  str->set_real(nr, decimals, &my_charset_bin);
  return str;
}

String *Item::val_string_from_int(String *str) {
  longlong nr = val_int();
  if (null_value) return nullptr;
  str->set_int(nr, unsigned_flag, &my_charset_bin);
  return str;
}

String *Item::val_string_from_decimal(String *str) {
  my_decimal dec_buf, *dec = val_decimal(&dec_buf);
  if (null_value) return error_str();
  my_decimal_round(E_DEC_FATAL_ERROR, dec, decimals, false, &dec_buf);
  my_decimal2string(E_DEC_FATAL_ERROR, &dec_buf, str);
  return str;
}

String *Item::val_string_from_datetime(String *str) {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_date(&ltime, TIME_FUZZY_DATE) ||
      (null_value = str->alloc(MAX_DATE_STRING_REP_LENGTH)))
    return error_str();
  make_datetime((Date_time_format *)nullptr, &ltime, str, decimals);
  return str;
}

String *Item::val_string_from_date(String *str) {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_date(&ltime, TIME_FUZZY_DATE) ||
      (null_value = str->alloc(MAX_DATE_STRING_REP_LENGTH)))
    return error_str();
  make_date((Date_time_format *)nullptr, &ltime, str);
  return str;
}

String *Item::val_string_from_time(String *str) {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_time(&ltime) || (null_value = str->alloc(MAX_DATE_STRING_REP_LENGTH)))
    return error_str();
  make_time((Date_time_format *)nullptr, &ltime, str, decimals);
  return str;
}

my_decimal *Item::val_decimal_from_real(my_decimal *decimal_value) {
  DBUG_TRACE;
  double nr = val_real();
  if (null_value) return nullptr;
  double2my_decimal(E_DEC_FATAL_ERROR, nr, decimal_value);
  return decimal_value;
}

my_decimal *Item::val_decimal_from_int(my_decimal *decimal_value) {
  longlong nr = val_int();
  if (null_value) return nullptr;
  int2my_decimal(E_DEC_FATAL_ERROR, nr, unsigned_flag, decimal_value);
  return decimal_value;
}

my_decimal *Item::val_decimal_from_string(my_decimal *decimal_value) {
  String *res;

  if (!(res = val_str(&str_value))) return nullptr;

  if (str2my_decimal(E_DEC_FATAL_ERROR & ~E_DEC_BAD_NUM, res->ptr(),
                     res->length(), res->charset(), decimal_value) &
      E_DEC_BAD_NUM) {
    ErrConvString err(res);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "DECIMAL", err.ptr());
  }
  return decimal_value;
}

my_decimal *Item::val_decimal_from_date(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_date(&ltime, TIME_FUZZY_DATE)) {
    /*
      The conversion may fail in strict mode. Do not return a NULL pointer,
      as the result may be used in subsequent arithmetic operations.
     */
    my_decimal_set_zero(decimal_value);
    null_value = true;  // set NULL, stop processing
    return decimal_value;
  }
  return date2my_decimal(&ltime, decimal_value);
}

my_decimal *Item::val_decimal_from_time(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_time(&ltime)) {
    my_decimal_set_zero(decimal_value);
    null_value = true;
    return nullptr;
  }
  return date2my_decimal(&ltime, decimal_value);
}

longlong Item::val_time_temporal() {
  MYSQL_TIME ltime;
  if ((null_value = get_time(&ltime))) return 0;
  return TIME_to_longlong_time_packed(ltime);
}

longlong Item::val_date_temporal() {
  MYSQL_TIME ltime;
  const sql_mode_t mode = current_thd->variables.sql_mode;
  const my_time_flags_t flags =
      TIME_FUZZY_DATE | (mode & MODE_INVALID_DATES ? TIME_INVALID_DATES : 0) |
      (mode & MODE_NO_ZERO_IN_DATE ? TIME_NO_ZERO_IN_DATE : 0) |
      (mode & MODE_NO_ZERO_DATE ? TIME_NO_ZERO_DATE : 0);
  if (get_date(&ltime, flags)) return error_int();
  return TIME_to_longlong_datetime_packed(ltime);
}

// TS-TODO: split into separate methods?
longlong Item::val_temporal_with_round(enum_field_types type, uint8 dec) {
  longlong nr = val_temporal_by_field_type();
  longlong diff =
      my_time_fraction_remainder(my_packed_time_get_frac_part(nr), dec);
  longlong abs_diff = diff > 0 ? diff : -diff;
  if (abs_diff * 2 >= (int)log_10_int[DATETIME_MAX_DECIMALS - dec]) {
    /* Needs rounding */
    switch (type) {
      case MYSQL_TYPE_TIME: {
        MYSQL_TIME ltime;
        TIME_from_longlong_time_packed(&ltime, nr);
        return my_time_adjust_frac(&ltime, dec,
                                   current_thd->is_fsp_truncate_mode())
                   ? 0
                   : TIME_to_longlong_time_packed(ltime);
      }
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATETIME: {
        MYSQL_TIME ltime;
        int warnings = 0;
        TIME_from_longlong_datetime_packed(&ltime, nr);
        return propagate_datetime_overflow(
                   current_thd, &warnings,
                   my_datetime_adjust_frac(&ltime, dec, &warnings,
                                           current_thd->is_fsp_truncate_mode()))
                   ? 0
                   : TIME_to_longlong_datetime_packed(ltime);
        return nr;
      }
      default:
        DBUG_ASSERT(0);
        break;
    }
  }
  /* Does not need rounding, do simple truncation. */
  nr -= diff;
  return nr;
}

double Item::val_real_from_decimal() {
  /* Note that fix_fields may not be called for Item_avg_field items */
  double result;
  my_decimal value_buff, *dec_val = val_decimal(&value_buff);
  if (null_value) return 0.0;
  my_decimal2double(E_DEC_FATAL_ERROR, dec_val, &result);
  return result;
}

longlong Item::val_int_from_decimal() {
  /* Note that fix_fields may not be called for Item_avg_field items */
  longlong result;
  my_decimal value, *dec_val = val_decimal(&value);
  if (null_value) return 0;
  my_decimal2int(E_DEC_FATAL_ERROR, dec_val, unsigned_flag, &result);
  return result;
}

longlong Item::val_int_from_time() {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  ulonglong value = 0;
  if (get_time(&ltime)) return 0LL;

  if (current_thd->is_fsp_truncate_mode())
    value = TIME_to_ulonglong_time(ltime);
  else
    value = TIME_to_ulonglong_time_round(ltime);

  return (ltime.neg ? -1 : 1) * value;
}

longlong Item::val_int_from_date() {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  return get_date(&ltime, TIME_FUZZY_DATE)
             ? 0LL
             : (longlong)TIME_to_ulonglong_date(ltime);
}

longlong Item::val_int_from_datetime() {
  DBUG_ASSERT(fixed == 1);
  MYSQL_TIME ltime;
  if (get_date(&ltime, TIME_FUZZY_DATE)) return 0LL;

  if (current_thd->is_fsp_truncate_mode())
    return TIME_to_ulonglong_datetime(ltime);
  else {
    return propagate_datetime_overflow(current_thd, [&](int *warnings) {
      return TIME_to_ulonglong_datetime_round(ltime, warnings);
    });
  }
}

type_conversion_status Item::save_time_in_field(Field *field) {
  MYSQL_TIME ltime;
  if (get_time(&ltime)) return set_field_to_null_with_conversions(field, false);
  field->set_notnull();
  return field->store_time(&ltime, decimals);
}

type_conversion_status Item::save_date_in_field(Field *field) {
  MYSQL_TIME ltime;
  my_time_flags_t flags = TIME_FUZZY_DATE;
  const sql_mode_t mode = field->table->in_use->variables.sql_mode;
  if (mode & MODE_INVALID_DATES) flags |= TIME_INVALID_DATES;
  if (get_date(&ltime, flags))
    return set_field_to_null_with_conversions(field, false);
  field->set_notnull();
  return field->store_time(&ltime, decimals);
}

/*
  Store the string value in field directly

  SYNOPSIS
    Item::save_str_value_in_field()
    field   a pointer to field where to store
    result  the pointer to the string value to be stored

  DESCRIPTION
    The method is used by Item_*::save_in_field_inner() implementations
    when we don't need to calculate the value to store
    See Item_string::save_in_field_inner() implementation for example

  IMPLEMENTATION
    Check if the Item is null and stores the NULL or the
    result value in the field accordingly.

  RETURN
    Nonzero value if error
*/

type_conversion_status Item::save_str_value_in_field(Field *field,
                                                     String *result) {
  if (null_value) return set_field_to_null(field);

  field->set_notnull();
  return field->store(result->ptr(), result->length(), collation.collation);
}

/**
  Aggregates data types from array of items into current item

  @param items  array of items to aggregate the type from

  This function aggregates all type information from the array of items.
  Found type is supposed to be used later as the result data type
  of a multi-argument function.
  Aggregation itself is performed partially by the Field::field_type_merge()
  function.
*/

void Item::aggregate_type(Bounds_checked_array<Item *> items) {
  uint itemno = 0;
  const uint count = items.size();
  while (itemno < count && items[itemno]->data_type() == MYSQL_TYPE_NULL)
    itemno++;

  if (itemno == count)  // All items have NULL type, consolidated type is NULL
  {
    set_data_type(MYSQL_TYPE_NULL);
    return;
  }

  DBUG_ASSERT(items[itemno]->result_type() != ROW_RESULT);

  enum_field_types new_type = items[itemno]->data_type();
  uint8 new_dec = items[itemno]->decimals;
  bool new_unsigned = items[itemno]->unsigned_flag;
  bool mixed_signs = false;

  for (itemno = itemno + 1; itemno < count; itemno++) {
    // Do not aggregate items with NULL type
    if (items[itemno]->data_type() == MYSQL_TYPE_NULL) continue;
    DBUG_ASSERT(items[itemno]->result_type() != ROW_RESULT);
    new_type = Field::field_type_merge(new_type, items[itemno]->data_type());
    mixed_signs |= (new_unsigned != items[itemno]->unsigned_flag);
    new_dec = max<uint8>(new_dec, items[itemno]->decimals);
  }
  if (mixed_signs && is_integer_type(new_type)) {
    bool bump_range = false;
    for (uint i = 0; i < count; i++)
      bump_range |= (items[i]->unsigned_flag &&
                     (items[i]->data_type() == new_type ||
                      items[i]->data_type() == MYSQL_TYPE_BIT));
    if (bump_range) {
      switch (new_type) {
        case MYSQL_TYPE_TINY:
          new_type = MYSQL_TYPE_SHORT;
          break;
        case MYSQL_TYPE_SHORT:
          new_type = MYSQL_TYPE_INT24;
          break;
        case MYSQL_TYPE_INT24:
          new_type = MYSQL_TYPE_LONG;
          break;
        case MYSQL_TYPE_LONG:
          new_type = MYSQL_TYPE_LONGLONG;
          break;
        case MYSQL_TYPE_LONGLONG:
          new_type = MYSQL_TYPE_NEWDECIMAL;
          break;
        default:
          break;
      }
    }
  }

  set_data_type(real_type_to_type(new_type));
  decimals = new_dec;
  unsigned_flag = new_unsigned && !mixed_signs;
  max_length = 0;
  return;
}

bool Item::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::contextualize(pc)) return true;

  // Add item to global list
  pc->thd->add_item(this);
  /*
    Item constructor can be called during execution other then SQL_COM
    command => we should check pc->select on zero
  */
  if (pc->select) {
    enum_parsing_context place = pc->select->parsing_place;
    if (place == CTX_SELECT_LIST || place == CTX_HAVING)
      pc->select->select_n_having_items++;
  }
  return false;
}

uint Item::decimal_precision() const {
  Item_result restype = result_type();

  if ((restype == DECIMAL_RESULT) || (restype == INT_RESULT)) {
    uint prec = my_decimal_length_to_precision(max_char_length(), decimals,
                                               unsigned_flag);
    return min<uint>(prec, DECIMAL_MAX_PRECISION);
  }
  switch (data_type()) {
    case MYSQL_TYPE_TIME:
      return decimals + TIME_INT_DIGITS;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      return decimals + DATETIME_INT_DIGITS;
    case MYSQL_TYPE_DATE:
      return decimals + DATE_INT_DIGITS;
    default:
      break;
  }
  return min<uint>(max_char_length(), DECIMAL_MAX_PRECISION);
}

uint Item::time_precision() {
  if (const_item() && result_type() == STRING_RESULT && !is_temporal()) {
    MYSQL_TIME ltime;
    String buf, *tmp;
    MYSQL_TIME_STATUS status;
    DBUG_ASSERT(fixed);
    // Nanosecond rounding is not needed, for performance purposes
    if ((tmp = val_str(&buf)) &&
        str_to_time(tmp, &ltime, TIME_FRAC_TRUNCATE, &status) == 0)
      return min(status.fractional_digits, uint{DATETIME_MAX_DECIMALS});
  }
  return min(decimals, uint8{DATETIME_MAX_DECIMALS});
}

uint Item::datetime_precision() {
  if (const_item() && result_type() == STRING_RESULT && !is_temporal()) {
    MYSQL_TIME ltime;
    String buf, *tmp;
    MYSQL_TIME_STATUS status;
    DBUG_ASSERT(fixed);
    // Nanosecond rounding is not needed, for performance purposes
    if ((tmp = val_str(&buf)) &&
        !propagate_datetime_overflow(
            current_thd, &status.warnings,
            str_to_datetime(tmp, &ltime, TIME_FRAC_TRUNCATE | TIME_FUZZY_DATE,
                            &status)))
      return min(status.fractional_digits, uint{DATETIME_MAX_DECIMALS});
  }
  return min(decimals, uint8{DATETIME_MAX_DECIMALS});
}

void Item::print_item_w_name(const THD *thd, String *str,
                             enum_query_type query_type) const {
  print(thd, str, query_type);

  if (item_name.is_set() && query_type != QT_NORMALIZED_FORMAT) {
    str->append(STRING_WITH_LEN(" AS "));
    append_identifier(thd, str, item_name.ptr(), item_name.length());
  }
}

/**
   @details
   "SELECT (subq) GROUP BY (same_subq)" confuses ONLY_FULL_GROUP_BY (it does
   not see that both subqueries are the same, raises an error).
   To avoid hitting this problem, if the original query was:
   "SELECT expression AS x GROUP BY x", we print "GROUP BY x", not
   "GROUP BY expression". Same for ORDER BY.
   This has practical importance for views created as
   "CREATE VIEW v SELECT (subq) AS x GROUP BY x"
   (print_order() is used to write the view's definition in the frm file).
   We make one exception: if the view is merge-able, its ORDER clause will be
   merged into the parent query's. If an identifier in the merged ORDER clause
   is allowed to be either an alias or an expression of the view's underlying
   tables, resolution is difficult: it may be to be found in the underlying
   tables of the view, or in the SELECT list of the view; unlike other ORDER
   elements directly originating from the parent query.
   To avoid this problem, if the view is merge-able, we print the
   expression. This does not cause problems with only_full_group_by, because a
   merge-able view never has GROUP BY. @see mysql_register_view().
*/
void Item::print_for_order(const THD *thd, String *str,
                           enum_query_type query_type, bool used_alias) const {
  if ((query_type & QT_NORMALIZED_FORMAT) != 0)
    str->append("?");
  else if (used_alias) {
    DBUG_ASSERT(item_name.is_set());
    // In the clause, user has referenced expression using an alias; we use it
    append_identifier(thd, str, item_name.ptr(), item_name.length());
  } else {
    if (type() == Item::INT_ITEM && basic_const_item()) {
      /*
        "ORDER BY N" means "order by the N-th element". To avoid such
        interpretation we write "ORDER BY ''", which is equivalent.
      */
      str->append("''");
    } else
      print(thd, str, query_type);
  }
}

void Item::cleanup() {
  DBUG_TRACE;
  fixed = false;
  marker = MARKER_NONE;
  if (orig_name.is_set()) item_name = orig_name;
}

bool Item::visitor_processor(uchar *arg) {
  Select_lex_visitor *visitor = pointer_cast<Select_lex_visitor *>(arg);
  return visitor->visit(this);
}

/**
  rename item (used for views, cleanup() return original name).

  @param new_name	new name of item;
*/

void Item::rename(char *new_name) {
  /*
    we can compare pointers to names here, because if name was not changed,
    pointer will be same
  */
  if (!orig_name.is_set() && new_name != item_name.ptr()) orig_name = item_name;
  item_name.set(new_name);
}

Item *Item::transform(Item_transformer transformer, uchar *arg) {
  return (this->*transformer)(arg);
}

bool Item_ident::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  context = pc->thd->lex->current_context();
  return false;
}

bool Item::check_function_as_value_generator(uchar *checker_args) {
  Check_function_as_value_generator_parameters *func_arg =
      pointer_cast<Check_function_as_value_generator_parameters *>(
          checker_args);
  Item_func *func_item = nullptr;
  if (type() == Item::FUNC_ITEM &&
      ((func_item = down_cast<Item_func *>(this)))) {
    func_arg->banned_function_name = func_item->func_name();
  }
  func_arg->err_code = func_arg->get_unnamed_function_error_code();
  return true;
}

void Item_ident::cleanup() {
  DBUG_TRACE;
  Item::cleanup();
  db_name = orig_db_name;
  table_name = orig_table_name;
  field_name = orig_field_name;
}

bool Item_ident::update_depended_from(uchar *arg) {
  auto *info = pointer_cast<Item_ident::Depended_change *>(arg);
  if (depended_from == info->old_depended_from)
    depended_from = info->new_depended_from;
  return false;
}

/**
  Store the pointer to this item field into a list if not already there.

  The method is used by Item::walk to collect all unique Item_field objects
  from a tree of Items into a set of items represented as a list.

  Item_cond::walk() and Item_func::walk() stop the evaluation of the
  processor function for its arguments once the processor returns
  true.Therefore in order to force this method being called for all item
  arguments in a condition the method must return false.

  @param arg  pointer to a List<Item_field>

  @return
    false to force the evaluation of collect_item_field_processor
    for the subsequent items.
*/

bool Item_field::collect_item_field_processor(uchar *arg) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("%s", field->field_name ? field->field_name : "noname"));
  List<Item_field> *item_list = (List<Item_field> *)arg;
  List_iterator<Item_field> item_list_it(*item_list);
  Item_field *curr_item;
  while ((curr_item = item_list_it++)) {
    if (curr_item->eq(this, true)) return false; /* Already in the set. */
  }
  item_list->push_back(this);
  return false;
}

bool Item_field::collect_item_field_or_view_ref_processor(uchar *arg) {
  auto *info = pointer_cast<Collect_item_fields_or_view_refs *>(arg);
  if (info->is_stopped(this)) return false;

  List_iterator<Item> item_list_it(*info->m_item_fields_or_view_refs);
  Item *curr_item;
  while ((curr_item = item_list_it++)) {
    if (curr_item->eq(this, true)) return false; /* Already in the set. */
  }
  info->m_item_fields_or_view_refs->push_back(this);
  return false;
}

bool Item_field::add_field_to_set_processor(uchar *arg) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("%s", field->field_name ? field->field_name : "noname"));
  TABLE *table = (TABLE *)arg;
  if (table_ref->table == table)
    bitmap_set_bit(&table->tmp_set, field->field_index);
  return false;
}

bool Item_field::add_field_to_cond_set_processor(uchar *) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("%s", field->field_name ? field->field_name : "noname"));
  bitmap_set_bit(&field->table->cond_set, field->field_index);
  return false;
}

bool Item_field::remove_column_from_bitmap(uchar *argument) {
  MY_BITMAP *bitmap = reinterpret_cast<MY_BITMAP *>(argument);
  bitmap_clear_bit(bitmap, field->field_index);
  return false;
}

/**
  Check if an Item_field references some field from a list of fields.

  Check whether the Item_field represented by 'this' references any
  of the fields in the keyparts passed via 'arg'. Used with the
  method Item::walk() to test whether any keypart in a sequence of
  keyparts is referenced in an expression.

  @param arg   Field being compared, arg must be of type Field

  @retval
    true  if 'this' references the field 'arg'
  @retval
    false otherwise
*/

bool Item_field::find_item_in_field_list_processor(uchar *arg) {
  KEY_PART_INFO *first_non_group_part = *((KEY_PART_INFO **)arg);
  KEY_PART_INFO *last_part = *(((KEY_PART_INFO **)arg) + 1);
  KEY_PART_INFO *cur_part;

  for (cur_part = first_non_group_part; cur_part != last_part; cur_part++) {
    if (field->eq(cur_part->field)) return true;
  }
  return false;
}

bool Item_field::check_function_as_value_generator(uchar *checker_args) {
  Check_function_as_value_generator_parameters *func_args =
      pointer_cast<Check_function_as_value_generator_parameters *>(
          checker_args);
  // We walk through the Item tree twice to check for disallowed functions;
  // once before resolving is done and once after resolving is done. Before
  // resolving is done, we don't have the field object available, and hence
  // the nullptr check.
  if (field == nullptr) {
    return false;
  }

  int fld_idx = func_args->col_index;
  DBUG_ASSERT(fld_idx > -1);

  /*
    Don't allow the GC (or default expression) to refer itself or another GC
    (or default expressions) that is defined after it.
  */
  if ((func_args->source != VGS_CHECK_CONSTRAINT) &&
      (field->is_gcol() ||
       field->has_insert_default_general_value_expression()) &&
      field->field_index >= fld_idx) {
    func_args->err_code = (func_args->source == VGS_GENERATED_COLUMN)
                              ? ER_GENERATED_COLUMN_NON_PRIOR
                              : ER_DEFAULT_VAL_GENERATED_NON_PRIOR;
    return true;
  }
  /*
    If a generated column, default expression or check constraint depends
    on an auto_increment column:
    - calculation of the generated value is done before write_row(),
    - but the auto_increment value is determined in write_row() by the
    engine.
    So this case is forbidden.
  */
  if (field->flags & AUTO_INCREMENT_FLAG) {
    func_args->err_code =
        (func_args->source == VGS_GENERATED_COLUMN)
            ? ER_GENERATED_COLUMN_REF_AUTO_INC
            : (func_args->source == VGS_DEFAULT_EXPRESSION)
                  ? ER_DEFAULT_VAL_GENERATED_REF_AUTO_INC
                  : ER_CHECK_CONSTRAINT_REFERS_AUTO_INCREMENT_COLUMN;
    return true;
  }

  return false;
}

/**
  Check privileges of base table column
*/

bool Item_field::check_column_privileges(uchar *arg) {
  THD *thd = (THD *)arg;

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);
  if (check_column_grant_in_table_ref(thd, table_ref, field_name,
                                      strlen(field_name),
                                      thd->want_privilege)) {
    return true;
  }

  return false;
}

/**
  Check privileges of view column.

  @note this function will be called for columns from views and derived tables,
  however privilege check for derived tables should be skipped
  (those columns are checked against the base tables).
*/

bool Item_view_ref::check_column_privileges(uchar *arg) {
  THD *thd = (THD *)arg;

  if (cached_table->is_derived())  // Rely on checking underlying tables
    return false;

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);

  DBUG_ASSERT(strlen(cached_table->get_table_name()) > 0);

  if (check_column_grant_in_table_ref(thd, cached_table, field_name,
                                      strlen(field_name), thd->want_privilege))
    return true;

  return false;
}

bool Item::may_evaluate_const(const THD *thd) const {
  return !(thd->lex->context_analysis_only & CONTEXT_ANALYSIS_ONLY_VIEW) &&
         (const_item() ||
          (const_for_execution() && thd->lex->is_query_tables_locked()));
}

bool Item::check_cols(uint c) {
  if (c != 1) {
    my_error(ER_OPERAND_COLUMNS, MYF(0), c);
    return true;
  }
  return false;
}

const Name_string null_name_string(nullptr, 0);

void Name_string::copy(const char *str, size_t length, const CHARSET_INFO *cs) {
  if (!length) {
    /* Empty string, used by AS or internal function like last_insert_id() */
    set(str ? "" : nullptr, 0);
    return;
  }
  if (cs->ctype) {
    /*
      This will probably need a better implementation in the future:
      a function in CHARSET_INFO structure.
    */
    while (length && !my_isgraph(cs, *str)) {  // Fix problem with yacc
      length--;
      str++;
    }
  }
  if (!my_charset_same(cs, system_charset_info)) {
    size_t res_length;
    char *tmp = sql_strmake_with_convert(str, length, cs, MAX_ALIAS_NAME,
                                         system_charset_info, &res_length);
    set(tmp, tmp ? res_length : 0);
  } else {
    size_t len = min<size_t>(length, MAX_ALIAS_NAME);
    char *tmp = sql_strmake(str, len);
    set(tmp, tmp ? len : 0);
  }
}

void Item_name_string::copy(const char *str_arg, size_t length_arg,
                            const CHARSET_INFO *cs_arg,
                            bool is_autogenerated_arg) {
  m_is_autogenerated = is_autogenerated_arg;
  copy(str_arg, length_arg, cs_arg);
  if (length_arg > length() && !is_autogenerated()) {
    ErrConvString tmp(str_arg, static_cast<uint>(length_arg), cs_arg);
    if (length() == 0)
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING, ER_NAME_BECOMES_EMPTY,
          ER_THD(current_thd, ER_NAME_BECOMES_EMPTY), tmp.ptr());
    else
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_REMOVED_SPACES,
                          ER_THD(current_thd, ER_REMOVED_SPACES), tmp.ptr());
  }
}

/**
  @details
  This function is called when:
  - Comparing items in the WHERE clause (when doing where optimization)
  - When trying to find an ORDER BY/GROUP BY item in the SELECT part
  - When matching fields in multiple equality objects (Item_equal)
*/

bool Item::eq(const Item *item, bool) const {
  /*
    Note, that this is never true if item is a Item_param:
    for all basic constants we have special checks, and Item_param's
    type() can be only among basic constant types.
  */
  return type() == item->type() && item_name.eq_safe(item->item_name);
}

Item *Item::safe_charset_converter(THD *thd, const CHARSET_INFO *tocs) {
  Item_func_conv_charset *conv =
      new Item_func_conv_charset(thd, this, tocs, true);
  return conv && conv->safe ? conv : nullptr;
}

/**
  @details
  Created mostly for mysql_prepare_table(). Important
  when a string ENUM/SET column is described with a numeric default value:

  CREATE TABLE t1(a SET('a') DEFAULT 1);

  We cannot use generic Item::safe_charset_converter(), because
  the latter returns a non-fixed Item, so val_str() crashes afterwards.
  Override Item_num method, to return a fixed item.
*/
Item *Item_num::safe_charset_converter(THD *thd, const CHARSET_INFO *tocs) {
  /*
    Item_num returns pure ASCII result,
    so conversion is needed only in case of "tricky" character
    sets like UCS2. If tocs is not "tricky", return the item itself.
  */
  if (!(tocs->state & MY_CS_NONASCII)) return this;

  uint conv_errors;
  char buf[64], buf2[64];
  String tmp(buf, sizeof(buf), &my_charset_bin);
  String cstr(buf2, sizeof(buf2), &my_charset_bin);
  String *ostr = val_str(&tmp);
  cstr.copy(ostr->ptr(), ostr->length(), ostr->charset(), tocs, &conv_errors);
  if (conv_errors > 0) {
    /*
      Safe conversion is not possible.
      We could not convert a string into the requested character set
      without data loss. The target charset does not cover all the
      characters from the string. Operation cannot be done correctly.
    */
    return nullptr;
  }

  char *ptr = thd->strmake(cstr.ptr(), cstr.length());
  if (ptr == nullptr) return nullptr;
  auto conv =
      new Item_string(ptr, cstr.length(), cstr.charset(), collation.derivation);
  if (conv == nullptr) return nullptr;

  /* Ensure that no one is going to change the result string */
  conv->mark_result_as_const();
  conv->fix_char_length(max_char_length());
  return conv;
}

Item *Item_func_pi::safe_charset_converter(THD *thd, const CHARSET_INFO *) {
  char buf[64];
  String tmp(buf, sizeof(buf), &my_charset_bin);
  String *s = val_str(&tmp);
  char *ptr = thd->strmake(s->ptr(), s->length());
  if (ptr == nullptr) return nullptr;
  auto conv =
      new Item_static_string_func(func_name, ptr, s->length(), s->charset());
  if (conv == nullptr) return nullptr;
  conv->mark_result_as_const();
  return conv;
}

Item *Item_string::safe_charset_converter(THD *thd, const CHARSET_INFO *tocs) {
  return charset_converter(thd, tocs, true);
}

/**
  Convert a string item into the requested character set.

  @param thd        Thread handle.
  @param tocs       Character set to to convert the string to.
  @param lossless   Whether data loss is acceptable.

  @return A new item representing the converted string.
*/
Item *Item_string::charset_converter(THD *thd, const CHARSET_INFO *tocs,
                                     bool lossless) {
  uint conv_errors;
  String tmp, cstr, *ostr = val_str(&tmp);
  cstr.copy(ostr->ptr(), ostr->length(), ostr->charset(), tocs, &conv_errors);
  if (lossless && conv_errors > 0) {
    /*
      Safe conversion is not possible.
      We could not convert a string into the requested character set
      without data loss. The target charset does not cover all the
      characters from the string. Operation cannot be done correctly.
    */
    return nullptr;
  }

  char *ptr = thd->strmake(cstr.ptr(), cstr.length());
  if (ptr == nullptr) return nullptr;
  auto conv =
      new Item_string(ptr, cstr.length(), cstr.charset(), collation.derivation);
  if (conv == nullptr) return nullptr;
  /* Ensure that no one is going to change the result string */
  conv->mark_result_as_const();
  return conv;
}

Item *Item_param::safe_charset_converter(THD *thd, const CHARSET_INFO *tocs) {
  if (may_evaluate_const(thd)) {
    String tmp, cstr, *ostr = val_str(&tmp);

    if (null_value) {
      auto cnvitem = new Item_null();
      if (cnvitem == nullptr) return nullptr;
      cnvitem->collation.set(tocs);
      return cnvitem;
    } else {
      uint conv_errors;
      cstr.copy(ostr->ptr(), ostr->length(), ostr->charset(), tocs,
                &conv_errors);

      if (conv_errors > 0) return nullptr;

      char *ptr = thd->strmake(cstr.ptr(), cstr.length());
      if (ptr == nullptr) return nullptr;
      auto cnvitem = new Item_string(ptr, cstr.length(), cstr.charset(),
                                     collation.derivation);
      if (cnvitem == nullptr) return nullptr;
      cnvitem->mark_result_as_const();
      return cnvitem;
    }
  }
  return Item::safe_charset_converter(thd, tocs);
}

Item *Item_static_string_func::safe_charset_converter(
    THD *thd, const CHARSET_INFO *tocs) {
  uint conv_errors;
  String tmp, cstr, *ostr = val_str(&tmp);
  cstr.copy(ostr->ptr(), ostr->length(), ostr->charset(), tocs, &conv_errors);
  if (conv_errors > 0) {
    /*
      Safe conversion is not possible.
      We could not convert a string into the requested character set
      without data loss. The target charset does not cover all the
      characters from the string. Operation cannot be done correctly.
    */
    return nullptr;
  }

  char *ptr = thd->strmake(cstr.ptr(), cstr.length());
  if (ptr == nullptr) return nullptr;
  auto conv = new Item_static_string_func(func_name, ptr, cstr.length(),
                                          cstr.charset(), collation.derivation);
  if (conv == nullptr) return nullptr;
  /* Ensure that no one is going to change the result string */
  conv->mark_result_as_const();
  return conv;
}

bool Item_string::eq(const Item *item, bool binary_cmp) const {
  if (type() == item->type() && item->basic_const_item()) {
    // Should be OK for a basic constant.
    Item *arg = const_cast<Item *>(item);
    String str;
    if (binary_cmp) return !stringcmp(&str_value, arg->val_str(&str));
    return (collation.collation == arg->collation.collation &&
            !sortcmp(&str_value, arg->val_str(&str), collation.collation));
  }
  return false;
}

bool Item::get_date_from_string(MYSQL_TIME *ltime, my_time_flags_t flags) {
  char buff[MAX_DATE_STRING_REP_LENGTH];
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  if (!(res = val_str(&tmp))) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
    return true;
  }
  return str_to_datetime_with_warn(res, ltime, flags);
}

bool Item::get_date_from_real(MYSQL_TIME *ltime, my_time_flags_t flags) {
  double value = val_real();
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
    return true;
  }
  return my_double_to_datetime_with_warn(value, ltime, flags);
}

bool Item::get_date_from_decimal(MYSQL_TIME *ltime, my_time_flags_t flags) {
  my_decimal buf, *decimal = val_decimal(&buf);
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
    return true;
  }
  return my_decimal_to_datetime_with_warn(decimal, ltime, flags);
}

bool Item::get_date_from_int(MYSQL_TIME *ltime, my_time_flags_t flags) {
  longlong value = val_int();
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
    return true;
  }
  return my_longlong_to_datetime_with_warn(value, ltime, flags);
}

bool Item::get_date_from_time(MYSQL_TIME *ltime) {
  MYSQL_TIME tm;
  if (get_time(&tm)) {
    DBUG_ASSERT(null_value || current_thd->is_error());
    return true;
  }
  time_to_datetime(current_thd, &tm, ltime);
  return false;
}

bool Item::get_date_from_numeric(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  switch (result_type()) {
    case REAL_RESULT:
      return get_date_from_real(ltime, fuzzydate);
    case DECIMAL_RESULT:
      return get_date_from_decimal(ltime, fuzzydate);
    case INT_RESULT:
      return get_date_from_int(ltime, fuzzydate);
    case STRING_RESULT:
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(0);
  }
  return (null_value = true);  // Impossible result_type
}

/**
  Get the value of the function as a MYSQL_TIME structure.
  As a extra convenience the time structure is reset on error!
*/

bool Item::get_date_from_non_temporal(MYSQL_TIME *ltime,
                                      my_time_flags_t fuzzydate) {
  DBUG_ASSERT(!is_temporal());
  switch (result_type()) {
    case STRING_RESULT:
      return get_date_from_string(ltime, fuzzydate);
    case REAL_RESULT:
      return get_date_from_real(ltime, fuzzydate);
    case DECIMAL_RESULT:
      return get_date_from_decimal(ltime, fuzzydate);
    case INT_RESULT:
      return get_date_from_int(ltime, fuzzydate);
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(0);
  }
  return (null_value = true);  // Impossible result_type
}

bool Item::get_time_from_string(MYSQL_TIME *ltime) {
  char buff[MAX_DATE_STRING_REP_LENGTH];
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  if (!(res = val_str(&tmp))) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    return true;
  }
  return str_to_time_with_warn(res, ltime);
}

bool Item::get_time_from_real(MYSQL_TIME *ltime) {
  double value = val_real();
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    return true;
  }
  return my_double_to_time_with_warn(value, ltime);
}

bool Item::get_time_from_decimal(MYSQL_TIME *ltime) {
  my_decimal buf, *decimal = val_decimal(&buf);
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    return true;
  }
  return my_decimal_to_time_with_warn(decimal, ltime);
}

bool Item::get_time_from_int(MYSQL_TIME *ltime) {
  DBUG_ASSERT(!is_temporal());
  longlong value = val_int();
  if (null_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    return true;
  }
  return my_longlong_to_time_with_warn(value, ltime);
}

bool Item::get_time_from_date(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed == 1);
  if (get_date(ltime, TIME_FUZZY_DATE))  // Need this check if NULL value
    return true;
  set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
  return false;
}

bool Item::get_time_from_datetime(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed == 1);
  if (get_date(ltime, TIME_FUZZY_DATE)) return true;
  datetime_to_time(ltime);
  return false;
}

bool Item::get_time_from_numeric(MYSQL_TIME *ltime) {
  DBUG_ASSERT(!is_temporal());
  switch (result_type()) {
    case REAL_RESULT:
      return get_time_from_real(ltime);
    case DECIMAL_RESULT:
      return get_time_from_decimal(ltime);
    case INT_RESULT:
      return get_time_from_int(ltime);
    case STRING_RESULT:
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(0);
  }
  return (null_value = true);  // Impossible result type
}

/**
  Get time value from int, real, decimal or string.

  As a extra convenience the time structure is reset on error!
*/

bool Item::get_time_from_non_temporal(MYSQL_TIME *ltime) {
  DBUG_ASSERT(!is_temporal());
  switch (result_type()) {
    case STRING_RESULT:
      return get_time_from_string(ltime);
    case REAL_RESULT:
      return get_time_from_real(ltime);
    case DECIMAL_RESULT:
      return get_time_from_decimal(ltime);
    case INT_RESULT:
      return get_time_from_int(ltime);
    case ROW_RESULT:
    case INVALID_RESULT:
      DBUG_ASSERT(0);
  }
  return (null_value = true);  // Impossible result type
}

/**
   If argument is NULL, sets null_value. Otherwise:
   if invalid DATETIME value, or a valid DATETIME value but which is out of
   the supported Unix timestamp range, sets 'tm' to 0.
*/
bool Item::get_timeval(struct timeval *tm, int *warnings) {
  MYSQL_TIME ltime;
  if (get_date(&ltime, TIME_FUZZY_DATE)) {
    if (null_value) return true; /* Value is NULL */
    goto zero;                   /* Could not extract date from the value */
  }
  if (datetime_to_timeval(current_thd, &ltime, tm, warnings))
    goto zero;  /* Value is out of the supported range */
  return false; /* Value is a good Unix timestamp */
zero:
  tm->tv_sec = tm->tv_usec = 0;
  return false;
}

const CHARSET_INFO *Item::default_charset() {
  return current_thd->variables.collation_connection;
}

/*
  Save value in field, but don't give any warnings

  NOTES
   This is used to temporary store and retrieve a value in a column,
   for example in opt_range to adjust the key value to fit the column.
*/

type_conversion_status Item::save_in_field_no_warnings(Field *field,
                                                       bool no_conversions) {
  DBUG_TRACE;
  TABLE *table = field->table;
  THD *thd = table->in_use;
  enum_check_fields tmp = thd->check_for_truncated_fields;
  my_bitmap_map *old_map = dbug_tmp_use_all_columns(table, table->write_set);
  sql_mode_t sql_mode = thd->variables.sql_mode;
  /*
    For cases like data truncation still warning is reported here. Which was
    avoided before with THD::abort_on_warning flag. Since the flag is removed
    now, until MODE_NO_ZERO_IN_DATE, MODE_NO_ZERO_DATE and
    MODE_ERROR_FOR_DIVISION_BY_ZERO are merged with strict mode, removing even
    strict modes from sql_mode here to avoid warnings.
  */
  thd->variables.sql_mode &=
      ~(MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE | MODE_STRICT_ALL_TABLES |
        MODE_STRICT_TRANS_TABLES);
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;

  const type_conversion_status res = save_in_field(field, no_conversions);

  thd->check_for_truncated_fields = tmp;
  dbug_tmp_restore_column_map(table->write_set, old_map);
  thd->variables.sql_mode = sql_mode;
  return res;
}

bool Item::is_blob_field() const {
  DBUG_ASSERT(fixed);

  enum_field_types type = data_type();
  return (type == MYSQL_TYPE_BLOB || type == MYSQL_TYPE_GEOMETRY ||
          // Char length, not the byte one, should be taken into account
          max_length / collation.collation->mbmaxlen >
              CONVERT_IF_BIGGER_TO_BLOB);
}

/*****************************************************************************
  Item_sp_variable methods
*****************************************************************************/

Item_sp_variable::Item_sp_variable(const Name_string sp_var_name)
    : m_thd(nullptr),
      m_name(sp_var_name)
#ifndef DBUG_OFF
      ,
      m_sp(nullptr)
#endif
{
}

bool Item_sp_variable::fix_fields(THD *thd, Item **) {
  m_thd = thd; /* NOTE: this must be set before any this_xxx() */
  Item *it = this_item();

  DBUG_ASSERT(it->fixed);

  max_length = it->max_length;
  decimals = it->decimals;
  unsigned_flag = it->unsigned_flag;
  collation.set(it->collation);
  set_data_type(it->data_type());

  fixed = true;

  return false;
}

double Item_sp_variable::val_real() {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  double ret = it->val_real();
  null_value = it->null_value;
  return ret;
}

longlong Item_sp_variable::val_int() {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  longlong ret = it->val_int();
  null_value = it->null_value;
  return ret;
}

String *Item_sp_variable::val_str(String *sp) {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  String *res = it->val_str(sp);

  null_value = it->null_value;

  if (!res) return nullptr;

  /*
    This way we mark returned value of val_str as const,
    so that various functions (e.g. CONCAT) won't try to
    modify the value of the Item. Analogous mechanism is
    implemented for Item_param.
    Without this trick Item_splocal could be changed as a
    side-effect of expression computation. Here is an example
    of what happens without it: suppose x is varchar local
    variable in a SP with initial value 'ab' Then
      select concat(x,'c');
    would change x's value to 'abc', as Item_func_concat::val_str()
    would use x's internal buffer to compute the result.
    This is intended behaviour of Item_func_concat. Comments to
    Item_param class contain some more details on the topic.
  */

  if (res != &str_value)
    str_value.set(res->ptr(), res->length(), res->charset());
  else
    res->mark_as_const();

  return &str_value;
}

my_decimal *Item_sp_variable::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  my_decimal *val = it->val_decimal(decimal_value);
  null_value = it->null_value;
  return val;
}

bool Item_sp_variable::val_json(Json_wrapper *wr) {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  bool result = it->val_json(wr);
  null_value = it->null_value;
  return result;
}

bool Item_sp_variable::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  return (null_value = it->get_date(ltime, fuzzydate));
}

bool Item_sp_variable::get_time(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed);
  Item *it = this_item();
  return (null_value = it->get_time(ltime));
}

bool Item_sp_variable::is_null() { return this_item()->is_null(); }

/*****************************************************************************
  Item_splocal methods
*****************************************************************************/

Item_splocal::Item_splocal(const Name_string sp_var_name, uint sp_var_idx,
                           enum_field_types sp_var_type, uint pos_in_q,
                           uint len_in_q)
    : Item_sp_variable(sp_var_name),
      m_var_idx(sp_var_idx),
      limit_clause_param(false),
      pos_in_query(pos_in_q),
      len_in_query(len_in_q) {
  maybe_null = true;

  sp_var_type = real_type_to_type(sp_var_type);
  m_type = sp_map_item_type(sp_var_type);
  set_data_type(sp_var_type);
  m_result_type = sp_map_result_type(sp_var_type);
}

Item *Item_splocal::this_item() {
  DBUG_ASSERT(m_sp == m_thd->sp_runtime_ctx->sp);

  return m_thd->sp_runtime_ctx->get_item(m_var_idx);
}

const Item *Item_splocal::this_item() const {
  DBUG_ASSERT(m_sp == m_thd->sp_runtime_ctx->sp);

  return m_thd->sp_runtime_ctx->get_item(m_var_idx);
}

Item **Item_splocal::this_item_addr(THD *thd, Item **) {
  DBUG_ASSERT(m_sp == thd->sp_runtime_ctx->sp);

  return thd->sp_runtime_ctx->get_item_addr(m_var_idx);
}

bool Item_splocal::val_json(Json_wrapper *result) {
  Item *it = this_item();
  bool ret = it->val_json(result);
  null_value = it->null_value;
  return ret;
}

void Item_splocal::print(const THD *, String *str, enum_query_type) const {
  str->reserve(m_name.length() + 8);
  str->append(m_name);
  str->append('@');
  qs_append(m_var_idx, str);
}

bool Item_splocal::set_value(THD *thd, sp_rcontext *ctx, Item **it) {
  return ctx->set_variable(thd, get_var_idx(), it);
}

/*****************************************************************************
  Item_case_expr methods
*****************************************************************************/

Item_case_expr::Item_case_expr(uint case_expr_id)
    : Item_sp_variable(Name_string(STRING_WITH_LEN("case_expr"))),
      m_case_expr_id(case_expr_id) {}

Item *Item_case_expr::this_item() {
  DBUG_ASSERT(m_sp == m_thd->sp_runtime_ctx->sp);

  return m_thd->sp_runtime_ctx->get_case_expr(m_case_expr_id);
}

const Item *Item_case_expr::this_item() const {
  DBUG_ASSERT(m_sp == m_thd->sp_runtime_ctx->sp);

  return m_thd->sp_runtime_ctx->get_case_expr(m_case_expr_id);
}

Item **Item_case_expr::this_item_addr(THD *thd, Item **) {
  DBUG_ASSERT(m_sp == thd->sp_runtime_ctx->sp);

  return thd->sp_runtime_ctx->get_case_expr_addr(m_case_expr_id);
}

void Item_case_expr::print(const THD *, String *str, enum_query_type) const {
  if (str->reserve(MAX_INT_WIDTH + sizeof("case_expr@")))
    return; /* purecov: inspected */
  (void)str->append(STRING_WITH_LEN("case_expr@"));
  qs_append(m_case_expr_id, str);
}

/*****************************************************************************
  Item_name_const methods
*****************************************************************************/

double Item_name_const::val_real() {
  DBUG_ASSERT(fixed);
  double ret = value_item->val_real();
  null_value = value_item->null_value;
  return ret;
}

longlong Item_name_const::val_int() {
  DBUG_ASSERT(fixed);
  longlong ret = value_item->val_int();
  null_value = value_item->null_value;
  return ret;
}

String *Item_name_const::val_str(String *sp) {
  DBUG_ASSERT(fixed);
  String *ret = value_item->val_str(sp);
  null_value = value_item->null_value;
  return ret;
}

my_decimal *Item_name_const::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed);
  my_decimal *val = value_item->val_decimal(decimal_value);
  null_value = value_item->null_value;
  return val;
}

bool Item_name_const::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed);
  return (null_value = value_item->get_date(ltime, fuzzydate));
}

bool Item_name_const::get_time(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed);
  return (null_value = value_item->get_time(ltime));
}

bool Item_name_const::is_null() { return value_item->is_null(); }

Item_name_const::Item_name_const(const POS &pos, Item *name_arg, Item *val)
    : super(pos), value_item(val), name_item(name_arg) {
  maybe_null = true;
}

bool Item_name_const::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res) || value_item->itemize(pc, &value_item) ||
      name_item->itemize(pc, &name_item))
    return true;
  /*
    The name and value argument to NAME_CONST can only be a literal constant.
    This (internal, although documented) feature is only supported for the
    stored procedure binlog's needs, cf. subst_spvars().

    Apart from plain literals, some extra logic are needed to support a
    collation specifier and to handle negative constant values.
  */
  valid_args = false;

  if (name_item->basic_const_item()) {
    Item_func *func = dynamic_cast<Item_func *>(value_item);
    Item *possible_const = value_item;

    if (func && (func->functype() == Item_func::COLLATE_FUNC ||
                 func->functype() == Item_func::NEG_FUNC)) {
      /*
        The value is not a literal constant. Accept it if it's a
        COLLATE_FUNC or a NEG_FUNC wrapping a literal constant.
      */
      possible_const = func->key_item();
    }

    /*
      There should now be no constant items which are functions left,
      (e.g. like TIME '1'), since none such are generated by subst_spvars() and
      sp_get_item_value(), which is where NAME_CONST calls are generated
      internally for the binary log: hence the second predicate below.  If user
      applications try to use such constructs, or any non-constant contents for
      NAME_CONST's value argument (#2), we generate an error.
    */
    valid_args = (possible_const->basic_const_item() &&
                  possible_const->type() != FUNC_ITEM);
  }

  if (!valid_args) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), "NAME_CONST");
    return true;
  }

  return false;
}

Item::Type Item_name_const::type() const {
  /*
    As
    1. one can try to create the Item_name_const passing non-constant
    arguments, although it's incorrect and
    2. the type() method can be called before the fix_fields() to get
    type information for a further type cast, e.g.
    if (item->type() == FIELD_ITEM)
      ((Item_field *) item)->...
    we return NULL_ITEM in the case to avoid wrong casting.

    valid_args guarantees value_item->basic_const_item(); if type is
    FUNC_ITEM, then we have a fudged item_func_neg() on our hands
    and return the underlying type.
    For Item_func_set_collation()
    e.g. NAME_CONST('name', 'value' COLLATE collation) we return its
    'value' argument type.
  */
  if (!valid_args) return NULL_ITEM;
  Item::Type value_type = value_item->type();
  if (value_type == FUNC_ITEM) {
    /*
      The second argument of NAME_CONST('name', 'value') must be
      a simple constant item or a NEG_FUNC/COLLATE_FUNC.
    */
    Item_func *func = down_cast<Item_func *>(value_item);
    DBUG_ASSERT(func->functype() == Item_func::NEG_FUNC ||
                func->functype() == Item_func::COLLATE_FUNC);
    return func->key_item()->type();
  }
  return value_type;
}

bool Item_name_const::fix_fields(THD *thd, Item **) {
  char buf[128];
  String *tmp;
  String s(buf, sizeof(buf), &my_charset_bin);
  s.length(0);

  if (value_item->fix_fields(thd, &value_item) ||
      name_item->fix_fields(thd, &name_item) || !value_item->const_item() ||
      !name_item->const_item() ||
      !(tmp = name_item->val_str(&s)))  // Can't have a NULL name
  {
    my_error(ER_RESERVED_SYNTAX, MYF(0), "NAME_CONST");
    return true;
  }
  if (item_name.is_autogenerated()) {
    item_name.copy(tmp->ptr(), (uint)tmp->length(), system_charset_info);
  }
  collation.set(value_item->collation.collation, DERIVATION_IMPLICIT,
                value_item->collation.repertoire);
  set_data_type(value_item->data_type());
  max_length = value_item->max_length;
  decimals = value_item->decimals;
  fixed = true;
  return false;
}

void Item_name_const::print(const THD *thd, String *str,
                            enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("NAME_CONST("));
  name_item->print(thd, str, query_type);
  str->append(',');
  value_item->print(thd, str, query_type);
  str->append(')');
}

/*
 need a special class to adjust printing : references to aggregate functions
 must not be printed as refs because the aggregate functions that are added to
 the front of select list are not printed as well.
*/
class Item_aggregate_ref : public Item_ref {
 public:
  Item_aggregate_ref(Name_resolution_context *context_arg, Item **item,
                     const char *table_name_arg, const char *field_name_arg,
                     SELECT_LEX *depended_from_arg)
      : Item_ref(context_arg, item, table_name_arg, field_name_arg) {
    depended_from = depended_from_arg;
  }

  void print(const THD *thd, String *str,
             enum_query_type query_type) const override {
    if (ref)
      (*ref)->print(thd, str, query_type);
    else
      Item_ident::print(thd, str, query_type);
  }
  Ref_Type ref_type() const override { return AGGREGATE_REF; }

  /**
    Walker processor used by SELECT_LEX::transform_grouped_to_derived to replace
    an aggregate's reference to one in the new derived table's (hidden) select
    list.

    @param  arg  An info object of type Item::Aggregate_ref_update
    @returns false
  */
  bool update_aggr_refs(uchar *arg) override {
    auto *info = pointer_cast<Item::Aggregate_ref_update *>(arg);
    if (*ref != info->m_target) return false;
    ref = info->m_owner->add_hidden_item(info->m_target);
    return false;
  }
};

/**
  1. Move SUM items out from item tree and replace with reference.

  The general goal of this is to get a list of group aggregates, and window
  functions, and their arguments, so that the code which manages internal tmp
  tables (creation, row copying) has a list of all aggregates (which require
  special management) and a list of their arguments (which must be carried
  from tmp table to tmp table until the aggregate can be computed).

  2. Move scalar subqueries out of the item tree and replace with reference
  when used in arguments to window functions for similar reasons (tmp tables).

  @param thd             Current session
  @param ref_item_array  Pointer to array of reference fields
  @param fields          All fields in select
  @param ref             Pointer to item. If nullptr, get it from
                         Item_sum::referenced_by[].
  @param skip_registered <=> function be must skipped for registered SUM items

    All found SUM items are added FIRST in the fields list and
    we replace the item with a reference.

    thd->fatal_error() may be called if we are out of memory

    The logic of skip_registered is:

      - split_sum_func() is called when an aggregate is part of a bigger
        expression, example: '1+max()'.

      - an Item_sum has referenced_by[0]!=nullptr when it is a group aggregate
        located in a subquery but aggregating in a more outer query.

      - this referenced_by is necessary because for such aggregates, there are
        two phases:

         - fix_fields() is called by the subquery, which puts the item into the
           outer SELECT_LEX::inner_sum_func_list.

         - the outer query scans that list, calls split_sum_func2(), it
           replaces the aggregate with an Item_ref, so it needs to correct the
           pointer-to-aggregate held by the '+' item; so it needs access to the
           pointer; this is possible because fix_fields() has stored the
           address of this pointer into referenced_by[0].

      - So when we call split_sum_func for any aggregate, if we are in the
        subquery, we do not want to modify the outer-aggregated aggregates, and
        as those are detectable because they have referenced_by[0]!=0: we pass
        'skip_registered=true'.

      - On the other hand, if we are in the outer query and scan
        inner_sum_func_list, it's time to modify the aggregate which was
        skipped by the subquery, so we pass 'skip_registered=false'.

      - Finally, if the subquery was transformed with IN-to-EXISTS, a new
        HAVING condition may have been added, which contains an Item_ref to the
        same Item_sum; that makes a second pointer, referenced_by[1],
        to remember.
        @todo rename skip_registered to some name which better evokes
        "outer-ness" of the item; subquery_none exercises this function
        (Bug#11762); and rename referenced_by too, as it's set only for
        outer-aggregated items.

  Examples of 1):

      (1) SELECT a+FIRST_VALUE(b*SUM(c/d)) OVER (...)

  Assume we have done fix_fields() on this SELECT list, which list is so far
  only '+'. This '+' contains a WF (and a group aggregate function), so the
  resolver (generally, SELECT_LEX::prepare()) calls Item::split_sum_func2 on
  the '+'; as this '+' is neither a WF nor a group aggregate, but contains
  some, it calls Item_func::split_sum_func which calls Item::split_sum_func2 on
  every argument of the '+':

   - for 'a', it adds it to 'fields' as a hidden item

   - then the FIRST_VALUE wf is added as a hidden item; this is necessary so
     that create_tmp_table() and copy_funcs can spot the WF.

   - next, for FIRST_VALUE: it is a WF, so its Item_sum::split_sum_func is
     called, as its arguments need to be added as hidden items so they can get
     carried forward between the tmp tables. This split_sum_func calls
     Item::split_sum_func2 on its argument (the '*'); this
     '*' is not a group aggregate but contains one, so its
     Item_func::split_sum_func is called, which calls Item::split_sum_func2 on
     every argument of the '*':
       - for 'b', adds it to 'fields' as a hidden item
       - for SUM: it is a group aggregate (and doesn't contain any WF) so it
         adds it to 'fields' as a hidden item.

  So we finally have, in 'fields':

      SUM, b, FIRST_VALUE, a, +

  Each time we add a hidden item we re-point its parent to the hidden item
  using an Item_aggregate_ref. For example, the args[0] of '+' is made to point
  to an Item_aggregate_ref which points to the hidden 'a'.

  Examples of 2):

       SELECT LAST_VALUE((SELECT upper.j FROM t1 LIMIT 1)) OVER (ORDER BY i)
       FROM t1 AS upper;
*/

void Item::split_sum_func2(THD *thd, Ref_item_array ref_item_array,
                           List<Item> &fields, Item **ref,
                           bool skip_registered) {
  DBUG_TRACE;
  /* An item of type Item_sum  is registered <=> referenced_by[0] != 0 */
  if (type() == SUM_FUNC_ITEM && skip_registered &&
      (down_cast<Item_sum *>(this))->referenced_by[0])
    return;

  // 'sum_func' means a group aggregate function
  const bool is_sum_func = type() == SUM_FUNC_ITEM && !m_is_window_function;
  if ((!is_sum_func && has_aggregation() && !m_is_window_function) ||
      (!m_is_window_function && has_wf()) ||
      (type() == FUNC_ITEM && ((down_cast<Item_func *>(this))->functype() ==
                                   Item_func::ISNOTNULLTEST_FUNC ||
                               (down_cast<Item_func *>(this))->functype() ==
                                   Item_func::TRIG_COND_FUNC)) ||
      type() == ROW_ITEM) {
    // Do not add item to hidden list; possibly split it
    split_sum_func(thd, ref_item_array, fields);
  } else if ((type() == SUM_FUNC_ITEM || !const_for_execution()) &&  // (1)
             (type() != SUBSELECT_ITEM ||                            // (2)
              (down_cast<Item_subselect *>(this)->substype() ==
                   Item_subselect::SINGLEROW_SUBS &&
               down_cast<Item_subselect *>(this)
                       ->unit->first_select()
                       ->fields_list.elements == 1)) &&
             (type() != REF_ITEM ||  // (3)
              (down_cast<Item_ref *>(this))->ref_type() ==
                  Item_ref::VIEW_REF)) {
    /*
      (1) Replace item with a reference so that we can easily calculate
      it (in case of sum functions) or copy it (in case of fields)

      The test above is to ensure we don't do a reference for things
      that are constants (INNER_TABLE_BIT is in effect a constant)
      or already referenced (for example an item in HAVING)

      (2) In order to handle queries like:
        SELECT FIRST_VALUE((SELECT .. FROM .. LIMIT 1)) OVER (..) FROM ...;
      we need to move subselects to hidden fields too. But since window
      functions accept only single-row and single-column subqueries other
      types are excluded.
      Indeed, a subquery of another type is wrapped in Item_in_optimizer at this
      stage, so when splitting Item_in_optimizer, if we added the underlying
      Item_subselect to "fields" below it would be later evaluated by
      copy_fields() (in tmp table processing), which would be incorrect as the
      Item_subselect cannot be evaluated - as it must always be evaluated
      through its parent Item_in_optimizer.

      (3) Exception from (1) is Item_view_ref which we need to wrap in
      Item_ref to allow fields from view being stored in tmp table.
    */
    DBUG_PRINT("info", ("replacing %s with reference", item_name.ptr()));
    uint el = fields.elements;

    SELECT_LEX *base_select;
    SELECT_LEX *depended_from = nullptr;
    if (type() == SUM_FUNC_ITEM && !m_is_window_function) {
      Item_sum *const item = down_cast<Item_sum *>(this);
      DBUG_ASSERT(thd->lex->current_select() == item->aggr_select);
      base_select = item->base_select;
      if (item->aggr_select != base_select) depended_from = item->aggr_select;
    } else {
      base_select = thd->lex->current_select();
    }

    ref_item_array[el] = this;
    Item_aggregate_ref *const item_ref =
        new Item_aggregate_ref(&base_select->context, &ref_item_array[el],
                               nullptr, item_name.ptr(), depended_from);
    if (!item_ref) return; /* purecov: inspected */
    fields.push_front(this);
    if (ref == nullptr) {
      DBUG_ASSERT(is_sum_func);
      // Let 'ref' be the two elements of referenced_by[].
      if ((ref = static_cast<Item_sum *>(this)->referenced_by[1]))
        thd->change_item_tree(ref, item_ref);
      ref = ((Item_sum *)this)->referenced_by[0];
      DBUG_ASSERT(ref);
    }
    thd->change_item_tree(ref, item_ref);

    /*
      A WF must both be added to hidden list (done above), and be split so its
      arguments are added into the hidden list (done below):
    */
    if (m_is_window_function) split_sum_func(thd, ref_item_array, fields);
  }
}

static bool left_is_superset(DTCollation *left, DTCollation *right) {
  /* Allow convert to Unicode */
  if (left->collation->state & MY_CS_UNICODE &&
      (left->derivation < right->derivation ||
       (left->derivation == right->derivation &&
        (!(right->collation->state & MY_CS_UNICODE) ||
         /* The code below makes 4-byte utf8 a superset over 3-byte utf8 */
         (left->collation->state & MY_CS_UNICODE_SUPPLEMENT &&
          !(right->collation->state & MY_CS_UNICODE_SUPPLEMENT) &&
          left->collation->mbmaxlen > right->collation->mbmaxlen &&
          left->collation->mbminlen == right->collation->mbminlen)))))
    return true;
  /* Allow convert from any Unicode to utf32 or utf8mb4 */
  if (test_all_bits(left->collation->state,
                    MY_CS_UNICODE | MY_CS_UNICODE_SUPPLEMENT) &&
      right->collation->state & MY_CS_UNICODE &&
      left->derivation == right->derivation)
    return true;
  /* Allow convert from ASCII */
  if (right->repertoire == MY_REPERTOIRE_ASCII &&
      (left->derivation < right->derivation ||
       (left->derivation == right->derivation &&
        !(left->repertoire == MY_REPERTOIRE_ASCII))))
    return true;
  /* Disallow conversion otherwise */
  return false;
}

/**
  Aggregate two collations together taking
  into account their coercibility (aka derivation):.

  0 == DERIVATION_EXPLICIT  - an explicitly written COLLATE clause @n
  1 == DERIVATION_NONE      - a mix of two different collations @n
  2 == DERIVATION_IMPLICIT  - a column @n
  3 == DERIVATION_COERCIBLE - a string constant.

  The most important rules are:
  -# If collations are the same:
  chose this collation, and the strongest derivation.
  -# If collations are different:
  - Character sets may differ, but only if conversion without
  data loss is possible. The caller provides flags whether
  character set conversion attempts should be done. If no
  flags are substituted, then the character sets must be the same.
  Currently processed flags are:
  MY_COLL_ALLOW_SUPERSET_CONV  - allow conversion to a superset
  MY_COLL_ALLOW_COERCIBLE_CONV - allow conversion of a coercible value
  - two EXPLICIT collations produce an error, e.g. this is wrong:
  CONCAT(expr1 collate latin1_swedish_ci, expr2 collate latin1_german_ci)
  - the side with smaller derivation value wins,
  i.e. a column is stronger than a string constant,
  an explicit COLLATE clause is stronger than a column.
  - if derivations are the same, we have DERIVATION_NONE,
  we'll wait for an explicit COLLATE clause which possibly can
  come from another argument later: for example, this is valid,
  but we don't know yet when collecting the first two arguments:
     @code
       CONCAT(latin1_swedish_ci_column,
              latin1_german1_ci_column,
              expr COLLATE latin1_german2_ci)
  @endcode

  @retval true If the two collations are incompatible and cannot be aggregated.

  @retval false If the two collations can be aggregated, possibly with
  DERIVATION_NONE to indicate that they need a third explicit collation as a
  tiebreaker.

*/

bool DTCollation::aggregate(DTCollation &dt, uint flags) {
  if (!my_charset_same(collation, dt.collation)) {
    /*
       We do allow to use binary strings (like BLOBS)
       together with character strings.
       Binaries have more precedence than a character
       string of the same derivation.
    */
    if (collation == &my_charset_bin) {
      if (derivation <= dt.derivation)
        ;  // Do nothing
      else {
        set(dt);
      }
    } else if (dt.collation == &my_charset_bin) {
      if (dt.derivation <= derivation) {
        set(dt);
      }
    } else if ((flags & MY_COLL_ALLOW_SUPERSET_CONV) &&
               left_is_superset(this, &dt)) {
      // Do nothing
    } else if ((flags & MY_COLL_ALLOW_SUPERSET_CONV) &&
               left_is_superset(&dt, this)) {
      set(dt);
    } else if ((flags & MY_COLL_ALLOW_COERCIBLE_CONV) &&
               derivation < dt.derivation &&
               dt.derivation >= DERIVATION_SYSCONST) {
      // Do nothing;
    } else if ((flags & MY_COLL_ALLOW_COERCIBLE_CONV) &&
               dt.derivation < derivation &&
               derivation >= DERIVATION_SYSCONST) {
      set(dt);
    } else {
      // Cannot apply conversion
      set(&my_charset_bin, DERIVATION_NONE, (dt.repertoire | repertoire));
      return true;
    }
  } else if (derivation < dt.derivation) {
    // Do nothing
  } else if (dt.derivation < derivation) {
    set(dt);
  } else {
    if (collation == dt.collation) {
      // Do nothing
    } else {
      if (derivation == DERIVATION_EXPLICIT) {
        set(nullptr, DERIVATION_NONE, 0);
        return true;
      }

      // If we have two different binary collations for the same character set,
      // and none of them is explicit, we don't know which to choose. For
      // example: utf8mb4_bin is a binary padding collation, utf8mb4_0900_bin is
      // a binary non-padding collation. Cannot determine if the resulting
      // collation should be padding or non-padding, unless they are also
      // aggregated with a third explicit collation.
      if ((collation->state & MY_CS_BINSORT) &&
          (dt.collation->state & MY_CS_BINSORT)) {
        set(DERIVATION_NONE);
        return false;
      }

      // When aggregating a binary and a non-binary collation for the same
      // character set, the binary collation is preferred.
      if (collation->state & MY_CS_BINSORT) return false;
      if (dt.collation->state & MY_CS_BINSORT) {
        set(dt);
        return false;
      }
      const CHARSET_INFO *bin =
          get_charset_by_csname(collation->csname, MY_CS_BINSORT, MYF(0));
      set(bin, DERIVATION_NONE);
    }
  }
  repertoire |= dt.repertoire;
  return false;
}

/******************************/
static void my_coll_agg_error(DTCollation &c1, DTCollation &c2,
                              const char *fname) {
  my_error(ER_CANT_AGGREGATE_2COLLATIONS, MYF(0), c1.collation->name,
           c1.derivation_name(), c2.collation->name, c2.derivation_name(),
           fname);
}

static void my_coll_agg_error(DTCollation &c1, DTCollation &c2, DTCollation &c3,
                              const char *fname) {
  my_error(ER_CANT_AGGREGATE_3COLLATIONS, MYF(0), c1.collation->name,
           c1.derivation_name(), c2.collation->name, c2.derivation_name(),
           c3.collation->name, c3.derivation_name(), fname);
}

static void my_coll_agg_error(Item **args, uint count, const char *fname,
                              int item_sep) {
  if (count == 2)
    my_coll_agg_error(args[0]->collation, args[item_sep]->collation, fname);
  else if (count == 3)
    my_coll_agg_error(args[0]->collation, args[item_sep]->collation,
                      args[2 * item_sep]->collation, fname);
  else
    my_error(ER_CANT_AGGREGATE_NCOLLATIONS, MYF(0), fname);
}

static bool agg_item_collations(DTCollation &c, const char *fname, Item **av,
                                uint count, uint flags, int item_sep) {
  uint i;
  Item **arg;
  bool unknown_cs = false;

  c.set(av[0]->collation);
  for (i = 1, arg = &av[item_sep]; i < count; i++, arg++) {
    if (c.aggregate((*arg)->collation, flags)) {
      if (c.derivation == DERIVATION_NONE && c.collation == &my_charset_bin) {
        unknown_cs = true;
        continue;
      }
      my_coll_agg_error(av, count, fname, item_sep);
      return true;
    }
  }

  if (unknown_cs && c.derivation != DERIVATION_EXPLICIT) {
    my_coll_agg_error(av, count, fname, item_sep);
    return true;
  }

  if ((flags & MY_COLL_DISALLOW_NONE) && c.derivation == DERIVATION_NONE) {
    my_coll_agg_error(av, count, fname, item_sep);
    return true;
  }

  /* If all arguments were numbers, reset to @@collation_connection */
  if (flags & MY_COLL_ALLOW_NUMERIC_CONV && c.derivation == DERIVATION_NUMERIC)
    c.set(Item::default_charset(), DERIVATION_COERCIBLE, MY_REPERTOIRE_NUMERIC);

  return false;
}

bool agg_item_collations_for_comparison(DTCollation &c, const char *fname,
                                        Item **av, uint count, uint flags) {
  return (agg_item_collations(c, fname, av, count,
                              flags | MY_COLL_DISALLOW_NONE, 1));
}

bool agg_item_set_converter(DTCollation &coll, const char *fname, Item **args,
                            uint nargs, uint, int item_sep) {
  Item *safe_args[2] = {nullptr, nullptr};

  /*
    For better error reporting: save the first and the second argument.
    We need this only if the the number of args is 3 or 2:
    - for a longer argument list, "Illegal mix of collations"
      doesn't display each argument's characteristics.
    - if nargs is 1, then this error cannot happen.
  */
  if (nargs >= 2 && nargs <= 3) {
    safe_args[0] = args[0];
    safe_args[1] = args[item_sep];
  }

  THD *thd = current_thd;

  /*
    In case we're in statement prepare, create conversion item
    in its memory: it will be reused on each execute.
  */
  Prepared_stmt_arena_holder ps_arena_holder(
      thd, thd->stmt_arena->is_stmt_prepare());

  uint i;
  Item **arg;
  for (i = 0, arg = args; i < nargs; i++, arg += item_sep) {
    size_t dummy_offset;
    if (!String::needs_conversion(1, (*arg)->collation.collation,
                                  coll.collation, &dummy_offset))
      continue;

    /*
      No needs to add converter if an "arg" is NUMERIC or DATETIME
      value (which is pure ASCII) and at the same time target DTCollation
      is ASCII-compatible. For example, no needs to rewrite:
        SELECT * FROM t1 WHERE datetime_field = '2010-01-01';
      to
        SELECT * FROM t1 WHERE CONVERT(datetime_field USING cs) = '2010-01-01';

      TODO: avoid conversion of any values with
      repertoire ASCII and 7bit-ASCII-compatible,
      not only numeric/datetime origin.
    */
    if ((*arg)->collation.derivation == DERIVATION_NUMERIC &&
        (*arg)->collation.repertoire == MY_REPERTOIRE_ASCII &&
        !((*arg)->collation.collation->state & MY_CS_NONASCII) &&
        !(coll.collation->state & MY_CS_NONASCII))
      continue;

    Item *conv = (*arg)->safe_charset_converter(thd, coll.collation);
    // @todo - check why the constructors may return error
    if (thd->is_error()) return true;
    if (conv == nullptr &&
        ((*arg)->collation.repertoire == MY_REPERTOIRE_ASCII))
      conv = new Item_func_conv_charset(thd, *arg, coll.collation, true);

    if (conv == nullptr) {
      if (nargs >= 2 && nargs <= 3) {
        /* restore the original arguments for better error message */
        args[0] = safe_args[0];
        args[item_sep] = safe_args[1];
      }
      my_coll_agg_error(args, nargs, fname, item_sep);
      return true;
    }
    if ((*arg)->type() == Item::FIELD_ITEM)
      ((Item_field *)(*arg))->no_const_subst = true;
    /*
      If in statement prepare, then we create a converter for two
      constant items, do it once and then reuse it.
      If we're in execution of a prepared statement, arena is NULL,
      and the conv was created in runtime memory. This can be
      the case only if the argument is a parameter marker ('?'),
      because for all true constants the charset converter has already
      been created in prepare. In this case register the change for
      rollback.
    */
    if (thd->stmt_arena->is_stmt_prepare())
      *arg = conv;
    else
      thd->change_item_tree(arg, conv);

    if (conv->fix_fields(thd, arg)) return true;
  }

  return false;
}

/*
  Collect arguments' character sets together.
  We allow to apply automatic character set conversion in some cases.
  The conditions when conversion is possible are:
  - arguments A and B have different charsets
  - A wins according to coercibility rules
    (i.e. a column is stronger than a string constant,
     an explicit COLLATE clause is stronger than a column)
  - character set of A is either superset for character set of B,
    or B is a string constant which can be converted into the
    character set of A without data loss.

  If all of the above is true, then it's possible to convert
  B into the character set of A, and then compare according
  to the collation of A.

  For functions with more than two arguments:

    collect(A,B,C) ::= collect(collect(A,B),C)

  Since this function calls THD::change_item_tree() on the passed Item **
  pointers, it is necessary to pass the original Item **'s, not copies.
  Otherwise their values will not be properly restored (see BUG#20769).
  If the items are not consecutive (eg. args[2] and args[5]), use the
  item_sep argument, ie.

    agg_item_charsets(coll, fname, &args[2], 2, flags, 3)

*/

bool agg_item_charsets(DTCollation &coll, const char *fname, Item **args,
                       uint nargs, uint flags, int item_sep) {
  if (agg_item_collations(coll, fname, args, nargs, flags, item_sep))
    return true;

  return agg_item_set_converter(coll, fname, args, nargs, flags, item_sep);
}

void Item_ident_for_show::make_field(Send_field *tmp_field) {
  tmp_field->table_name = tmp_field->org_table_name = table_name;
  tmp_field->db_name = db_name;
  tmp_field->col_name = tmp_field->org_col_name = field->field_name;
  tmp_field->charsetnr = field->charset()->number;
  tmp_field->length = field->field_length;
  tmp_field->type = field->type();
  tmp_field->flags = field->table->is_nullable()
                         ? (field->flags & ~NOT_NULL_FLAG)
                         : field->flags;
  tmp_field->decimals = field->decimals();
  tmp_field->field = false;
}

bool Item_ident_for_show::fix_fields(THD *, Item **) {
  maybe_null = field->is_nullable();
  decimals = field->decimals();
  unsigned_flag = field->flags & UNSIGNED_FLAG;
  collation.set(field->charset(), field->derivation(), field->repertoire());
  set_data_type(field->type());
  max_length = char_to_byte_length_safe(field->char_length(),
                                        collation.collation->mbmaxlen);

  fixed = true;

  return false;
}

/**********************************************/
Item_field::Item_field(Field *f)
    : Item_ident(nullptr, NullS, *f->table_name, f->field_name),
      orig_field(nullptr),
      item_equal(nullptr),
      no_const_subst(false),
      have_privileges(0),
      any_privileges(false) {
  if (f->table->pos_in_table_list != nullptr) {
    DBUG_ASSERT(f->table->pos_in_table_list->select_lex != nullptr);
    context = &(f->table->pos_in_table_list->select_lex->context);
  }
  set_field(f);
  /*
    field_name and table_name should not point to garbage
    if this item is to be reused
  */
  orig_table_name = orig_field_name = "";
}

/**
  Constructor used inside setup_wild().

  Ensures that field, table, and database names will live as long as
  Item_field (this is important in prepared statements).
*/

Item_field::Item_field(THD *thd, Name_resolution_context *context_arg, Field *f)
    : Item_ident(context_arg, f->table->s->db.str, *f->table_name,
                 f->field_name),
      orig_field(nullptr),
      item_equal(nullptr),
      no_const_subst(false),
      have_privileges(0),
      any_privileges(false) {
  /*
    We always need to provide Item_field with a fully qualified field
    name to avoid ambiguity when executing prepared statements like
    SELECT * from d1.t1, d2.t1; (assuming d1.t1 and d2.t1 have columns
    with same names).
    This is because prepared statements never deal with wildcards in
    select list ('*') and always fix fields using fully specified path
    (i.e. db.table.column).
    No check for OOM: if db_name is NULL, we'll just get
    "Field not found" error.
    We need to copy db_name, table_name and field_name because they must
    be allocated in the statement memory, not in table memory (the table
    structure can go away and pop up again between subsequent executions
    of a prepared statement or after the close_tables_for_reopen() call
    in mysql_multi_update_prepare() or due to wildcard expansion in stored
    procedures).
    @todo: Reconsider this when preparation is refactored.
  */
  {
    if (db_name) orig_db_name = thd->mem_strdup(db_name);
    if (table_name) orig_table_name = thd->mem_strdup(table_name);
    if (field_name) orig_field_name = thd->mem_strdup(field_name);
    /*
      We don't restore 'name' in cleanup because it's not changed
      during execution. Still we need it to point to persistent
      memory if this item is to be reused.
    */
    item_name.set(orig_field_name);
  }
  set_field(f);
}

Item_field::Item_field(Name_resolution_context *context_arg, const char *db_arg,
                       const char *table_name_arg, const char *field_name_arg)
    : Item_ident(context_arg, db_arg, table_name_arg, field_name_arg),
      table_ref(nullptr),
      field(nullptr),
      orig_field(nullptr),
      item_equal(nullptr),
      no_const_subst(false),
      have_privileges(0),
      any_privileges(false) {
  SELECT_LEX *select = current_thd->lex->current_select();
  collation.set(DERIVATION_IMPLICIT);
  if (select && select->parsing_place != CTX_HAVING)
    select->select_n_where_fields++;
}

Item_field::Item_field(const POS &pos, const char *db_arg,
                       const char *table_name_arg, const char *field_name_arg)
    : Item_ident(pos, db_arg, table_name_arg, field_name_arg),
      table_ref(nullptr),
      field(nullptr),
      orig_field(nullptr),
      item_equal(nullptr),
      no_const_subst(false),
      have_privileges(0),
      any_privileges(false) {
  collation.set(DERIVATION_IMPLICIT);
}

bool Item_field::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  SELECT_LEX *const select = pc->select;
  if (select->parsing_place != CTX_HAVING) select->select_n_where_fields++;

  if (select->parsing_place == CTX_SELECT_LIST && field_name &&
      field_name[0] == '*' && field_name[1] == 0)
    select->with_wild++;
  return false;
}

/**
  Constructor need to process subselect with temporary tables (see Item)
*/

Item_field::Item_field(THD *thd, Item_field *item)
    : Item_ident(thd, item),
      table_ref(item->table_ref),
      field(item->field),
      orig_field(item->orig_field),
      result_field(item->result_field),
      item_equal(item->item_equal),
      no_const_subst(item->no_const_subst),
      have_privileges(item->have_privileges),
      any_privileges(item->any_privileges) {
  collation.set(DERIVATION_IMPLICIT);
}

/**
  Calculate the max column length not taking into account the
  limitations over integer types.

  When storing data into fields the server currently just ignores the
  limits specified on integer types, e.g. 1234 can safely be stored in
  an int(2) and will not cause an error.
  Thus when creating temporary tables and doing transformations
  we must adjust the maximum field length to reflect this fact.
  We take the un-restricted maximum length and adjust it similarly to
  how the declared length is adjusted wrt unsignedness etc.
  TODO: this all needs to go when we disable storing 1234 in int(2).

  @param field_par   Original field the use to calculate the lengths
  @param max_length  Item's calculated explicit max length
  @return            The adjusted max length
*/

inline static uint32 adjust_max_effective_column_length(Field *field_par,
                                                        uint32 max_length) {
  uint32 new_max_length = field_par->max_display_length();
  uint32 sign_length = (field_par->flags & UNSIGNED_FLAG) ? 0 : 1;

  switch (field_par->type()) {
    case MYSQL_TYPE_INT24:
      /*
        Compensate for MAX_MEDIUMINT_WIDTH being 1 too long (8)
        compared to the actual number of digits that can fit into
        the column.
      */
      new_max_length += 1;
      /* fall through */
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:

      /* Take out the sign and add a conditional sign */
      new_max_length = new_max_length - 1 + sign_length;
      break;

    /* BINGINT is always 20 no matter the sign */
    case MYSQL_TYPE_LONGLONG:
    /* make gcc happy */
    default:
      break;
  }

  /* Adjust only if the actual precision based one is bigger than specified */
  return new_max_length > max_length ? new_max_length : max_length;
}

void Item_field::set_field(Field *field_par) {
  table_ref = field_par->table->pos_in_table_list;
  DBUG_ASSERT(!table_ref || table_ref->table == field_par->table);

  field = result_field = field_par;  // for easy coding with fields
  maybe_null = field->is_nullable() || field->is_tmp_nullable() ||
               field->table->is_nullable();
  decimals = field->decimals();
  table_name = *field_par->table_name;
  field_name = field_par->field_name;
  db_name = field_par->table->s->db.str;
  unsigned_flag = field_par->flags & UNSIGNED_FLAG;
  collation.set(field_par->charset(), field_par->derivation(),
                field_par->repertoire());
  set_data_type(field_par->type());
  max_length = char_to_byte_length_safe(field_par->char_length(),
                                        collation.collation->mbmaxlen);

  max_length = adjust_max_effective_column_length(field_par, max_length);

  if (field->table->s->tmp_table == SYSTEM_TMP_TABLE) any_privileges = false;
  if (!orig_field) orig_field = field_par;
  if (!can_use_prefix_key)
    field->table->covering_keys.subtract(field->part_of_prefixkey);

  fixed = true;
}

/**
  Reset this item to point to a field from the new temporary table.
  This is used when we create a new temporary table for each execution
  of prepared statement.
*/

void Item_field::reset_field(Field *f) {
  set_field(f);
  /* 'name' is pointing at field->field_name of old field */
  item_name.set(f->field_name);
}

const char *Item_ident::full_name() const {
  char *tmp;
  if (!table_name || !field_name)
    return field_name ? field_name
                      : item_name.is_set() ? item_name.ptr() : "tmp_field";
  if (db_name && db_name[0]) {
    tmp = (char *)(*THR_MALLOC)
              ->Alloc(strlen(db_name) + strlen(table_name) +
                      strlen(field_name) + 3);
    strxmov(tmp, db_name, ".", table_name, ".", field_name, NullS);
  } else {
    if (table_name[0]) {
      tmp = (char *)(*THR_MALLOC)
                ->Alloc(strlen(table_name) + strlen(field_name) + 2);
      strxmov(tmp, table_name, ".", field_name, NullS);
    } else
      return field_name;
  }
  return tmp;
}

void Item_ident::print(const THD *thd, String *str, enum_query_type query_type,
                       const char *db_name_arg,
                       const char *table_name_arg) const {
  char d_name_buff[MAX_ALIAS_NAME], t_name_buff[MAX_ALIAS_NAME];
  const char *d_name = db_name_arg, *t_name = table_name_arg;

  if (lower_case_table_names == 1 ||
      // mode '2' does not apply to aliases:
      (lower_case_table_names == 2 && !alias_name_used())) {
    if (table_name_arg && table_name_arg[0]) {
      my_stpcpy(t_name_buff, table_name_arg);
      my_casedn_str(files_charset_info, t_name_buff);
      t_name = t_name_buff;
    }
    if (db_name_arg && db_name_arg[0]) {
      my_stpcpy(d_name_buff, db_name_arg);
      my_casedn_str(files_charset_info, d_name_buff);
      d_name = d_name_buff;
    }
  }

  if (!table_name_arg || !field_name || !field_name[0]) {
    const char *nm = (field_name && field_name[0])
                         ? field_name
                         : item_name.is_set() ? item_name.ptr() : "tmp_field";
    append_identifier(thd, str, nm, strlen(nm));
    return;
  }

  if (db_name_arg && db_name_arg[0] && !(query_type & QT_NO_DB) &&
      !alias_name_used()) {
    const size_t d_name_len = strlen(d_name);
    if (!((query_type & QT_NO_DEFAULT_DB) &&
          db_is_default_db(d_name, d_name_len, thd))) {
      append_identifier(thd, str, d_name, d_name_len);
      str->append('.');
    }
  }
  if (table_name_arg[0] && !(query_type & QT_NO_TABLE)) {
    append_identifier(thd, str, t_name, strlen(t_name));
    str->append('.');
  }
  append_identifier(thd, str, field_name, strlen(field_name));
}

String *Item_field::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if ((null_value = field->is_null())) return nullptr;
  str->set_charset(str_value.charset());
  return field->val_str(str, &str_value);
}

bool Item_field::val_json(Json_wrapper *result) {
  DBUG_ASSERT(fixed);
  DBUG_ASSERT(data_type() == MYSQL_TYPE_JSON || returns_array());
  null_value = field->is_null();
  if (null_value) return false;
  return down_cast<Field_json *>(field)->val_json(result);
}

double Item_field::val_real() {
  DBUG_ASSERT(fixed == 1);
  if ((null_value = field->is_null())) return 0.0;
  return field->val_real();
}

longlong Item_field::val_int() {
  DBUG_ASSERT(fixed == 1);
  if ((null_value = field->is_null())) return 0;
  return field->val_int();
}

longlong Item_field::val_time_temporal() {
  DBUG_ASSERT(fixed == 1);
  if ((null_value = field->is_null())) return 0;
  return field->val_time_temporal();
}

longlong Item_field::val_date_temporal() {
  DBUG_ASSERT(fixed == 1);
  if ((null_value = field->is_null())) return 0;
  return field->val_date_temporal();
}

my_decimal *Item_field::val_decimal(my_decimal *decimal_value) {
  null_value = field->is_null();
  DBUG_PRINT("enter", ("Item_field::val_decimal field: %p ptr: %p null: %d",
                       field, field->ptr, null_value));

  if (null_value) return nullptr;
  return field->val_decimal(decimal_value);
}

bool Item_field::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  if ((null_value = field->is_null()) || field->get_date(ltime, fuzzydate)) {
    memset(ltime, 0, sizeof(*ltime));
    return true;
  }
  return false;
}

bool Item_field::get_time(MYSQL_TIME *ltime) {
  if ((null_value = field->is_null()) || field->get_time(ltime)) {
    memset(ltime, 0, sizeof(*ltime));
    return true;
  }
  return false;
}

bool Item_field::get_timeval(struct timeval *tm, int *warnings) {
  if ((null_value = field->is_null())) return true;
  if (field->get_timestamp(tm, warnings)) tm->tv_sec = tm->tv_usec = 0;
  return false;
}

bool Item_field::eq(const Item *item, bool) const {
  const Item *real_item = const_cast<Item *>(item)->real_item();
  if (real_item->type() != FIELD_ITEM) return false;

  const Item_field *item_field = down_cast<const Item_field *>(real_item);

  /*
    When a field is passed forward in execution via an internal tmp table,
    leading to the creation of more Item_field, each Item_field has a
    different Field object, so that object can't be used to identify a
    field. The original table's name and original field's name cannot serve
    either, consider: SELECT a FROM t1 WHERE b IN (SELECT a FROM t1)
    where the semijoin-merged 'a' and the top query's 'a' are both named t1.a
    and coexist in the top query. Thus, we use orig_field, which is passed
    from source item to tmp table's item in change_to_use_tmp_fields().
  */
  if (item_field->orig_field && orig_field)
    return item_field->orig_field == orig_field;

  if (item_field->field && field) return item_field->field == field;
  /*
    We may come here when we are trying to find a function in a GROUP BY
    clause from the select list.
    In this case the '100 % correct' way to do this would be to first
    run fix_fields() on the GROUP BY item and then retry this function, but
    I think it's better to relax the checking a bit as we will in
    most cases do the correct thing by just checking the field name.
    (In cases where we would choose wrong we would have to generate a
    ER_NON_UNIQ_ERROR).
  */
  return (item_field->item_name.eq_safe(field_name) &&
          (!item_field->table_name || !table_name ||
           (!my_strcasecmp(table_alias_charset, item_field->table_name,
                           table_name) &&
            (!item_field->db_name || !db_name ||
             (item_field->db_name && !strcmp(item_field->db_name, db_name))))));
}

table_map Item_field::used_tables() const {
  if (!table_ref) return 1;  // Temporary table; always table 0
  if (table_ref->table->const_table) return 0;  // const item
  return depended_from ? OUTER_REF_TABLE_BIT : table_ref->map();
}

bool Item_field::used_tables_for_level(uchar *arg) {
  const TABLE_LIST *tr = field->table->pos_in_table_list;
  // Used by resolver only, so can never reach a "const" table.
  DBUG_ASSERT(!tr->table->const_table);
  Used_tables *const ut = pointer_cast<Used_tables *>(arg);
  /*
    When the qualifying query for the field (table_ref->select_lex) is the same
    level as the requested level, add the table's map.
    When the qualifying query for the field is outer relative to the
    requested level, add an outer reference.
  */
  if (ut->select == tr->select_lex)
    ut->used_tables |= tr->map();
  else if (ut->select->nest_level > tr->select_lex->nest_level)
    ut->used_tables |= OUTER_REF_TABLE_BIT;

  return false;
}

void Item_ident::fix_after_pullout(SELECT_LEX *parent_select,
                                   SELECT_LEX *removed_select) {
  /*
    Some field items may be created for use in execution only, without
    a name resolution context. They have already been used in execution,
    so no transformation is necessary here.

    @todo: Provide strict phase-division in optimizer, to make sure that
           execution-only objects do not exist during transformation stage.
           Then, this test would be deemed unnecessary.
  */
  if (context == nullptr) {
    DBUG_ASSERT(type() == FIELD_ITEM);
    return;
  }

  // context->select_lex should already have been updated.
  DBUG_ASSERT(context->select_lex != removed_select);

  if (context->select_lex == parent_select) {
    if (parent_select == depended_from) depended_from = nullptr;
  } else {
    /*
      The definition scope of this field item reference is inner to the removed
      select_lex object.
      No new resolution is needed, but we may need to update the dependency.
    */
    if (removed_select == depended_from) depended_from = parent_select;
  }

  if (depended_from) {
    /*
      Refresh used_tables information for subqueries between the definition
      scope and resolution scope of the field item reference.
    */
    SELECT_LEX *child_select = context->select_lex;

    while (child_select->outer_select() != depended_from) {
      /*
        The subquery on this level is outer-correlated with respect to the field
      */
      child_select->master_unit()->accumulate_used_tables(OUTER_REF_TABLE_BIT);
      child_select = child_select->outer_select();
    }

    /*
      child_select is select_lex immediately inner to the depended_from level.
      Now, locate the subquery predicate that contains this select_lex and
      update used tables information.
    */
    Used_tables ut(depended_from);
    (void)walk(&Item::used_tables_for_level, enum_walk::SUBQUERY_POSTFIX,
               pointer_cast<uchar *>(&ut));
    child_select->master_unit()->accumulate_used_tables(ut.used_tables);
  }
}

Item *Item_field::get_tmp_table_item(THD *thd) {
  DBUG_TRACE;
  Item_field *new_item = new Item_field(thd, this);
  if (!new_item) return nullptr; /* purecov: inspected */

  new_item->field = new_item->result_field;
  new_item->table_ref = nullptr;  // Internal temporary table has no table_ref

  return new_item;
}

longlong Item_field::val_int_endpoint(bool, bool *) {
  longlong res = val_int();
  return null_value ? LLONG_MIN : res;
}

/**
  Init an item from a string we KNOW points to a valid longlong.
  str_arg does not necessary has to be a \\0 terminated string.
  This is always 'signed'. Unsigned values are created with Item_uint()
*/
void Item_int::init(const char *str_arg, uint length) {
  const char *end_ptr = str_arg + length;
  int error;
  value = my_strtoll10(str_arg, &end_ptr, &error);
  max_length = (uint)(end_ptr - str_arg);
  item_name.copy(str_arg, max_length);
  fixed = true;
}

my_decimal *Item_int::val_decimal(my_decimal *decimal_value) {
  int2my_decimal(E_DEC_FATAL_ERROR, value, unsigned_flag, decimal_value);
  return decimal_value;
}

String *Item_int::val_str(String *str) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  str->set_int(value, unsigned_flag, collation.collation);
  return str;
}

void Item_int::print(const THD *, String *str,
                     enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }
  // my_charset_bin is good enough for numbers

  // don't rewrite booleans as ints. see bug#21296173
  const Name_string *const name = &item_name;
  const bool is_literal_false = name->is_set() && name->eq("FALSE");
  const bool is_literal_true = name->is_set() && name->eq("TRUE");
  if (is_literal_false || is_literal_true) {
    str->append(item_name.ptr(), item_name.length(), str->charset());
  } else {
    if (unsigned_flag)
      str->append_ulonglong(value);
    else
      str->append_longlong(value);
  }
}

String *Item_uint::val_str(String *str) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  str->set((ulonglong)value, collation.collation);
  return str;
}

void Item_uint::print(const THD *, String *str,
                      enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }
  str->append_ulonglong(value);
}

Item_decimal::Item_decimal(const POS &pos, const char *str_arg, uint length,
                           const CHARSET_INFO *charset)
    : super(pos) {
  str2my_decimal(E_DEC_FATAL_ERROR, str_arg, length, charset, &decimal_value);
  item_name.set(str_arg);
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_value.frac;
  fixed = true;
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.intg + decimals, decimals, unsigned_flag);
}

Item_decimal::Item_decimal(longlong val, bool unsig) {
  int2my_decimal(E_DEC_FATAL_ERROR, val, unsig, &decimal_value);
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_value.frac;
  fixed = true;
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.intg + decimals, decimals, unsigned_flag);
}

Item_decimal::Item_decimal(double val) {
  double2my_decimal(E_DEC_FATAL_ERROR, val, &decimal_value);
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_value.frac;
  fixed = true;
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.intg + decimals, decimals, unsigned_flag);
}

Item_decimal::Item_decimal(const Name_string &name_arg,
                           const my_decimal *val_arg, uint decimal_par,
                           uint length) {
  my_decimal2decimal(val_arg, &decimal_value);
  item_name = name_arg;
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_par;
  max_length = length;
  fixed = true;
}

Item_decimal::Item_decimal(my_decimal *value_par) {
  my_decimal2decimal(value_par, &decimal_value);
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_value.frac;
  fixed = true;
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.intg + decimals, decimals, unsigned_flag);
}

Item_decimal::Item_decimal(const uchar *bin, int precision, int scale) {
  binary2my_decimal(E_DEC_FATAL_ERROR, bin, &decimal_value, precision, scale);
  set_data_type(MYSQL_TYPE_NEWDECIMAL);
  decimals = (uint8)decimal_value.frac;
  fixed = true;
  max_length = my_decimal_precision_to_length_no_truncation(precision, decimals,
                                                            unsigned_flag);
}

longlong Item_decimal::val_int() {
  longlong result;
  my_decimal2int(E_DEC_FATAL_ERROR, &decimal_value, unsigned_flag, &result);
  return result;
}

double Item_decimal::val_real() {
  double result;
  my_decimal2double(E_DEC_FATAL_ERROR, &decimal_value, &result);
  return result;
}

String *Item_decimal::val_str(String *result) {
  result->set_charset(&my_charset_numeric);
  my_decimal2string(E_DEC_FATAL_ERROR, &decimal_value, result);
  return result;
}

void Item_decimal::print(const THD *, String *str,
                         enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }
  StringBuffer<MAX_DOUBLE_STR_LENGTH + 1> tmp;  // +1 for terminating null
  my_decimal2string(E_DEC_FATAL_ERROR, &decimal_value, &tmp);
  str->append(tmp);
}

bool Item_decimal::eq(const Item *item, bool) const {
  if (type() == item->type() && item->basic_const_item()) {
    /*
      We need to cast off const to call val_decimal(). This should
      be OK for a basic constant. Additionally, we can pass nullptr as
      a true decimal constant will return its internal decimal
      storage and ignore the argument.
    */
    Item *arg = const_cast<Item *>(item);
    const my_decimal *value = arg->val_decimal(nullptr);
    return !my_decimal_cmp(&decimal_value, value);
  }
  return false;
}

void Item_decimal::set_decimal_value(const my_decimal *value_par) {
  my_decimal2decimal(value_par, &decimal_value);
  decimals = (uint8)decimal_value.frac;
  unsigned_flag = !decimal_value.sign();
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.intg + decimals, decimals, unsigned_flag);
}

String *Item_float::val_str(String *str) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  str->set_real(value, decimals, &my_charset_bin);
  return str;
}

my_decimal *Item_float::val_decimal(my_decimal *decimal_value) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  double2my_decimal(E_DEC_FATAL_ERROR, value, decimal_value);
  return (decimal_value);
}

/**
   @sa enum_query_type.
   For us to be able to print a query (in debugging, optimizer trace, EXPLAIN
   EXTENDED) without changing the query's result, this function must not
   modify the item's content. Not even a @c realloc() of @c str_value is
   permitted:
   @c Item_func_concat::val_str(), @c Item_func_repeat::val_str(),
   @c Item_func_encode::val_str() depend on the allocated length;
   a change of this length can influence results of CONCAT(), REPEAT(),
   ENCODE()...
*/
void Item_string::print(const THD *, String *str,
                        enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }

  const bool print_introducer =
      (query_type & QT_FORCE_INTRODUCERS) ||
      (!(query_type & QT_WITHOUT_INTRODUCERS) && is_cs_specified());

  if (print_introducer) {
    str->append('_');
    auto charset_name = collation.collation->csname;
    if (native_strcasecmp(charset_name, "utf8") == 0)
      str->append("utf8mb3");
    else
      str->append(charset_name);
  }

  str->append('\'');

  if (query_type & QT_TO_SYSTEM_CHARSET) {
    if (print_introducer) {
      /*
        Because we wrote an introducer, we must print str_value in its
        charset, and the resulting bytes must not be changed until they
        reach the end client.
        But the caller is asking for system_charset_info, and may later
        convert into character_set_results. That means two conversions: we
        must ensure that they don't change our printed bytes.
        So we print str_value in the least common denominator of the three
        charsets involved: ASCII. Non-ASCII characters are printed as \xFF
        sequences (which is ASCII too). This way, our bytes will not be
        changed.
      */
      ErrConvString tmp(str_value.ptr(), str_value.length(), &my_charset_bin);
      str->append(tmp.ptr());
    } else {
      // Convert to system charset.
      convert_and_print(&str_value, str, system_charset_info);
    }
  } else if (query_type & QT_TO_ARGUMENT_CHARSET) {
    if (print_introducer)
      convert_and_print(&str_value, str, collation.collation);
    else
      /*
        Convert the string literals to str->charset(),
        which is typically equal to charset_set_client.
      */
      convert_and_print(&str_value, str, str->charset());
  } else {
    // Caller wants a result in the charset of str_value.
    str_value.print(str);
  }

  str->append('\'');
}

double double_from_string_with_check(const CHARSET_INFO *cs, const char *cptr,
                                     const char *end) {
  int error;
  double tmp;

  const char *endptr = end;
  tmp = my_strntod(cs, cptr, end - cptr, &endptr, &error);
  if (error || (end != endptr && !check_if_only_end_space(cs, endptr, end))) {
    ErrConvString err(cptr, end - cptr, cs);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "DOUBLE", err.ptr());
  }
  return tmp;
}

double Item_string::val_real() {
  DBUG_ASSERT(fixed == 1);
  return double_from_string_with_check(str_value.charset(), str_value.ptr(),
                                       str_value.ptr() + str_value.length());
}

longlong longlong_from_string_with_check(const CHARSET_INFO *cs,
                                         const char *cptr, const char *end) {
  int err_int;
  longlong tmp;
  const char *endptr = end;

  tmp = (*(cs->cset->strtoll10))(cs, cptr, &endptr, &err_int);
  /*
    TODO: Give error if we wanted a signed integer and we got an unsigned
    one
  */
  if ((err_int > 0 ||
       (end != endptr && !check_if_only_end_space(cs, endptr, end)))) {
    ErrConvString err(cptr, cs);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "INTEGER", err.ptr());
  }
  return tmp;
}

/**
  @todo
  Give error if we wanted a signed integer and we got an unsigned one
*/
longlong Item_string::val_int() {
  DBUG_ASSERT(fixed == 1);
  return longlong_from_string_with_check(str_value.charset(), str_value.ptr(),
                                         str_value.ptr() + str_value.length());
}

my_decimal *Item_string::val_decimal(my_decimal *decimal_value) {
  return val_decimal_from_string(decimal_value);
}

bool Item_null::eq(const Item *item, bool) const {
  return item->type() == type();
}

double Item_null::val_real() {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  null_value = true;
  return 0.0;
}
longlong Item_null::val_int() {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  null_value = true;
  return 0;
}

String *Item_null::val_str(String *) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  null_value = true;
  return nullptr;
}

my_decimal *Item_null::val_decimal(my_decimal *) { return nullptr; }

bool Item_null::val_json(Json_wrapper *) {
  null_value = true;
  return false;
}

Item *Item_null::safe_charset_converter(THD *, const CHARSET_INFO *tocs) {
  collation.set(tocs);
  return this;
}

/*********************** Item_param related ******************************/

/**
  Default function of Item_param::set_param_func, so in case
  of malformed packet the server won't SIGSEGV.
*/

static void default_set_param_func(Item_param *param,
                                   uchar **pos MY_ATTRIBUTE((unused)),
                                   ulong len MY_ATTRIBUTE((unused))) {
  param->state = Item_param::NO_VALUE;
}

Item_param::Item_param(const POS &pos, MEM_ROOT *root, uint pos_in_query_arg)
    : super(pos),
      state(NO_VALUE),
      item_result_type(STRING_RESULT),
      /* Don't pretend to be a literal unless value for this item is set. */
      item_type(PARAM_ITEM),
      pos_in_query(pos_in_query_arg),
      set_param_func(default_set_param_func),
      limit_clause_param(false),
      m_out_param_info(nullptr),
      m_clones(root) {
  item_name.set("?");
  set_data_type(MYSQL_TYPE_VARCHAR);
  /*
    Since we can't say whenever this item can be NULL or cannot be NULL
    before mysql_stmt_execute(), so we assuming that it can be NULL until
    value is set.
  */
  maybe_null = true;
}

bool Item_param::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  /*
    see commentaries in PTI_limit_option_param_marker::itemize()
  */
  DBUG_ASSERT(*res == this);

  LEX *lex = pc->thd->lex;
  if (!lex->parsing_options.allows_variable) {
    my_error(ER_VIEW_SELECT_VARIABLE, MYF(0));
    return true;
  }
  if (lex->reparse_common_table_expr_at) {
    /*
      This parameter is a clone, find the Item_param which corresponds to it
      in the original statement - its "master".
      Calculate the expected position of this master in the original
      statement:
    */
    uint master_pos = pos_in_query + lex->reparse_common_table_expr_at;
    List_iterator_fast<Item_param> it(lex->param_list);
    Item_param *master;
    while ((master = it++)) {
      if (master_pos == master->pos_in_query) {
        // Register it against its master
        return master->add_clone(this);
      }
    }
    DBUG_ASSERT(false); /* purecov: inspected */
  }

  return false;
}

void Item_param::sync_clones() {
  for (auto c : m_clones) {
    // Scalar-type members:
    c->maybe_null = maybe_null;
    c->null_value = null_value;
    c->max_length = max_length;
    c->decimals = decimals;
    c->state = state;
    c->item_type = item_type;
    c->item_result_type = item_result_type;
    c->set_param_func = set_param_func;
    c->value = value;
    c->unsigned_flag = unsigned_flag;
    // Class-type members:
    c->decimal_value = decimal_value;
    /*
      Note that String's assignment op properly sets m_is_alloced to 'false',
      which is correct here: c->str_value doesn't own anything.
    */
    c->str_value = str_value;
    c->str_value_ptr = str_value_ptr;
    c->collation = collation;
  }
}

void Item_param::set_null() {
  DBUG_TRACE;
  /* These are cleared after each execution by reset() method */
  null_value = true;
  /*
    Because of NULL and string values we need to set max_length for each new
    placeholder value: user can submit NULL for any placeholder type, and
    string length can be different in each execution.
  */
  max_length = 0;
  decimals = 0;
  state = NULL_VALUE;
  item_type = Item::NULL_ITEM;
}

void Item_param::set_int(longlong i, uint32 max_length_arg) {
  DBUG_TRACE;
  value.integer = i;
  state = INT_VALUE;
  max_length = max_length_arg;
  decimals = 0;
  maybe_null = false;
}

void Item_param::set_double(double d) {
  DBUG_TRACE;
  value.real = d;
  state = REAL_VALUE;
  max_length = DBL_DIG + 8;
  decimals = DECIMAL_NOT_SPECIFIED;
  maybe_null = false;
}

/**
  Set decimal parameter value from string.

  @param str      character string
  @param length   string length

  @note
    As we use character strings to send decimal values in
    binary protocol, we use str2my_decimal to convert it to
    internal decimal value.
*/

void Item_param::set_decimal(const char *str, ulong length) {
  DBUG_TRACE;

  const char *end = str + length;
  str2my_decimal(E_DEC_FATAL_ERROR, str, &decimal_value, &end);
  state = DECIMAL_VALUE;
  decimals = decimal_value.frac;
  max_length = my_decimal_precision_to_length_no_truncation(
      decimal_value.precision(), decimals, unsigned_flag);
  maybe_null = false;
}

void Item_param::set_decimal(const my_decimal *dv) {
  state = DECIMAL_VALUE;

  my_decimal2decimal(dv, &decimal_value);

  decimals = (uint8)decimal_value.frac;
  unsigned_flag = !decimal_value.sign();
  max_length = my_decimal_precision_to_length(decimal_value.intg + decimals,
                                              decimals, unsigned_flag);
}

/**
  Set parameter value from MYSQL_TIME value.

  @param tm              datetime value to set (time_type is ignored)
  @param time_type       type of datetime value
  @param max_length_arg  max length of datetime value as string

  @note
    If we value to be stored is not normalized, zero value will be stored
    instead and proper warning will be produced. This function relies on
    the fact that even wrong value sent over binary protocol fits into
    MAX_DATE_STRING_REP_LENGTH buffer.
*/
void Item_param::set_time(MYSQL_TIME *tm, enum_mysql_timestamp_type time_type,
                          uint32 max_length_arg) {
  DBUG_TRACE;

  value.time = *tm;
  value.time.time_type = time_type;
  decimals = tm->second_part ? DATETIME_MAX_DECIMALS : 0;

  if (check_datetime_range(value.time)) {
    /*
      TODO : Add error handling for Item_param::set_* functions.
      make_truncated_value_warning() can return error in STRICT mode.
    */
    (void)make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                       ErrConvString(&value.time, decimals),
                                       time_type, NullS);
    set_zero_time(&value.time, MYSQL_TIMESTAMP_ERROR);
  }

  state = TIME_VALUE;
  maybe_null = false;
  max_length = max_length_arg;
}

bool Item_param::set_str(const char *str, size_t length) {
  DBUG_TRACE;
  /*
    Assign string with no conversion: data is converted only after it's
    been written to the binary log.
  */
  uint dummy_errors;
  if (str_value.copy(str, length, &my_charset_bin, &my_charset_bin,
                     &dummy_errors))
    return true;
  state = STRING_VALUE;
  max_length = length;
  maybe_null = false;
  /* max_length and decimals are set after charset conversion */
  /* sic: str may be not null-terminated, don't add DBUG_PRINT here */
  return false;
}

bool Item_param::set_longdata(const char *str, ulong length) {
  DBUG_TRACE;

  /*
    If client character set is multibyte, end of long data packet
    may hit at the middle of a multibyte character.  Additionally,
    if binary log is open we must write long data value to the
    binary log in character set of client. This is why we can't
    convert long data to connection character set as it comes
    (here), and first have to concatenate all pieces together,
    write query to the binary log and only then perform conversion.
  */
  if (str_value.length() + length > current_thd->variables.max_allowed_packet) {
    my_message(ER_UNKNOWN_ERROR,
               "Parameter of prepared statement which is set through "
               "mysql_send_long_data() is longer than "
               "'max_allowed_packet' bytes",
               MYF(0));
    return true;
  }

  if (str_value.append(str, length, &my_charset_bin)) return true;
  state = LONG_DATA_VALUE;
  maybe_null = false;

  return false;
}

/**
  Set parameter value from user variable value.

  @param thd   Current thread
  @param entry User variable structure (NULL means use NULL value)

  @retval
    0 OK
  @retval
    1 Out of memory
*/

bool Item_param::set_from_user_var(THD *thd, const user_var_entry *entry) {
  DBUG_TRACE;
  if (entry && entry->ptr()) {
    item_result_type = entry->type();
    unsigned_flag = entry->unsigned_flag;
    if (limit_clause_param) {
      bool unused;
      set_int(entry->val_int(&unused), MY_INT64_NUM_DECIMAL_DIGITS);
      item_type = Item::INT_ITEM;
      return !unsigned_flag && value.integer < 0 ? true : false;
    }
    switch (item_result_type) {
      case REAL_RESULT:
        set_double(*pointer_cast<const double *>(entry->ptr()));
        item_type = Item::REAL_ITEM;
        break;
      case INT_RESULT:
        set_int(*pointer_cast<const longlong *>(entry->ptr()),
                MY_INT64_NUM_DECIMAL_DIGITS);
        item_type = Item::INT_ITEM;
        break;
      case STRING_RESULT: {
        const CHARSET_INFO *fromcs = entry->collation.collation;
        const CHARSET_INFO *tocs = thd->variables.collation_connection;
        size_t dummy_offset;

        value.cs_info.character_set_of_placeholder = fromcs;
        value.cs_info.character_set_client =
            thd->variables.character_set_client;
        /*
          Setup source and destination character sets so that they
          are different only if conversion is necessary: this will
          make later checks easier.
        */
        value.cs_info.final_character_set_of_str_value =
            String::needs_conversion(0, fromcs, tocs, &dummy_offset) ? tocs
                                                                     : fromcs;
        /*
          Exact value of max_length is not known unless data is converted to
          charset of connection, so we have to set it later.
        */
        item_type = Item::STRING_ITEM;

        if (set_str(entry->ptr(), entry->length())) return true;
        break;
      }
      case DECIMAL_RESULT: {
        const my_decimal *ent_value = (const my_decimal *)entry->ptr();
        my_decimal2decimal(ent_value, &decimal_value);
        state = DECIMAL_VALUE;
        decimals = ent_value->frac;
        max_length = my_decimal_precision_to_length_no_truncation(
            ent_value->precision(), decimals, unsigned_flag);
        item_type = Item::DECIMAL_ITEM;
        break;
      }
      default:
        DBUG_ASSERT(0);
        set_null();
    }
  } else
    set_null();

  return false;
}

/**
  Resets parameter after execution.

  @note
    We clear null_value here instead of setting it in set_* methods,
    because we want more easily handle case for long data.
*/

void Item_param::reset() {
  DBUG_TRACE;
  /* Shrink string buffer if it's bigger than max possible CHAR column */
  if (str_value.alloced_length() > MAX_CHAR_WIDTH)
    str_value.mem_free();
  else
    str_value.length(0);
  str_value_ptr.length(0);
  /*
    We must prevent all charset conversions until data has been written
    to the binary log.
  */
  str_value.set_charset(&my_charset_bin);
  collation.set(&my_charset_bin, DERIVATION_COERCIBLE);
  state = NO_VALUE;
  maybe_null = true;
  null_value = false;
  /*
    Don't reset item_type to PARAM_ITEM: it's only needed to guard
    us from item optimizations at prepare stage, when item doesn't yet
    contain a literal of some kind.
    In all other cases when this object is accessed its value is
    set (this assumption is guarded by 'state' and
    DBUG_ASSERTS(state != NO_VALUE) in all Item_param::get_*
    methods).
  */
}

type_conversion_status Item_param::save_in_field_inner(Field *field,
                                                       bool no_conversions) {
  field->set_notnull();

  switch (state) {
    case INT_VALUE:
      return field->store(value.integer, unsigned_flag);
    case REAL_VALUE:
      return field->store(value.real);
    case DECIMAL_VALUE:
      return field->store_decimal(&decimal_value);
    case TIME_VALUE:
      field->store_time(&value.time);
      return TYPE_OK;
    case STRING_VALUE:
    case LONG_DATA_VALUE:
      return field->store(str_value.ptr(), str_value.length(),
                          str_value.charset());
    case NULL_VALUE:
      return set_field_to_null_with_conversions(field, no_conversions);
    case NO_VALUE:
    default:
      DBUG_ASSERT(0);
  }
  return TYPE_ERR_BAD_VALUE;
}

bool Item_param::get_time(MYSQL_TIME *res) {
  if (state == TIME_VALUE) {
    *res = value.time;
    return false;
  }
  /*
    If parameter value isn't supplied assertion will fire in val_str()
    which is called from Item::get_time_from_string().
  */
  return is_temporal() ? get_time_from_string(res)
                       : get_time_from_non_temporal(res);
}

bool Item_param::get_date(MYSQL_TIME *res, my_time_flags_t fuzzydate) {
  if (state == TIME_VALUE) {
    *res = value.time;
    return false;
  }
  return is_temporal() ? get_date_from_string(res, fuzzydate)
                       : get_date_from_non_temporal(res, fuzzydate);
}

double Item_param::val_real() {
  switch (state) {
    case REAL_VALUE:
      return value.real;
    case INT_VALUE:
      return (double)value.integer;
    case DECIMAL_VALUE: {
      double result;
      my_decimal2double(E_DEC_FATAL_ERROR, &decimal_value, &result);
      return result;
    }
    case STRING_VALUE:
    case LONG_DATA_VALUE: {
      return double_from_string_with_check(
          str_value.charset(), str_value.ptr(),
          str_value.ptr() + str_value.length());
    }
    case TIME_VALUE:
      /*
        This works for example when user says SELECT ?+0.0 and supplies
        time value for the placeholder.
      */
      return TIME_to_double(value.time);
    case NULL_VALUE:
      return 0.0;
    default:
      DBUG_ASSERT(0);
  }
  return 0.0;
}

longlong Item_param::val_int() {
  switch (state) {
    case REAL_VALUE:
      return (longlong)rint(value.real);
    case INT_VALUE:
      return value.integer;
    case DECIMAL_VALUE: {
      longlong i;
      my_decimal2int(E_DEC_FATAL_ERROR, &decimal_value, unsigned_flag, &i);
      return i;
    }
    case STRING_VALUE:
    case LONG_DATA_VALUE: {
      int dummy_err;
      return my_strntoll(str_value.charset(), str_value.ptr(),
                         str_value.length(), 10, nullptr, &dummy_err);
    }
    case TIME_VALUE:
      return (longlong)propagate_datetime_overflow(current_thd, [&](int *w) {
        return TIME_to_ulonglong_round(value.time, w);
      });
    case NULL_VALUE:
      return 0;
    default:
      DBUG_ASSERT(0);
  }
  return 0;
}

my_decimal *Item_param::val_decimal(my_decimal *dec) {
  switch (state) {
    case DECIMAL_VALUE:
      return &decimal_value;
    case REAL_VALUE:
      double2my_decimal(E_DEC_FATAL_ERROR, value.real, dec);
      return dec;
    case INT_VALUE:
      int2my_decimal(E_DEC_FATAL_ERROR, value.integer, unsigned_flag, dec);
      return dec;
    case STRING_VALUE:
    case LONG_DATA_VALUE:
      str2my_decimal(E_DEC_FATAL_ERROR, str_value.ptr(), str_value.length(),
                     str_value.charset(), dec);
      return dec;
    case TIME_VALUE:
      return date2my_decimal(&value.time, dec);
    case NULL_VALUE:
      return nullptr;
    default:
      DBUG_ASSERT(0);
  }
  return nullptr;
}

String *Item_param::val_str(String *str) {
  switch (state) {
    case STRING_VALUE:
    case LONG_DATA_VALUE:
      return &str_value_ptr;
    case REAL_VALUE:
      str->set_real(value.real, DECIMAL_NOT_SPECIFIED, &my_charset_bin);
      return str;
    case INT_VALUE:
      str->set(value.integer, &my_charset_bin);
      return str;
    case DECIMAL_VALUE:
      if (my_decimal2string(E_DEC_FATAL_ERROR, &decimal_value, str) <= 1)
        return str;
      return nullptr;
    case TIME_VALUE: {
      if (str->reserve(MAX_DATE_STRING_REP_LENGTH)) break;
      str->length(my_TIME_to_str(value.time, str->ptr(),
                                 min(decimals, uint8{DATETIME_MAX_DECIMALS})));
      str->set_charset(&my_charset_bin);
      return str;
    }
    case NULL_VALUE:
      return nullptr;
    default:
      DBUG_ASSERT(0);
  }
  return str;
}

/**
  Return Param item values in string format, for generating the dynamic
  query used in update/binary logs.

  @todo
    - Change interface and implementation to fill log data in place
    and avoid one more memcpy/alloc between str and log string.
    - In case of error we need to notify replication
    that binary log contains wrong statement
*/

const String *Item_param::query_val_str(const THD *thd, String *str) const {
  switch (state) {
    case INT_VALUE:
      str->set_int(value.integer, unsigned_flag, &my_charset_bin);
      break;
    case REAL_VALUE:
      str->set_real(value.real, DECIMAL_NOT_SPECIFIED, &my_charset_bin);
      break;
    case DECIMAL_VALUE:
      if (my_decimal2string(E_DEC_FATAL_ERROR, &decimal_value, str) > 1)
        return &my_null_string;
      break;
    case TIME_VALUE: {
      char *buf, *ptr;
      str->length(0);
      /*
        TODO: in case of error we need to notify replication
        that binary log contains wrong statement
      */
      if (str->reserve(MAX_DATE_STRING_REP_LENGTH + 3)) break;

      /* Create date string inplace */
      buf = str->c_ptr_quick();
      ptr = buf;
      *ptr++ = '\'';
      ptr += my_TIME_to_str(value.time, ptr,
                            min(decimals, uint8{DATETIME_MAX_DECIMALS}));
      *ptr++ = '\'';
      str->length((uint32)(ptr - buf));
      break;
    }
    case STRING_VALUE:
    case LONG_DATA_VALUE: {
      str->length(0);
      append_query_string(thd, value.cs_info.character_set_client, &str_value,
                          str);
      break;
    }
    case NULL_VALUE:
      return &my_null_string;
    default:
      DBUG_ASSERT(0);
  }
  return str;
}

/**
  Convert string from client character set to the character set of
  connection.
*/

bool Item_param::convert_str_value() {
  if (state == STRING_VALUE || state == LONG_DATA_VALUE) {
    if (value.cs_info.final_character_set_of_str_value == nullptr ||
        value.cs_info.character_set_of_placeholder == nullptr)
      return true;
    /*
      Check is so simple because all charsets were set up properly
      in setup_one_conversion_function, where typecode of
      placeholder was also taken into account: the variables are different
      here only if conversion is really necessary.
    */
    if (value.cs_info.final_character_set_of_str_value !=
        value.cs_info.character_set_of_placeholder) {
      uint dummy_errors;
      StringBuffer<STRING_BUFFER_USUAL_SIZE> convert_buffer;
      if (convert_buffer.copy(str_value.ptr(), str_value.length(),
                              value.cs_info.character_set_of_placeholder,
                              value.cs_info.final_character_set_of_str_value,
                              &dummy_errors))
        return true;
      if (str_value.copy(convert_buffer)) return true;
    } else
      str_value.set_charset(value.cs_info.final_character_set_of_str_value);
    /* Here str_value is guaranteed to be in final_character_set_of_str_value */

    max_length = static_cast<uint32>(str_value.numchars() *
                                     str_value.charset()->mbmaxlen);

    /* For the strings converted to numeric form within some functions */
    decimals = DECIMAL_NOT_SPECIFIED;
    /*
      str_value_ptr is returned from val_str(). It must be not alloced
      to prevent it's modification by val_str() invoker.
    */
    str_value_ptr.set(str_value.ptr(), str_value.length(), str_value.charset());
    /* Synchronize item charset with value charset */
    collation.set(str_value.charset(), DERIVATION_COERCIBLE);
  }
  return false;
}

Item *Item_param::clone_item() const {
  /* see comments in the header file */
  switch (state) {
    case NULL_VALUE:
      return new Item_null(item_name);
    case INT_VALUE:
      return (unsigned_flag
                  ? new Item_uint(item_name, value.integer, max_length)
                  : new Item_int(item_name, value.integer, max_length));
    case REAL_VALUE:
      return new Item_float(item_name, value.real, decimals, max_length);
    case STRING_VALUE:
    case LONG_DATA_VALUE:
      return new Item_string(item_name, str_value.ptr(), str_value.length(),
                             str_value.charset());
    case TIME_VALUE:
      break;
    case NO_VALUE:
    default:
      DBUG_ASSERT(0);
  };
  return nullptr;
}

bool Item_param::eq(const Item *arg, bool binary_cmp) const {
  if (!basic_const_item() || !arg->basic_const_item() || arg->type() != type())
    return false;
  /*
    We need to cast off const to call val_int(). This should be OK for
    a basic constant.
  */
  Item *item = const_cast<Item *>(arg);
  String str;

  switch (state) {
    case NULL_VALUE:
      return true;
    case INT_VALUE:
      return value.integer == item->val_int() &&
             unsigned_flag == item->unsigned_flag;
    case REAL_VALUE:
      return value.real == item->val_real();
    case STRING_VALUE:
    case LONG_DATA_VALUE:
      if (binary_cmp) return !stringcmp(&str_value, item->val_str(&str));
      return !sortcmp(&str_value, item->val_str(&str), collation.collation);
    default:
      break;
  }
  return false;
}

/* End of Item_param related */

void Item_param::print(const THD *thd, String *str,
                       enum_query_type query_type) const {
  if (state == NO_VALUE ||
      query_type & (QT_NORMALIZED_FORMAT | QT_NO_DATA_EXPANSION)) {
    str->append('?');
  } else {
    char buffer[STRING_BUFFER_USUAL_SIZE];
    String tmp(buffer, sizeof(buffer), &my_charset_bin);
    const String *res;
    res = query_val_str(thd, &tmp);
    str->append(*res);
  }
}

/**
  Preserve the original parameter types and values
  when re-preparing a prepared statement.

  @details Copy parameter type information and conversion
  function pointers from a parameter of the old statement
  to the corresponding parameter of the new one.

  Move parameter values from the old parameters to the new
  one. We simply "exchange" the values, which allows
  to save on allocation and character set conversion in
  case a parameter is a string or a blob/clob.

  The old parameter gets the value of this one, which
  ensures that all memory of this parameter is freed
  correctly.

  @param[in]  src   parameter item of the original
                    prepared statement
*/

void Item_param::set_param_type_and_swap_value(Item_param *src) {
  unsigned_flag = src->unsigned_flag;
  set_data_type(src->data_type());
  set_param_func = src->set_param_func;
  item_type = src->item_type;
  item_result_type = src->item_result_type;

  collation.set(src->collation);
  maybe_null = src->maybe_null;
  null_value = src->null_value;
  set_data_type(src->data_type());
  max_length = src->max_length;
  decimals = src->decimals;
  state = src->state;
  value = src->value;

  decimal_value.swap(src->decimal_value);
  str_value.swap(src->str_value);
  str_value_ptr.swap(src->str_value_ptr);
}

/**
  This operation is intended to store some item value in Item_param to be
  used later.

  @param it     a pointer to an item in the tree

  @return Error status
    @retval true on error
    @retval false on success
*/

bool Item_param::set_value(THD *, sp_rcontext *, Item **it) {
  Item *arg = *it;

  if (arg->is_null()) {
    set_null();
    return false;
  }

  null_value = false;

  switch (arg->result_type()) {
    case STRING_RESULT: {
      char str_buffer[STRING_BUFFER_USUAL_SIZE];
      String sv_buffer(str_buffer, sizeof(str_buffer), &my_charset_bin);
      String *sv = arg->val_str(&sv_buffer);

      if (!sv) return true;

      set_str(sv->c_ptr_safe(), sv->length());
      str_value_ptr.set(str_value.ptr(), str_value.length(),
                        str_value.charset());
      collation.set(str_value.charset(), DERIVATION_COERCIBLE);
      decimals = 0;
      item_type = Item::STRING_ITEM;
      break;
    }

    case REAL_RESULT:
      set_double(arg->val_real());
      item_type = Item::REAL_ITEM;
      break;

    case INT_RESULT:
      set_int(arg->val_int(), arg->max_length);
      item_type = Item::INT_ITEM;
      break;

    case DECIMAL_RESULT: {
      my_decimal dv_buf;
      my_decimal *dv = arg->val_decimal(&dv_buf);

      if (!dv) return true;

      set_decimal(dv);
      item_type = Item::DECIMAL_ITEM;
      break;
    }

    default:
      /* That can not happen. */

      DBUG_ASSERT(false);  // Abort in debug mode.

      set_null();  // Set to NULL in release mode.
      item_type = Item::NULL_ITEM;
      return false;
  }

  item_result_type = arg->result_type();
  return false;
}

/**
  Setter of Item_param::m_out_param_info.

  m_out_param_info is used to store information about store routine
  OUT-parameters, such as stored routine name, database, stored routine
  variable name. It is supposed to be set in sp_head::execute() after
  Item_param::set_value() is called.
*/

void Item_param::set_out_param_info(Send_field *info) {
  m_out_param_info = info;
  set_data_type(m_out_param_info->type);
}

/**
  Getter of Item_param::m_out_param_info.

  m_out_param_info is used to store information about store routine
  OUT-parameters, such as stored routine name, database, stored routine
  variable name. It is supposed to be retrieved in
  Protocol::send_parameters() during creation of OUT-parameter result set.
*/

const Send_field *Item_param::get_out_param_info() const {
  return m_out_param_info;
}

/**
  Fill meta-data information for the corresponding column in a result set.
  If this is an OUT-parameter of a stored procedure, preserve meta-data of
  stored-routine variable.

  @param field container for meta-data to be filled
*/

void Item_param::make_field(Send_field *field) {
  Item::make_field(field);

  if (!m_out_param_info) return;

  /*
    This is an OUT-parameter of stored procedure. We should use
    OUT-parameter info to fill out the names.
  */

  field->db_name = m_out_param_info->db_name;
  field->table_name = m_out_param_info->table_name;
  field->org_table_name = m_out_param_info->org_table_name;
  field->col_name = m_out_param_info->col_name;
  field->org_col_name = m_out_param_info->org_col_name;

  field->length = m_out_param_info->length;
  field->charsetnr = m_out_param_info->charsetnr;
  field->flags = m_out_param_info->flags;
  field->decimals = m_out_param_info->decimals;
  field->type = m_out_param_info->type;
}

/****************************************************************************
  Item_copy
****************************************************************************/
Item_copy *Item_copy::create(Item *item) {
  switch (item->result_type()) {
    case STRING_RESULT:
      if (item->data_type() == MYSQL_TYPE_JSON)
        return new Item_copy_json(item);
      else
        return new Item_copy_string(item);
    case REAL_RESULT:
      return new Item_copy_float(item);
    case INT_RESULT:
      return item->unsigned_flag ? new Item_copy_uint(item)
                                 : new Item_copy_int(item);
    case DECIMAL_RESULT:
      return new Item_copy_decimal(item);
    default:
      DBUG_ASSERT(0);
  }
  /* should not happen */
  return nullptr;
}

/****************************************************************************
  Item_copy_string
****************************************************************************/

double Item_copy_string::val_real() {
  int err_not_used;
  const char *end_not_used;
  return (null_value
              ? 0.0
              : my_strntod(str_value.charset(), str_value.ptr(),
                           str_value.length(), &end_not_used, &err_not_used));
}

longlong Item_copy_string::val_int() {
  int err;
  return null_value ? 0LL
                    : my_strntoll(str_value.charset(), str_value.ptr(),
                                  str_value.length(), 10, nullptr, &err);
}

type_conversion_status Item_copy_string::save_in_field_inner(Field *field,
                                                             bool) {
  return save_str_value_in_field(field, &str_value);
}

bool Item_copy_string::copy(const THD *thd) {
  String *res = item->val_str(&str_value);
  if (res != nullptr) str_value.copy(*res);
  null_value = item->null_value;
  return thd->is_error();
}

String *Item_copy_string::val_str(String *) {
  // Item_copy_string is used without fix_fields call
  if (null_value) return (String *)nullptr;
  return &str_value;
}

my_decimal *Item_copy_string::val_decimal(my_decimal *decimal_value) {
  // Item_copy_string is used without fix_fields call
  if (null_value) return (my_decimal *)nullptr;
  str2my_decimal(E_DEC_FATAL_ERROR, str_value.ptr(), str_value.length(),
                 str_value.charset(), decimal_value);
  return (decimal_value);
}

bool Item_copy_string::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  return get_date_from_string(ltime, fuzzydate);
}

bool Item_copy_string::get_time(MYSQL_TIME *ltime) {
  return get_time_from_string(ltime);
}

/****************************************************************************
 Item_copy_json
 ****************************************************************************/

Item_copy_json::Item_copy_json(Item *item_arg)
    : Item_copy(item_arg), m_value(new (*THR_MALLOC) Json_wrapper()) {}

Item_copy_json::~Item_copy_json() { destroy(m_value); }

bool Item_copy_json::copy(const THD *thd) {
  if (item->val_json(m_value)) return true;

  null_value = item->null_value;

  if (!null_value) {
    // need own copy, cf. also Item_cache_json::cache_value
    m_value->to_dom(thd);
  }

  return false;
}

/**
  Copy the cached JSON value into a wrapper.
  @param[out] wr the wrapper that receives the JSON value
*/
bool Item_copy_json::val_json(Json_wrapper *wr) {
  if (null_value) return false;

  *wr = *m_value;
  return false;
}

String *Item_copy_json::val_str(String *s) {
  if (null_value) return nullptr;

  s->length(0);
  if (m_value->to_string(s, true, item_name.ptr())) return error_str();
  return s;
}

my_decimal *Item_copy_json::val_decimal(my_decimal *decimal_value) {
  if (null_value) return nullptr;

  return m_value->coerce_decimal(decimal_value, item_name.ptr());
}

double Item_copy_json::val_real() {
  if (null_value) return 0.0;

  return m_value->coerce_real(item_name.ptr());
}

longlong Item_copy_json::val_int() {
  if (null_value) return 0;

  return m_value->coerce_int(item_name.ptr());
}

bool Item_copy_json::get_date(MYSQL_TIME *ltime, my_time_flags_t) {
  if (null_value) return true;

  return m_value->coerce_date(ltime, item_name.ptr());
}

bool Item_copy_json::get_time(MYSQL_TIME *ltime) {
  if (null_value) return true;

  return m_value->coerce_time(ltime, item_name.ptr());
}

type_conversion_status Item_copy_json::save_in_field_inner(Field *field, bool) {
  if (null_value) return set_field_to_null(field);

  if (field->type() == MYSQL_TYPE_JSON) {
    Field_json *f = down_cast<Field_json *>(field);
    f->set_notnull();
    return f->store_json(m_value);
  } else {
    str_value.length(0);
    if (m_value->to_string(&str_value, true, item_name.ptr()))
      return set_field_to_null(field); /* purecov: inspected */
    return save_str_value_in_field(field, &str_value);
  }
}

/****************************************************************************
  Item_copy_int
****************************************************************************/

bool Item_copy_int::copy(const THD *thd) {
  cached_value = item->val_int();
  null_value = item->null_value;
  return thd->is_error();
}

static type_conversion_status save_int_value_in_field(Field *field, longlong nr,
                                                      bool null_value,
                                                      bool unsigned_flag);

type_conversion_status Item_copy_int::save_in_field_inner(Field *field, bool) {
  return save_int_value_in_field(field, cached_value, null_value,
                                 unsigned_flag);
}

String *Item_copy_int::val_str(String *str) {
  if (null_value) return (String *)nullptr;

  str->set(cached_value, &my_charset_bin);
  return str;
}

my_decimal *Item_copy_int::val_decimal(my_decimal *decimal_value) {
  if (null_value) return (my_decimal *)nullptr;

  int2my_decimal(E_DEC_FATAL_ERROR, cached_value, unsigned_flag, decimal_value);
  return decimal_value;
}

/****************************************************************************
  Item_copy_uint
****************************************************************************/

String *Item_copy_uint::val_str(String *str) {
  if (null_value) return (String *)nullptr;

  str->set((ulonglong)cached_value, &my_charset_bin);
  return str;
}

/****************************************************************************
  Item_copy_float
****************************************************************************/

bool Item_copy_float::copy(const THD *thd) {
  cached_value = item->val_real();
  null_value = item->null_value;
  return thd->is_error();
}

String *Item_copy_float::val_str(String *str) {
  if (null_value)
    return (String *)nullptr;
  else {
    double nr = val_real();
    str->set_real(nr, decimals, &my_charset_bin);
    return str;
  }
}

my_decimal *Item_copy_float::val_decimal(my_decimal *decimal_value) {
  if (null_value)
    return (my_decimal *)nullptr;
  else {
    double nr = val_real();
    double2my_decimal(E_DEC_FATAL_ERROR, nr, decimal_value);
    return decimal_value;
  }
}

type_conversion_status Item_copy_float::save_in_field_inner(Field *field,
                                                            bool) {
  // TODO: call set_field_to_null_with_conversions below
  if (null_value) return set_field_to_null(field);

  field->set_notnull();
  return field->store(cached_value);
}

/****************************************************************************
  Item_copy_decimal
****************************************************************************/

type_conversion_status Item_copy_decimal::save_in_field_inner(Field *field,
                                                              bool) {
  // TODO: call set_field_to_null_with_conversions below
  if (null_value) return set_field_to_null(field);
  field->set_notnull();
  return field->store_decimal(&cached_value);
}

String *Item_copy_decimal::val_str(String *result) {
  if (null_value) return (String *)nullptr;
  result->set_charset(&my_charset_bin);
  my_decimal2string(E_DEC_FATAL_ERROR, &cached_value, result);
  return result;
}

double Item_copy_decimal::val_real() {
  if (null_value)
    return 0.0;
  else {
    double result;
    my_decimal2double(E_DEC_FATAL_ERROR, &cached_value, &result);
    return result;
  }
}

longlong Item_copy_decimal::val_int() {
  if (null_value)
    return 0LL;
  else {
    longlong result;
    my_decimal2int(E_DEC_FATAL_ERROR, &cached_value, unsigned_flag, &result);
    return result;
  }
}

bool Item_copy_decimal::copy(const THD *thd) {
  my_decimal *nr = item->val_decimal(&cached_value);
  if (nr) {
    my_decimal_round(E_DEC_FATAL_ERROR, nr, decimals, false, nr);
    if (nr != &cached_value) my_decimal2decimal(nr, &cached_value);
  }
  null_value = item->null_value;
  return thd->is_error();
}

/*
  Functions to convert item to field (for send_result_set_metadata)
*/

bool Item::fix_fields(THD *, Item **) {
  DBUG_ASSERT(is_contextualized());

  // We do not check fields which are fixed during construction
  DBUG_ASSERT(fixed == 0 || basic_const_item());
  fixed = true;
  return false;
}

double Item_ref_null_helper::val_real() {
  auto tmp = super::val_real();
  owner->was_null |= null_value;
  return tmp;
}

longlong Item_ref_null_helper::val_int() {
  auto tmp = super::val_int();
  owner->was_null |= null_value;
  return tmp;
}

longlong Item_ref_null_helper::val_time_temporal() {
  auto tmp = super::val_time_temporal();
  owner->was_null |= null_value;
  return tmp;
}

longlong Item_ref_null_helper::val_date_temporal() {
  auto tmp = super::val_date_temporal();
  owner->was_null |= null_value;
  return tmp;
}

my_decimal *Item_ref_null_helper::val_decimal(my_decimal *decimal_value) {
  auto tmp = super::val_decimal(decimal_value);
  owner->was_null |= null_value;
  return tmp;
}

bool Item_ref_null_helper::val_bool() {
  auto tmp = super::val_bool();
  owner->was_null |= null_value;
  return tmp;
}

String *Item_ref_null_helper::val_str(String *s) {
  auto tmp = super::val_str(s);
  owner->was_null |= null_value;
  return tmp;
}

bool Item_ref_null_helper::get_date(MYSQL_TIME *ltime,
                                    my_time_flags_t fuzzydate) {
  auto tmp = super::get_date(ltime, fuzzydate);
  owner->was_null |= null_value;
  return tmp;
}

/**
  Mark item and SELECT_LEXs as dependent if item was resolved in
  outer SELECT.

  @param thd             Current session.
  @param last            select from which current item depend
  @param current         current select
  @param resolved_item   item which was resolved in outer SELECT
  @param mark_item       item which should be marked; resolved_item will be
  marked anyway.
*/

static void mark_as_dependent(THD *thd, SELECT_LEX *last, SELECT_LEX *current,
                              Item_ident *resolved_item,
                              Item_ident *mark_item) {
  const char *db_name = (resolved_item->db_name ? resolved_item->db_name : "");
  const char *table_name =
      (resolved_item->table_name ? resolved_item->table_name : "");
  /* store pointer on SELECT_LEX from which item is dependent */
  if (mark_item) mark_item->depended_from = last;
  /*
    resolved_item is the one we are resolving (and we just found that it is an
    outer ref), its context is surely the subquery (see assertion below), so
    we set depended_from for it.
  */
  resolved_item->depended_from = last;
  DBUG_ASSERT(resolved_item->context->select_lex == current);

  current->mark_as_dependent(last, false);
  if (thd->lex->is_explain()) {
    /*
      UNION's result has select_number == INT_MAX which is printed as -1 and
      this is confusing. Instead, the number of the first SELECT in the UNION
      is printed as names in ORDER BY are resolved against select list of the
      first SELECT.
    */
    uint sel_nr = (last->select_number < INT_MAX)
                      ? last->select_number
                      : last->master_unit()->first_select()->select_number;
    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_WARN_FIELD_RESOLVED,
                        ER_THD(thd, ER_WARN_FIELD_RESOLVED), db_name,
                        (db_name[0] ? "." : ""), table_name,
                        (table_name[0] ? "." : ""), resolved_item->field_name,
                        current->select_number, sel_nr);
  }
}

/**
  Search a GROUP BY clause for a field with a certain name.

  Search the GROUP BY list for a column named as find_item. When searching
  preference is given to columns that are qualified with the same table (and
  database) name as the one being searched for.

  @param find_item     the item being searched for
  @param group_list    GROUP BY clause

  @return
    - the found item on success
    - NULL if find_item is not in group_list
*/

static Item **find_field_in_group_list(Item *find_item, ORDER *group_list) {
  const char *db_name;
  const char *table_name;
  const char *field_name;
  ORDER *found_group = nullptr;
  int found_match_degree = 0;
  Item_ident *cur_field;
  int cur_match_degree = 0;
  char name_buff[NAME_LEN + 1];

  if (find_item->type() == Item::FIELD_ITEM ||
      find_item->type() == Item::REF_ITEM) {
    db_name = ((Item_ident *)find_item)->db_name;
    table_name = ((Item_ident *)find_item)->table_name;
    field_name = ((Item_ident *)find_item)->field_name;
  } else
    return nullptr;

  if (db_name && lower_case_table_names) {
    /* Convert database to lower case for comparison */
    strmake(name_buff, db_name, sizeof(name_buff) - 1);
    my_casedn_str(files_charset_info, name_buff);
    db_name = name_buff;
  }

  DBUG_ASSERT(field_name != nullptr);

  for (ORDER *cur_group = group_list; cur_group; cur_group = cur_group->next) {
    if ((*(cur_group->item))->real_item()->type() == Item::FIELD_ITEM) {
      cur_field = (Item_ident *)*cur_group->item;
      cur_match_degree = 0;

      DBUG_ASSERT(cur_field->field_name != nullptr);

      if (!my_strcasecmp(system_charset_info, cur_field->field_name,
                         field_name))
        ++cur_match_degree;
      else
        continue;

      if (cur_field->table_name && table_name) {
        /* If field_name is qualified by a table name. */
        if (my_strcasecmp(table_alias_charset, cur_field->table_name,
                          table_name))
          /* Same field names, different tables. */
          return nullptr;

        ++cur_match_degree;
        if (cur_field->db_name && db_name) {
          /* If field_name is also qualified by a database name. */
          if (strcmp(cur_field->db_name, db_name))
            /* Same field names, different databases. */
            return nullptr;
          ++cur_match_degree;
        }
      }

      if (cur_match_degree > found_match_degree) {
        found_match_degree = cur_match_degree;
        found_group = cur_group;
      } else if (found_group && (cur_match_degree == found_match_degree) &&
                 !(*(found_group->item))->eq(cur_field, false)) {
        /*
          If the current resolve candidate matches equally well as the current
          best match, they must reference the same column, otherwise the field
          is ambiguous.
        */
        my_error(ER_NON_UNIQ_ERROR, MYF(0), find_item->full_name(),
                 current_thd->where);
        return nullptr;
      }
    }
  }

  if (found_group)
    return found_group->item;
  else
    return nullptr;
}

/**
  Resolve a column reference in a sub-select.

  Resolve a column reference (usually inside a HAVING clause) against the
  SELECT and GROUP BY clauses of the query described by 'select'. The name
  resolution algorithm searches both the SELECT and GROUP BY clauses, and in
  case of a name conflict prefers GROUP BY column names over SELECT names. If
  both clauses contain different fields with the same names, a warning is
  issued that name of 'ref' is ambiguous. We extend ANSI SQL in that when no
  GROUP BY column is found, then a HAVING name is resolved as a possibly
  derived SELECT column.

  @param thd     current thread
  @param ref     column reference being resolved
  @param select  the select that ref is resolved against

  @note
    The resolution procedure is:
    - Search for a column or derived column named col_ref_i [in table T_j]
    in the SELECT clause of Q.
    - Search for a column named col_ref_i [in table T_j]
    in the GROUP BY clause of Q.
    - If found different columns with the same name in GROUP BY and SELECT,
    issue a warning
    - return the found GROUP BY column if any,
    - else return the found SELECT column if any.


  @return
    - NULL - there was an error, and the error was already reported
    - not_found_item - the item was not resolved, no error was reported
    - resolved item - if the item was resolved
*/

static Item **resolve_ref_in_select_and_group(THD *thd, Item_ident *ref,
                                              SELECT_LEX *select) {
  DBUG_TRACE;
  Item **select_ref = nullptr;
  ORDER *group_list = select->group_list.first;
  uint counter;
  enum_resolution_type resolution;

  /*
    Search for a column or derived column named as 'ref' in the SELECT
    clause of the current select.
  */
  if (!(select_ref =
            find_item_in_list(thd, ref, *(select->get_item_list()), &counter,
                              REPORT_EXCEPT_NOT_FOUND, &resolution)))
    return nullptr; /* Some error occurred. */
  if (resolution == RESOLVED_AGAINST_ALIAS) ref->set_alias_of_expr();

  /* If this is a non-aggregated field inside HAVING, search in GROUP BY. */
  if (select->having_fix_field && !ref->has_aggregation() && group_list) {
    Item **group_by_ref = find_field_in_group_list(ref, group_list);

    /* Check if the fields found in SELECT and GROUP BY are the same field. */
    if (group_by_ref && (select_ref != not_found_item) &&
        !((*group_by_ref)->eq(*select_ref, false))) {
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_NON_UNIQ_ERROR,
                          ER_THD(thd, ER_NON_UNIQ_ERROR), ref->full_name(),
                          thd->where);
    }

    if (group_by_ref != nullptr) return group_by_ref;
  }

  if (select_ref == not_found_item) return not_found_item;

  if ((*select_ref)->has_wf()) {
    /*
      We can't reference an alias to a window function expr from within
      a subquery or a HAVING clause
    */
    my_error(ER_WINDOW_INVALID_WINDOW_FUNC_ALIAS_USE, MYF(0), ref->field_name);
    return nullptr;
  }

  /*
    The pointer in base_ref_items is nullptr if the column reference
    is a reference to itself, such as 'a' in:

      SELECT (SELECT ... WHERE a = 1) AS a ...

    Or if it's a reference to an expression that comes later in the
    select list, such as 'b' in:

      SELECT (SELECT ... WHERE b = 1) AS a, (SELECT ...) AS b ...

    Raise an error if such invalid references are encountered.
  */
  if (select->base_ref_items[counter] == nullptr) {
    my_error(ER_ILLEGAL_REFERENCE, MYF(0), ref->item_name.ptr(),
             "forward reference in item list");
    return nullptr;
  }

  DBUG_ASSERT((*select_ref)->fixed);

  return &select->base_ref_items[counter];
}

/**
  Resolve the name of an outer select column reference.

  The method resolves the column reference represented by 'this' as a column
  present in outer selects that contain current select.

  In prepared statements, because of cache, find_field_in_tables()
  can resolve fields even if they don't belong to current context.
  In this case this method only finds appropriate context and marks
  current select as dependent. The found reference of field should be
  provided in 'from_field'.

  @param[in] thd             current thread
  @param[in,out] from_field  found field reference or (Field*)not_found_field
  @param[in,out] reference   view column if this item was resolved to a
    view column

  @note
    This is the inner loop of Item_field::fix_fields:
  @code
        for each outer query Q_k beginning from the inner-most one
        {
          search for a column or derived column named col_ref_i
          [in table T_j] in the FROM clause of Q_k;

          if such a column is not found
            Search for a column or derived column named col_ref_i
            [in table T_j] in the SELECT and GROUP clauses of Q_k.
        }
  @endcode

  @retval
    1   column succefully resolved and fix_fields() should continue.
  @retval
    0   column fully fixed and fix_fields() should return false
  @retval
    -1  error occurred
*/

int Item_field::fix_outer_field(THD *thd, Field **from_field,
                                Item **reference) {
  bool field_found = (*from_field != not_found_field);
  bool upward_lookup = false;

  /*
    If there are outer contexts (outer selects, but current select is
    not derived table or view) try to resolve this reference in the
    outer contexts.

    We treat each subselect as a separate namespace, so that different
    subselects may contain columns with the same names. The subselects
    are searched starting from the innermost.
  */
  Name_resolution_context *last_checked_context = context;
  Item **ref = not_found_item;
  Name_resolution_context *outer_context = context->outer_context;
  SELECT_LEX *select = nullptr;
  SELECT_LEX_UNIT *cur_unit = nullptr;
  enum_parsing_context place = CTX_NONE;
  SELECT_LEX *cur_select = context->select_lex;
  for (; outer_context; outer_context = outer_context->outer_context) {
    select = outer_context->select_lex;

    last_checked_context = outer_context;
    upward_lookup = true;

    /*
      We want to locate the qualifying query of our Item_field 'this'.
      'this' is simply contained in a subquery (SELECT_LEX_UNIT) which is
      immediately contained
      - in a scalar/row subquery (Item_subselect), or
      - in a table subquery itself immediately contained in a quantified
      predicate (Item_subselect) or a derived table (TABLE_LIST).
      'this' has an 'outer_context' where it should be searched first.
      'outer_context' is the context of a query block or sometimes
      of a specific part of a query block (e.g. JOIN... ON condition).
      We go up from 'context' to 'outer_context', from inner to outer
      subqueries. On that bottom-up path, we stop at the subquery unit which
      is simply contained in 'outer_context': it belongs to an
      Item_subselect/TABLE_LIST object which we note OUTER_CONTEXT_OBJECT.
      Then the search of 'this' in 'outer_context' is influenced by
      where OUTER_CONTEXT_OBJECT is in 'outer_context'. For example, if
      OUTER_CONTEXT_OBJECT is in WHERE, a search by alias is not done.
      Thus, given an 'outer_context' to search in, the first step is
      to determine OUTER_CONTEXT_OBJECT. Then we search for 'this' in
      'outer_context'. Then, if search is successful, we mark objects, from
      'context' up to 'outer_context', as follows:
      - OUTER_CONTEXT_OBJECT is marked as "using table map this->map()";
      - more inner subqueries are marked as "dependent on outer reference"
      (correlated, UNCACHEABLE_DEPENDENT bit)
      If search is not successful, retry with the yet-more-outer context
      (determine the new OUTER_CONTEXT_OBJECT, etc).

      Note that any change here must be duplicated in Item_ref::fix_fields.
    */
    DBUG_PRINT("outer_field",
               ("must reach target ctx (having SL#%d)", select->select_number));
    /*
      Walk from the innermost query block to the outermost until we find
      OUTER_CONTEXT_OBJECT; cur_select and cur_unit track where the walk
      currently is.
    */
    while (true) {
      if (!cur_select) goto loop;
      DBUG_PRINT("outer_field",
                 ("in loop, in ctx of SL#%d", cur_select->select_number));
      if (cur_select == select) {
        /*
          @todo after WL#6570 we won't re-resolve so this if() should be
          removed.
          In such prep stmt (main.subquery_sj_firstmatch_bkaunique):
          SELECT COUNT(*) FROM t1 GROUP BY t1.a
          HAVING t1.a IN (SELECT t3.a FROM t3
               WHERE t3.b IN (SELECT b FROM t2 WHERE t2.a=t1.a))
          In PREPARE we do semijoin on subq of WHERE. We get
          SELECT ... HAVING EXISTS (SELECT FROM t3 SJ t2 WHERE t2.a=t1.a AND
          ...) In EXECUTE, when we resolve t1.a we come here; context of t1.a is
          the context of now-gone SELECT#3 but which has been repointed to
          SELECT#2. The outer context of this context is the original context
          of SELECT#2. I.e. both context and outer_context belong to same
          SELECT#2.
          So, when we resolve t1.a we are not able to determine 'E', a
          subquery expression containing t1.a and contained in the owner of
          outer_context, as the innermost such subquery is SELECT#2, and the
          said owner is SELECT#2 too.
          So we have this special branch to make sure our loop doesn't go
          crazy and crashing. After "goto loop", outer_context becomes that
          of SELECT#1 and so we are able to determine 'E' (Item_subselect of
          SELECT#2) containing t1.a and contained in the owner of outer_context
          (SELECT#1) so things work as expected. And anyway, cached_table
          provides the right table to use.
        */
        DBUG_ASSERT(!cur_select->first_execution);
        goto loop;  // we're misplaced
      }
      cur_unit = cur_select->master_unit();
      if (cur_unit->outer_select() == select)
        break;  // the immediate container of cur_unit is OUTER_CONTEXT_OBJECT
      DBUG_PRINT("outer_field",
                 ("in loop, in ctx of SL#%d, not yet immediate child of target",
                  cur_select->select_number));
      // cur_unit belongs to an object inside OUTER_CONTEXT_OBJECT, mark it and
      // go up:
      cur_unit->accumulate_used_tables(OUTER_REF_TABLE_BIT);
      cur_select = cur_unit->outer_select();
    }

    DBUG_PRINT("outer_field", ("out of loop, reached target ctx (having SL#%d)",
                               cur_select->select_number));

    // Place of OUTER_CONTEXT_OBJECT in 'outer_context' e.g. WHERE :
    place = cur_unit->place();

    // A non-lateral derived table cannot see tables of its owning query
    if (place == CTX_DERIVED && select->end_lateral_table == nullptr) continue;

    /*
      If field was already found by first call
      to find_field_in_tables(), we only need to find appropriate context.
    */
    if (field_found && outer_context->select_lex != cached_table->select_lex) {
      DBUG_PRINT("outer_field", ("but cached is of SL#%d, continue",
                                 cached_table->select_lex->select_number));
      continue;
    }

    /*
      In case of a view, find_field_in_tables() writes the pointer to
      the found view field into '*reference', in other words, it
      substitutes this Item_field with the found expression.
    */
    if (field_found ||
        (*from_field = find_field_in_tables(
             thd, this, outer_context->first_name_resolution_table,
             outer_context->last_name_resolution_table, reference,
             IGNORE_EXCEPT_NON_UNIQUE, thd->want_privilege, true)) !=
            not_found_field) {
      if (*from_field) {
        if (*from_field != view_ref_found) {
          cur_unit->accumulate_used_tables(
              (*from_field)->table->pos_in_table_list->map());
          set_field(*from_field);

          if (!last_checked_context->select_lex->having_fix_field &&
              select->group_list.elements &&
              (place == CTX_SELECT_LIST || place == CTX_HAVING)) {
            Item_outer_ref *rf;
            /*
              If an outer field is resolved in a grouping select then it
              is replaced for an Item_outer_ref object. Otherwise an
              Item_field object is used.
            */
            if (!(rf = new Item_outer_ref(context, this, select))) return -1;
            rf->in_sum_func = thd->lex->in_sum_func;
            thd->change_item_tree(reference, rf);
            if (rf->fix_fields(thd, nullptr)) return -1;
          }
          /*
            A reference is resolved to a nest level that's outer or the same as
            the nest level of the enclosing set function : adjust the value of
            max_aggr_level for the function if it's needed.
          */
          if (thd->lex->in_sum_func &&
              thd->lex->in_sum_func->base_select->nest_level >=
                  select->nest_level) {
            Item::Type ref_type = (*reference)->type();
            thd->lex->in_sum_func->max_aggr_level =
                max(thd->lex->in_sum_func->max_aggr_level,
                    int8(select->nest_level));
            set_field(*from_field);
            fixed = true;
            mark_as_dependent(thd, last_checked_context->select_lex,
                              context->select_lex, this,
                              ((ref_type == REF_ITEM || ref_type == FIELD_ITEM)
                                   ? (Item_ident *)(*reference)
                                   : nullptr));
            return 0;
          }
        } else {
          Item::Type ref_type = (*reference)->type();
          Used_tables ut(select);
          (void)(*reference)
              ->walk(&Item::used_tables_for_level, enum_walk::SUBQUERY_POSTFIX,
                     pointer_cast<uchar *>(&ut));
          cur_unit->accumulate_used_tables(ut.used_tables);

          if (select->group_list.elements && place == CTX_HAVING) {
            /*
              If an outer field is resolved in a grouping query block then it
              is replaced with an Item_outer_ref object. Otherwise an
              Item_field object is used.
            */
            Item_outer_ref *const rf = new Item_outer_ref(
                context, down_cast<Item_ident *>(*reference), select);
            if (rf == nullptr) return -1;
            rf->in_sum_func = thd->lex->in_sum_func;
            thd->change_item_tree(reference, rf);
            if (rf->fix_fields(thd, nullptr)) return -1;
          }

          if (thd->lex->in_sum_func &&
              thd->lex->in_sum_func->base_select->nest_level >=
                  select->nest_level)
            thd->lex->in_sum_func->max_aggr_level =
                max(thd->lex->in_sum_func->max_aggr_level,
                    int8(select->nest_level));

          if ((*reference)->used_tables() != 0)
            mark_as_dependent(thd, last_checked_context->select_lex,
                              context->select_lex, this,
                              ref_type == REF_ITEM || ref_type == FIELD_ITEM
                                  ? down_cast<Item_ident *>(*reference)
                                  : NULL);
          /*
            A reference to a view field had been found and we
            substituted it instead of this Item (find_field_in_tables
            does it by assigning the new value to *reference), so now
            we can return from this function.
          */
          return 0;
        }
      }
      break;
    }

    /* Search in SELECT and GROUP lists of the outer select. */
    if (select_alias_referencable(place) &&
        outer_context->resolve_in_select_list) {
      if (!(ref = resolve_ref_in_select_and_group(thd, this, select)))
        return -1; /* Some error occurred (e.g. ambiguous names). */
      if (ref != not_found_item) {
        // The item which we found is already fixed
        DBUG_ASSERT((*ref)->fixed);
        cur_unit->accumulate_used_tables((*ref)->used_tables());
        break;
      }
    }

    /*
      Reference is not found in this select => this subquery depend on
      outer select (or we just trying to find wrong identifier, in this
      case it does not matter which used tables bits we set)
    */
    DBUG_PRINT("outer_field",
               ("out of loop, reached end of big block, continue"));
    cur_unit->accumulate_used_tables(OUTER_REF_TABLE_BIT);
  loop:;
  }

  DBUG_ASSERT(ref != nullptr);
  if (!*from_field) return -1;
  if (ref == not_found_item && *from_field == not_found_field) {
    if (upward_lookup) {
      // We can't say exactly what absent table or field
      my_error(ER_BAD_FIELD_ERROR, MYF(0), full_name(), thd->where);
    } else {
      /* Call find_field_in_tables only to report the error */
      find_field_in_tables(thd, this, context->first_name_resolution_table,
                           context->last_name_resolution_table, reference,
                           REPORT_ALL_ERRORS,
                           any_privileges ? 0 : thd->want_privilege, true);
    }
    return -1;
  } else if (ref != not_found_item) {
    Item *save;
    Item_ref *rf;

    /* Should have been checked in resolve_ref_in_select_and_group(). */
    DBUG_ASSERT((*ref)->fixed);
    /*
      Here, a subset of actions performed by Item_ref::set_properties
      is not enough. So we pass ptr to NULL into Item_ref
      constructor, so no initialization is performed, and call
      fix_fields() below.
    */
    save = *ref;
    *ref = nullptr;  // Don't call set_properties()
    bool use_plain_ref = place == CTX_HAVING || !select->group_list.elements;
    rf = use_plain_ref
             ? new Item_ref(context, ref, table_name, field_name,
                            m_alias_of_expr)
             : new Item_outer_ref(context, ref, table_name, field_name,
                                  m_alias_of_expr, select);
    *ref = save;
    if (!rf) return -1;

    if (!use_plain_ref)
      ((Item_outer_ref *)rf)->in_sum_func = thd->lex->in_sum_func;

    thd->change_item_tree(reference, rf);
    /*
      rf is Item_ref => never substitute other items (in this case)
      during fix_fields() => we can use rf after fix_fields()
    */
    DBUG_ASSERT(!rf->fixed);  // Assured by Item_ref()
    if (rf->fix_fields(thd, reference) || rf->check_cols(1)) return -1;
    if (rf->used_tables() != 0)
      mark_as_dependent(thd, last_checked_context->select_lex,
                        context->select_lex, this, rf);
    return 0;
  } else {
    mark_as_dependent(thd, last_checked_context->select_lex,
                      context->select_lex, this, (Item_ident *)*reference);
    if (last_checked_context->select_lex->having_fix_field) {
      Item_ref *rf;
      rf = new Item_ref(context,
                        (cached_table->db[0] ? cached_table->db : nullptr),
                        cached_table->alias, field_name);
      if (!rf) return -1;
      thd->change_item_tree(reference, rf);
      /*
        rf is Item_ref => never substitute other items (in this case)
        during fix_fields() => we can use rf after fix_fields()
      */
      DBUG_ASSERT(!rf->fixed);  // Assured by Item_ref()
      if (rf->fix_fields(thd, reference) || rf->check_cols(1)) return -1;
      return 0;
    }
  }
  return 1;
}

/**
  Check if the column reference that is currently being resolved, will be set
  to NULL if its qualifying query returns zero rows.

  This is true for non-aggregated column references in the SELECT list,
  if the query block uses aggregation without grouping. For example:

      SELECT COUNT(*), col FROM t WHERE some_condition

  Here, if the table `t` is empty, or `some_condition` doesn't match any rows
  in `t`, the query returns one row where `col` is NULL, even if `col` is a
  not-nullable column.

  Such column references are rejected if the ONLY_FULL_GROUP_BY SQL mode is
  enabled, in a later resolution phase.
*/
bool is_null_on_empty_table(THD *thd, Item_field *i) {
  /*
    Nullability of a column item 'i' is normally determined from table's or
    view's definition. Additionally, an item may be nullable because its table
    is on the right side of a left join; but this has been handled by
    propagate_nullability() before coming here (@see TABLE::set_nullable() and
    Field::maybe_null()).
    If the table is in the left part of a left join, or is in an inner join, a
    non-nullable item may be set to NULL (table->set_null_row()) if, during
    optimization, its table is found to be empty (e.g. in read_system()) or the
    FROM clause of the qualifying query QQ of its table is found to return no
    rows. This makes a case where a non-nullable 'i' is set to NULL. Certain
    expressions containing the item, if evaluated, may find this abnormal
    behaviour. Fortunately, in the scenario described above, QQ's result is
    generally empty and so no expression is evaluated. Then we don't even
    optimize subquery expressions as their optimization may lead to evaluation
    of the item (e.g. in create_ref_for_key()).
    However there is one exception where QQ's result is not empty even though
    FROM clause's result is: when QQ is implicitely aggregated. In that case,
    return_zero_rows() sets all tables' columns to NULL and any expression in
    QQ's SELECT list is evaluated; to prepare for this, we mark the item 'i'
    as nullable below.
    - If item is not outer reference, we can reliably know if QQ is
    aggregated by testing QQ->with_sum_func
    - if it's outer reference, QQ->with_sum_func may not yet be set, e.g. if
    there is single set function referenced later in subquery and not yet
    resolved; but then context.select_lex->with_sum_func is surely set (it's set
    at parsing time), so we test both members.
    - in_sum_func is the innermost set function SF containing the item;
    - if item is not an outer reference, and in_sum_func is set, SF is
    necessarily aggregated in QQ, and will not be evaluated (just be replaced
    with its "clear" value 0 or NULL), so we needn't mark 'i' as nullable;
    - if item is an outer reference and in_sum_func is set, we cannot yet know
    where SF is aggregated, it depends on other arguments of SF, so make a
    pessimistic assumption.
    Finally we test resolve_place; indeed, when QQ's result is empty, we only
    evaluate:
    - SELECT list
    - or HAVING, but columns of HAVING are always also present in SELECT list
    so are Item_ref to SELECT list and get nullability from that,
    - or ORDER BY but actually no as it's optimized away in such single-row
    query.
    Note: we test with_sum_func (== references a set function);
    agg_func_used() (== is aggregation query) would be better but is not
    reliable yet at this stage.
  */
  SELECT_LEX *sl = i->context->select_lex;
  SELECT_LEX *qsl = i->depended_from;

  if (qsl != nullptr)
    return qsl->resolve_place == SELECT_LEX::RESOLVE_SELECT_LIST &&
           (sl->with_sum_func || qsl->with_sum_func) &&
           qsl->group_list.elements == 0;
  else
    return sl->resolve_place == SELECT_LEX::RESOLVE_SELECT_LIST &&
           sl->with_sum_func && sl->group_list.elements == 0 &&
           thd->lex->in_sum_func == nullptr;
}

/**
  Resolve the name of a column reference.

  The method resolves the column reference represented by 'this' as a column
  present in one of: FROM clause, SELECT clause, GROUP BY clause of a query
  Q, or in outer queries that contain Q.

  The name resolution algorithm used is (where [T_j] is an optional table
  name that qualifies the column name):

  @code
    resolve_column_reference([T_j].col_ref_i)
    {
      search for a column or derived column named col_ref_i
      [in table T_j] in the FROM clause of Q;

      if such a column is NOT found AND    // Lookup in outer queries.
         there are outer queries
      {
        for each outer query Q_k beginning from the inner-most one
        {
          search for a column or derived column named col_ref_i
          [in table T_j] in the FROM clause of Q_k;

          if such a column is not found
            Search for a column or derived column named col_ref_i
            [in table T_j] in the SELECT and GROUP clauses of Q_k.
        }
      }
    }
  @endcode

    Notice that compared to Item_ref::fix_fields, here we first search the FROM
    clause, and then we search the SELECT and GROUP BY clauses.

  @param[in]     thd        current thread
  @param[in,out] reference  view column if this item was resolved to a
    view column

  @retval
    true  if error
  @retval
    false on success
*/

bool Item_field::fix_fields(THD *thd, Item **reference) {
  DBUG_ASSERT(fixed == 0);
  Field *from_field = not_found_field;
  bool outer_fixed = false;

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);

  if (!field)  // If field is not checked
  {
    /*
      In case of view, find_field_in_tables() write pointer to view field
      expression to 'reference', i.e. it substitute that expression instead
      of this Item_field
    */
    from_field = find_field_in_tables(
        thd, this, context->first_name_resolution_table,
        context->last_name_resolution_table, reference,
        thd->lex->use_only_table_context ? REPORT_ALL_ERRORS
                                         : IGNORE_EXCEPT_NON_UNIQUE,
        any_privileges ? 0 : thd->want_privilege, true);
    if (thd->is_error()) goto error;
    if (from_field == not_found_field) {
      int ret;
      /* Look up in current select's item_list to find aliased fields */
      if (thd->lex->current_select()->is_item_list_lookup) {
        uint counter;
        enum_resolution_type resolution;
        Item **res =
            find_item_in_list(thd, this, thd->lex->current_select()->item_list,
                              &counter, REPORT_EXCEPT_NOT_FOUND, &resolution);
        if (!res) return true;
        if (resolution == RESOLVED_AGAINST_ALIAS) set_alias_of_expr();
        if (res != not_found_item) {
          if ((*res)->type() == Item::FIELD_ITEM) {
            /*
              It's an Item_field referencing another Item_field in the select
              list.
              Use the field from the Item_field in the select list and leave
              the Item_field instance in place.
            */

            Item_field *const item_field = (Item_field *)(*res);
            Field *const new_field = item_field->field;

            if (new_field == nullptr) {
              /* The column to which we link isn't valid. */
              my_error(ER_BAD_FIELD_ERROR, MYF(0), item_field->item_name.ptr(),
                       thd->where);
              return true;
            }

            set_field(new_field);

            cached_table = table_ref;

            // The found column may be an outer reference
            if (item_field->depended_from)
              mark_as_dependent(thd, item_field->depended_from,
                                context->select_lex, this, this);

            return false;
          } else {
            /*
              It's not an Item_field in the select list so we must make a new
              Item_ref to point to the Item in the select list and replace the
              Item_field created by the parser with the new Item_ref.
              Ex: SELECT func1(col) as c ... ORDER BY func2(c);
              NOTE: If we are fixing an alias reference inside ORDER/GROUP BY
              item tree, then we use new Item_ref as an
              intermediate value to resolve referenced item only.
              In this case the new Item_ref item is unused.
            */
            Item_ref *rf = new Item_ref(context, res, table_name, field_name,
                                        resolution == RESOLVED_AGAINST_ALIAS);
            if (!rf) return true;

            const bool group_fix_field =
                thd->lex->current_select()->group_fix_field;
            if (!rf->fixed) {
              // No need for recursive resolving of aliases.
              thd->lex->current_select()->group_fix_field = false;
              bool fix_error =
                  rf->fix_fields(thd, (Item **)&rf) || rf->check_cols(1);
              thd->lex->current_select()->group_fix_field = group_fix_field;
              if (fix_error) return true;
            }
            if (group_fix_field && m_alias_of_expr)
              thd->change_item_tree(reference, *rf->ref);
            else
              thd->change_item_tree(reference, rf);

            return false;
          }
        }
      }
      if ((ret = fix_outer_field(thd, &from_field, reference)) < 0) goto error;
      outer_fixed = true;
      if (!ret) return false;
    } else if (!from_field)
      goto error;

    /*
      We should resolve this as an outer field reference if
      1. we haven't done it before, and
      2. the select_lex of the table that contains this field is
         different from the select_lex of the current name resolution
         context.
     */
    if (!outer_fixed &&                                                     // 1
        cached_table && cached_table->select_lex && context->select_lex &&  // 2
        cached_table->select_lex != context->select_lex) {
      int ret;
      if ((ret = fix_outer_field(thd, &from_field, reference)) < 0) goto error;
      outer_fixed = true;
      if (!ret) return false;
    }

    /*
      If inside an aggregation function, set the correct aggregation level.
      Even if a view reference is found, the level is still the query block
      associated with the context of the current item:
    */
    DBUG_ASSERT(
        from_field != view_ref_found ||
        context->select_lex ==
            dynamic_cast<Item_ident *>(*reference)->context->select_lex);
    if (thd->lex->in_sum_func &&
        thd->lex->in_sum_func->base_select->nest_level ==
            context->select_lex->nest_level)
      thd->lex->in_sum_func->max_aggr_level =
          max(thd->lex->in_sum_func->max_aggr_level,
              int8(context->select_lex->nest_level));

    // If view column reference, Item in *reference is completely resolved:
    if (from_field == view_ref_found) {
      if (is_null_on_empty_table(thd, this)) {
        (*reference)->maybe_null = true;
        if ((*reference)->real_item()->type() == Item::FIELD_ITEM) {
          // See below for explanation.
          TABLE *table =
              down_cast<Item_field *>((*reference)->real_item())->field->table;
          table->set_nullable();
        }
      }
      return false;
    }

    if (from_field->is_hidden_from_user()) {
      // This field is hidden from users, so report it as "not found".
      my_error(ER_BAD_FIELD_ERROR, MYF(0), from_field->field_name, thd->where);
      return true;
    }

    // Not view reference, not outer reference; need to set properties:
    set_field(from_field);
  } else if (thd->mark_used_columns != MARK_COLUMNS_NONE) {
    TABLE *table = field->table;
    MY_BITMAP *current_bitmap;
    MY_BITMAP *other_bitmap MY_ATTRIBUTE((unused));
    if (thd->mark_used_columns == MARK_COLUMNS_READ) {
      current_bitmap = table->read_set;
      other_bitmap = table->write_set;
    } else {
      current_bitmap = table->write_set;
      other_bitmap = table->read_set;
    }
    if (!bitmap_test_and_set(current_bitmap, field->field_index))
      DBUG_ASSERT(bitmap_is_set(other_bitmap, field->field_index));
  }
  if (any_privileges) {
    const char *db, *tab;
    db = cached_table->get_db_name();
    tab = cached_table->get_table_name();
    DBUG_ASSERT(field->table == table_ref->table);
    if (!(have_privileges =
              (get_column_grant(thd, &table_ref->grant, db, tab, field_name) &
               VIEW_ANY_ACL))) {
      my_error(ER_COLUMNACCESS_DENIED_ERROR, MYF(0), "ANY",
               thd->security_context()->priv_user().str,
               thd->security_context()->host_or_ip().str, field_name, tab);
      goto error;
    }
  }
  fixed = true;
  if (is_null_on_empty_table(thd, this)) {
    maybe_null = true;

    // The Item is now nullable, but the underlying field still isn't,
    // and Copy_field uses the underlying field. Thus,
    // ZeroRowsAggregatedIterator sets the _table_ row to NULL instead, and
    // thus, it needs to be nullable. This is similar to how inner tables of
    // outer joins need to be nullable.
    field->table->set_nullable();
  }
  return false;

error:
  return true;
}

Item *Item_field::safe_charset_converter(THD *thd, const CHARSET_INFO *tocs) {
  no_const_subst = true;
  return Item::safe_charset_converter(thd, tocs);
}

void Item_field::cleanup() {
  DBUG_TRACE;
  Item_ident::cleanup();
  /*
    Even if this object was created by direct link to field in setup_wild()
    it will be linked correctly next time by name of field and table alias.
    I.e. we can drop 'field'.
   */
  table_ref = nullptr;
  field = nullptr;
  result_field = nullptr;
  item_equal = nullptr;
  item_equal_all_join_nests = nullptr;
  null_value = false;
}

/**
  Find a field among specified multiple equalities.

  The function first searches the field among multiple equalities
  of the current level (in the cond_equal->current_level list).
  If it fails, it continues searching in upper levels accessed
  through a pointer cond_equal->upper_levels.
  The search terminates as soon as a multiple equality containing
  the field is found.

  @param cond_equal   reference to list of multiple equalities where
                      the field (this object) is to be looked for

  @return
    - First Item_equal containing the field, if success
    - nullptr, otherwise
*/

Item_equal *Item_field::find_item_equal(COND_EQUAL *cond_equal) const {
  while (cond_equal) {
    for (Item_equal &item : cond_equal->current_level) {
      if (item.contains(field)) return &item;
    }
    /*
      The field is not found in any of the multiple equalities
      of the current level. Look for it in upper levels
    */
    cond_equal = cond_equal->upper_levels;
  }
  return nullptr;
}

/**
  Check whether a field can be substituted by an equal item.

  The function checks whether a substitution of the field
  occurrence for an equal item is valid.

  @param arg   *arg != NULL <-> the field is in the context where
               substitution for an equal item is valid

  @note
    The following statement is not always true:
  @n
    x=y => F(x)=F(x/y).
  @n
    This means substitution of an item for an equal item not always
    yields an equavalent condition. Here's an example:
    @code
    'a'='a '
    (LENGTH('a')=1) != (LENGTH('a ')=2)
  @endcode
    Such a substitution is surely valid if either the substituted
    field is not of a STRING type or if it is an argument of
    a comparison predicate.

  @retval
    true   substitution is valid
  @retval
    false  otherwise
*/

bool Item_field::subst_argument_checker(uchar **arg) {
  return (result_type() != STRING_RESULT) || (*arg);
}

/**
  Convert a numeric value to a zero-filled string

  @param[in,out]  item   the item to operate on
  @param          field  The field that this value is equated to

  This function converts a numeric value to a string. In this conversion
  the zero-fill flag of the field is taken into account.
  This is required so the resulting string value can be used instead of
  the field reference when propagating equalities.
*/

static void convert_zerofill_number_to_string(Item **item,
                                              const Field_num *field) {
  char buff[MAX_FIELD_WIDTH], *pos;
  String tmp(buff, sizeof(buff), field->charset()), *res;

  res = (*item)->val_str(&tmp);
  if ((*item)->null_value)
    *item = new Item_null();
  else {
    field->prepend_zeros(res);
    pos = sql_strmake(res->ptr(), res->length());
    *item = new Item_string(pos, res->length(), field->charset());
  }
}

/**
  Set a pointer to the multiple equality the field reference belongs to
  (if any).

  The function looks for a multiple equality containing the field item
  among those referenced by arg.
  In the case such equality exists the function does the following.
  If the found multiple equality contains a constant, then the field
  reference is substituted for this constant, otherwise it sets a pointer
  to the multiple equality in the field item.


  @param arg    reference to list of multiple equalities where
                the field (this object) is to be looked for

  @note
    This function is supposed to be called as a callback parameter in calls
    of the compile method.

  @return
    - pointer to the replacing constant item, if the field item was substituted
    - pointer to the field item, otherwise.
*/

Item *Item_field::equal_fields_propagator(uchar *arg) {
  if (no_const_subst) return this;
  item_equal = find_item_equal((COND_EQUAL *)arg);
  Item *item = nullptr;
  if (item_equal) item = item_equal->get_const();
  /*
    Disable const propagation for items used in different comparison contexts.
    This must be done because, for example, Item_hex_string->val_int() is not
    the same as (Item_hex_string->val_str() in BINARY column)->val_int().
    We cannot simply disable the replacement in a particular context (
    e.g. <bin_col> = <int_col> AND <bin_col> = <hex_string>) since
    Items don't know the context they are in and there are functions like
    IF (<hex_string>, 'yes', 'no').
  */
  if (!item || !has_compatible_context(item))
    item = this;
  else if (field && (field->flags & ZEROFILL_FLAG) && IS_NUM(field->type())) {
    /*
      We don't need to zero-fill timestamp columns here because they will be
      first converted to a string (in date/time format) and compared as such if
      compared with another string.
    */
    if (item && field->type() != FIELD_TYPE_TIMESTAMP &&
        cmp_context != INT_RESULT)
      convert_zerofill_number_to_string(&item, (Field_num *)field);
    else
      item = this;
  }
  return item;
}

/**
  If this field is the target is the target of replacement, replace it with
  the info object's item or, if the item is found inside a subquery, the target
  is an outer reference, so we create a new Item_field, mark it accordingly
  and replace with that instead.

  @param arg  An info object of type Item::Item_field_replacement.
  @returns the resulting item, replaced or not, or nullptr if error
*/
Item *Item_field::replace_item_field(uchar *arg) {
  auto *info = pointer_cast<Item::Item_field_replacement *>(arg);

  if (field == info->m_target) {
    if (info->m_curr_block == info->m_trans_block) return info->m_item;

    // The field is an outer reference, so we cannot reuse transformed query
    // block's Item_field; make a new one for this query block
    THD *const thd = current_thd;
    Item_field *outer_field = new (thd->mem_root) Item_field(thd, info->m_item);
    if (outer_field == nullptr) return nullptr; /* purecov: inspected */
    outer_field->depended_from = info->m_trans_block;
    outer_field->context = &info->m_curr_block->context;
    return outer_field;
  }

  return this;
}

/**
  Update the name resolution context for the field if its existing
  context stems from Context_info::old_block to that of
  Context_info::new_block. Also update any the context of original fields
  the current one may have replaced.

  @param  arg   pointer to an object of type Item_field::Context_info
  @returns  false (no error possible)
*/
bool Item_field::update_context(uchar *arg) {
  auto *info = pointer_cast<Item_field::Context_info *>(arg);
  if (context->select_lex == info->old_block) {
    context = &info->new_block->context;
  }
  return false;
}

/**
  Mark the item to not be part of substitution if it's not a binary item.

  See comments in Arg_comparator::set_compare_func() for details.
*/
bool Item_field::set_no_const_sub(uchar *) {
  if (field->charset() != &my_charset_bin) no_const_subst = true;
  return false;
}

/**
  Replace an Item_field for an equal Item_field that evaluated earlier
  (if any).

  The function returns a pointer to an item that is taken from
  the very beginning of the item_equal list which the Item_field
  object refers to (belongs to) unless item_equal contains  a constant
  item. In this case the function returns this constant item,
  (if the substitution does not require conversion).
  If the Item_field object does not refer any Item_equal object
  'this' is returned .

  @note
    This function is supposed to be called as a callback parameter in calls
    of the thransformer method.

  @return
    - pointer to a replacement Item_field if there is a better equal item or
      a pointer to a constant equal item;
    - this - otherwise.
*/

Item *Item_field::replace_equal_field(uchar *) {
  if (item_equal) {
    Item *const_item = item_equal->get_const();
    if (const_item) {
      if (!has_compatible_context(const_item)) return this;
      return const_item;
    }
    Item_field *subst = item_equal->get_subst_item(this);
    DBUG_ASSERT(subst);
    DBUG_ASSERT(table_ref == subst->table_ref ||
                table_ref->table != subst->table_ref->table);
    if (table_ref != subst->table_ref && !field->eq(subst->field)) {
      // We may have to undo the substitution that is done here when setting up
      // hash join; the new field may be a field from a table that is not
      // reachable from hash join. Store which multi-equality we found the field
      // substitution in, so that we can go back and find a field that the hash
      // join can reach.
      subst->set_item_equal_all_join_nests(item_equal);
      return subst;
    }
  }
  return this;
}

void Item::init_make_field(Send_field *tmp_field,
                           enum enum_field_types field_type_arg) {
  const char *empty_name = "";
  tmp_field->db_name = empty_name;
  tmp_field->org_table_name = empty_name;
  tmp_field->org_col_name = empty_name;
  tmp_field->table_name = empty_name;
  tmp_field->col_name = item_name.ptr();
  tmp_field->charsetnr = collation.collation->number;
  tmp_field->flags =
      (maybe_null ? 0 : NOT_NULL_FLAG) |
      (my_binary_compare(charset_for_protocol()) ? BINARY_FLAG : 0);
  tmp_field->type = field_type_arg;
  tmp_field->length = max_length;
  tmp_field->decimals = decimals;
  if (unsigned_flag) tmp_field->flags |= UNSIGNED_FLAG;
  tmp_field->field = false;
}

void Item::make_field(Send_field *tmp_field) {
  init_make_field(tmp_field, data_type());
}

void Item_empty_string::make_field(Send_field *tmp_field) {
  init_make_field(tmp_field, string_field_type(max_length));
}

/**
  Verifies that the input string is well-formed according to its character set.
  @param str
  @param send_error   If true, call my_error if string is not well-formed.
  @param truncate     If true, set to null/truncate if not well-formed.

  @return
  If well-formed: input string.
  If not well-formed:
    if truncate is true and strict mode:     NULL pointer and we set this
                                             Item's value to NULL.
    if truncate is true and not strict mode: input string truncated up to
                                             last good character.
    if truncate is false:                    input string is returned.
 */
String *Item::check_well_formed_result(String *str, bool send_error,
                                       bool truncate) {
  /* Check whether we got a well-formed string */
  const CHARSET_INFO *cs = str->charset();

  size_t valid_length;
  bool length_error;

  if (validate_string(cs, str->ptr(), str->length(), &valid_length,
                      &length_error)) {
    const char *str_end = str->ptr() + str->length();
    const char *print_byte = str->ptr() + valid_length;
    THD *thd = current_thd;
    char hexbuf[7];
    size_t diff = min(size_t(str_end - print_byte), size_t(3));
    octet2hex(hexbuf, print_byte, diff);
    if (send_error && length_error) {
      my_error(ER_INVALID_CHARACTER_STRING, MYF(0), cs->csname, hexbuf);
      return nullptr;
    }
    if (truncate && length_error) {
      if (thd->is_strict_sql_mode()) {
        null_value = true;
        str = nullptr;
      } else {
        thd->really_audit_instrumented_event =
            thd->variables.audit_instrumented_event;
        str->length(valid_length);
      }
    }
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_INVALID_CHARACTER_STRING,
        ER_THD(thd, ER_INVALID_CHARACTER_STRING), cs->csname, hexbuf);
    thd->really_audit_instrumented_event = 0;
  }
  return str;
}

/*
  Compare two items using a given collation

  SYNOPSIS
    eq_by_collation()
    item               item to compare with
    binary_cmp         true <-> compare as binaries
    cs                 collation to use when comparing strings

  DESCRIPTION
    This method works exactly as Item::eq if the collation cs coincides with
    the collation of the compared objects. Otherwise, first the collations that
    differ from cs are replaced for cs and then the items are compared by
    Item::eq. After the comparison the original collations of items are
    restored.

  RETURN
    1    compared items has been detected as equal
    0    otherwise
*/

bool Item::eq_by_collation(Item *item, bool binary_cmp,
                           const CHARSET_INFO *cs) {
  const CHARSET_INFO *save_cs = nullptr;
  const CHARSET_INFO *save_item_cs = nullptr;
  if (collation.collation != cs) {
    save_cs = collation.collation;
    collation.collation = cs;
  }
  if (item->collation.collation != cs) {
    save_item_cs = item->collation.collation;
    item->collation.collation = cs;
  }
  bool res = eq(item, binary_cmp);
  if (save_cs) collation.collation = save_cs;
  if (save_item_cs) item->collation.collation = save_item_cs;
  return res;
}

/**
  Create a field to hold a string value from an item.

  If max_length > CONVERT_IF_BIGGER_TO_BLOB create a blob @n
  If max_length > 0 create a varchar @n
  If max_length == 0 create a CHAR(0)

  @param table		Table for which the field is created
*/

Field *Item::make_string_field(TABLE *table) const {
  Field *field;
  DBUG_ASSERT(collation.collation);
  if (data_type() == MYSQL_TYPE_JSON)
    field =
        new (*THR_MALLOC) Field_json(max_length, maybe_null, item_name.ptr());
  else if (max_length / collation.collation->mbmaxlen >
           CONVERT_IF_BIGGER_TO_BLOB)
    field = new (*THR_MALLOC) Field_blob(
        max_length, maybe_null, item_name.ptr(), collation.collation, true);
  /* Item_type_holder holds the exact type, do not change it */
  else if (max_length > 0 &&
           (type() != Item::TYPE_HOLDER || data_type() != MYSQL_TYPE_STRING))
    field = new (*THR_MALLOC) Field_varstring(
        max_length, maybe_null, item_name.ptr(), table->s, collation.collation);
  else
    field = new (*THR_MALLOC) Field_string(
        max_length, maybe_null, item_name.ptr(), collation.collation);
  if (field) field->init(table);
  return field;
}

/**
  Create a field based on field_type of argument.

  For now, this is only used to create a field for
  IFNULL(x,something) and time functions

  @return Created field
  @retval NULL  error
*/

Field *Item::tmp_table_field_from_field_type(TABLE *table,
                                             bool fixed_length) const {
  /*
    The field functions defines a field to be not null if null_ptr is not 0
  */
  Field *field;

  switch (data_type()) {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      field = Field_new_decimal::create_from_item(this);
      break;
    case MYSQL_TYPE_TINY:
      field = new (*THR_MALLOC)
          Field_tiny(max_length, maybe_null, item_name.ptr(), unsigned_flag);
      break;
    case MYSQL_TYPE_SHORT:
      field = new (*THR_MALLOC)
          Field_short(max_length, maybe_null, item_name.ptr(), unsigned_flag);
      break;
    case MYSQL_TYPE_LONG:
      field = new (*THR_MALLOC)
          Field_long(max_length, maybe_null, item_name.ptr(), unsigned_flag);
      break;
    case MYSQL_TYPE_LONGLONG:
      field = new (*THR_MALLOC) Field_longlong(max_length, maybe_null,
                                               item_name.ptr(), unsigned_flag);
      break;
    case MYSQL_TYPE_FLOAT:
      field = new (*THR_MALLOC) Field_float(
          max_length, maybe_null, item_name.ptr(), decimals, unsigned_flag);
      break;
    case MYSQL_TYPE_DOUBLE:
      field = new (*THR_MALLOC) Field_double(
          max_length, maybe_null, item_name.ptr(), decimals, unsigned_flag);
      break;
    case MYSQL_TYPE_INT24:
      field = new (*THR_MALLOC)
          Field_medium(max_length, maybe_null, item_name.ptr(), unsigned_flag);
      break;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_NEWDATE:
      field = new (*THR_MALLOC) Field_newdate(maybe_null, item_name.ptr());
      break;
    case MYSQL_TYPE_TIME:
      field =
          new (*THR_MALLOC) Field_timef(maybe_null, item_name.ptr(), decimals);
      break;
    case MYSQL_TYPE_TIMESTAMP:
      field = new (*THR_MALLOC)
          Field_timestampf(maybe_null, item_name.ptr(), decimals);
      break;
    case MYSQL_TYPE_DATETIME:
      field = new (*THR_MALLOC)
          Field_datetimef(maybe_null, item_name.ptr(), decimals);
      break;
    case MYSQL_TYPE_YEAR:
      DBUG_ASSERT(max_length == 4);  // Field_year is only for length 4.
      field = new (*THR_MALLOC) Field_year(maybe_null, item_name.ptr());
      break;
    case MYSQL_TYPE_BIT:
      field = new (*THR_MALLOC)
          Field_bit_as_char(max_length, maybe_null, item_name.ptr());
      break;
    default:
      /* This case should never be chosen */
      DBUG_ASSERT(0);
      /* If something goes awfully wrong, it's better to get a string than die
       */
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_NULL:
      if (fixed_length && max_length <= CONVERT_IF_BIGGER_TO_BLOB) {
        field = new (*THR_MALLOC) Field_string(
            max_length, maybe_null, item_name.ptr(), collation.collation);
        break;
      }
      /* Fall through to make_string_field() */
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
      return make_string_field(table);
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
      if (this->type() == Item::TYPE_HOLDER)
        field = new (*THR_MALLOC) Field_blob(
            max_length, maybe_null, item_name.ptr(), collation.collation, true);
      else
        field = new (*THR_MALLOC)
            Field_blob(max_length, maybe_null, item_name.ptr(),
                       collation.collation, false);
      break;  // Blob handled outside of case
    case MYSQL_TYPE_GEOMETRY:
      field = new (*THR_MALLOC) Field_geom(
          max_length, maybe_null, item_name.ptr(), get_geometry_type(), {});
      break;
    case MYSQL_TYPE_JSON:
      field =
          new (*THR_MALLOC) Field_json(max_length, maybe_null, item_name.ptr());
  }
  if (field) field->init(table);
  return field;
}

/* ARGSUSED */
void Item_field::make_field(Send_field *tmp_field) {
  field->make_send_field(tmp_field);
  DBUG_ASSERT(tmp_field->table_name != nullptr);
  if (item_name.is_set())
    tmp_field->col_name = item_name.ptr();  // Use user supplied name
  if (table_name) tmp_field->table_name = table_name;
  if (db_name) tmp_field->db_name = db_name;
  tmp_field->field = true;
}

/**
  Set a field's value from a item.
*/

void Item_field::save_org_in_field(Field *to) {
  if (field->is_null()) {
    null_value = true;
    set_field_to_null_with_conversions(to, true);
  } else {
    to->set_notnull();
    field_conv(to, field);
    null_value = false;
  }
}

type_conversion_status Item_field::save_in_field_inner(Field *to,
                                                       bool no_conversions) {
  type_conversion_status res;
  DBUG_TRACE;
  if (field->is_null()) {
    null_value = true;
    const type_conversion_status status =
        set_field_to_null_with_conversions(to, no_conversions);
    return status;
  }
  to->set_notnull();

  /*
    If we're setting the same field as the one we're reading from there's
    nothing to do. This can happen in 'SET x = x' type of scenarios.
  */
  if (to == field) {
    null_value = false;
    return TYPE_OK;
  }

  res = field_conv(to, field);
  null_value = false;
  return res;
}

/**
  Store null in field.

  This is used on INSERT.
  Allow NULL to be inserted in timestamp and auto_increment values.

  @param field		Field where we want to store NULL
  @param no_conversions  Set to 1 if we should return 1 if field can't
                         take null values.
                         If set to 0 we will do store the 'default value'
                         if the field is a special field. If not we will
                         give an error.

  @retval
    0   ok
  @retval
    1   Field doesn't support NULL values and can't handle 'field = NULL'
*/

type_conversion_status Item_null::save_in_field_inner(Field *field,
                                                      bool no_conversions) {
  return set_field_to_null_with_conversions(field, no_conversions);
}

type_conversion_status Item::save_in_field(Field *field, bool no_conversions) {
  DBUG_TRACE;
  // In case this is a hidden column used for a functional index, insert
  // an error handler that catches any errors that tries to print out the
  // name of the hidden column. It will instead print out the functional
  // index name.
  Functional_index_error_handler functional_index_error_handler(
      field, (field->table ? field->table->in_use : current_thd));

  const type_conversion_status ret = save_in_field_inner(field, no_conversions);

  /*
    If an error was raised during evaluation of the item,
    save_in_field_inner() might not notice and return TYPE_OK. Make
    sure that we return not OK if there was an error.
  */
  if (ret == TYPE_OK && field->table && field->table->in_use->is_error())
    return TYPE_ERR_BAD_VALUE;

  return ret;
}

/*
  This implementation can lose str_value content, so if the
  Item uses str_value to store something, it should
  reimplement its ::save_in_field_inner() as Item_string, for example, does.

  Note: all Item_XXX::val_str(str) methods must NOT rely on the fact that
  str != str_value. For example, see fix for bug #44743.
*/

type_conversion_status Item::save_in_field_inner(Field *field,
                                                 bool no_conversions) {
  // Storing of arrays should be handled by specialized subclasses.
  DBUG_ASSERT(!returns_array());

  if (result_type() == STRING_RESULT) {
    const enum Type typ = type();

    if (typ == FUNC_ITEM || typ == SUBSELECT_ITEM) {
      enum_field_types ft = data_type();
      // Avoid JSON dom/binary serialization to/from string
      if (ft == MYSQL_TYPE_JSON) {
        if (field->type() == MYSQL_TYPE_JSON) {
          // Store the value in the JSON binary format.
          Field_json *f = down_cast<Field_json *>(field);
          Json_wrapper wr;
          if (val_json(&wr)) return TYPE_ERR_BAD_VALUE;

          if (null_value) return set_field_to_null(field);

          field->set_notnull();
          return f->store_json(&wr);
        }

        const enum_field_types field_type = field->type();
        if (is_temporal_type(field_type)) {
          MYSQL_TIME t;
          bool res = true;
          switch (field_type) {
            case MYSQL_TYPE_TIME:
              res = get_time(&t);
              break;
            case MYSQL_TYPE_DATETIME:
            case MYSQL_TYPE_TIMESTAMP:
            case MYSQL_TYPE_DATE:
            case MYSQL_TYPE_NEWDATE:
              res = get_date(&t, 0);
              break;
            default:
              DBUG_ASSERT(0);
          }
          if (res) {
            null_value = true;
            return set_field_to_null_with_conversions(field, no_conversions);
          }
          field->set_notnull();
          return field->store_time(&t);
        }
        if (field->type() == MYSQL_TYPE_NEWDECIMAL) {
          my_decimal decimal_value;
          my_decimal *value = val_decimal(&decimal_value);
          if (null_value)
            return set_field_to_null_with_conversions(field, no_conversions);
          field->set_notnull();
          return field->store_decimal(value);
        }
        if (field->type() == MYSQL_TYPE_INT24 ||
            field->type() == MYSQL_TYPE_TINY ||
            field->type() == MYSQL_TYPE_SHORT ||
            field->type() == MYSQL_TYPE_LONG ||
            field->type() == MYSQL_TYPE_LONGLONG) {
          longlong nr = val_int();
          if (null_value)
            return set_field_to_null_with_conversions(field, no_conversions);
          field->set_notnull();
          return field->store(nr, unsigned_flag);
        }
        if (field->type() == MYSQL_TYPE_FLOAT ||
            field->type() == MYSQL_TYPE_DOUBLE) {
          double nr = val_real();
          if (null_value)
            return set_field_to_null_with_conversions(field, no_conversions);
          field->set_notnull();
          return field->store(nr);
        }
      }
    }

    String *result;
    const CHARSET_INFO *cs = collation.collation;
    char buff[MAX_FIELD_WIDTH];  // Alloc buffer for small columns
    str_value.set_quick(buff, sizeof(buff), cs);
    result = val_str(&str_value);
    if (null_value) {
      str_value.set_quick(nullptr, 0, cs);
      return set_field_to_null_with_conversions(field, no_conversions);
    }

    /* NOTE: If null_value == false, "result" must be not NULL.  */

    field->set_notnull();
    type_conversion_status error =
        field->store(result->ptr(), result->length(),
                     field->type() == MYSQL_TYPE_JSON ? result->charset() : cs);
    str_value.set_quick(nullptr, 0, cs);
    return error;
  }

  if (result_type() == REAL_RESULT && field->result_type() == STRING_RESULT) {
    double nr = val_real();
    if (null_value)
      return set_field_to_null_with_conversions(field, no_conversions);
    field->set_notnull();
    return field->store(nr);
  }

  if (result_type() == REAL_RESULT) {
    double nr = val_real();
    if (null_value)
      return set_field_to_null_with_conversions(field, no_conversions);
    field->set_notnull();
    return field->store(nr);
  }

  if (result_type() == DECIMAL_RESULT) {
    my_decimal decimal_value;
    my_decimal *value = val_decimal(&decimal_value);
    if (null_value)
      return set_field_to_null_with_conversions(field, no_conversions);
    field->set_notnull();
    return field->store_decimal(value);
  }

  longlong nr = val_int();
  if (null_value)
    return set_field_to_null_with_conversions(field, no_conversions);
  field->set_notnull();
  return field->store(nr, unsigned_flag);
}

type_conversion_status Item_string::save_in_field_inner(Field *field, bool) {
  String *result;
  result = val_str(&str_value);
  return save_str_value_in_field(field, result);
}

type_conversion_status Item_uint::save_in_field_inner(Field *field,
                                                      bool no_conversions) {
  /* Item_int::save_in_field_inner handles both signed and unsigned. */
  return Item_int::save_in_field_inner(field, no_conversions);
}

/**
  Store an int in a field

  @param field           The field where the int value is to be stored
  @param nr              The value to store in field
  @param null_value      True if the value to store is NULL, false otherwise
  @param unsigned_flag   Whether or not the int value is signed or unsigned

  @retval TYPE_OK   Storing of value went fine without warnings or errors
  @retval !TYPE_OK  Warning/error as indicated by type_conversion_status enum
                    value
*/
static type_conversion_status save_int_value_in_field(Field *field, longlong nr,
                                                      bool null_value,
                                                      bool unsigned_flag) {
  // TODO: call set_field_to_null_with_conversions below
  if (null_value) return set_field_to_null(field);
  field->set_notnull();
  return field->store(nr, unsigned_flag);
}

/**
  Store this item's int-value in a field

  @param field           The field where the int value is to be stored
  @param no_conversions  Only applies if the value to store is NULL
                         (null_value is true) and NULL is not allowed
                         in field. In that case: if no_coversion is
                         true, do nothing and return with error
                         TYPE_ERR_NULL_CONSTRAINT_VIOLATION. If
                         no_coversion is false, the field's default
                         value is stored if one exists. Otherwise an
                         error is returned.

  @retval TYPE_OK   Storing of value went fine without warnings or errors
  @retval !TYPE_OK  Warning/error as indicated by type_conversion_status enum
                    value
*/
type_conversion_status Item_int::save_in_field_inner(Field *field, bool) {
  return save_int_value_in_field(field, val_int(), null_value, unsigned_flag);
}

type_conversion_status Item_temporal::save_in_field_inner(Field *field, bool) {
  const enum_field_types field_type = field->type();
  longlong nr = is_temporal_type_with_time(field_type)
                    ? val_temporal_with_round(field_type, field->decimals())
                    : val_date_temporal();
  // TODO: call set_field_to_null_with_conversions below
  if (null_value) return set_field_to_null(field);
  field->set_notnull();
  return field->store_packed(nr);
}

type_conversion_status Item_decimal::save_in_field_inner(Field *field, bool) {
  if (null_value) return set_field_to_null(field);

  field->set_notnull();
  return field->store_decimal(&decimal_value);
}

bool Item_int::eq(const Item *arg, bool) const {
  // No need to check for null value as integer constant can't be NULL
  if (arg->basic_const_item() && arg->type() == type()) {
    /*
      We need to cast off const to call val_int(). This should be OK for
      a basic constant.
    */
    Item *item = const_cast<Item *>(arg);
    return item->val_int() == value && item->unsigned_flag == unsigned_flag;
  }
  return false;
}

Item *Item_int_with_ref::clone_item() const {
  DBUG_ASSERT(ref->const_item());
  /*
    We need to evaluate the constant to make sure it works with
    parameter markers.
  */
  return (ref->unsigned_flag
              ? new Item_uint(ref->item_name, ref->val_int(), ref->max_length)
              : new Item_int(ref->item_name, ref->val_int(), ref->max_length));
}

Item *Item_time_with_ref::clone_item() const {
  DBUG_ASSERT(ref->const_item());
  /*
    We need to evaluate the constant to make sure it works with
    parameter markers.
  */
  return new Item_temporal(MYSQL_TYPE_TIME, ref->item_name,
                           ref->val_time_temporal(), ref->max_length);
}

Item *Item_datetime_with_ref::clone_item() const {
  DBUG_ASSERT(ref->const_item());
  /*
    We need to evaluate the constant to make sure it works with
    parameter markers.
  */
  return new Item_temporal(MYSQL_TYPE_DATETIME, ref->item_name,
                           ref->val_date_temporal(), ref->max_length);
}

void Item_temporal_with_ref::print(const THD *, String *str,
                                   enum_query_type) const {
  char buff[MAX_DATE_STRING_REP_LENGTH];
  MYSQL_TIME ltime;
  TIME_from_longlong_packed(&ltime, data_type(), value);
  str->append("'");
  my_TIME_to_str(ltime, buff, decimals);
  str->append(buff);
  str->append('\'');
}

Item_num *Item_uint::neg() {
  Item_decimal *item = new Item_decimal(value, true);
  return item->neg();
}

static uint nr_of_decimals(const char *str, const char *end) {
  const char *decimal_point;

  /* Find position for '.' */
  for (;;) {
    if (str == end) return 0;
    if (*str == 'e' || *str == 'E') return DECIMAL_NOT_SPECIFIED;
    if (*str++ == '.') break;
  }
  decimal_point = str;
  for (; str < end && my_isdigit(system_charset_info, *str); str++)
    ;
  if (str < end && (*str == 'e' || *str == 'E')) return DECIMAL_NOT_SPECIFIED;
  /*
    QQ:
    The number of decimal digist in fact should be (str - decimal_point - 1).
    But it seems the result of nr_of_decimals() is never used!

    In case of 'e' and 'E' nr_of_decimals returns DECIMAL_NOT_SPECIFIED.
    In case if there is no 'e' or 'E' parser code in sql_yacc.yy
    never calls Item_float::Item_float() - it creates Item_decimal instead.

    The only piece of code where we call Item_float::Item_float(str, len)
    without having 'e' or 'E' is item_xmlfunc.cc, but this Item_float
    never appears in metadata itself. Changing the code to return
    (str - decimal_point - 1) does not make any changes in the test results.

    This should be addressed somehow.
    Looks like a reminder from before real DECIMAL times.
  */
  return (uint)(str - decimal_point);
}

/**
  This function is only called during parsing:
  - when parsing SQL query from sql_yacc.yy
  - when parsing XPath query from item_xmlfunc.cc
  We will signal an error if value is not a true double value (overflow):
  eng: Illegal %s '%-.192s' value found during parsing

  Note: str_arg does not necessarily have to be a null terminated string,
  e.g. it is NOT when called from item_xmlfunc.cc or sql_yacc.yy.
*/

void Item_float::init(const char *str_arg, uint length) {
  int error;
  const char *end_not_used;
  value = my_strntod(&my_charset_bin, str_arg, length, &end_not_used, &error);
  if (error) {
    char tmp[NAME_LEN + 1];
    snprintf(tmp, sizeof(tmp), "%.*s", length, str_arg);
    my_error(ER_ILLEGAL_VALUE_FOR_TYPE, MYF(0), "double", tmp);
  }
  presentation.copy(str_arg, length);
  item_name.copy(str_arg, length);
  set_data_type(MYSQL_TYPE_DOUBLE);
  decimals = (uint8)nr_of_decimals(str_arg, str_arg + length);
  max_length = length;
  fixed = true;
}

type_conversion_status Item_float::save_in_field_inner(Field *field, bool) {
  double nr = val_real();
  // TODO: call set_field_to_null_with_conversions below
  if (null_value) return set_field_to_null(field);
  field->set_notnull();
  return field->store(nr);
}

void Item_float::print(const THD *, String *str,
                       enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }
  if (presentation.ptr()) {
    str->append(presentation.ptr());
    return;
  }
  char buffer[20];
  String num(buffer, sizeof(buffer), &my_charset_bin);
  num.set_real(value, decimals, &my_charset_bin);
  str->append(num);
}

/*
  hex item
  In string context this is a binary string.
  In number context this is a longlong value.
*/

bool Item_float::eq(const Item *arg, bool) const {
  if (arg->basic_const_item() && arg->type() == type()) {
    /*
      We need to cast off const to call val_int(). This should be OK for
      a basic constant.
    */
    Item *item = const_cast<Item *>(arg);
    return item->val_real() == value;
  }
  return false;
}

inline uint char_val(char X) {
  return (uint)(X >= '0' && X <= '9'
                    ? X - '0'
                    : X >= 'A' && X <= 'Z' ? X - 'A' + 10 : X - 'a' + 10);
}

Item_hex_string::Item_hex_string() { hex_string_init("", 0); }

Item_hex_string::Item_hex_string(const char *str, uint str_length) {
  hex_string_init(str, str_length);
}

Item_hex_string::Item_hex_string(const POS &pos, const LEX_STRING &literal)
    : super(pos) {
  hex_string_init(literal.str, literal.length);
}

LEX_CSTRING Item_hex_string::make_hex_str(const char *str, size_t str_length) {
  size_t max_length = (str_length + 1) / 2;
  char *ptr = (char *)(*THR_MALLOC)->Alloc(max_length + 1);
  if (ptr == nullptr) return NULL_CSTR;
  LEX_CSTRING ret = {ptr, max_length};
  char *end = ptr + max_length;
  if (max_length * 2 != str_length)
    *ptr++ = char_val(*str++);  // Not even, assume 0 prefix
  while (ptr != end) {
    *ptr++ = (char)(char_val(str[0]) * 16 + char_val(str[1]));
    str += 2;
  }
  *ptr = 0;  // needed if printed in error message
  return ret;
}

void Item_hex_string::hex_string_init(const char *str, uint str_length) {
  LEX_CSTRING s = make_hex_str(str, str_length);
  str_value.set(s.str, s.length, &my_charset_bin);
  set_data_type(MYSQL_TYPE_VARCHAR);
  max_length = s.length;
  collation.set(&my_charset_bin, DERIVATION_COERCIBLE);
  fixed = true;
  unsigned_flag = true;
}

longlong Item_hex_string::val_int() {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  const char *end = str_value.ptr() + str_value.length();
  const char *ptr;

  if (str_value.length() > sizeof(longlong)) {
    /*
      Too many bytes for longlong; lost bytes are [start, lost_end[ ; there is
      no loss of data in conversion only if they are all zeroes.
    */
    const char *lost_end = end - sizeof(longlong);
    for (ptr = str_value.ptr(); ptr < lost_end; ++ptr)
      if (*ptr != 0) {
        // Human-readable, size-limited printout of the hex:
        char errbuff[MYSQL_ERRMSG_SIZE], *errptr = errbuff;
        *errptr++ = 'x';
        *errptr++ = '\'';
        for (ptr = str_value.ptr(); ptr < end; ++ptr) {
          if (errptr > errbuff + sizeof(errbuff) - 4) break;
          *errptr++ = _dig_vec_lower[((uchar)*ptr) >> 4];
          *errptr++ = _dig_vec_lower[((uchar)*ptr) & 0x0F];
        }
        *errptr++ = '\'';
        *errptr++ = 0;
        THD *thd = current_thd;
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
            ER_THD(thd, ER_TRUNCATED_WRONG_VALUE), "BINARY", errbuff);
        return -1;
      }
  }

  ptr = end - str_value.length();
  ulonglong value = 0;
  for (; ptr != end; ptr++) value = (value << 8) + (ulonglong)(uchar)*ptr;
  return (longlong)value;
}

my_decimal *Item_hex_string::val_decimal(my_decimal *decimal_value) {
  // following assert is redundant, because fixed=1 assigned in constructor
  DBUG_ASSERT(fixed == 1);
  ulonglong value = (ulonglong)val_int();
  int2my_decimal(E_DEC_FATAL_ERROR, value, true, decimal_value);
  return (decimal_value);
}

type_conversion_status Item_hex_string::save_in_field_inner(Field *field,
                                                            bool) {
  field->set_notnull();
  if (field->result_type() == STRING_RESULT)
    return field->store(str_value.ptr(), str_value.length(),
                        collation.collation);

  ulonglong nr;
  size_t length = str_value.length();
  if (!length) {
    field->reset();
    return TYPE_WARN_OUT_OF_RANGE;
  }
  if (length > 8) {
    nr = field->flags & UNSIGNED_FLAG ? ULLONG_MAX : LLONG_MAX;
    goto warn;
  }
  nr = (ulonglong)val_int();
  if ((length == 8) && !(field->flags & UNSIGNED_FLAG) && (nr > LLONG_MAX)) {
    nr = LLONG_MAX;
    goto warn;
  }
  return field->store((longlong)nr, true);  // Assume hex numbers are unsigned

warn:
  const type_conversion_status res = field->store((longlong)nr, true);
  if (res == TYPE_OK)
    field->set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
  return res;
}

void Item_hex_string::print(const THD *, String *str,
                            enum_query_type query_type) const {
  if (query_type & QT_NORMALIZED_FORMAT) {
    str->append("?");
    return;
  }
  const uchar *ptr = pointer_cast<const uchar *>(str_value.ptr());
  const uchar *end = ptr + str_value.length();
  str->append("0x");
  for (; ptr != end; ptr++) {
    str->append(_dig_vec_lower[*ptr >> 4]);
    str->append(_dig_vec_lower[*ptr & 0x0F]);
  }
}

bool Item_hex_string::eq(const Item *item, bool binary_cmp) const {
  if (item->basic_const_item() && item->type() == type()) {
    // Should be OK for a basic constant.
    Item *arg = const_cast<Item *>(item);
    String str;
    if (binary_cmp) return !stringcmp(&str_value, arg->val_str(&str));
    return !sortcmp(&str_value, arg->val_str(&str), collation.collation);
  }
  return false;
}

Item *Item_hex_string::safe_charset_converter(THD *, const CHARSET_INFO *tocs) {
  String tmp, *str = val_str(&tmp);

  auto conv = new Item_string(str->ptr(), str->length(), tocs);
  if (conv == nullptr) return nullptr;
  conv->mark_result_as_const();
  return conv;
}

/*
  bin item.
  In string context this is a binary string.
  In number context this is a longlong value.
*/

LEX_CSTRING Item_bin_string::make_bin_str(const char *str, size_t str_length) {
  const char *end = str + str_length - 1;
  uchar bits = 0;
  uint power = 1;

  size_t max_length = (str_length + 7) >> 3;
  char *ptr = (char *)(*THR_MALLOC)->Alloc(max_length + 1);
  if (ptr == nullptr) return NULL_CSTR;

  LEX_CSTRING ret{ptr, max_length};

  if (max_length > 0) {
    ptr += max_length - 1;
    ptr[1] = 0;  // Set end null for string
    for (; end >= str; end--) {
      if (power == 256) {
        power = 1;
        *ptr-- = bits;
        bits = 0;
      }
      if (*end == '1') bits |= power;
      power <<= 1;
    }
    *ptr = (char)bits;
  } else
    ptr[0] = 0;

  return ret;
}

void Item_bin_string::bin_string_init(const char *str, size_t str_length) {
  LEX_CSTRING s = make_bin_str(str, str_length);
  max_length = s.length;
  str_value.set(s.str, s.length, &my_charset_bin);
  collation.set(&my_charset_bin, DERIVATION_COERCIBLE);
  fixed = true;
}

/**
  Pack data in buffer for sending.
*/

bool Item_null::send(Protocol *protocol, String *) {
  return protocol->store_null();
}

Item_json::Item_json(unique_ptr_destroy_only<Json_wrapper> value,
                     const Item_name_string &name)
    : m_value(std::move(value)) {
  set_data_type_json();
  item_name = name;
}

Item_json::~Item_json() = default;

void Item_json::print(const THD *, String *str, enum_query_type) const {
  str->append("json'");
  m_value->to_string(str, true, "");
  str->append("'");
}

bool Item_json::val_json(Json_wrapper *result) {
  *result = *m_value;
  return false;
}

/*
  The functions below are rarely called, some of them are probably unreachable
  from SQL, because Item_json is used in a more limited way than other
  subclasses of Item_basic_constant. Most notably, there is no JSON literal
  syntax which gets translated into Item_json objects by the parser.
*/

double Item_json::val_real() { return m_value->coerce_real(item_name.ptr()); }

longlong Item_json::val_int() { return m_value->coerce_int(item_name.ptr()); }

String *Item_json::val_str(String *str) {
  str->length(0);
  if (m_value->to_string(str, true, item_name.ptr())) return error_str();
  return str;
}

my_decimal *Item_json::val_decimal(my_decimal *buf) {
  return m_value->coerce_decimal(buf, item_name.ptr());
}

bool Item_json::get_date(MYSQL_TIME *ltime, my_time_flags_t) {
  return m_value->coerce_date(ltime, item_name.ptr());
}

bool Item_json::get_time(MYSQL_TIME *ltime) {
  return m_value->coerce_time(ltime, item_name.ptr());
}

Item *Item_json::clone_item() const {
  THD *const thd = current_thd;
  auto wr = make_unique_destroy_only<Json_wrapper>(thd->mem_root,
                                                   m_value->clone_dom(thd));
  if (wr == nullptr) return nullptr;
  return new Item_json(std::move(wr), item_name);
}

/**
  This is only called from items that is not of type item_field.
*/

bool Item::send(Protocol *protocol, String *buffer) {
  switch (data_type()) {
    default:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_JSON: {
      const String *res = val_str(buffer);
      if (res != nullptr)
        return protocol->store_string(res->ptr(), res->length(),
                                      res->charset());
      break;
    }
    case MYSQL_TYPE_TINY: {
      longlong nr = val_int();
      if (!null_value) return protocol->store_tiny(nr);
      break;
    }
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_YEAR: {
      longlong nr = val_int();
      if (!null_value) return protocol->store_short(nr);
      break;
    }
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG: {
      longlong nr = val_int();
      if (!null_value) return protocol->store_long(nr);
      break;
    }
    case MYSQL_TYPE_LONGLONG: {
      longlong nr = val_int();
      if (!null_value) return protocol->store_longlong(nr, unsigned_flag);
      break;
    }
    case MYSQL_TYPE_FLOAT: {
      float nr = static_cast<float>(val_real());
      if (!null_value) return protocol->store_float(nr, decimals, 0);
      break;
    }
    case MYSQL_TYPE_DOUBLE: {
      double nr = val_real();
      if (!null_value) return protocol->store_double(nr, decimals, 0);
      break;
    }
    case MYSQL_TYPE_DATE: {
      MYSQL_TIME tm;
      get_date(&tm, TIME_FUZZY_DATE);
      if (!null_value) return protocol->store_date(tm);
      break;
    }
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP: {
      MYSQL_TIME tm;
      get_date(&tm, TIME_FUZZY_DATE);
      if (!null_value) return protocol->store_datetime(tm, decimals);
      break;
    }
    case MYSQL_TYPE_TIME: {
      MYSQL_TIME tm;
      get_time(&tm);
      if (!null_value) return protocol->store_time(tm, decimals);
      break;
    }
  }

  DBUG_ASSERT(null_value);
  return protocol->store_null();
}

bool Item::update_null_value() {
  char buff[STRING_BUFFER_USUAL_SIZE];
  String str(buff, sizeof(buff), collation.collation);
  return evaluate(current_thd, &str);
}

/**
  Evaluate item, possibly using the supplied buffer

  @param thd    Thread context
  @param buffer Buffer, in case item needs a large one

  @returns false if success, true if error
*/

bool Item::evaluate(THD *thd, String *buffer) {
  bool result = false;  // Will be set if null_value == 0

  switch (data_type()) {
    default:
      DBUG_ASSERT(false);
      (void)val_str(buffer);
      break;
    case MYSQL_TYPE_JSON: {
      Json_wrapper wr;
      (void)val_json(&wr);
    } break;
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BIT: {
      (void)val_str(buffer);
      break;
    }
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG: {
      (void)val_int();
      break;
    }
    case MYSQL_TYPE_NEWDECIMAL: {
      my_decimal decimal_value;
      (void)val_decimal(&decimal_value);
      break;
    }

    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE: {
      (void)val_real();
      break;
    }
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIMESTAMP: {
      MYSQL_TIME tm;
      (void)get_date(&tm, TIME_FUZZY_DATE);
      break;
    }
    case MYSQL_TYPE_TIME: {
      MYSQL_TIME tm;
      (void)get_time(&tm);
      break;
    }
  }
  result = thd->is_error();
  return result;
}

/**
  Check if an item is a constant one and can be cached.

  @param [out] arg If != NULL <=> Cache this item.

  @return true  Go deeper in item tree.
  @return false Don't go deeper in item tree.
*/

bool Item::cache_const_expr_analyzer(uchar **arg) {
  cache_const_expr_arg *carg = (cache_const_expr_arg *)*arg;
  if (!carg->cache_item) {
    Item *item = real_item();
    /*
      Cache constant items unless it's a basic constant, constant field or
      a subquery (they use their own cache), or it is already cached.
    */
    if (const_for_execution() &&
        !(basic_const_item() || item->basic_const_item() ||
          item->type() == Item::FIELD_ITEM || item->type() == SUBSELECT_ITEM ||
          item->type() == CACHE_ITEM))
      /*
        Note that we use cache_item as a flag (NULL vs non-NULL), but we
        are storing the pointer so that we can assert that we cache the
        correct item in Item::cache_const_expr_transformer().
      */
      carg->cache_item = this;
    /*
      JSON functions can read JSON from strings or use SQL scalars by
      converting them to JSON scalars. Such conversion takes time and on
      repetitive calls result is significant performance penalty.

      Check if such data can be cached:
      1) this item is constant
      2) this item is an arg to a funciton
      3) it's a source of JSON data
      4) this item's type isn't JSON so conversion will be required
      5) it's not cached already

      Difference with the block above is that this one caches any const item,
      because the goal here is to avoid conversion, rather than re-evaluation.
    */
    else if (const_for_execution() &&  // 1
             carg->stack.elements > 0 &&
             carg->stack.head()->type() == FUNC_ITEM)  // 2
    {
      Item_func *head = down_cast<Item_func *>(carg->stack.head());
      enum_const_item_cache what_cache;
      if ((what_cache = head->can_cache_json_arg(this)) &&  // 3
          data_type() != MYSQL_TYPE_JSON &&                 // 4
          item->type() != CACHE_ITEM)                       // 5
      {
        carg->cache_item = this;
        carg->cache_arg = what_cache;
      }
    }
    // Push only if we're going down the tree, so transformer will pop the item
    carg->stack.push_front(item);
    /*
      If this item will be cached, no need to explore items further down
      in the tree, but the transformer must be called, so return 'true'.
      If this item will not be cached, items further doen in the tree
      must be explored, so return 'true'.
    */
    return true;
  }
  /*
    An item above in the tree is to be cached, so need to cache the present
    item, and no need to go down the tree.
  */
  return false;
}

/**
  Set the maximum number of characters required by any of the items in args.
*/
void Item::aggregate_char_length(Item **args, uint nitems) {
  uint32 char_length = 0;
  /*
    To account for character sets with different number of bytes per character,
    set char_length equal to max_length if the aggregated character set is
    binary to prevent truncation of data as some characters require more than
    one byte.
  */
  bool bin_charset = collation.collation == &my_charset_bin;
  for (uint i = 0; i < nitems; i++)
    char_length = max(char_length, bin_charset ? args[i]->max_length
                                               : args[i]->max_char_length());
  if (char_length * collation.collation->mbmaxlen > max_length)
    fix_char_length(char_length);
}

/**
  Set max_length and decimals of function if function is floating point and
  result length/precision depends on argument ones.

  @param item    Argument array.
  @param nitems  Number of arguments in the array.
*/
void Item::aggregate_float_properties(Item **item, uint nitems) {
  DBUG_ASSERT(result_type() == REAL_RESULT);
  uint32 length = 0;
  uint8 decimals_cnt = 0;
  uint32 maxl = 0;
  for (uint i = 0; i < nitems; i++) {
    if (decimals_cnt != DECIMAL_NOT_SPECIFIED) {
      decimals_cnt = max(decimals_cnt, item[i]->decimals);
      length = max(length, (item[i]->max_length - item[i]->decimals));
    }
    maxl = max(maxl, item[i]->max_length);
  }
  if (decimals_cnt != DECIMAL_NOT_SPECIFIED) {
    maxl = length;
    length += decimals_cnt;
    if (length < maxl)  // If previous operation gave overflow
      maxl = UINT_MAX32;
    else
      maxl = length;
  }

  this->max_length = maxl;
  this->decimals = decimals_cnt;
}

/**
  Set precision and decimals of function when this depends on arguments'
  values for these quantities.

  @param item    Argument array.
  @param nitems  Number of arguments in the array.
*/
void Item::aggregate_decimal_properties(Item **item, uint nitems) {
  DBUG_ASSERT(result_type() == DECIMAL_RESULT);
  int max_int_part = 0;
  uint8 decimal_cnt = 0;
  for (uint i = 0; i < nitems; i++) {
    decimal_cnt = max(decimal_cnt, item[i]->decimals);
    max_int_part = max(max_int_part, item[i]->decimal_int_part());
  }
  int precision = min(max_int_part + decimal_cnt, DECIMAL_MAX_PRECISION);
  set_data_type_decimal(precision, decimal_cnt);
}

/**
  Set fractional seconds precision for temporal functions.

  @param item    Argument array
  @param nitems  Number of arguments in the array.
*/
void Item::aggregate_temporal_properties(Item **item, uint nitems) {
  DBUG_ASSERT(result_type() == STRING_RESULT);
  uint8 decimal_cnt = 0;

  switch (data_type()) {
    case MYSQL_TYPE_DATETIME:
      for (uint i = 0; i < nitems; i++)
        decimal_cnt = max(decimal_cnt, uint8(item[i]->datetime_precision()));
      decimal_cnt = min(decimal_cnt, uint8(DATETIME_MAX_DECIMALS));
      set_data_type_datetime(decimal_cnt);
      break;

    case MYSQL_TYPE_TIMESTAMP:
      for (uint i = 0; i < nitems; i++)
        decimal_cnt = max(decimal_cnt, uint8(item[i]->datetime_precision()));
      decimal_cnt = min(decimal_cnt, uint8(DATETIME_MAX_DECIMALS));
      set_data_type_timestamp(decimal_cnt);
      break;

    case MYSQL_TYPE_NEWDATE:
      DBUG_ASSERT(false);
      set_data_type_date();
      set_data_type(MYSQL_TYPE_NEWDATE);
      break;

    case MYSQL_TYPE_DATE:
      set_data_type_date();
      break;

    case MYSQL_TYPE_TIME:
      for (uint i = 0; i < nitems; i++)
        decimal_cnt = max(decimal_cnt, uint8(item[i]->time_precision()));
      decimal_cnt = min(decimal_cnt, uint8(DATETIME_MAX_DECIMALS));
      set_data_type_time(decimal_cnt);
      break;

    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

/**
  Aggregate string properties (character set, collation and maximum length) for
  string function.

  @param name        Name of function
  @param items       Argument array.
  @param nitems      Number of arguments.

  @retval            False on success, true on error.
*/
bool Item::aggregate_string_properties(const char *name, Item **items,
                                       uint nitems) {
  DBUG_ASSERT(result_type() == STRING_RESULT);
  if (agg_item_charsets_for_string_result(collation, name, items, nitems, 1))
    return true;
  if (is_temporal_type(data_type())) {
    /*
      aggregate_temporal_properties() will set collation to numeric, causing
      the character set to be explicitly set to latin1, which may not match the
      aggregated character set. The collation must therefore be restored after
      the temporal properties have been computed.
    */
    auto aggregated_collation = collation;
    aggregate_temporal_properties(items, nitems);
    collation.set(aggregated_collation);
    /*
      Set max_length again as the aggregated character set may have different
      number of bytes per character than latin1.
    */
    fix_char_length(max_length);
  } else
    decimals = min(decimals, uint8(DECIMAL_NOT_SPECIFIED));
  aggregate_char_length(items, nitems);

  /*
    If the resulting data type is a fixed length character or binary string
    and the result maximum length in characters is longer than the MySQL
    maximum CHAR/BINARY size, convert to a variable-sized type.
  */
  if (data_type() == MYSQL_TYPE_STRING &&
      max_char_length() > MAX_FIELD_CHARLENGTH)
    set_data_type(MYSQL_TYPE_VARCHAR);

  return false;
}

/**
  This function is used to resolve type for numeric result type of CASE,
  COALESCE, IF and LEAD/LAG. COALESCE is a CASE abbreviation according to the
  standard.

  @param result_type The desired result type
  @param item        The arguments of func
  @param nitems      The number of arguments
*/
void Item::aggregate_num_type(Item_result result_type, Item **item,
                              uint nitems) {
  collation.set_numeric();
  switch (result_type) {
    case DECIMAL_RESULT:
      aggregate_decimal_properties(item, nitems);
      break;
    case REAL_RESULT:
      aggregate_float_properties(item, nitems);
      break;
    case INT_RESULT:
    case STRING_RESULT:
      aggregate_char_length(item, nitems);
      decimals = 0;
      break;
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  }
}

/**
  Cache item if needed.

  @param arg   Descriptor of what and how to cache @see cache_const_expr_arg

  @return cache if cache needed.
  @return this otherwise.
*/

Item *Item::cache_const_expr_transformer(uchar *arg) {
  cache_const_expr_arg *carg = (cache_const_expr_arg *)arg;
  carg->stack.pop();
  if (carg->cache_item)  // Item is to be cached, note that it is used as a flag
  {
    DBUG_ASSERT(carg->cache_item == this);
    Item_cache *cache;
    /*
      Flag applies to present item, must reset it so it does not affect
      the parent item.
    */
    carg->cache_item = nullptr;
    // Cache arg of a JSON function to avoid repetitive conversion
    if (carg->cache_arg != CACHE_NONE) {
      Item *itm = this;
      Item_func *caller = down_cast<Item_func *>(carg->stack.head());
      String buf;
      Json_wrapper wr;
      enum_const_item_cache what_cache = carg->cache_arg;

      carg->cache_arg = CACHE_NONE;
      if (what_cache == CACHE_JSON_VALUE) {
        // Cache parse result of JSON string
        if (get_json_wrapper(&itm, 0, &buf, caller->func_name(), &wr) ||
            null_value) {
          return current_thd->is_error() ? nullptr : this;
        }
      } else {
        // Cache SQL scalar converted to JSON
        DBUG_ASSERT(what_cache == CACHE_JSON_ATOM);
        String conv_buf;
        if (get_json_atom_wrapper(&itm, 0, caller->func_name(), &buf, &conv_buf,
                                  &wr, nullptr, true) ||
            null_value) {
          return current_thd->is_error() ? nullptr : this;
        }
      }
      // Should've been checked at get_*_wrapper()
      DBUG_ASSERT(wr.type() != enum_json_type::J_ERROR);
      Item_cache_json *jcache = new Item_cache_json();
      if (!jcache) return nullptr;
      jcache->setup(this);
      jcache->store_value(this, &wr);
      cache = jcache;
    } else {
      cache = Item_cache::get_cache(this);
      if (!cache) return nullptr;
      cache->setup(this);
      cache->store(this);
    }
    /*
      This item is cached - for subqueries this effectively means that they
      are optimized away.
    */
    mark_subqueries_optimized_away();
    return cache;
  }
  return this;
}

bool Item_field::send(Protocol *protocol, String *) {
  return protocol->store_field(result_field);
}

/*
  Add the field to the select list and substitute it for the reference to
  the field.

  SYNOPSIS
    Item_field::update_value_transformer()
    select_arg      current select

  DESCRIPTION
    If the field doesn't belong to the table being inserted into then it is
    added to the select list, pointer to it is stored in the ref_item_array
    of the select and the field itself is substituted for the Item_ref object.
    This is done in order to get correct values from update fields that
    belongs to the SELECT part in the INSERT .. SELECT .. ON DUPLICATE KEY
    UPDATE statement.

  RETURN
    0             if error occurred
    ref           if all conditions are met
    this field    otherwise
*/

Item *Item_field::update_value_transformer(uchar *select_arg) {
  SELECT_LEX *select = (SELECT_LEX *)select_arg;
  DBUG_ASSERT(fixed);

  if (field->table != select->context.table_list->table &&
      type() != Item::TRIGGER_FIELD_ITEM) {
    Item **tmp = select->add_hidden_item(this);
    return new Item_ref(&select->context, tmp, table_name, field_name);
  }
  return this;
}

void Item_field::print(const THD *thd, String *str,
                       enum_query_type query_type) const {
  if (field && field->is_field_for_functional_index()) {
    field->gcol_info->expr_item->print(thd, str, query_type);
    return;
  }

  if (field && field->table && field->table->const_table &&
      !(query_type & QT_NO_DATA_EXPANSION)) {
    char buff[MAX_FIELD_WIDTH];
    String tmp(buff, sizeof(buff), str->charset());
    field->val_str(&tmp);
    if (field->is_null())
      str->append("NULL");
    else {
      str->append('\'');
      str->append(tmp);
      str->append('\'');
    }
    return;
  }
  if ((table_name == nullptr || table_name[0] == 0) && field &&
      field->orig_table)
    Item_ident::print(thd, str, query_type, field->orig_table->s->db.str,
                      field->orig_table->alias);
  else
    Item_ident::print(thd, str, query_type);
}

/**
  Calculate condition filtering effect for "WHERE field", which
  implicitly means "WHERE field <> 0". The filtering effect is
  therefore identical to that of Item_func_ne.
*/
float Item_field::get_filtering_effect(THD *, table_map filter_for_table,
                                       table_map,
                                       const MY_BITMAP *fields_to_ignore,
                                       double rows_in_table) {
  if (used_tables() != filter_for_table ||
      bitmap_is_set(fields_to_ignore, field->field_index))
    return COND_FILTER_ALLPASS;

  return 1.0f - get_cond_filter_default_probability(rows_in_table,
                                                    COND_FILTER_EQUALITY);
}

float Item_field::get_cond_filter_default_probability(
    double max_distinct_values, float default_filter) const {
  DBUG_ASSERT(max_distinct_values >= 1.0);

  // Some field types have a limited number of possible values
  switch (field->real_type()) {
    case MYSQL_TYPE_ENUM: {
      // ENUM can only have the values defined in the typelib
      const uint enum_values = static_cast<Field_enum *>(field)->typelib->count;
      max_distinct_values =
          min(static_cast<double>(enum_values), max_distinct_values);
      break;
    }
    case MYSQL_TYPE_BIT: {
      // BIT(N) can have no more than 2^N distinct values
      const uint bits = static_cast<Field_bit *>(field)->field_length;
      const double combos = pow(2.0, (int)bits);
      max_distinct_values = min(combos, max_distinct_values);
      break;
    }
    default:
      break;
  }
  return max(static_cast<float>(1 / max_distinct_values), default_filter);
}

Item_ref::Item_ref(Name_resolution_context *context_arg, Item **item,
                   const char *table_name_arg, const char *field_name_arg,
                   bool alias_of_expr_arg)
    : Item_ident(context_arg, NullS, table_name_arg, field_name_arg),
      ref(item),
      chop_ref(!ref) {
  m_alias_of_expr = alias_of_expr_arg;
  /*
    This constructor used to create some internals references over fixed items
  */
  if (ref && *ref && (*ref)->fixed) set_properties();
}

/**
  Resolve the name of a reference to a column reference.

  The method resolves the column reference represented by 'this' as a column
  present in one of: GROUP BY clause, SELECT clause, outer queries. It is
  used typically for columns in the HAVING clause which are not under
  aggregate functions.

  POSTCONDITION @n
  Item_ref::ref is 0 or points to a valid item.

  @note
    The name resolution algorithm used is (where [T_j] is an optional table
    name that qualifies the column name):

  @code
        resolve_extended([T_j].col_ref_i)
        {
          Search for a column or derived column named col_ref_i [in table T_j]
          in the SELECT and GROUP clauses of Q.

          if such a column is NOT found AND    // Lookup in outer queries.
             there are outer queries
          {
            for each outer query Q_k beginning from the inner-most one
           {
              Search for a column or derived column named col_ref_i
              [in table T_j] in the SELECT and GROUP clauses of Q_k.

              if such a column is not found AND
                 - Q_k is not a group query AND
                 - Q_k is not inside an aggregate function
                 OR
                 - Q_(k-1) is not in a HAVING or SELECT clause of Q_k
              {
                search for a column or derived column named col_ref_i
                [in table T_j] in the FROM clause of Q_k;
              }
            }
          }
        }
  @endcode
  @n
    This procedure treats GROUP BY and SELECT clauses as one namespace for
    column references in HAVING. Notice that compared to
    Item_field::fix_fields, here we first search the SELECT and GROUP BY
    clauses, and then we search the FROM clause.

  @param[in]     thd        current thread
  @param[in,out] reference  view column if this item was resolved to a
    view column

  @todo
    Here we could first find the field anyway, and then test this
    condition, so that we can give a better error message -
    ER_WRONG_FIELD_WITH_GROUP, instead of the less informative
    ER_BAD_FIELD_ERROR which we produce now.

  @retval
    true  if error
  @retval
    false on success
*/

bool Item_ref::fix_fields(THD *thd, Item **reference) {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 0);

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);

  if (!ref || ref == not_found_item) {
    DBUG_ASSERT(context->select_lex == thd->lex->current_select());
    if (!(ref =
              resolve_ref_in_select_and_group(thd, this, context->select_lex)))
      goto error; /* Some error occurred (e.g. ambiguous names). */

    if (ref == not_found_item) /* This reference was not resolved. */
    {
      Name_resolution_context *last_checked_context = context;
      Name_resolution_context *outer_context = context->outer_context;
      Field *from_field;
      ref = nullptr;

      if (!outer_context) {
        /* The current reference cannot be resolved in this query. */
        my_error(ER_BAD_FIELD_ERROR, MYF(0), this->full_name(),
                 current_thd->where);
        goto error;
      }

      /*
        If there is an outer context (select), try to
        resolve this reference in the outer select(s).

        We treat each subselect as a separate namespace, so that different
        subselects may contain columns with the same names. The subselects are
        searched starting from the innermost.
      */
      from_field = not_found_field;

      SELECT_LEX *cur_select = context->select_lex;

      do {
        SELECT_LEX *select = outer_context->select_lex;
        last_checked_context = outer_context;
        SELECT_LEX_UNIT *cur_unit = nullptr;
        enum_parsing_context place = CTX_NONE;

        // See comments and similar loop in Item_field::fix_outer_field()
        while (true) {
          if (!cur_select) goto loop;
          if (cur_select == select) {
            DBUG_ASSERT(!cur_select->first_execution);
            goto loop;  // we're misplaced; @todo remove in WL#6570
          }
          cur_unit = cur_select->master_unit();
          if (cur_unit->outer_select() == select) break;
          cur_unit->accumulate_used_tables(OUTER_REF_TABLE_BIT);
          cur_select = cur_unit->outer_select();
        }

        place = cur_unit->place();

        if (place == CTX_DERIVED && select->end_lateral_table == nullptr)
          goto loop;

        /* Search in the SELECT and GROUP lists of the outer select. */
        if (select_alias_referencable(place) &&
            outer_context->resolve_in_select_list) {
          if (!(ref = resolve_ref_in_select_and_group(thd, this, select)))
            goto error; /* Some error occurred (e.g. ambiguous names). */
          if (ref != not_found_item) {
            DBUG_ASSERT((*ref)->fixed);
            cur_unit->accumulate_used_tables((*ref)->used_tables());
            break;
          }
          /*
            Set ref to 0 to ensure that we get an error in case we replaced
            this item with another item and still use this item in some
            other place of the parse tree.
          */
          ref = nullptr;
        }

        /*
          Check table fields only if the subquery is used somewhere out of
          HAVING or the outer SELECT does not use grouping (i.e. tables are
          accessible).
          TODO:
          Here we could first find the field anyway, and then test this
          condition, so that we can give a better error message -
          ER_WRONG_FIELD_WITH_GROUP, instead of the less informative
          ER_BAD_FIELD_ERROR which we produce now.
        */
        if ((place != CTX_HAVING ||
             (!select->with_sum_func && select->group_list.elements == 0))) {
          /*
            In case of view, find_field_in_tables() write pointer to view
            field expression to 'reference', i.e. it substitute that
            expression instead of this Item_ref
          */
          from_field = find_field_in_tables(
              thd, this, outer_context->first_name_resolution_table,
              outer_context->last_name_resolution_table, reference,
              IGNORE_EXCEPT_NON_UNIQUE, thd->want_privilege, true);
          if (!from_field) goto error;
          if (from_field == view_ref_found) {
            Item::Type refer_type = (*reference)->type();
            cur_unit->accumulate_used_tables((*reference)->used_tables());
            DBUG_ASSERT((*reference)->type() == REF_ITEM);
            mark_as_dependent(
                thd, last_checked_context->select_lex, context->select_lex,
                this,
                ((refer_type == REF_ITEM || refer_type == FIELD_ITEM)
                     ? (Item_ident *)(*reference)
                     : nullptr));
            /*
              view reference found, we substituted it instead of this
              Item, so can quit
            */
            return false;
          }
          if (from_field != not_found_field) {
            if (cached_table && cached_table->select_lex &&
                outer_context->select_lex &&
                cached_table->select_lex != outer_context->select_lex) {
              /*
                Due to cache, find_field_in_tables() can return field which
                doesn't belong to provided outer_context. In this case we have
                to find proper field context in order to fix field correctly.
              */
              do {
                outer_context = outer_context->outer_context;
                select = outer_context->select_lex;
                cur_unit = last_checked_context->select_lex->master_unit();
                last_checked_context = outer_context;
              } while (outer_context && outer_context->select_lex &&
                       cached_table->select_lex != outer_context->select_lex);
              place = cur_unit->place();
            }
            cur_unit->accumulate_used_tables(
                from_field->table->pos_in_table_list->map());
            break;
          }
        }
        DBUG_ASSERT(from_field == not_found_field);

        /* Reference is not found => depend on outer (or just error). */
        cur_unit->accumulate_used_tables(OUTER_REF_TABLE_BIT);

      loop:
        outer_context = outer_context->outer_context;
      } while (outer_context);

      DBUG_ASSERT(from_field != nullptr && from_field != view_ref_found);
      if (from_field != not_found_field) {
        Item_field *fld;

        {
          Prepared_stmt_arena_holder ps_arena_holder(thd);
          fld = new Item_field(thd, context, from_field);
          if (!fld) goto error;
        }

        thd->change_item_tree(reference, fld);
        mark_as_dependent(thd, last_checked_context->select_lex,
                          context->select_lex, this, fld);
        /*
          A reference is resolved to a nest level that's outer or the same as
          the nest level of the enclosing set function : adjust the value of
          max_aggr_level for the function if it's needed.
        */
        if (thd->lex->in_sum_func &&
            thd->lex->in_sum_func->base_select->nest_level >=
                last_checked_context->select_lex->nest_level)
          thd->lex->in_sum_func->max_aggr_level =
              max(thd->lex->in_sum_func->max_aggr_level,
                  int8(last_checked_context->select_lex->nest_level));
        return false;
      }
      if (ref == nullptr) {
        /* The item was not a table field and not a reference */
        my_error(ER_BAD_FIELD_ERROR, MYF(0), this->full_name(),
                 current_thd->where);
        goto error;
      }
      /* Should be checked in resolve_ref_in_select_and_group(). */
      DBUG_ASSERT((*ref)->fixed);
      mark_as_dependent(thd, last_checked_context->select_lex,
                        context->select_lex, this, this);
      /*
        A reference is resolved to a nest level that's outer or the same as
        the nest level of the enclosing set function : adjust the value of
        max_aggr_level for the function if it's needed.
      */
      if (thd->lex->in_sum_func &&
          thd->lex->in_sum_func->base_select->nest_level >=
              last_checked_context->select_lex->nest_level)
        thd->lex->in_sum_func->max_aggr_level =
            max(thd->lex->in_sum_func->max_aggr_level,
                int8(last_checked_context->select_lex->nest_level));
    }
  }

  // The reference should be fixed at this point.
  DBUG_ASSERT((*ref)->fixed);

  /*
    Reject invalid references to aggregates.

    1) We only accept references to aggregates in a HAVING clause.
    (This restriction is not strictly necessary, but we don't want to
    lift it without making sure that such queries are handled
    correctly. Lifting the restriction will make bugs such as
    bug#13633829 and bug#22588319 (aka bug#80116) affect a larger set
    of queries.)

    2) An aggregate cannot be referenced from the GROUP BY clause of
    the query block where the aggregation happens, since grouping
    happens before aggregation.
  */
  if (((*ref)->has_aggregation() &&
       !thd->lex->current_select()->having_fix_field) ||  // 1
      walk(&Item::has_aggregate_ref_in_group_by,          // 2
           enum_walk::SUBQUERY_POSTFIX, nullptr)) {
    my_error(ER_ILLEGAL_REFERENCE, MYF(0), full_name(),
             "reference to group function");
    goto error;
  }

  set_properties();

  if ((*ref)->check_cols(1)) goto error;
  return false;

error:
  return true;
}

void Item_ref::set_properties() {
  DBUG_TRACE;

  set_data_type((*ref)->data_type());
  max_length = (*ref)->max_length;
  maybe_null = (*ref)->maybe_null;
  decimals = (*ref)->decimals;
  collation.set((*ref)->collation);
  /*
    We have to remember if we refer to a sum function, to ensure that
    split_sum_func() doesn't try to change the reference.
  */
  set_accum_properties(*ref);
  unsigned_flag = (*ref)->unsigned_flag;
  fixed = true;
  if ((*ref)->type() == FIELD_ITEM &&
      ((Item_ident *)(*ref))->is_alias_of_expr())
    set_alias_of_expr();
}

void Item_ref::cleanup() {
  DBUG_TRACE;
  Item_ident::cleanup();
  result_field = nullptr;
  if (chop_ref) ref = nullptr;
}

/**
  Transform an Item_ref object with a transformer callback function.

  The function first applies the transform function to the item
  referenced by this Item_ref object. If this replaces the item with a
  new one, this item object is returned as the result of the
  transform. Otherwise the transform function is applied to the
  Item_ref object itself.
*/

Item *Item_ref::transform(Item_transformer transformer, uchar *arg) {
  DBUG_ASSERT((*ref) != nullptr);

  /* Transform the object we are referencing. */
  Item *new_item = (*ref)->transform(transformer, arg);
  if (new_item == nullptr) return nullptr;

  /*
    If the object is transformed into a new object, discard the Item_ref
    object and return the new object as result.
  */
  if (new_item != *ref) return new_item;

  /* Transform the item ref object. */
  Item *transformed_item = (this->*transformer)(arg);
  // DBUG_ASSERT(transformed_item == this);
  return transformed_item;
}

/**
  Compile an Item_ref object with a processor and a transformer
  callback function.

  First the function applies the analyzer to the Item_ref
  object. Second it applies the compile function to the object the
  Item_ref object is referencing. If this replaces the item with a new
  one, this object is returned as the result of the compile.
  Otherwise we apply the transformer to the Item_ref object itself.
*/

Item *Item_ref::compile(Item_analyzer analyzer, uchar **arg_p,
                        Item_transformer transformer, uchar *arg_t) {
  if (!(this->*analyzer)(arg_p)) return this;

  DBUG_ASSERT((*ref) != nullptr);
  Item *new_item = (*ref)->compile(analyzer, arg_p, transformer, arg_t);
  if (new_item == nullptr) return nullptr;

  /*
    If the object is compiled into a new object, discard the Item_ref
    object and return the new object as result.
  */
  if (new_item != *ref) return new_item;

  return (this->*transformer)(arg_t);
}

void Item_ref::print(const THD *thd, String *str,
                     enum_query_type query_type) const {
  if (ref) {
    if (m_alias_of_expr && (*ref)->type() != Item::CACHE_ITEM &&
        ref_type() != VIEW_REF && !table_name && item_name.ptr()) {
      Simple_cstring str1 = (*ref)->real_item()->item_name;
      append_identifier(thd, str, str1.ptr(), str1.length());
    } else
      (*ref)->print(thd, str, query_type);
  } else
    Item_ident::print(thd, str, query_type);
}

bool Item_ref::send(Protocol *prot, String *tmp) {
  if (result_field != nullptr) return prot->store_field(result_field);
  return (*ref)->send(prot, tmp);
}

double Item_ref::val_real() {
  DBUG_ASSERT(fixed);
  double tmp = (*ref)->val_real();
  null_value = (*ref)->null_value;
  return tmp;
}

longlong Item_ref::val_int() {
  DBUG_ASSERT(fixed);
  longlong tmp = (*ref)->val_int();
  null_value = (*ref)->null_value;
  return tmp;
}

longlong Item_ref::val_time_temporal() {
  DBUG_ASSERT(fixed);
  DBUG_ASSERT((*ref)->is_temporal());
  longlong tmp = (*ref)->val_time_temporal();
  null_value = (*ref)->null_value;
  return tmp;
}

longlong Item_ref::val_date_temporal() {
  DBUG_ASSERT(fixed);
  DBUG_ASSERT((*ref)->is_temporal());
  longlong tmp = (*ref)->val_date_temporal();
  null_value = (*ref)->null_value;
  return tmp;
}

bool Item_ref::val_bool() {
  DBUG_ASSERT(fixed);
  bool tmp = (*ref)->val_bool();
  null_value = (*ref)->null_value;
  return tmp;
}

String *Item_ref::val_str(String *tmp) {
  DBUG_ASSERT(fixed);
  tmp = (*ref)->val_str(tmp);
  null_value = (*ref)->null_value;
  return tmp;
}

bool Item_ref::val_json(Json_wrapper *result) {
  DBUG_ASSERT(fixed);
  bool ok = (*ref)->val_json(result);
  null_value = (*ref)->null_value;
  return ok;
}

bool Item_ref::is_null() {
  DBUG_ASSERT(fixed);
  bool tmp = (*ref)->is_null();
  null_value = (*ref)->null_value;
  return tmp;
}

bool Item_ref::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed);
  bool result = (*ref)->get_date(ltime, fuzzydate);
  null_value = (*ref)->null_value;
  return result;
}

my_decimal *Item_ref::val_decimal(my_decimal *decimal_value) {
  my_decimal *val = (*ref)->val_decimal(decimal_value);
  null_value = (*ref)->null_value;
  return val;
}

type_conversion_status Item_ref::save_in_field_inner(Field *to,
                                                     bool no_conversions) {
  type_conversion_status res;
  res = (*ref)->save_in_field(to, no_conversions);
  null_value = (*ref)->null_value;
  return res;
}

void Item_ref::make_field(Send_field *field) {
  (*ref)->make_field(field);
  /* Non-zero in case of a view */
  if (item_name.is_set()) field->col_name = item_name.ptr();
  if (table_name) field->table_name = table_name;
  if (db_name) field->db_name = db_name;
  if (orig_field_name) field->org_col_name = orig_field_name;
  if (orig_table_name) field->org_table_name = orig_table_name;
}

Item *Item_ref::get_tmp_table_item(THD *thd) {
  DBUG_TRACE;
  if (!result_field) {
    Item *result = (*ref)->get_tmp_table_item(thd);
    return result;
  }

  Item_field *item = new Item_field(result_field);
  if (item) {
    item->table_name = table_name;
    item->db_name = db_name;
  }
  return item;
}

void Item_ref_null_helper::print(const THD *thd, String *str,
                                 enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("<ref_null_helper>("));
  if (ref)
    (*ref)->print(thd, str, query_type);
  else
    str->append('?');
  str->append(')');
}

/**
  Prepare referenced field then call usual Item_ref::fix_fields .

  @param thd         Current session.
  @param reference   reference on reference where this item stored

  @retval
    false   OK
  @retval
    true    Error
*/

bool Item_view_ref::fix_fields(THD *thd, Item **reference) {
  DBUG_ASSERT(*ref);  // view field reference must be defined

  // (*ref)->check_cols() will be made in Item_ref::fix_fields
  if ((*ref)->fixed) {
    /*
      Underlying Item_field objects may be shared. Make sure that the use
      is marked regardless of how many ref items that point to this field.
    */
    Mark_field mf(thd->mark_used_columns);
    (*ref)->walk(&Item::mark_field_in_map, enum_walk::POSTFIX, (uchar *)&mf);
  } else {
    if ((*ref)->fix_fields(thd, ref)) return true; /* purecov: inspected */
  }
  if (super::fix_fields(thd, reference)) return true;

  if (cached_table->is_inner_table_of_outer_join()) {
    maybe_null = true;
    first_inner_table = cached_table->any_outer_leaf_table();
    // @todo delete this when WL#6570 is implemented
    (*ref)->maybe_null = true;
  }
  return false;
}

/**
  Prepare referenced outer field then call usual Item_ref::fix_fields

  @param thd         thread handler
  @param reference   reference on reference where this item stored

  @details
    The function serves 3 purposes
    - adds field to the current select list
    - creates an object to use to reference the item (Item_ref)
    - fixes reference (Item_ref object)

    If a field isn't already on the select list and the base_ref_items array
    is provided then it is added to the all_fields list and the pointer to
    it is saved in the base_ref_items array.

    When the class is chosen it substitutes the original field in the
    Item_outer_ref object.

  @returns true if error
*/

bool Item_outer_ref::fix_fields(THD *thd, Item **reference) {
  /* outer_ref->check_cols() will be made in Item_ref::fix_fields */
  if ((*ref) && !(*ref)->fixed && ((*ref)->fix_fields(thd, reference)))
    return true;
  if (super::fix_fields(thd, reference)) return true;
  if (!outer_ref) outer_ref = *ref;
  if ((*ref)->type() == Item::FIELD_ITEM)
    table_name = ((Item_field *)outer_ref)->table_name;

  Item *item = outer_ref;
  Item **item_ref = ref;

  /*
    TODO: this field item already might be present in the select list.
    In this case instead of adding new field item we could use an
    existing one. The change will lead to less operations for copying fields,
    smaller temporary tables and less data passed through filesort.
  */
  DBUG_ASSERT(!qualifying->base_ref_items.is_null());
  if (!found_in_select_list) {
    /*
      Add the field item to the select list of the current select.
      If it's needed reset each Item_ref item that refers this field with
      a new reference taken from ref_item_array.
    */
    item_ref = qualifying->add_hidden_item(item);
  }

  Item_ref *const new_ref = new Item_ref(context, item_ref, table_name,
                                         field_name, is_alias_of_expr());
  if (!new_ref) return true; /* purecov: inspected */
  outer_ref = new_ref;
  ref = &outer_ref;

  qualifying->select_list_tables |= item->used_tables();

  return false;
}

void Item_outer_ref::fix_after_pullout(SELECT_LEX *parent_select,
                                       SELECT_LEX *removed_select) {
  /*
    If this assertion holds, we need not call fix_after_pullout() on both
    *ref and outer_ref, and Item_ref::fix_after_pullout() is sufficient.
  */
  DBUG_ASSERT(*ref == outer_ref);

  Item_ref::fix_after_pullout(parent_select, removed_select);
}

void Item_ref::fix_after_pullout(SELECT_LEX *parent_select,
                                 SELECT_LEX *removed_select) {
  (*ref)->fix_after_pullout(parent_select, removed_select);

  Item_ident::fix_after_pullout(parent_select, removed_select);
}

/**
  Compare two view column references for equality.

  A view column reference is considered equal to another column
  reference if the second one is a view column and if both column
  references resolve to the same item. It is assumed that both
  items are of the same type.

  @param item        item to compare with

  @retval
    true    Referenced item is equal to given item
  @retval
    false   otherwise
*/

bool Item_view_ref::eq(const Item *item, bool) const {
  if (item->type() == REF_ITEM) {
    const Item_ref *item_ref = down_cast<const Item_ref *>(item);
    if (item_ref->ref_type() == VIEW_REF) {
      Item *item_ref_ref = *(item_ref->ref);
      return ((*ref)->real_item() == item_ref_ref->real_item());
    }
  }
  return false;
}

longlong Item_view_ref::val_int() {
  if (has_null_row()) {
    null_value = true;
    return 0;
  }
  return super::val_int();
}

double Item_view_ref::val_real() {
  if (has_null_row()) {
    null_value = true;
    return 0.0;
  }
  return super::val_real();
}

my_decimal *Item_view_ref::val_decimal(my_decimal *dec) {
  if (has_null_row()) {
    null_value = true;
    return nullptr;
  }
  return super::val_decimal(dec);
}

String *Item_view_ref::val_str(String *str) {
  if (has_null_row()) {
    null_value = true;
    return nullptr;
  }
  return super::val_str(str);
}

bool Item_view_ref::val_bool() {
  if (has_null_row()) {
    null_value = true;
    return false;
  }
  return super::val_bool();
}

bool Item_view_ref::val_json(Json_wrapper *wr) {
  if (has_null_row()) {
    null_value = true;
    return false;
  }
  return super::val_json(wr);
}

bool Item_view_ref::is_null() {
  if (has_null_row()) return true;

  return (*ref)->is_null();
}

bool Item_view_ref::send(Protocol *prot, String *tmp) {
  if (has_null_row()) return prot->store_null();
  return super::send(prot, tmp);
}

type_conversion_status Item_view_ref::save_in_field_inner(Field *field,
                                                          bool no_conversions) {
  if (has_null_row())
    return set_field_to_null_with_conversions(field, no_conversions);

  return super::save_in_field_inner(field, no_conversions);
}

bool Item_view_ref::change_context_processor(uchar *arg) {
  context = pointer_cast<Change_context *>(arg)->m_context;

  if (pointer_cast<Change_context *>(arg)->m_permanent) {
    // We need to change context for any item that will get rolled back as well
    I_List_iterator<Item_change_record> it(current_thd->change_list);
    Item_change_record *change;

    while ((change = it++)) {
      if (change->new_value == this) {
        if (change->old_value->walk(&Item::change_context_processor,
                                    enum_walk::POSTFIX, arg))
          return true; /* purecov: inspected */
      }
    }
  }

  return false;
}

bool Item_view_ref::collect_item_field_or_view_ref_processor(uchar *arg) {
  auto *info = pointer_cast<Collect_item_fields_or_view_refs *>(arg);
  if (info->is_stopped(this)) return false;
  if (context->select_lex == info->m_transformed_block)
    info->m_item_fields_or_view_refs->push_back(this);
  info->stop_at(this);
  return false;
}

Item *Item_view_ref::replace_item_view_ref(uchar *arg) {
  auto *info = pointer_cast<Item::Item_view_ref_replacement *>(arg);
  Item *real_item = Item_ref::real_item();
  if (real_item == info->m_target) {
    Item_field *new_field =
        new (current_thd->mem_root) Item_field(info->m_field);
    if (new_field == nullptr) return nullptr;
    new_field->set_orig_names();
    // Set correct metadata for the new field incl. any alias.
    if (orig_name.length() != 0) {
      // The one moved to new_derived has its orig_name set
      new_field->item_name.set(orig_name.ptr());
      new_field->orig_name.set(orig_name.ptr());
    } else {
      // this is a duplicated view reference, not touched yet.
      new_field->item_name.set(item_name.ptr());
      new_field->orig_name.set(item_name.ptr());
    }
    if (info->m_curr_block == info->m_trans_block) {
      return new_field;
    }

    // The is an outer reference, so we cannot reuse transformed query
    // block's Item_field; make a new one for this query block
    new_field->depended_from = info->m_trans_block;
    new_field->context = &info->m_curr_block->context;
    return new_field;
  }
  return this;
}

bool Item_default_value::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  if (arg != nullptr) {
    if (arg->itemize(pc, &arg)) return true;
    if (arg->is_splocal()) {
      Item_splocal *il = static_cast<Item_splocal *>(arg);

      my_error(ER_WRONG_COLUMN_NAME, MYF(0), il->m_name.ptr());
      return true;
    }
  }
  return false;
}

bool Item_default_value::eq(const Item *item, bool binary_cmp) const {
  return item->type() == DEFAULT_VALUE_ITEM &&
         down_cast<const Item_default_value *>(item)->arg->eq(arg, binary_cmp);
}

bool Item_default_value::fix_fields(THD *thd, Item **) {
  Item *real_arg;
  Item_field *field_arg;
  Field *def_field;
  DBUG_ASSERT(fixed == 0);

  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, context->view_error_handler, context->view_error_handler_arg);
  if (!arg) {
    fixed = true;
    return false;
  }
  if (!arg->fixed && arg->fix_fields(thd, &arg)) return true;

  real_arg = arg->real_item();
  if (real_arg->type() != FIELD_ITEM) {
    my_error(ER_NO_DEFAULT_FOR_FIELD, MYF(0), arg->item_name.ptr());
    return true;
  }

  field_arg = (Item_field *)real_arg;
  if (field_arg->field->flags & NO_DEFAULT_VALUE_FLAG) {
    my_error(ER_NO_DEFAULT_FOR_FIELD, MYF(0), field_arg->field->field_name);
    return true;
  }

  if (field_arg->field->has_insert_default_general_value_expression()) {
    my_error(ER_DEFAULT_AS_VAL_GENERATED, MYF(0));
    return true;
  }

  def_field = field_arg->field->clone(thd->mem_root);
  if (def_field == nullptr) return true;

  def_field->move_field_offset(def_field->table->default_values_offset());
  set_field(def_field);

  // Needs cached_table for some Item traversal functions:
  cached_table = table_ref;

  return false;
}

void Item_default_value::print(const THD *thd, String *str,
                               enum_query_type query_type) const {
  if (!arg) {
    str->append(STRING_WITH_LEN("default"));
    return;
  }
  str->append(STRING_WITH_LEN("default("));
  arg->print(thd, str, query_type);
  str->append(')');
}

type_conversion_status Item_default_value::save_in_field_inner(
    Field *field_arg, bool no_conversions) {
  if (!arg) {
    if ((field_arg->flags & NO_DEFAULT_VALUE_FLAG &&
         field_arg->m_default_val_expr == nullptr) &&
        field_arg->real_type() != MYSQL_TYPE_ENUM) {
      if (field_arg->reset()) {
        my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
        return TYPE_ERR_BAD_VALUE;
      }

      if (context->view_error_handler) {
        TABLE_LIST *view = cached_table->top_table();
        push_warning_printf(
            field_arg->table->in_use, Sql_condition::SL_WARNING,
            ER_NO_DEFAULT_FOR_VIEW_FIELD,
            ER_THD(field_arg->table->in_use, ER_NO_DEFAULT_FOR_VIEW_FIELD),
            view->view_db.str, view->view_name.str);
      } else {
        push_warning_printf(
            field_arg->table->in_use, Sql_condition::SL_WARNING,
            ER_NO_DEFAULT_FOR_FIELD,
            ER_THD(field_arg->table->in_use, ER_NO_DEFAULT_FOR_FIELD),
            field_arg->field_name);
      }
      return TYPE_ERR_BAD_VALUE;
    }

    // If this DEFAULT's value is actually an expression, mark the columns
    // it uses for reading. For inserts where the name is not explicitly
    // mentioned, this is set in COPY_INFO::get_function_default_columns
    if (field_arg->has_insert_default_general_value_expression()) {
      for (uint j = 0; j < field_arg->table->s->fields; j++) {
        if (bitmap_is_set(&field_arg->m_default_val_expr->base_columns_map,
                          j)) {
          bitmap_set_bit(field_arg->table->read_set, j);
        }
      }
    }

    field_arg->set_default();
    return field_arg->validate_stored_val(current_thd);
  }
  return Item_field::save_in_field_inner(field_arg, no_conversions);
}

Item *Item_default_value::transform(Item_transformer transformer, uchar *args) {
  /*
    If the value of arg is NULL, then this object represents a constant,
    so further transformation is unnecessary (and impossible).
  */
  if (arg == nullptr) return this;

  Item *new_item = arg->transform(transformer, args);
  if (new_item == nullptr) return nullptr; /* purecov: inspected */

  /*
    THD::change_item_tree() should be called only if the tree was
    really transformed, i.e. when a new item has been created.
    Otherwise we'll be allocating a lot of unnecessary memory for
    change records at each execution.
  */
  if (arg != new_item) current_thd->change_item_tree(&arg, new_item);
  return (this->*transformer)(args);
}

bool Item_insert_value::eq(const Item *item, bool binary_cmp) const {
  return item->type() == INSERT_VALUE_ITEM &&
         (down_cast<const Item_insert_value *>(item))->arg->eq(arg, binary_cmp);
}

bool Item_insert_value::fix_fields(THD *thd, Item **reference) {
  DBUG_ASSERT(fixed == 0);
  /* We should only check that arg is in first table */
  if (!arg->fixed) {
    bool res;
    TABLE_LIST *orig_next_table = context->last_name_resolution_table;
    context->last_name_resolution_table = context->first_name_resolution_table;
    res = arg->fix_fields(thd, &arg);
    context->last_name_resolution_table = orig_next_table;
    if (res) return true;
  }

  if (arg->type() == REF_ITEM) arg = static_cast<Item_ref *>(arg)->ref[0];
  if (arg->type() != FIELD_ITEM) {
    my_error(ER_BAD_FIELD_ERROR, MYF(0), "", "VALUES() function");
    return true;
  }

  Item_field *field_arg = (Item_field *)arg;

  if (field_arg->field->table->insert_values &&
      thd->lex->in_update_value_clause) {
    Field *def_field = field_arg->field->clone(thd->mem_root);
    if (def_field == nullptr) return true;

    def_field->move_field_offset((ptrdiff_t)(def_field->table->insert_values -
                                             def_field->table->record[0]));
    /*
      Put the original and cloned Field_blob objects in
      'insert_update_values_map' map. This will be used to make a
      separate copy of blob value, in case 'UPDATE' clause is executed in
      'INSERT...UPDATE' statement. See mysql_prepare_blob_values()
      for more info. We are only checking for MYSQL_TYPE_BLOB and
      MYSQL_TYPE_GEOMETRY. Sub types of blob like TINY BLOB, LONG BLOB, JSON,
      are internally stored are BLOB only. Same applies to geometry type.
    */
    if ((def_field->type() == MYSQL_TYPE_BLOB ||
         def_field->type() == MYSQL_TYPE_GEOMETRY)) {
      try {
        thd->lex->insert_values_map(field_arg->field, def_field);
      } catch (std::bad_alloc const &) {
        my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "fix_fields");
        return true;
      }
    }

    set_field(def_field);

    // The VALUES function is deprecated.
    if (m_is_values_function)
      push_deprecated_warn(
          thd, "VALUES function",
          "an alias (INSERT INTO ... VALUES (...) AS alias) and replace "
          "VALUES(col) in the ON DUPLICATE KEY UPDATE clause with alias.col");
  } else {
    // VALUES() is used out-of-scope - its value is always NULL
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    Item *const item = new Item_null(this->item_name);
    if (!item) return true;
    *reference = item;

    // The VALUES function is deprecated. It always returns NULL in this
    // context, but if it is inside an ON DUPLICATE KEY UPDATE clause, the user
    // probably meant something else. In that case, suggest an alternative
    // syntax which doesn't always return NULL.
    DBUG_ASSERT(m_is_values_function);
    if (thd->lex->in_update_value_clause) {
      push_warning(thd, Sql_condition::SL_WARNING, ER_WARN_DEPRECATED_SYNTAX,
                   ER_THD(thd, ER_WARN_DEPRECATED_VALUES_FUNCTION_ALWAYS_NULL));
    } else {
      push_deprecated_warn_no_replacement(thd, "VALUES function");
    }
  }
  return false;
}

void Item_insert_value::print(const THD *thd, String *str,
                              enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("values("));
  arg->print(thd, str, query_type);
  str->append(')');
}

/**
  Find index of Field object which will be appropriate for item
  representing field of row being changed in trigger.

  @param table_triggers     Table_trigger_field_support instance. Do not use
                            TABLE::triggers as it might be not initialized at
                            the moment.
  @param table_grant_info   GRANT_INFO of the subject table

  @note
    This function does almost the same as fix_fields() for Item_field but is
    invoked right after trigger definition parsing. Since at this stage we can't
    say exactly what Field object (corresponding to TABLE::record[0] or
    TABLE::record[1]) should be bound to this Item, we only find out index of
    the Field and then select concrete Field object in fix_fields() (by that
    time Table_trigger_dispatcher::old_field/ new_field should point to proper
    array of Fields).  It also binds Item_trigger_field to
    Table_trigger_field_support object for table of trigger which uses this
    item.
    Another difference is that the field is not marked in read_set/write_set.
*/

void Item_trigger_field::setup_field(
    Table_trigger_field_support *table_triggers, GRANT_INFO *table_grant_info) {
  /*
    Try to find field by its name and if it will be found
    set field_idx properly.
  */
  (void)find_field_in_table(table_triggers->get_subject_table(), field_name,
                            strlen(field_name), false, &field_idx);
  triggers = table_triggers;
  table_grants = table_grant_info;
}

bool Item_trigger_field::eq(const Item *item, bool) const {
  return item->type() == TRIGGER_FIELD_ITEM &&
         trigger_var_type ==
             down_cast<const Item_trigger_field *>(item)->trigger_var_type &&
         !my_strcasecmp(
             system_charset_info, field_name,
             down_cast<const Item_trigger_field *>(item)->field_name);
}

void Item_trigger_field::set_required_privilege(bool rw) {
  /*
    Require SELECT and UPDATE privilege if this field will be read and
    set, and only UPDATE privilege for setting the field.
  */
  want_privilege = (rw ? SELECT_ACL | UPDATE_ACL : UPDATE_ACL);
}

bool Item_trigger_field::set_value(THD *thd, sp_rcontext * /*ctx*/, Item **it) {
  Item *item = sp_prepare_func_item(thd, it);

  if (!item) return true;

  if (!fixed) {
    if (fix_fields(thd, nullptr)) return true;
  }

  // NOTE: field->table->copy_blobs should be false here, but let's
  // remember the value at runtime to avoid subtle bugs.
  bool copy_blobs_saved = field->table->copy_blobs;

  field->table->copy_blobs = true;

  int err_code = item->save_in_field(field, false);

  field->table->copy_blobs = copy_blobs_saved;

  return err_code < 0;
}

bool Item_trigger_field::fix_fields(THD *thd, Item **) {
  /*
    Since trigger is object tightly associated with TABLE object most
    of its set up can be performed during trigger loading i.e. trigger
    parsing! So we have little to do in fix_fields. :)
  */

  DBUG_ASSERT(fixed == 0);

  /* Set field. */

  if (field_idx != (uint)-1) {
    /*
      Check access privileges for the subject table. We check privileges only
      in runtime.
    */

    if (table_grants) {
      if (check_grant_column(
              thd, table_grants, triggers->get_subject_table()->s->db.str,
              triggers->get_subject_table()->s->table_name.str, field_name,
              strlen(field_name), thd->security_context(), want_privilege))
        return true;
    }

    field = triggers->get_trigger_variable_field(trigger_var_type, field_idx);

    set_field(field);
    fixed = true;
    return false;
  }

  my_error(ER_BAD_FIELD_ERROR, MYF(0), field_name,
           (trigger_var_type == TRG_NEW_ROW) ? "NEW" : "OLD");
  return true;
}

void Item_trigger_field::print(const THD *, String *str,
                               enum_query_type) const {
  str->append((trigger_var_type == TRG_NEW_ROW) ? "NEW" : "OLD", 3);
  str->append('.');
  str->append(field_name);
}

void Item_trigger_field::cleanup() {
  want_privilege = original_privilege;
  /*
    Since special nature of Item_trigger_field we should not do most of
    things from Item_field::cleanup() or Item_ident::cleanup() here.
  */
  Item::cleanup();
}

Item_result item_cmp_type(Item_result a, Item_result b) {
  if (a == b) {
    DBUG_ASSERT(a != INVALID_RESULT);
    return a;
  } else if (a == ROW_RESULT || b == ROW_RESULT) {
    return ROW_RESULT;
  }
  if ((a == INT_RESULT || a == DECIMAL_RESULT) &&
      (b == INT_RESULT || b == DECIMAL_RESULT)) {
    return DECIMAL_RESULT;
  }
  return REAL_RESULT;
}

/**
  Substitute a const item with a simpler const item, if possible.

  @param thd         Current session.
  @param[in,out] ref Const item to be processed, contains simplest possible
                     item on return.
  @param comp_item   Item that provides result type for generated const item

  @returns false if success, true if error
*/

bool resolve_const_item(THD *thd, Item **ref, Item *comp_item) {
  Item *item = *ref;
  DBUG_ASSERT(item->const_item());

  Item *new_item = nullptr;
  if (item->basic_const_item()) return false;  // Can't be better
  Item_result res_type =
      item_cmp_type(comp_item->result_type(), item->result_type());
  switch (res_type) {
    case STRING_RESULT: {
      if (item->data_type() == MYSQL_TYPE_JSON) {
        auto wr = make_unique_destroy_only<Json_wrapper>(thd->mem_root);
        if (wr == nullptr) return true;
        if (item->val_json(wr.get())) return true;
        if (item->null_value)
          new_item = new Item_null(item->item_name);
        else
          new_item = new Item_json(std::move(wr), item->item_name);
        break;
      }
      char buff[MAX_FIELD_WIDTH];
      String tmp(buff, sizeof(buff), &my_charset_bin), *result;
      result = item->val_str(&tmp);
      if (thd->is_error()) return true;
      if (item->null_value)
        new_item = new Item_null(item->item_name);
      else if (item->is_temporal()) {
        enum_field_types type = item->data_type() == MYSQL_TYPE_TIMESTAMP
                                    ? MYSQL_TYPE_DATETIME
                                    : item->data_type();
        new_item = create_temporal_literal(thd, result->ptr(), result->length(),
                                           result->charset(), type, true);
      } else {
        size_t length = result->length();
        char *tmp_str = sql_strmake(result->ptr(), length);
        new_item = new Item_string(item->item_name, tmp_str, length,
                                   result->charset());
      }
      break;
    }
    case INT_RESULT: {
      longlong result = item->val_int();
      if (thd->is_error()) return true;
      uint length = item->max_length;
      bool null_value = item->null_value;
      if (null_value)
        new_item = new Item_null(item->item_name);
      else if (item->unsigned_flag)
        new_item = new Item_uint(item->item_name, result, length);
      else
        new_item = new Item_int(item->item_name, result, length);
      break;
    }
    case ROW_RESULT: {
      /*
        Substitute constants only in Item_rows. Don't affect other Items
        with ROW_RESULT (eg Item_singlerow_subselect).

        For such Items more optimal is to detect if it is constant and replace
        it with Item_row. This would optimize queries like this:
        SELECT * FROM t1 WHERE (a,b) = (SELECT a,b FROM t2 LIMIT 1);
      */
      if (!(item->type() == Item::ROW_ITEM &&
            comp_item->type() == Item::ROW_ITEM))
        return false;
      Item_row *item_row = (Item_row *)item;
      Item_row *comp_item_row = (Item_row *)comp_item;
      /*
        If item and comp_item are both Item_rows and have same number of cols
        then process items in Item_row one by one.
        We can't ignore NULL values here as this item may be used with <=>, in
        which case NULL's are significant.
      */
      DBUG_ASSERT(item->result_type() == comp_item->result_type());
      DBUG_ASSERT(item_row->cols() == comp_item_row->cols());
      uint col = item_row->cols();
      while (col-- > 0)
        if (resolve_const_item(thd, item_row->addr(col),
                               comp_item_row->element_index(col)))
          return true;
      break;
    }
    case REAL_RESULT: {  // It must REAL_RESULT
      double result = item->val_real();
      if (thd->is_error()) return true;
      uint length = item->max_length, decimals = item->decimals;
      bool null_value = item->null_value;
      new_item = (null_value ? (Item *)new Item_null(item->item_name)
                             : (Item *)new Item_float(item->item_name, result,
                                                      decimals, length));
      break;
    }
    case DECIMAL_RESULT: {
      my_decimal decimal_value;
      my_decimal *result = item->val_decimal(&decimal_value);
      if (thd->is_error()) return true;
      bool null_value = item->null_value;
      new_item = (null_value ? (Item *)new Item_null(item->item_name)
                             : (Item *)new Item_decimal(item->item_name, result,
                                                        item->decimals,
                                                        item->max_length));
      break;
    }
    default:
      DBUG_ASSERT(0);
  }
  if (new_item == nullptr) return true;

  thd->change_item_tree(ref, new_item);

  return false;
}

/**
  Compare the value stored in field with the expression from the query.

  @param thd     Current session.
  @param field   Field which the Item is stored in after conversion
  @param item    Original expression from query

  @return Returns an integer greater than, equal to, or less than 0 if
          the value stored in the field is greater than, equal to,
          or less than the original Item. A 0 may also be returned if
          out of memory.

  @note We use this in the range optimizer/partition pruning,
        because in some cases we can't store the value in the field
        without some precision/character loss.

        We similarly use it to verify that expressions like
        BIGINT_FIELD @<cmp@> @<literal value@>
        is done correctly (as int/decimal/float according to literal type).
*/

int stored_field_cmp_to_item(THD *thd, Field *field, Item *item) {
  Item_result res_type =
      item_cmp_type(field->result_type(), item->result_type());
  if (field->type() == MYSQL_TYPE_TIME &&
      item->data_type() == MYSQL_TYPE_TIME) {
    longlong field_value = field->val_time_temporal();
    longlong item_value = item->val_time_temporal();
    return field_value < item_value ? -1 : field_value > item_value ? 1 : 0;
  }
  if (is_temporal_type_with_date(field->type()) && item->is_temporal()) {
    /*
      Note, in case of TIME data type we also go here
      and call item->val_date_temporal(), because we want
      TIME to be converted to DATE/DATETIME properly.
      Only non-temporal data types go though get_mysql_time_from_str()
      in the below code branch.
    */
    longlong field_value = field->val_date_temporal();
    longlong item_value = item->val_date_temporal();
    return field_value < item_value ? -1 : field_value > item_value ? 1 : 0;
  }
  if (res_type == STRING_RESULT) {
    char item_buff[MAX_FIELD_WIDTH];
    char field_buff[MAX_FIELD_WIDTH];

    String item_tmp(item_buff, sizeof(item_buff), &my_charset_bin);
    String field_tmp(field_buff, sizeof(field_buff), &my_charset_bin);
    String *item_result = item->val_str(&item_tmp);
    /*
      Some implementations of Item::val_str(String*) actually modify
      the field Item::null_value, hence we can't check it earlier.
    */
    if (item->null_value) return 0;
    String *field_result = field->val_str(&field_tmp);

    if (is_temporal_type_with_date(field->type())) {
      enum_mysql_timestamp_type type =
          field_type_to_timestamp_type(field->type());
      const char *field_name = field->field_name;
      MYSQL_TIME field_time, item_time;
      get_mysql_time_from_str(thd, field_result, type, field_name, &field_time);
      get_mysql_time_from_str(thd, item_result, type, field_name, &item_time);

      return my_time_compare(field_time, item_time);
    }
    return sortcmp(field_result, item_result, field->charset());
  }
  if (res_type == INT_RESULT) return 0;  // Both are of type int
  if (res_type == DECIMAL_RESULT) {
    my_decimal item_buf, *item_val, field_buf, *field_val;
    item_val = item->val_decimal(&item_buf);
    if (item->null_value) return 0;
    field_val = field->val_decimal(&field_buf);
    return my_decimal_cmp(field_val, item_val);
  }
  /*
    The patch for Bug#13463415 started using this function for comparing
    BIGINTs. That uncovered a bug in Visual Studio 32bit optimized mode.
    Prefixing the auto variables with volatile fixes the problem....
  */
  volatile double result = item->val_real();
  if (item->null_value) return 0;
  volatile double field_result = field->val_real();
  if (field_result < result)
    return -1;
  else if (field_result > result)
    return 1;
  return 0;
}

Item_cache *Item_cache::get_cache(const Item *item) {
  return get_cache(item, item->result_type());
}

/**
  Get a cache item of given type.

  @param item         value to be cached
  @param type         required type of cache

  @return cache item
*/

Item_cache *Item_cache::get_cache(const Item *item, const Item_result type) {
  switch (type) {
    case INT_RESULT:
      return new Item_cache_int(item->data_type());
    case REAL_RESULT:
      return new Item_cache_real();
    case DECIMAL_RESULT:
      return new Item_cache_decimal();
    case STRING_RESULT:
      /* Not all functions that return DATE/TIME are actually DATE/TIME funcs.
       */
      if (item->is_temporal())
        return new Item_cache_datetime(item->data_type());
      if (item->data_type() == MYSQL_TYPE_JSON) return new Item_cache_json();
      return new Item_cache_str(item);
    case ROW_RESULT:
      return new Item_cache_row();
    default:
      // should never be in real life
      DBUG_ASSERT(0);
      return nullptr;
  }
}

void Item_cache::store(Item *item) {
  example = item;
  if (!item) {
    DBUG_ASSERT(maybe_null);
    null_value = true;
  }
  value_cached = false;
}

void Item_cache::print(const THD *thd, String *str,
                       enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("<cache>("));
  if (example)
    example->print(thd, str, query_type);
  else
    Item::print(thd, str, query_type);
  str->append(')');
}

bool Item_cache::walk(Item_processor processor, enum_walk walk, uchar *arg) {
  return ((walk & enum_walk::PREFIX) && (this->*processor)(arg)) ||
         (example && example->walk(processor, walk, arg)) ||
         ((walk & enum_walk::POSTFIX) && (this->*processor)(arg));
}

bool Item_cache::has_value() {
  if (value_cached || cache_value()) {
    /*
      Only expect NULL if the cache is nullable, or if an error was
      raised when reading the value into the cache.
    */
    DBUG_ASSERT(!null_value || maybe_null || current_thd->is_error());
    return !null_value;
  }
  return false;
}

bool Item_cache_int::cache_value() {
  if (!example) return false;
  value_cached = true;
  value = example->val_int();
  null_value = example->null_value;
  unsigned_flag = example->unsigned_flag;
  return true;
}

void Item_cache_int::store_value(Item *item, longlong val_arg) {
  /* An explicit values is given, save it. */
  value_cached = true;
  value = val_arg;
  null_value = item->null_value;
  unsigned_flag = item->unsigned_flag;
}

String *Item_cache_int::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  str->set_int(value, unsigned_flag, default_charset());
  return str;
}

my_decimal *Item_cache_int::val_decimal(my_decimal *decimal_val) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  int2my_decimal(E_DEC_FATAL_ERROR, value, unsigned_flag, decimal_val);
  return decimal_val;
}

double Item_cache_int::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return 0.0;
  if (unsigned_flag) return static_cast<unsigned long long>(value);
  return value;
}

longlong Item_cache_int::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return 0;
  return value;
}

bool Item_cache_datetime::cache_value_int() {
  if (!example) return false;

  value_cached = true;
  // Mark cached string value obsolete
  str_value_cached = false;

  DBUG_ASSERT(data_type() == example->data_type());
  int_value = example->val_temporal_by_field_type();
  null_value = example->null_value;
  unsigned_flag = example->unsigned_flag;

  return true;
}

bool Item_cache_datetime::cache_value() {
  if (!example) return false;

  if (cmp_context == INT_RESULT) return cache_value_int();

  str_value_cached = true;
  // Mark cached int value obsolete
  value_cached = false;
  /* Assume here that the underlying item will do correct conversion.*/
  String *res = example->val_str(&cached_string);
  if (res && res != &cached_string) cached_string.copy(*res);
  null_value = example->null_value;
  unsigned_flag = example->unsigned_flag;
  return true;
}

void Item_cache_datetime::store_value(Item *item, longlong val_arg) {
  /* An explicit values is given, save it. */
  value_cached = true;
  int_value = val_arg;
  null_value = item->null_value;
  unsigned_flag = item->unsigned_flag;
}

void Item_cache_datetime::store(Item *item) {
  Item_cache::store(item);
  str_value_cached = false;
}

String *Item_cache_datetime::val_str(String *) {
  DBUG_ASSERT(fixed == 1);

  if ((value_cached || str_value_cached) && null_value) return nullptr;

  if (!str_value_cached) {
    /*
      When it's possible the Item_cache_datetime uses INT datetime
      representation due to speed reasons. But still, it always has the STRING
      result type and thus it can be asked to return a string value.
      It is possible that at this time cached item doesn't contain correct
      string value, thus we have to convert cached int value to string and
      return it.
    */
    if (value_cached) {
      MYSQL_TIME ltime;
      TIME_from_longlong_packed(&ltime, data_type(), int_value);
      if ((null_value =
               my_TIME_to_str(&ltime, &cached_string,
                              min(decimals, uint8{DATETIME_MAX_DECIMALS}))))
        return nullptr;
      str_value_cached = true;
    } else if (!cache_value() || null_value)
      return nullptr;
  }
  return &cached_string;
}

my_decimal *Item_cache_datetime::val_decimal(my_decimal *decimal_val) {
  DBUG_ASSERT(fixed == 1);

  if (str_value_cached) {
    switch (data_type()) {
      case MYSQL_TYPE_TIME:
        return val_decimal_from_time(decimal_val);
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATE:
        return val_decimal_from_date(decimal_val);
      default:
        DBUG_ASSERT(0);
        return nullptr;
    }
  }

  if ((!value_cached && !cache_value_int()) || null_value) return nullptr;
  return my_decimal_from_datetime_packed(decimal_val, data_type(), int_value);
}

bool Item_cache_datetime::get_date(MYSQL_TIME *ltime,
                                   my_time_flags_t fuzzydate) {
  if ((value_cached || str_value_cached) && null_value) return true;

  if (str_value_cached)  // TS-TODO: reuse MYSQL_TIME_cache eventually.
    return get_date_from_string(ltime, fuzzydate);

  if ((!value_cached && !cache_value_int()) || null_value)
    return (null_value = true);

  switch (data_type()) {
    case MYSQL_TYPE_TIME: {
      MYSQL_TIME tm;
      TIME_from_longlong_time_packed(&tm, int_value);
      time_to_datetime(current_thd, &tm, ltime);
      return false;
    }
    case MYSQL_TYPE_DATE: {
      int warnings = 0;
      TIME_from_longlong_date_packed(ltime, int_value);
      return check_date(*ltime, non_zero_date(*ltime), fuzzydate, &warnings);
    }
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP: {
      int warnings = 0;
      TIME_from_longlong_datetime_packed(ltime, int_value);
      return check_date(*ltime, non_zero_date(*ltime), fuzzydate, &warnings);
    }
    default:
      DBUG_ASSERT(0);
  }
  return true;
}

bool Item_cache_datetime::get_time(MYSQL_TIME *ltime) {
  if ((value_cached || str_value_cached) && null_value) return true;

  if (str_value_cached)  // TS-TODO: reuse MYSQL_TIME_cache eventually.
    return get_time_from_string(ltime);

  if ((!value_cached && !cache_value_int()) || null_value) return true;

  switch (data_type()) {
    case MYSQL_TYPE_TIME:
      TIME_from_longlong_time_packed(ltime, int_value);
      return false;
    case MYSQL_TYPE_DATE:
      set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
      return false;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      TIME_from_longlong_datetime_packed(ltime, int_value);
      datetime_to_time(ltime);
      return false;
    default:
      DBUG_ASSERT(0);
  }
  return true;
}

double Item_cache_datetime::val_real() { return val_real_from_decimal(); }

longlong Item_cache_datetime::val_time_temporal() {
  DBUG_ASSERT(fixed == 1);
  if ((!value_cached && !cache_value_int()) || null_value) return 0;
  if (is_temporal_with_date()) {
    /* Convert packed date to packed time */
    MYSQL_TIME ltime;
    return get_time_from_date(&ltime)
               ? 0
               : TIME_to_longlong_packed(ltime, data_type());
  }
  return int_value;
}

longlong Item_cache_datetime::val_date_temporal() {
  DBUG_ASSERT(fixed == 1);
  if ((!value_cached && !cache_value_int()) || null_value) return 0;
  if (data_type() == MYSQL_TYPE_TIME) {
    /* Convert packed time to packed date */
    MYSQL_TIME ltime;
    return get_date_from_time(&ltime) ? 0
                                      : TIME_to_longlong_datetime_packed(ltime);
  }
  return int_value;
}

longlong Item_cache_datetime::val_int() { return val_int_from_decimal(); }

Item_cache_json::Item_cache_json()
    : Item_cache(MYSQL_TYPE_JSON),
      m_value(new (*THR_MALLOC) Json_wrapper()),
      m_is_sorted(false) {}

Item_cache_json::~Item_cache_json() { destroy(m_value); }

/**
  Read the JSON value and cache it.
  @return true if the value was successfully cached, false otherwise
*/
bool Item_cache_json::cache_value() {
  if (!example || !m_value) return false;

  value_cached = !json_value(&example, 0, m_value);
  null_value = example->null_value;

  if (value_cached && !null_value) {
    // the row buffer might change, so need own copy
    m_value->to_dom(current_thd);
  }
  m_is_sorted = false;
  return value_cached;
}

void Item_cache_json::store_value(Item *expr, Json_wrapper *wr) {
  value_cached = true;
  if ((null_value = expr->null_value))
    m_value = nullptr;
  else {
    *m_value = *wr;
    // the row buffer might change, so need own copy
    m_value->to_dom(current_thd);
  }
  m_is_sorted = false;
}

/**
  Copy the cached JSON value into a wrapper.
  @param[out] wr the wrapper that receives the JSON value
*/
bool Item_cache_json::val_json(Json_wrapper *wr) {
  if (has_value()) *wr = *m_value;
  return false;
}

/// Get the name of the cached field of an Item_cache_json instance.
inline static const char *whence(const Field *cached_field) {
  return cached_field ? cached_field->field_name : "?";
}

String *Item_cache_json::val_str(String *tmp) {
  if (has_value()) {
    tmp->length(0);
    m_value->to_string(tmp, true, whence(cached_field));
    return tmp;
  }

  return nullptr;
}

double Item_cache_json::val_real() {
  Json_wrapper wr;

  if (val_json(&wr)) return 0.0;

  if (null_value) return 0.0;

  return wr.coerce_real(whence(cached_field));
}

my_decimal *Item_cache_json::val_decimal(my_decimal *decimal_value) {
  Json_wrapper wr;

  if (val_json(&wr)) return decimal_value;

  if (null_value) return decimal_value;

  return wr.coerce_decimal(decimal_value, whence(cached_field));
}

bool Item_cache_json::get_date(MYSQL_TIME *ltime, my_time_flags_t) {
  Json_wrapper wr;

  if (val_json(&wr)) return true;

  if (null_value) return true;

  return wr.coerce_date(ltime, whence(cached_field));
}

bool Item_cache_json::get_time(MYSQL_TIME *ltime) {
  Json_wrapper wr;

  if (val_json(&wr)) return true;

  if (null_value) return true;

  return wr.coerce_time(ltime, whence(cached_field));
}

longlong Item_cache_json::val_int() {
  Json_wrapper wr;
  if (val_json(&wr)) return 0;

  if (null_value) return true;

  return wr.coerce_int(whence(cached_field));
}

void Item_cache_json::sort() {
  DBUG_ASSERT(!m_is_sorted);
  if (has_value() && m_value->type() == enum_json_type::J_ARRAY) {
    m_value->sort();
    m_is_sorted = true;
  }
}

bool Item_cache_real::cache_value() {
  if (!example) return false;
  value_cached = true;
  value = example->val_real();
  null_value = example->null_value;
  return true;
}

void Item_cache_real::store_value(Item *expr, double d) {
  value_cached = true;
  value = d;
  null_value = expr->null_value;
}

double Item_cache_real::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return 0.0;
  return value;
}

longlong Item_cache_real::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return 0;
  return (longlong)rint(value);
}

String *Item_cache_real::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  str->set_real(value, decimals, default_charset());
  return str;
}

my_decimal *Item_cache_real::val_decimal(my_decimal *decimal_val) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  double2my_decimal(E_DEC_FATAL_ERROR, value, decimal_val);
  return decimal_val;
}

bool Item_cache_decimal::cache_value() {
  if (!example) return false;
  value_cached = true;
  my_decimal *val = example->val_decimal(&decimal_value);
  if (!(null_value = example->null_value) && val != &decimal_value)
    my_decimal2decimal(val, &decimal_value);
  return true;
}

void Item_cache_decimal::store_value(Item *expr, my_decimal *d) {
  value_cached = true;
  null_value = expr->null_value;
  my_decimal cpy(*d);
  decimal_value.swap(cpy);
}

double Item_cache_decimal::val_real() {
  DBUG_ASSERT(fixed);
  double res;
  if (!has_value()) return 0.0;
  my_decimal2double(E_DEC_FATAL_ERROR, &decimal_value, &res);
  return res;
}

longlong Item_cache_decimal::val_int() {
  DBUG_ASSERT(fixed);
  longlong res;
  if (!has_value()) return 0;
  my_decimal2int(E_DEC_FATAL_ERROR, &decimal_value, unsigned_flag, &res);
  return res;
}

String *Item_cache_decimal::val_str(String *str) {
  DBUG_ASSERT(fixed);
  if (!has_value()) return nullptr;
  my_decimal_round(E_DEC_FATAL_ERROR, &decimal_value, decimals, false,
                   &decimal_value);
  my_decimal2string(E_DEC_FATAL_ERROR, &decimal_value, str);
  return str;
}

my_decimal *Item_cache_decimal::val_decimal(my_decimal *) {
  DBUG_ASSERT(fixed);
  if (!has_value()) return nullptr;
  return &decimal_value;
}

bool Item_cache_str::cache_value() {
  if (!example) return false;
  value_cached = true;
  value_buff.set(buffer, sizeof(buffer), example->collation.collation);
  value = example->val_str(&value_buff);
  if ((null_value = example->null_value))
    value = nullptr;
  else if (value != nullptr && value->ptr() != buffer) {
    /*
      We copy string value to avoid changing value if 'item' is table field
      in queries like following (where t1.c is varchar):
      select a,
             (select a,b,c from t1 where t1.a=t2.a) = ROW(a,2,'a'),
             (select c from t1 where a=t2.a)
        from t2;
    */
    value_buff.copy(*value);
    value = &value_buff;
  }
  return true;
}

void Item_cache_str::store_value(Item *expr, String &s) {
  value_cached = true;
  if ((null_value = expr->null_value))
    value = nullptr;
  else {
    value_buff.copy(s);
    value = &value_buff;
  }
}

double Item_cache_str::val_real() {
  DBUG_ASSERT(fixed == 1);
  int err_not_used;
  const char *end_not_used;
  if (!has_value()) return 0.0;
  if (value)
    return my_strntod(value->charset(), value->ptr(), value->length(),
                      &end_not_used, &err_not_used);
  return (double)0;
}

longlong Item_cache_str::val_int() {
  DBUG_ASSERT(fixed == 1);
  int err;
  if (!has_value()) return 0;
  if (value)
    return my_strntoll(value->charset(), value->ptr(), value->length(), 10,
                       nullptr, &err);
  else
    return (longlong)0;
}

String *Item_cache_str::val_str(String *) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  return value;
}

my_decimal *Item_cache_str::val_decimal(my_decimal *decimal_val) {
  DBUG_ASSERT(fixed == 1);
  if (!has_value()) return nullptr;
  if (value)
    str2my_decimal(E_DEC_FATAL_ERROR, value->ptr(), value->length(),
                   value->charset(), decimal_val);
  else
    decimal_val = nullptr;
  return decimal_val;
}

type_conversion_status Item_cache_str::save_in_field_inner(
    Field *field, bool no_conversions) {
  if (!value_cached && !cache_value())
    return TYPE_ERR_BAD_VALUE;  // Fatal: couldn't cache the value
  if (null_value)
    return set_field_to_null_with_conversions(field, no_conversions);
  const type_conversion_status res =
      Item_cache::save_in_field_inner(field, no_conversions);
  if (is_varbinary && field->type() == MYSQL_TYPE_STRING && value != nullptr &&
      value->length() < field->field_length)
    return TYPE_WARN_OUT_OF_RANGE;
  return res;
}

bool Item_cache_row::allocate(uint num) {
  item_count = num;
  THD *thd = current_thd;
  return (!(values = (Item_cache **)thd->mem_calloc(sizeof(Item_cache *) *
                                                    item_count)));
}

bool Item_cache_row::setup(Item *item) {
  example = item;
  if (!values && allocate(item->cols())) return true;
  for (uint i = 0; i < item_count; i++) {
    Item *el = item->element_index(i);
    Item_cache *tmp;
    if (!(tmp = values[i] = Item_cache::get_cache(el))) return true;
    tmp->setup(el);
    add_accum_properties(tmp);
  }
  return false;
}

void Item_cache_row::store(Item *item) {
  example = item;
  if (!item) {
    DBUG_ASSERT(maybe_null);
    null_value = true;
    return;
  }
  for (uint i = 0; i < item_count; i++)
    values[i]->store(item->element_index(i));
}

bool Item_cache_row::cache_value() {
  if (!example) return false;
  value_cached = true;
  example->bring_value();
  null_value = example->null_value;

  const bool cached_item_is_assigned =
      example->type() != SUBSELECT_ITEM ||
      down_cast<Item_subselect *>(example)->assigned();

  for (uint i = 0; i < item_count; i++) {
    if (!cached_item_is_assigned) {
      // Subquery with zero rows, so make cached item null also.
      values[i]->store_null();
    } else {
      values[i]->cache_value();
    }

    null_value |= values[i]->null_value;
  }
  return true;
}

void Item_cache_row::illegal_method_call(
    const char *method MY_ATTRIBUTE((unused))) const {
  DBUG_TRACE;
  DBUG_PRINT("error", ("!!! %s method was called for row item", method));
  DBUG_ASSERT(0);
  my_error(ER_OPERAND_COLUMNS, MYF(0), 1);
}

bool Item_cache_row::check_cols(uint c) {
  if (c != item_count) {
    my_error(ER_OPERAND_COLUMNS, MYF(0), c);
    return true;
  }
  return false;
}

bool Item_cache_row::null_inside() {
  for (uint i = 0; i < item_count; i++) {
    if (values[i]->cols() > 1) {
      if (values[i]->null_inside()) return true;
    } else {
      /*
        TODO : Implement error handling for this function as
        update_null_value() can return error.
      */
      values[i]->update_null_value();
      if (values[i]->null_value) return true;
    }
  }
  return false;
}

void Item_cache_row::bring_value() {
  if (!example) return;
  example->bring_value();
  null_value = example->null_value;
  for (uint i = 0; i < item_count; i++) values[i]->bring_value();
}

Item_aggregate_type::Item_aggregate_type(THD *thd, Item *item)
    : Item(thd, item), enum_set_typelib(nullptr) {
  DBUG_ASSERT(item->fixed);
  maybe_null = item->maybe_null;
  set_data_type(real_data_type(item));
  get_full_info(item);
  if (item->data_type() == MYSQL_TYPE_GEOMETRY)
    geometry_type = item->get_geometry_type();
  else
    geometry_type = Field::GEOM_GEOMETRY;
}

/**
  Return expression type of Item_aggregate_type.

  @return
    Item_result (type of internal MySQL expression result)
*/

Item_result Item_aggregate_type::result_type() const {
  return Field::result_merge_type(data_type());
}

/**
  Find real data type of item.

  @return
    data type which should be used to store item value
*/

enum_field_types Item_aggregate_type::real_data_type(Item *item) {
  item = item->real_item();

  switch (item->type()) {
    case FIELD_ITEM: {
      /*
        Item_fields::field_type ask Field_type() but sometimes field return
        a different type, like for enum/set, so we need to ask real type.
      */
      Field *field = ((Item_field *)item)->field;
      enum_field_types type = field->real_type();
      if (field->is_created_from_null_item) return MYSQL_TYPE_NULL;
      /* work around about varchar type field detection */
      if (type == MYSQL_TYPE_STRING && field->type() == MYSQL_TYPE_VAR_STRING)
        return MYSQL_TYPE_VAR_STRING;
      return type;
    }
    case SUM_FUNC_ITEM: {
      /*
        Argument of aggregate function sometimes should be asked about field
        type
      */
      Item_sum *item_sum = (Item_sum *)item;
      if (item_sum->keep_field_type())
        return real_data_type(item_sum->get_arg(0));
      break;
    }
    case FUNC_ITEM:
      if (((Item_func *)item)->functype() == Item_func::GUSERVAR_FUNC) {
        /*
          There are work around of problem with changing variable type on the
          fly and variable always report "string" as field type to get
          acceptable information for client in send_field, so we make field
          type from expression type.
        */
        switch (item->result_type()) {
          case STRING_RESULT:
            return MYSQL_TYPE_VARCHAR;
          case INT_RESULT:
            return MYSQL_TYPE_LONGLONG;
          case REAL_RESULT:
            return MYSQL_TYPE_DOUBLE;
          case DECIMAL_RESULT:
            return MYSQL_TYPE_NEWDECIMAL;
          case ROW_RESULT:
          default:
            DBUG_ASSERT(0);
            return MYSQL_TYPE_VARCHAR;
        }
      }
      break;
    default:
      break;
  }
  return item->data_type();
}

/**
  Find field type which can carry current Item_aggregate_type type and
  type of given Item.

  @param thd     the thread/connection descriptor
  @param item    given item to join its parameters with this item ones

  @retval
    true   error - types are incompatible
  @retval
    false  OK
*/

bool Item_aggregate_type::join_types(THD *thd, Item *item) {
  DBUG_TRACE;
  DBUG_PRINT("info:",
             ("was type %d len %d, dec %d name %s", data_type(), max_length,
              decimals, (item_name.is_set() ? item_name.ptr() : "<NULL>")));
  DBUG_PRINT("info:", ("in type %d len %d, dec %d", real_data_type(item),
                       item->max_length, item->decimals));
  /*
    aggregate_type() will modify the data type of this item. Create a copy of
    this item containing the original data type and other properties to ensure
    correct conversion from existing item types to aggregated type.
  */
  Item *item_copy = Item_copy::create(this);

  /*
    Down the call stack when calling aggregate_string_properties(), we might
    end up in THD::change_item_tree() if we for instance need to convert the
    character set on one side of a union:

      SELECT "foo" UNION SELECT CONVERT("foo" USING utf8mb3);
    might be converted into:
      SELECT CONVERT("foo" USING utf8mb3) UNION
      SELECT CONVERT("foo" USING utf8mb3);

    If we are in a prepared statement or a stored routine (any non-conventional
    query that needs rollback of any item tree modifications), we neeed to
    remember what Item we changed ("foo" in this case) and where that Item is
    located (in the "args" array in this case) so we can roll back the changes
    done to the Item tree when the execution is done. When we enter the rollback
    code (THD::rollback_item_tree_changes()), the location of the Item need to
    be accessible, so that is why the "args" array must be allocated on a
    MEM_ROOT and not on the stack. Note that THD::change_item_tree() isn't
    necessary, since the Item array we are modifying isn't a part of the
    original Item tree.
  */
  Item **args = new (thd->mem_root) Item *[2] { item_copy, item };
  aggregate_type(make_array(&args[0], 2));
  // UNION with ENUM/SET fields requires type information from real_data_type()
  set_data_type(real_type_to_type(Field::field_type_merge(
      real_data_type(item_copy), real_data_type(item))));

  Item_result merge_type = Field::result_merge_type(data_type());
  if (merge_type == STRING_RESULT) {
    if (aggregate_string_properties("UNION", args, 2)) return true;
    /*
      For geometry columns, we must also merge subtypes. If the
      subtypes are different, use GEOMETRY.
    */
    if (data_type() == MYSQL_TYPE_GEOMETRY &&
        (item->data_type() != MYSQL_TYPE_GEOMETRY ||
         geometry_type != item->get_geometry_type()))
      geometry_type = Field::GEOM_GEOMETRY;
  } else
    aggregate_num_type(merge_type, args, 2);
  maybe_null |= item->maybe_null;
  get_full_info(item);
  DBUG_PRINT("info", ("become type: %d  len: %u  dec: %u", (int)data_type(),
                      max_length, (uint)decimals));
  return false;
}

/**
  Calculate lenth for merging result for given Item type.

  @param item  Item for length detection

  @return
    length
*/

uint32 Item_aggregate_type::display_length(Item *item) {
  if (item->type() == Item::FIELD_ITEM)
    return ((Item_field *)item)->max_disp_length();

  switch (item->data_type()) {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
      return item->max_length;
    case MYSQL_TYPE_TINY:
      return 4;
    case MYSQL_TYPE_SHORT:
      return 6;
    case MYSQL_TYPE_LONG:
      return MY_INT32_NUM_DECIMAL_DIGITS;
    case MYSQL_TYPE_FLOAT:
      return 25;
    case MYSQL_TYPE_DOUBLE:
      return 53;
    case MYSQL_TYPE_NULL:
      return 0;
    case MYSQL_TYPE_LONGLONG:
      return 20;
    case MYSQL_TYPE_INT24:
      return 8;
    default:
      DBUG_ASSERT(0);  // we should never go there
      return 0;
  }
}

/**
  Make temporary table field according collected information about type
  of UNION result.

  @param table  temporary table for which we create fields
  @param strict If strict mode is on

  @return
    created field
*/

Field *Item_aggregate_type::make_field_by_type(TABLE *table, bool strict) {
  /*
    The field functions defines a field to be not null if null_ptr is not 0
  */
  Field *field;

  switch (data_type()) {
    case MYSQL_TYPE_ENUM:
      DBUG_ASSERT(enum_set_typelib);
      field = new (*THR_MALLOC)
          Field_enum(max_length, maybe_null, item_name.ptr(),
                     get_enum_pack_length(enum_set_typelib->count),
                     enum_set_typelib, collation.collation);
      if (field) field->init(table);
      break;
    case MYSQL_TYPE_SET:
      DBUG_ASSERT(enum_set_typelib);
      field = new (*THR_MALLOC)
          Field_set(max_length, maybe_null, item_name.ptr(),
                    get_set_pack_length(enum_set_typelib->count),
                    enum_set_typelib, collation.collation);
      if (field) field->init(table);
      break;
    case MYSQL_TYPE_NULL:
      field = make_string_field(table);
      break;
    default:
      field = tmp_table_field_from_field_type(table, false);
      break;
  }
  if (field == nullptr) return nullptr;

  if (strict && is_temporal_type_with_date(field->type()) &&
      !field->is_nullable()) {
    /*
      This function is used for CREATE SELECT UNION [ALL] ... , and, if
      expression is non-nullable, the resulting column is declared
      non-nullable with a default of 0. However, in strict mode, for dates,
      0000-00-00 is invalid; in that case, don't give any default.
    */
    field->flags |= NO_DEFAULT_VALUE_FLAG;
  }
  field->set_derivation(collation.derivation);
  return field;
}

/**
  Get full information from Item about enum/set fields to be able to create
  them later.

  @param item    Item for information collection
*/
void Item_aggregate_type::get_full_info(Item *item) {
  if (data_type() == MYSQL_TYPE_ENUM || data_type() == MYSQL_TYPE_SET) {
    if (item->type() == Item::SUM_FUNC_ITEM &&
        (((Item_sum *)item)->sum_func() == Item_sum::MAX_FUNC ||
         ((Item_sum *)item)->sum_func() == Item_sum::MIN_FUNC))
      item = (down_cast<Item_sum *>(item))->get_arg(0);
    /*
      We can have enum/set type after merging only if we have one enum|set
      field (or MIN|MAX(enum|set field)) and number of NULL fields
    */
    if (enum_set_typelib) {
      DBUG_ASSERT(real_data_type(item) == MYSQL_TYPE_NULL);
    } else {
      Item *real_item = item->real_item();
      Item_field *item_field = down_cast<Item_field *>(real_item);
      Field_enum *field_enum = down_cast<Field_enum *>(item_field->field);
      DBUG_ASSERT((real_data_type(item) == MYSQL_TYPE_ENUM ||
                   real_data_type(item) == MYSQL_TYPE_SET) &&
                  field_enum->typelib);
      enum_set_typelib = field_enum->typelib;
    }
  }
}

double Item_type_holder::val_real() {
  DBUG_ASSERT(0);  // should never be called
  return 0.0;
}

longlong Item_type_holder::val_int() {
  DBUG_ASSERT(0);  // should never be called
  return 0;
}

my_decimal *Item_type_holder::val_decimal(my_decimal *) {
  DBUG_ASSERT(0);  // should never be called
  return nullptr;
}

String *Item_type_holder::val_str(String *) {
  DBUG_ASSERT(0);  // should never be called
  return nullptr;
}

bool Item_type_holder::get_date(MYSQL_TIME *, my_time_flags_t) {
  DBUG_ASSERT(0);
  return true;
}

bool Item_type_holder::get_time(MYSQL_TIME *) {
  DBUG_ASSERT(0);
  return true;
}

type_conversion_status Item_values_column::save_in_field_inner(
    Field *to, bool no_conversions) {
  type_conversion_status res;
  res = m_value_ref->save_in_field(to, no_conversions);
  null_value = m_value_ref->null_value;
  return res;
}

Item_values_column::Item_values_column(THD *thd, Item *ref) : super(thd, ref) {
  fixed = true;
}

/* purecov: begin deadcode */

bool Item_values_column::eq(const Item *item, bool binary_cmp) const {
  DBUG_ASSERT(false);
  const Item *it = const_cast<Item *>(item)->real_item();
  return m_value_ref && m_value_ref->eq(it, binary_cmp);
}

/* purecov: end */

double Item_values_column::val_real() {
  DBUG_ASSERT(fixed);
  double tmp = m_value_ref->val_real();
  null_value = m_value_ref->null_value;
  return tmp;
}

longlong Item_values_column::val_int() {
  DBUG_ASSERT(fixed);
  longlong tmp = m_value_ref->val_int();
  null_value = m_value_ref->null_value;
  return tmp;
}

/* purecov: begin deadcode */

my_decimal *Item_values_column::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(false);
  DBUG_ASSERT(fixed);
  my_decimal *val = m_value_ref->val_decimal(decimal_value);
  null_value = m_value_ref->null_value;
  return val;
}

bool Item_values_column::val_bool() {
  DBUG_ASSERT(false);
  DBUG_ASSERT(fixed);
  bool tmp = m_value_ref->val_bool();
  null_value = m_value_ref->null_value;
  return tmp;
}

bool Item_values_column::val_json(Json_wrapper *result) {
  DBUG_ASSERT(false);
  DBUG_ASSERT(fixed);
  bool ok = m_value_ref->val_json(result);
  null_value = m_value_ref->null_value;
  return ok;
}

/* purecov: end */

String *Item_values_column::val_str(String *tmp) {
  DBUG_ASSERT(fixed);
  tmp = m_value_ref->val_str(tmp);
  null_value = m_value_ref->null_value;
  return tmp;
}

bool Item_values_column::is_null() {
  DBUG_ASSERT(fixed);
  /*
    Item_values_column is dualistic in nature: It represents both a set
    of values, and, during evaluation, an individual value in this set.
    This assert will ensure that we only check nullability of individual
    values, since a set of values is never NULL. Note that setting
    RAND_TABLE_BIT in the constructor prevents this function from being called
    during resolving.
  */
  DBUG_ASSERT(m_value_ref != nullptr);
  bool tmp = m_value_ref->is_null();
  null_value = m_value_ref->null_value;
  return tmp;
}

bool Item_values_column::get_date(MYSQL_TIME *ltime,
                                  my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed);
  bool result = m_value_ref->get_date(ltime, fuzzydate);
  null_value = m_value_ref->null_value;
  return result;
}

bool Item_values_column::get_time(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed);
  DBUG_ASSERT(m_value_ref != nullptr);
  return m_value_ref->get_time(ltime);
}

void Item_values_column::add_used_tables(Item *value) {
  m_aggregated_used_tables |= value->used_tables();
}

void Item_result_field::cleanup() {
  DBUG_TRACE;
  Item::cleanup();
  result_field = nullptr;
}

void Item_result_field::raise_numeric_overflow(const char *type_name) {
  char buf[256];
  String str(buf, sizeof(buf), system_charset_info);
  str.length(0);
  print(current_thd, &str, QT_NO_DATA_EXPANSION);
  str.append('\0');
  my_error(ER_DATA_OUT_OF_RANGE, MYF(0), type_name, str.ptr());
}

/**
  Helper method: Convert string to the given charset, then print.

  @param from_str     String to be converted.
  @param to_str       Query string.
  @param to_cs        Character set to which the string is to be converted.
*/
void convert_and_print(const String *from_str, String *to_str,
                       const CHARSET_INFO *to_cs) {
  if (my_charset_same(from_str->charset(), to_cs)) {
    from_str->print(to_str);  // already in to_cs, no need to convert
  } else                      // need to convert
  {
    THD *thd = current_thd;
    LEX_STRING lex_str;
    thd->convert_string(&lex_str, to_cs, from_str->ptr(), from_str->length(),
                        from_str->charset());
    String tmp(lex_str.str, lex_str.length, to_cs);
    tmp.print(to_str);
  }
}

/**
   Tells if this is a column of a table whose qualifying query block is 'sl'.
   I.e. Item_field or Item_view_ref resolved in 'sl'. Used for
   aggregate checks.

   @note This returns false for an alias to a SELECT list expression,
   even though the SELECT list expression might itself be a column of the
   @<table expression@>; i.e. when the function runs on "foo" in HAVING of
   "select t1.a as foo from t1 having foo @> 1", it returns false. First, it
   pedantically makes sense: "foo" in HAVING is a reference to a column of the
   @<query expression@>, not of the @<table expression@>. Second, this behaviour
   makes sense for our purpose:
     - This is an alias to a SELECT list expression.
     - If doing DISTINCT-related checks, this alias can be ignored.
     - If doing GROUP-BY-related checks, the aliased expression was already
   checked when we checked the SELECT list, so can be ignored.

   @retval true3 yes
   @retval false3 no
   @retval unknown3 it's a non-direct-view Item_ref, we don't know if it
   contains a column => caller please analyze "*ref"
*/
Bool3 Item_ident::local_column(const SELECT_LEX *sl) const

{
  DBUG_ASSERT(fixed);
  if (m_alias_of_expr) return Bool3::false3();
  const Type t = type();
  if (t == FIELD_ITEM ||
      (t == REF_ITEM &&
       static_cast<const Item_ref *>(this)->ref_type() == Item_ref::VIEW_REF)) {
    if (depended_from)  // outer reference
    {
      if (depended_from == sl)
        return Bool3::true3();  // qualifying query is 'sl'
    } else if (context == nullptr) {
      /*
        Must be an underlying column of a generated column
        as we've dove so deep, we know the gcol is local to 'sl', and so is
        this column.
      */
      DBUG_ASSERT(t == FIELD_ITEM);
      return Bool3::true3();
    } else if (context->select_lex == sl)
      return Bool3::true3();  // qualifying query is 'sl'
  } else if (t == REF_ITEM) {
    /*
      We also know that this is not an alias. Must be an internal Item_ref
      (like Item_aggregate_ref, Item_outer_ref), go down into it:
    */
    return Bool3::unknown3();
  }
  return Bool3::false3();
}

bool Item_ident::aggregate_check_distinct(uchar *arg) {
  Distinct_check *const dc = reinterpret_cast<Distinct_check *>(arg);

  if (dc->is_stopped(this)) return false;

  SELECT_LEX *const sl = dc->select;
  const Bool3 local = local_column(sl);
  if (local.is_false()) {
    // not a column => ignored, skip child. Other tree parts deserve checking.
    dc->stop_at(this);
    return false;
  }
  if (local.is_unknown()) return false;  // dive in child item

  /*
    Point (2) of Distinct_check::check_query() is true: column is
    from table whose qualifying query block is 'sl'.
  */
  uint counter;
  enum_resolution_type resolution;
  Item **const res =
      find_item_in_list(current_thd, this, sl->item_list, &counter,
                        REPORT_EXCEPT_NOT_FOUND, &resolution);

  if (res == not_found_item) {
    /*
      Point (3) of Distinct_check::check_query() is true: column is
      not in SELECT list.
    */
    dc->failed_ident = this;
    // Abort processing of the entire item tree.
    return true;
  }
  /*
    If success, do not dive in the child either! Indeed if this is
    Item_.*view_ref to an expression coming from a merged view, we mustn't
    check its underlying base-table columns, it may give false errors,
    consider:
    create view v as select x*2 as b from ...;
    select distinct b from v order by b+1;
    'b' of ORDER BY is in SELECT list so query is valid, we mustn't check
    the underlying 'x' (which is not in SELECT list).
  */
  dc->stop_at(this);
  return false;
}

bool Item_ident::aggregate_check_group(uchar *arg) {
  Group_check *const gc = reinterpret_cast<Group_check *>(arg);
  return gc->do_ident_check(this, 0, Group_check::CHECK_GROUP);
}

bool Item_ident::is_strong_side_column_not_in_fd(uchar *arg) {
  std::pair<Group_check *, table_map> *p =
      reinterpret_cast<std::pair<Group_check *, table_map> *>(arg);
  // p->first is Group_check, p->second is map of strong tables.
  return p->first->do_ident_check(this, p->second,
                                  Group_check::CHECK_STRONG_SIDE_COLUMN);
}

bool Item_ident::is_column_not_in_fd(uchar *arg) {
  Group_check *const gc = reinterpret_cast<Group_check *>(arg);
  return gc->do_ident_check(this, 0, Group_check::CHECK_COLUMN);
}

/**
   The aim here is to find a real_item() which is of type Item_field.
*/
bool Item_ref::repoint_const_outer_ref(uchar *arg) {
  *(pointer_cast<bool *>(arg)) = true;
  return false;
}

/**
   If this object is the real_item of an Item_ref, repoint the result_field to
   field.
*/
bool Item_field::repoint_const_outer_ref(uchar *arg) {
  bool *is_outer_ref = pointer_cast<bool *>(arg);
  if (*is_outer_ref) result_field = field;
  *is_outer_ref = false;
  return false;
}

/**
  Generated fields don't need db/table names. Strip them off as inplace ALTER
  can reallocate them, making pointers invalid.
*/
bool Item_field::strip_db_table_name_processor(uchar *) {
  db_name = nullptr;
  table_name = nullptr;
  return false;
}

bool Item_ref::references_select_expr_of(uchar *arg) {
  const SELECT_LEX *sl = pointer_cast<const SELECT_LEX *>(arg);
  if (depended_from)  // outer reference
  {
    if (depended_from == sl) return true;
  } else if (context->select_lex == sl)
    return true;
  return false;
}

string ItemToString(const Item *item) {
  String str;
  const ulonglong save_bits = current_thd->variables.option_bits;
  current_thd->variables.option_bits &= ~OPTION_QUOTE_SHOW_CREATE;
  item->print(
      current_thd, &str,
      enum_query_type(QT_NO_DEFAULT_DB | QT_SUBSELECT_AS_ONLY_SELECT_NUMBER));
  current_thd->variables.option_bits = save_bits;
  return to_string(str);
}

Item_field *FindEqualField(Item_field *item_field, table_map reachable_tables) {
  if (item_field->item_equal_all_join_nests == nullptr) {
    return item_field;
  }

  // We have established in
  // 'Item_func_eq::ensure_multi_equality_fields_are_available' that this
  // item references a field that is outside of our reach. We also have a
  // multi-equality (item_equal_all_join_nests is set), so we go through all
  // fields in the multi-equality and find the first that is within our reach.
  // The table_map provided in 'reachable_tables' defines the tables within our
  // reach.
  Item_equal_iterator item_equal_iterator(
      *item_field->item_equal_all_join_nests);
  Item_field *it;

  while ((it = item_equal_iterator++)) {
    if (it->field == item_field->field) {
      continue;
    }

    table_map item_field_used_tables = it->used_tables();
    if ((item_field_used_tables & reachable_tables) == item_field_used_tables) {
      Item_field *new_item_field = new Item_field(current_thd, item_field);
      new_item_field->reset_field(it->field);
      return new_item_field;
    }
  }

  return item_field;
}
