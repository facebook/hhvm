#ifndef SQL_ITEM_REGEXP_FUNC_H_
#define SQL_ITEM_REGEXP_FUNC_H_

/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file item_regexp_func.h

  The function classes for regular expression functions. They have a common
  base class Item_func_regexp, which is also the prefix of their class
  names. After the %Item_func prefix comes the name of the SQL function,
  e.g. Item_func_regexp_instr represents the SQL function `REGEXP_INSTR`.

  Type resolution
  ===============

  The type and name resolution procedure is hooked into by the
  Item_func_regexp class, which implement both
  Item_result_field::resolve_type() and Item::fix_fields().

  Collations
  ==========

  The regular expression library doesn't deal with collations at all, but we
  need them because the 'winning' collation of the pattern and the subject
  strings dictates case-sensitivity. The winning collation is defined by
  coercion rules, and we don't delve into that here. See
  Item_func::agg_arg_charsets_for_comparison(). The call to this function is
  done in resolve_type() as this appears to be an unwritten convention.

  Implementation
  ==============

  All communication with the regular expression library is done through a
  Regexp_facade object, instantiated in Item_func_regexp::fix_fields().

  @todo We now clean up ICU heap memory in Item_func_regexp::cleanup. Should
  it be done more rarely? On session close?
*/

#include <unicode/uregex.h>

#include <string>

#include "my_dbug.h"      // DBUG_ASSERT
#include "my_inttypes.h"  // MY_INT32_NUM_DECIMAL_DIGITS
#include "sql/item_cmpfunc.h"
#include "sql/item_strfunc.h"
#include "sql/mysqld.h"  // make_unique_destroy_only
#include "sql/regexp/regexp_facade.h"
#include "sql_string.h"  // String

/**
  Base class for all regular expression function classes. Is responsible for
  creating the Regexp_facade object.
*/
class Item_func_regexp : public Item_func {
 public:
  Item_func_regexp(const POS &pos, PT_item_list *opt_list)
      : Item_func(pos, opt_list) {}

  /**
    Resolves the collation to use for comparison. The type resolution is done
    in the subclass constructors.

    For all regular expression functions, i.e. REGEXP_INSTR, REGEXP_LIKE,
    REGEXP_REPLACE and REGEXP_SUBSTR, it goes that the first two arguments
    have to agree on a common collation. This collation is used to control
    case-sensitivity.

    @see fix_fields()
  */
  bool resolve_type(THD *) override;

  /// Decides on the mode for matching, case sensitivity etc.
  bool fix_fields(THD *thd, Item **) override;

  /// The expression for the subject string.
  Item *subject() const { return args[0]; }

  /// The expression for the pattern string.
  Item *pattern() const { return args[1]; }

  /// The value of the `position` argument, or its default if absent.
  Mysql::Nullable<int> position() const {
    int the_index = pos_arg_pos();
    if (the_index != -1 && arg_count >= static_cast<uint>(the_index) + 1) {
      int value = args[the_index]->val_int();
      /*
        Note: Item::null_value() can't be trusted alone here; there are cases
        (for the DATE data type in particular) where we can have it set
        without Item::maybe_null being set! This really should be cleaned up,
        but until that happens, we need to have a more conservative check.
      */
      if (args[the_index]->maybe_null && args[the_index]->null_value)
        return Mysql::Nullable<int>();
      else
        return value;
    }
    return 1;
  }

  /// The value of the `occurrence` argument, or its default if absent.
  Mysql::Nullable<int> occurrence() const {
    int the_index = occ_arg_pos();
    if (the_index != -1 && arg_count >= static_cast<uint>(the_index) + 1) {
      int value = args[the_index]->val_int();
      /*
        Note: Item::null_value() can't be trusted alone here; there are cases
        (for the DATE data type in particular) where we can have it set
        without Item::maybe_null being set! This really should be cleaned up,
        but until that happens, we need to have a more conservative check.
      */
      if (args[the_index]->maybe_null && args[the_index]->null_value)
        return Mysql::Nullable<int>();
      else
        return value;
    }
    return 0;
  }

  /// The value of the `match_parameter` argument, or an empty string if absent.
  Mysql::Nullable<std::string> match_parameter() const {
    int the_index = match_arg_pos();
    if (the_index != -1 && arg_count >= static_cast<uint>(the_index) + 1) {
      StringBuffer<5> buf;  // Longer match_parameter doesn't make sense.
      String *s = args[the_index]->val_str(&buf);
      if (s != nullptr)
        return to_string(*s);
      else
        return Mysql::Nullable<std::string>();
    }
    return std::string{};
  }

  void cleanup() override;

 protected:
  String *convert_int_to_str(String *str) {
    DBUG_ASSERT(fixed == 1);
    longlong nr = val_int();
    if (null_value) return nullptr;
    str->set_int(nr, unsigned_flag, collation.collation);
    return str;
  }

  my_decimal *convert_int_to_decimal(my_decimal *value) {
    DBUG_ASSERT(fixed == 1);
    longlong nr = val_int();
    if (null_value) return nullptr; /* purecov: inspected */
    int2my_decimal(E_DEC_FATAL_ERROR, nr, unsigned_flag, value);
    return value;
  }

  double convert_int_to_real() {
    DBUG_ASSERT(fixed == 1);
    return val_int();
  }

  double convert_str_to_real() {
    DBUG_ASSERT(fixed == 1);
    int err_not_used;
    const char *end_not_used;
    String *res = val_str(&str_value);
    if (res == nullptr) return 0.0;
    return my_strntod(res->charset(), res->ptr(), res->length(), &end_not_used,
                      &err_not_used);
  }

  longlong convert_str_to_int() {
    DBUG_ASSERT(fixed == 1);
    int err;
    String *res = val_str(&str_value);
    if (res == nullptr) return 0;
    return my_strntoll(res->charset(), res->ptr(), res->length(), 10, nullptr,
                       &err);
  }

  /**
    The position in the argument list of 'position'. -1 means that the default
    should be used.
  */
  virtual int pos_arg_pos() const = 0;

  /**
    The position in the argument list of 'occurrence'. -1 means that the default
    should be used.
  */
  virtual int occ_arg_pos() const = 0;

  /// The position in the argument list of match_parameter.
  virtual int match_arg_pos() const = 0;

  bool set_pattern();

  unique_ptr_destroy_only<regexp::Regexp_facade> m_facade;
};

class Item_func_regexp_instr : public Item_func_regexp {
 public:
  Item_func_regexp_instr(const POS &pos, PT_item_list *opt_list)
      : Item_func_regexp(pos, opt_list) {
    set_data_type_longlong();
  }

  Item_result result_type() const override { return INT_RESULT; }

  bool fix_fields(THD *thd, Item **arguments) override;

  String *val_str(String *str) override { return convert_int_to_str(str); }

  double val_real() override { return convert_int_to_real(); }

  longlong val_int() override;

  const char *func_name() const override { return "regexp_instr"; }

  /// The value of the `return_option` argument, or its default if absent.
  Mysql::Nullable<int> return_option() const {
    int the_index = retopt_arg_pos();
    if (the_index != -1 && arg_count >= static_cast<uint>(the_index) + 1) {
      int value = args[the_index]->val_int();
      if (args[the_index]->null_value)
        return Mysql::Nullable<int>();
      else
        return value;
    }
    return 0;
  }

  /**
    @{

    Copy-pasted from Item_int_func. Usually, an SQL function returning INTEGER
    just inherits Item_int_func and thus the implementation, but these classes
    need to have Item_func_regexp as base class because of fix_fields().
  */
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }

  bool get_time(MYSQL_TIME *t) override { return get_time_from_int(t); }
  /// @}

 protected:
  int pos_arg_pos() const override { return 2; }
  int occ_arg_pos() const override { return 3; }
  /// The position in the argument list of `occurrence`.
  int retopt_arg_pos() const { return 4; }
  int match_arg_pos() const override { return 5; }
};

class Item_func_regexp_like : public Item_func_regexp {
 public:
  Item_func_regexp_like(const POS &pos, PT_item_list *opt_list)
      : Item_func_regexp(pos, opt_list) {
    set_data_type_bool();
  }

  Item_result result_type() const override { return INT_RESULT; }

  String *val_str(String *str) override { return convert_int_to_str(str); }

  double val_real() override { return convert_int_to_real(); }

  longlong val_int() override;

  const char *func_name() const override { return "regexp_like"; }

  bool is_bool_func() const override { return true; }

  /**
    @{

    Copy-pasted from Item_int_func. Usually, an SQL function returning INTEGER
    just inherits Item_int_func and thus the implementation, but these classes
    need to have Item_func_regexp as base class because of fix_fields().
  */
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }

  bool get_time(MYSQL_TIME *t) override { return get_time_from_int(t); }
  /// @}

 protected:
  int pos_arg_pos() const override { return -1; }
  int occ_arg_pos() const override { return -1; }
  int match_arg_pos() const override { return 2; }
};

class Item_func_regexp_replace : public Item_func_regexp {
 public:
  Item_func_regexp_replace(const POS &pos, PT_item_list *opt_list)
      : Item_func_regexp(pos, opt_list) {
    set_data_type_string_init();
  }

  Item_result result_type() const override { return STRING_RESULT; }

  bool resolve_type(THD *) final;

  Item *replacement() { return args[2]; }

  longlong val_int() override { return convert_str_to_int(); }

  String *val_str(String *result) override;

  double val_real() override { return convert_str_to_real(); }

  const char *func_name() const override { return "regexp_replace"; }

  /**
    @{

    Copy-pasted from Item_str_func. Usually, an SQL function returning INTEGER
    just inherits Item_str_func and thus the implementation, but these classes
    need to have Item_func_regexp as base class because of fix_fields().
  */
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_string(ltime, fuzzydate);
  }

  bool get_time(MYSQL_TIME *t) override { return get_time_from_string(t); }
  /// @}

 protected:
  int pos_arg_pos() const override { return 3; }
  int occ_arg_pos() const override { return 4; }
  int match_arg_pos() const override { return 5; }
};

class Item_func_regexp_substr : public Item_func_regexp {
 public:
  Item_func_regexp_substr(const POS &pos, PT_item_list *opt_list)
      : Item_func_regexp(pos, opt_list) {
    set_data_type_string_init();
  }

  Item_result result_type() const override { return STRING_RESULT; }

  bool resolve_type(THD *) final;

  longlong val_int() override { return convert_str_to_int(); }

  String *val_str(String *result) override;

  double val_real() override { return convert_str_to_real(); }

  const char *func_name() const override { return "regexp_substr"; }

  /**
    @{

    Copy-pasted from Item_str_func. Usually, an SQL function returning INTEGER
    just inherits Item_str_func and thus the implementation, but these classes
    need to have Item_func_regexp as base class because of fix_fields().
  */
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_string(ltime, fuzzydate);
  }

  bool get_time(MYSQL_TIME *t) override { return get_time_from_string(t); }
  /// @}

 protected:
  int pos_arg_pos() const override { return 2; }
  int occ_arg_pos() const override { return 3; }
  int match_arg_pos() const override { return 4; }
};

class Item_func_icu_version final : public Item_static_string_func {
  using super = Item_static_string_func;

 public:
  explicit Item_func_icu_version(const POS &pos);

  bool itemize(Parse_context *pc, Item **res) override;
};

#endif  // SQL_ITEM_REGEXP_FUNC_H_
