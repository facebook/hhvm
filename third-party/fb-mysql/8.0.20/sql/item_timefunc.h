#ifndef ITEM_TIMEFUNC_INCLUDED
#define ITEM_TIMEFUNC_INCLUDED

/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* Function items used by mysql */

#include <stddef.h>
#include <sys/types.h>
#include <algorithm>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_table_map.h"
#include "my_time.h"
#include "mysql/udf_registration_types.h"
#include "mysql_time.h"
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/item_strfunc.h"  // Item_str_func
#include "sql/parse_tree_node_base.h"
#include "sql/set_var.h"
#include "sql/sql_const.h"
#include "sql_string.h"

class MY_LOCALE;
class PT_item_list;
class THD;
class Time_zone;
class my_decimal;
struct Date_time_format;
struct Interval;
struct TABLE;

bool get_interval_value(Item *args, interval_type int_type, String *str_value,
                        Interval *interval);

class Item_func_period_add final : public Item_int_func {
 public:
  Item_func_period_add(const POS &pos, Item *a, Item *b)
      : Item_int_func(pos, a, b) {}
  longlong val_int() override;
  const char *func_name() const override { return "period_add"; }
  bool resolve_type(THD *) override {
    fix_char_length(6); /* YYYYMM */
    return false;
  }
};

class Item_func_period_diff final : public Item_int_func {
 public:
  Item_func_period_diff(const POS &pos, Item *a, Item *b)
      : Item_int_func(pos, a, b) {}
  longlong val_int() override;
  const char *func_name() const override { return "period_diff"; }
  bool resolve_type(THD *) override {
    fix_char_length(6); /* YYYYMM */
    return false;
  }
};

class Item_func_to_days final : public Item_int_func {
 public:
  Item_func_to_days(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "to_days"; }
  enum Functype functype() const override { return TO_DAYS_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(6);
    maybe_null = true;
    return false;
  }
  enum_monotonicity_info get_monotonicity_info() const override;
  longlong val_int_endpoint(bool left_endp, bool *incl_endp) override;
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_to_seconds final : public Item_int_func {
 public:
  Item_func_to_seconds(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "to_seconds"; }
  bool resolve_type(THD *) override {
    fix_char_length(MY_INT64_NUM_DECIMAL_DIGITS);
    maybe_null = true;
    return false;
  }
  enum_monotonicity_info get_monotonicity_info() const override;
  longlong val_int_endpoint(bool left_endp, bool *incl_endp) override;
  bool check_partition_func_processor(uchar *) override { return false; }

  bool intro_version(uchar *int_arg) override {
    int *input_version = (int *)int_arg;
    /* This function was introduced in 5.5 */
    int output_version = std::max(*input_version, 50500);
    *input_version = output_version;
    return false;
  }

  /* Only meaningful with date part and optional time part */
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_dayofmonth final : public Item_int_func {
 public:
  Item_func_dayofmonth(Item *a) : Item_int_func(a) {}
  Item_func_dayofmonth(const POS &pos, Item *a) : Item_int_func(pos, a) {}

  longlong val_int() override;
  const char *func_name() const override { return "dayofmonth"; }
  enum Functype functype() const override { return DAY_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(2); /* 1..31 */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

/**
  TS-TODO: This should probably have Item_int_func as parent class.
*/
class Item_func_month final : public Item_func {
 public:
  Item_func_month(const POS &pos, Item *a) : Item_func(pos, a) {
    set_data_type(MYSQL_TYPE_LONGLONG);
    collation.set_numeric();
  }
  longlong val_int() override;
  double val_real() override {
    DBUG_ASSERT(fixed);
    return (double)Item_func_month::val_int();
  }
  String *val_str(String *str) override {
    longlong nr = val_int();
    if (null_value) return nullptr;
    str->set(nr, collation.collation);
    return str;
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override { return get_time_from_int(ltime); }
  const char *func_name() const override { return "month"; }
  enum Functype functype() const override { return MONTH_FUNC; }
  enum Item_result result_type() const override { return INT_RESULT; }
  bool resolve_type(THD *) override {
    fix_char_length(2);
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_monthname final : public Item_str_func {
  MY_LOCALE *locale;

 public:
  Item_func_monthname(const POS &pos, Item *a) : Item_str_func(pos, a) {}
  const char *func_name() const override { return "monthname"; }
  String *val_str(String *str) override;
  bool resolve_type(THD *thd) override;
  bool check_partition_func_processor(uchar *) override { return true; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_dayofyear final : public Item_int_func {
 public:
  Item_func_dayofyear(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "dayofyear"; }
  bool resolve_type(THD *) override {
    fix_char_length(3);
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_hour final : public Item_int_func {
 public:
  Item_func_hour(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "hour"; }
  enum Functype functype() const override { return HOUR_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(2); /* 0..23 */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_time_args();
  }
};

class Item_func_minute final : public Item_int_func {
 public:
  Item_func_minute(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "minute"; }
  enum Functype functype() const override { return MINUTE_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(2); /* 0..59 */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_time_args();
  }
};

class Item_func_quarter final : public Item_int_func {
 public:
  Item_func_quarter(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "quarter"; }
  bool resolve_type(THD *) override {
    fix_char_length(1); /* 1..4 */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_second final : public Item_int_func {
 public:
  Item_func_second(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "second"; }
  enum Functype functype() const override { return SECOND_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(2); /* 0..59 */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_time_args();
  }
};

class Item_func_week final : public Item_int_func {
  typedef Item_int_func super;

 public:
  Item_func_week(Item *a, Item *b) : Item_int_func(a, b) {}
  Item_func_week(const POS &pos, Item *a, Item *b) : super(pos, a, b) {}

  bool itemize(Parse_context *pc, Item **res) override;

  longlong val_int() override;
  const char *func_name() const override { return "week"; }
  enum Functype functype() const override { return WEEK_FUNC; }
  bool resolve_type(THD *) override {
    fix_char_length(2); /* 0..54 */
    maybe_null = true;
    return false;
  }
};

class Item_func_yearweek final : public Item_int_func {
 public:
  Item_func_yearweek(const POS &pos, Item *a, Item *b)
      : Item_int_func(pos, a, b) {}
  longlong val_int() override;
  const char *func_name() const override { return "yearweek"; }
  bool resolve_type(THD *) override {
    fix_char_length(6); /* YYYYWW */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

class Item_func_year final : public Item_int_func {
 public:
  Item_func_year(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "year"; }
  enum Functype functype() const override { return YEAR_FUNC; }
  enum_monotonicity_info get_monotonicity_info() const override;
  longlong val_int_endpoint(bool left_endp, bool *incl_endp) override;
  bool resolve_type(THD *) override {
    fix_char_length(5); /* 9999 plus sign */
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

/**
  TS-TODO: This should probably have Item_int_func as parent class.
*/
class Item_func_weekday : public Item_func {
  bool odbc_type;

 public:
  Item_func_weekday(const POS &pos, Item *a, bool type_arg)
      : Item_func(pos, a), odbc_type(type_arg) {
    set_data_type(MYSQL_TYPE_LONGLONG);
    collation.set_numeric();
  }
  longlong val_int() override;
  double val_real() override {
    DBUG_ASSERT(fixed);
    return static_cast<double>(val_int());
  }
  String *val_str(String *str) override {
    DBUG_ASSERT(fixed == 1);
    str->set(val_int(), &my_charset_bin);
    return null_value ? nullptr : str;
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override { return get_time_from_int(ltime); }
  const char *func_name() const override {
    return (odbc_type ? "dayofweek" : "weekday");
  }
  enum Functype functype() const override { return WEEKDAY_FUNC; }
  enum Item_result result_type() const override { return INT_RESULT; }
  bool resolve_type(THD *) override {
    fix_char_length(1);
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_date_args();
  }
};

/**
  TS-TODO: Item_func_dayname should be derived from Item_str_func.
  In the current implementation funny things can happen:
  select dayname(now())+1 -> 4
*/
class Item_func_dayname final : public Item_func_weekday {
  MY_LOCALE *locale;

 public:
  Item_func_dayname(const POS &pos, Item *a)
      : Item_func_weekday(pos, a, false) {}
  const char *func_name() const override { return "dayname"; }
  String *val_str(String *str) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_string(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_string(ltime);
  }
  enum Item_result result_type() const override { return STRING_RESULT; }
  bool resolve_type(THD *thd) override;
  bool check_partition_func_processor(uchar *) override { return true; }
};

/*
  Abstract class for functions returning "struct timeval".
*/
class Item_timeval_func : public Item_func {
 public:
  explicit Item_timeval_func(const POS &pos) : Item_func(pos) {}

  Item_timeval_func(Item *a) : Item_func(a) {}
  Item_timeval_func(const POS &pos, Item *a) : Item_func(pos, a) {}
  /**
    Return timestamp in "struct timeval" format.
    @param[out] tm The value is store here.
    @retval false On success
    @retval true  On error
  */
  virtual bool val_timeval(struct timeval *tm) = 0;
  longlong val_int() override;
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *decimal_value) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_numeric(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_numeric(ltime);
  }
  enum Item_result result_type() const override {
    return decimals ? DECIMAL_RESULT : INT_RESULT;
  }
};

class Item_func_unix_timestamp final : public Item_timeval_func {
  typedef Item_timeval_func super;

 public:
  explicit Item_func_unix_timestamp(const POS &pos) : Item_timeval_func(pos) {}

  Item_func_unix_timestamp(Item *a) : Item_timeval_func(a) {}

  Item_func_unix_timestamp(const POS &pos, Item *a)
      : Item_timeval_func(pos, a) {}

  const char *func_name() const override { return "unix_timestamp"; }

  bool itemize(Parse_context *pc, Item **res) override;
  enum_monotonicity_info get_monotonicity_info() const override;
  longlong val_int_endpoint(bool left_endp, bool *incl_endp) override;
  bool check_partition_func_processor(uchar *) override { return false; }
  /*
    UNIX_TIMESTAMP() depends on the current timezone
    (and thus may not be used as a partitioning function)
    when its argument is NOT of the TIMESTAMP type.
  */
  bool check_valid_arguments_processor(uchar *) override {
    return !has_timestamp_args();
  }
  bool resolve_type(THD *) override {
    collation.set_numeric();
    const uint8 dec = arg_count == 0 ? 0 : args[0]->datetime_precision();
    if (dec > 0) {
      set_data_type_decimal(11 + dec, dec);
    } else {
      set_data_type_longlong();
      decimals = 0;
      max_length = 11;
    }
    return false;
  }
  bool val_timeval(struct timeval *tm) override;

  bool check_function_as_value_generator(uchar *p_arg) override {
    /*
      TODO: Allow UNIX_TIMESTAMP called with an argument to be a part
      of the expression for a generated column too.
    */
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(p_arg);
    func_arg->banned_function_name = func_name();
    return ((func_arg->source == VGS_GENERATED_COLUMN) ||
            (func_arg->source == VGS_CHECK_CONSTRAINT));
  }
};

class Item_func_time_to_sec final : public Item_int_func {
 public:
  Item_func_time_to_sec(const POS &pos, Item *item)
      : Item_int_func(pos, item) {}
  longlong val_int() override;
  const char *func_name() const override { return "time_to_sec"; }
  bool resolve_type(THD *) override {
    fix_char_length(10);
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_time_args();
  }
};

/**
  Abstract class for functions returning TIME, DATE, DATETIME types
  whose data type is known at constructor time.
*/
class Item_temporal_func : public Item_func {
 protected:
  bool check_precision();

 public:
  Item_temporal_func() : Item_func() {}
  explicit Item_temporal_func(const POS &pos) : Item_func(pos) {}

  Item_temporal_func(Item *a) : Item_func(a) {}
  Item_temporal_func(const POS &pos, Item *a) : Item_func(pos, a) {}

  Item_temporal_func(const POS &pos, Item *a, Item *b) : Item_func(pos, a, b) {}

  Item_temporal_func(Item *a, Item *b, Item *c) : Item_func(a, b, c) {}
  Item_temporal_func(const POS &pos, Item *a, Item *b, Item *c)
      : Item_func(pos, a, b, c) {}
  Item_temporal_func(const POS &pos, Item *a, Item *b, Item *c, Item *d)
      : Item_func(pos, a, b, c, d) {}
  Item_temporal_func(const POS &pos, PT_item_list *list)
      : Item_func(pos, list) {}

  Item_result result_type() const override { return STRING_RESULT; }
  const CHARSET_INFO *charset_for_protocol() const override {
    return &my_charset_bin;
  }
  Field *tmp_table_field(TABLE *table) override {
    return tmp_table_field_from_field_type(table, false);
  }
  uint time_precision() override {
    DBUG_ASSERT(fixed);
    return decimals;
  }
  uint datetime_precision() override {
    DBUG_ASSERT(fixed);
    return decimals;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
};

/**
  Abstract class for functions returning TIME, DATE, DATETIME or string values,
  whose data type depends on parameters and is set at fix_field time.
*/
class Item_temporal_hybrid_func : public Item_str_func {
 protected:
  sql_mode_t sql_mode;  // sql_mode value is cached here in resolve_type()
  String ascii_buf;     // Conversion buffer
  /**
    Get "native" temporal value as MYSQL_TIME
    @param[out] ltime       The value is stored here.
    @param[in]  fuzzy_date  Date flags.
    @retval     false       On success.
    @retval     true        On error.
  */
  virtual bool val_datetime(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) = 0;
  type_conversion_status save_in_field_inner(Field *field,
                                             bool no_conversions) override;

 public:
  Item_temporal_hybrid_func(Item *a, Item *b)
      : Item_str_func(a, b), sql_mode(0) {}
  Item_temporal_hybrid_func(const POS &pos, Item *a, Item *b)
      : Item_str_func(pos, a, b), sql_mode(0) {}

  Item_result result_type() const override { return STRING_RESULT; }
  const CHARSET_INFO *charset_for_protocol() const override {
    /*
      Can return TIME, DATE, DATETIME or VARCHAR depending on arguments.
      Send using "binary" when TIME, DATE or DATETIME,
      or using collation.collation when VARCHAR
      (which is fixed from @collation_connection in resolve_type()).
    */
    DBUG_ASSERT(fixed == 1);
    return data_type() == MYSQL_TYPE_STRING ? collation.collation
                                            : &my_charset_bin;
  }
  Field *tmp_table_field(TABLE *table) override {
    return tmp_table_field_from_field_type(table, false);
  }
  longlong val_int() override { return val_int_from_decimal(); }
  double val_real() override { return val_real_from_decimal(); }
  my_decimal *val_decimal(my_decimal *decimal_value) override;
  /**
    Return string value in ASCII character set.
  */
  String *val_str_ascii(String *str) override;
  /**
    Return string value in @@character_set_connection.
  */
  String *val_str(String *str) override {
    return val_str_from_val_str_ascii(str, &ascii_buf);
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
};

/*
  This can't be a Item_str_func, because the val_real() functions are special
*/

/**
  Abstract class for functions returning DATE values.
*/
class Item_date_func : public Item_temporal_func {
 protected:
  type_conversion_status save_in_field_inner(Field *field, bool) override {
    return save_date_in_field(field);
  }

 public:
  Item_date_func() : Item_temporal_func() { set_data_type_date(); }
  explicit Item_date_func(const POS &pos) : Item_temporal_func(pos) {
    set_data_type_date();
  }
  Item_date_func(Item *a) : Item_temporal_func(a) { set_data_type_date(); }
  Item_date_func(const POS &pos, Item *a) : Item_temporal_func(pos, a) {
    set_data_type_date();
  }
  Item_date_func(const POS &pos, Item *a, Item *b)
      : Item_temporal_func(pos, a, b) {
    set_data_type_date();
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_date(ltime);
  }
  String *val_str(String *str) override { return val_string_from_date(str); }
  longlong val_int() override { return val_int_from_date(); }
  longlong val_date_temporal() override;
  double val_real() override { return static_cast<double>(val_int()); }
  const char *func_name() const override { return "date"; }
  enum Functype functype() const override { return DATE_FUNC; }
  bool resolve_type(THD *) override { return false; }
  my_decimal *val_decimal(my_decimal *decimal_value) override {
    DBUG_ASSERT(fixed == 1);
    return val_decimal_from_date(decimal_value);
  }
  // All date functions must implement get_date()
  // to avoid use of generic Item::get_date()
  // which converts to string and then parses the string as DATE.
  virtual bool get_date(MYSQL_TIME *res,
                        my_time_flags_t fuzzy_date) override = 0;
};

/**
  Abstract class for functions returning DATETIME values.
*/
class Item_datetime_func : public Item_temporal_func {
 protected:
  type_conversion_status save_in_field_inner(Field *field, bool) override {
    return save_date_in_field(field);
  }

 public:
  Item_datetime_func() : Item_temporal_func() {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos) : Item_temporal_func(pos) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(Item *a) : Item_temporal_func(a) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos, Item *a) : Item_temporal_func(pos, a) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos, Item *a, Item *b)
      : Item_temporal_func(pos, a, b) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(Item *a, Item *b, Item *c) : Item_temporal_func(a, b, c) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos, Item *a, Item *b, Item *c)
      : Item_temporal_func(pos, a, b, c) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos, Item *a, Item *b, Item *c, Item *d)
      : Item_temporal_func(pos, a, b, c, d) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }
  Item_datetime_func(const POS &pos, PT_item_list *list)
      : Item_temporal_func(pos, list) {
    set_data_type(MYSQL_TYPE_DATETIME);
  }

  double val_real() override { return val_real_from_decimal(); }
  String *val_str(String *str) override {
    return val_string_from_datetime(str);
  }
  longlong val_int() override { return val_int_from_datetime(); }
  longlong val_date_temporal() override;
  my_decimal *val_decimal(my_decimal *decimal_value) override {
    DBUG_ASSERT(fixed == 1);
    return val_decimal_from_date(decimal_value);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_datetime(ltime);
  }
  // All datetime functions must implement get_date()
  // to avoid use of generic Item::get_date()
  // which converts to string and then parses the string as DATETIME.
  virtual bool get_date(MYSQL_TIME *res,
                        my_time_flags_t fuzzy_date) override = 0;
};

/**
  Abstract class for functions returning TIME values.
*/
class Item_time_func : public Item_temporal_func {
 protected:
  type_conversion_status save_in_field_inner(Field *field, bool) override {
    return save_time_in_field(field);
  }

 public:
  Item_time_func() : Item_temporal_func() { set_data_type(MYSQL_TYPE_TIME); }
  explicit Item_time_func(const POS &pos) : Item_temporal_func(pos) {
    set_data_type(MYSQL_TYPE_TIME);
  }
  Item_time_func(Item *a) : Item_temporal_func(a) {
    set_data_type(MYSQL_TYPE_TIME);
  }
  Item_time_func(const POS &pos, Item *a) : Item_temporal_func(pos, a) {
    set_data_type(MYSQL_TYPE_TIME);
  }
  Item_time_func(const POS &pos, Item *a, Item *b)
      : Item_temporal_func(pos, a, b) {
    set_data_type(MYSQL_TYPE_TIME);
  }
  Item_time_func(const POS &pos, Item *a, Item *b, Item *c)
      : Item_temporal_func(pos, a, b, c) {
    set_data_type(MYSQL_TYPE_TIME);
  }
  double val_real() override { return val_real_from_decimal(); }
  my_decimal *val_decimal(my_decimal *decimal_value) override {
    DBUG_ASSERT(fixed);
    return val_decimal_from_time(decimal_value);
  }
  longlong val_int() override { return val_int_from_time(); }
  longlong val_time_temporal() override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t) override {
    return get_date_from_time(res);
  }
  String *val_str(String *str) override { return val_string_from_time(str); }
  // All time functions must implement get_time()
  // to avoid use of generic Item::get_time()
  // which converts to string and then parses the string as TIME.
  virtual bool get_time(MYSQL_TIME *res) override = 0;
};

/**
  Cache for MYSQL_TIME value with various representations.

  - MYSQL_TIME representation (time) is initialized during set_XXX().
  - Packed representation (time_packed) is also initialized during set_XXX().
  - String representation (string_buff) is also initialized during set_XXX();
*/
class MYSQL_TIME_cache {
  MYSQL_TIME time;                               ///< MYSQL_TIME representation
  longlong time_packed;                          ///< packed representation
  char string_buff[MAX_DATE_STRING_REP_LENGTH];  ///< string representation
  uint string_length;                            ///< length of string
  uint8 dec;                                     ///< Number of decimals

  /**
    Store MYSQL_TIME representation into the given MYSQL_TIME variable.
  */
  void get_TIME(MYSQL_TIME *ltime) const {
    DBUG_ASSERT(time.time_type != MYSQL_TIMESTAMP_NONE);
    *ltime = time;
  }

 public:
  MYSQL_TIME_cache() : time_packed(0), string_length(0), dec(0) {
    time.time_type = MYSQL_TIMESTAMP_NONE;
    string_buff[0] = '\0';
  }
  /**
    Set time and time_packed from a DATE value.
  */
  void set_date(MYSQL_TIME *ltime);
  /**
    Set time and time_packed from a TIME value.
  */
  void set_time(MYSQL_TIME *ltime, uint8 dec_arg);
  /**
    Set time and time_packed from a DATETIME value.
  */
  void set_datetime(MYSQL_TIME *ltime, uint8 dec_arg,
                    const Time_zone *tz = nullptr);
  /**
    Set time and time_packed according to DATE value
    in "struct timeval" representation and its time zone.
  */
  void set_date(struct timeval tv, Time_zone *tz);
  /**
    Set time and time_packed according to TIME value
    in "struct timeval" representation and its time zone.
  */
  void set_time(struct timeval tv, uint8 dec_arg, Time_zone *tz);
  /**
    Set time and time_packed according to DATETIME value
    in "struct timeval" representation and its time zone.
  */
  void set_datetime(struct timeval tv, uint8 dec_arg, Time_zone *tz);
  /**
    Test if cached value is equal to another MYSQL_TIME_cache value.
  */
  bool eq(const MYSQL_TIME_cache &tm) const {
    return val_packed() == tm.val_packed();
  }

  /**
    Return number of decimal digits.
  */
  uint8 decimals() const {
    DBUG_ASSERT(time.time_type != MYSQL_TIMESTAMP_NONE);
    return dec;
  }

  /**
    Return packed representation.
  */
  longlong val_packed() const {
    DBUG_ASSERT(time.time_type != MYSQL_TIMESTAMP_NONE);
    return time_packed;
  }
  /**
    Store MYSQL_TIME representation into the given date/datetime variable
    checking date flags.
  */
  bool get_date(MYSQL_TIME *ltime, uint fuzzyflags) const;
  /**
    Store MYSQL_TIME representation into the given time variable.
  */
  bool get_time(MYSQL_TIME *ltime) const {
    get_TIME(ltime);
    return false;
  }
  /**
    Return pointer to MYSQL_TIME representation.
  */
  MYSQL_TIME *get_TIME_ptr() {
    DBUG_ASSERT(time.time_type != MYSQL_TIMESTAMP_NONE);
    return &time;
  }
  /**
    Store string representation into String.
  */
  String *val_str(String *str);
  /**
    Return C string representation.
  */
  const char *cptr() const { return string_buff; }
};

/**
  DATE'2010-01-01'
*/
class Item_date_literal final : public Item_date_func {
  MYSQL_TIME_cache cached_time;

 public:
  /**
    Constructor for Item_date_literal.
    @param ltime  DATE value.
  */
  Item_date_literal(MYSQL_TIME *ltime) {
    cached_time.set_date(ltime);
    set_data_type_date();
    fixed = true;
  }
  const char *func_name() const override { return "date_literal"; }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  longlong val_date_temporal() override {
    DBUG_ASSERT(fixed);
    return cached_time.val_packed();
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override {
    DBUG_ASSERT(fixed);
    return cached_time.get_date(ltime, fuzzy_date);
  }
  String *val_str(String *str) override {
    DBUG_ASSERT(fixed);
    return cached_time.val_str(str);
  }
  bool resolve_type(THD *) override { return false; }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool basic_const_item() const override { return true; }
  table_map used_tables() const override { return 0; }
  table_map not_null_tables() const override { return used_tables(); }
  void cleanup() override {
    // See Item_basic_const::cleanup()
    if (orig_name.is_set()) item_name = orig_name;
  }
  bool eq(const Item *item, bool binary_cmp) const override;
};

/**
  TIME'10:10:10'
*/
class Item_time_literal final : public Item_time_func {
  MYSQL_TIME_cache cached_time;

 public:
  /**
    Constructor for Item_time_literal.
    @param ltime    TIME value.
    @param dec_arg  number of fractional digits in ltime.
  */
  Item_time_literal(MYSQL_TIME *ltime, uint dec_arg) {
    set_data_type_time(std::min(dec_arg, uint(DATETIME_MAX_DECIMALS)));
    cached_time.set_time(ltime, decimals);
    fixed = true;
  }
  const char *func_name() const override { return "time_literal"; }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  longlong val_time_temporal() override {
    DBUG_ASSERT(fixed);
    return cached_time.val_packed();
  }
  bool get_time(MYSQL_TIME *ltime) override {
    DBUG_ASSERT(fixed);
    return cached_time.get_time(ltime);
  }
  String *val_str(String *str) override {
    DBUG_ASSERT(fixed);
    return cached_time.val_str(str);
  }
  bool resolve_type(THD *) override { return false; }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool basic_const_item() const override { return true; }
  table_map used_tables() const override { return 0; }
  table_map not_null_tables() const override { return used_tables(); }
  void cleanup() override {
    // See Item_basic_const::cleanup()
    if (orig_name.is_set()) item_name = orig_name;
  }
  bool eq(const Item *item, bool binary_cmp) const override;
};

/**
  TIMESTAMP'2001-01-01 10:20:30'
*/
class Item_datetime_literal final : public Item_datetime_func {
  MYSQL_TIME_cache cached_time;

 public:
  /**
    Constructor for Item_datetime_literal.
    @param ltime   DATETIME value.
    @param dec_arg Number of fractional digits in ltime.
    @param tz      The current time zone, used for converting literals with
                   time zone upon storage.
  */
  Item_datetime_literal(MYSQL_TIME *ltime, uint dec_arg, const Time_zone *tz) {
    set_data_type_datetime(std::min(dec_arg, uint{DATETIME_MAX_DECIMALS}));
    cached_time.set_datetime(ltime, decimals, tz);
    fixed = true;
  }
  const char *func_name() const override { return "datetime_literal"; }
  enum Functype functype() const override { return DATETIME_LITERAL; }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  longlong val_date_temporal() override {
    DBUG_ASSERT(fixed);
    return cached_time.val_packed();
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override {
    DBUG_ASSERT(fixed);
    return cached_time.get_date(ltime, fuzzy_date);
  }
  String *val_str(String *str) override {
    DBUG_ASSERT(fixed);
    return cached_time.val_str(str);
  }
  bool resolve_type(THD *) override { return false; }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool basic_const_item() const override { return true; }
  table_map used_tables() const override { return 0; }
  table_map not_null_tables() const override { return used_tables(); }
  void cleanup() override {
    // See Item_basic_const::cleanup()
    if (orig_name.is_set()) item_name = orig_name;
  }
  bool eq(const Item *item, bool binary_cmp) const override;
};

/// Abstract CURTIME function. Children should define what time zone is used.
class Item_func_curtime : public Item_time_func {
  typedef Item_time_func super;

  MYSQL_TIME_cache cached_time;  // Initialized in resolve_type()
 protected:
  // Abstract method that defines which time zone is used for conversion.
  virtual Time_zone *time_zone() = 0;

 public:
  /**
    Constructor for Item_func_curtime.
    @param pos     Position of token in the parser.
    @param dec_arg Number of fractional digits.
  */
  Item_func_curtime(const POS &pos, uint8 dec_arg) : Item_time_func(pos) {
    decimals = dec_arg;
  }

  bool itemize(Parse_context *pc, Item **res) override;

  bool resolve_type(THD *thd) override;
  longlong val_time_temporal() override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_packed();
  }
  bool get_time(MYSQL_TIME *ltime) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.get_time(ltime);
  }
  String *val_str(String *) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_str(&str_value);
  }
  bool check_function_as_value_generator(uchar *checker_args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return ((func_arg->source == VGS_GENERATED_COLUMN) ||
            (func_arg->source == VGS_CHECK_CONSTRAINT));
  }
};

class Item_func_curtime_local final : public Item_func_curtime {
 protected:
  Time_zone *time_zone() override;

 public:
  Item_func_curtime_local(const POS &pos, uint8 dec_arg)
      : Item_func_curtime(pos, dec_arg) {}
  const char *func_name() const override { return "curtime"; }
};

class Item_func_curtime_utc final : public Item_func_curtime {
 protected:
  Time_zone *time_zone() override;

 public:
  Item_func_curtime_utc(const POS &pos, uint8 dec_arg)
      : Item_func_curtime(pos, dec_arg) {}
  const char *func_name() const override { return "utc_time"; }
};

/**
  Abstract CURDATE function.

  @sa Item_func_curtime
 */
class Item_func_curdate : public Item_date_func {
  typedef Item_date_func super;

  MYSQL_TIME_cache cached_time;  // Initialized in resolve_type()
 protected:
  virtual Time_zone *time_zone() = 0;

 public:
  explicit Item_func_curdate(const POS &pos) : Item_date_func(pos) {}

  bool itemize(Parse_context *pc, Item **res) override;

  bool resolve_type(THD *) override;
  longlong val_date_temporal() override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_packed();
  }
  bool get_date(MYSQL_TIME *res, my_time_flags_t) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.get_time(res);
  }
  String *val_str(String *) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_str(&str_value);
  }
  bool check_function_as_value_generator(uchar *checker_args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return ((func_arg->source == VGS_GENERATED_COLUMN) ||
            (func_arg->source == VGS_CHECK_CONSTRAINT));
  }
};

class Item_func_curdate_local final : public Item_func_curdate {
 protected:
  Time_zone *time_zone() override;

 public:
  explicit Item_func_curdate_local(const POS &pos) : Item_func_curdate(pos) {}
  const char *func_name() const override { return "curdate"; }
};

class Item_func_curdate_utc final : public Item_func_curdate {
 protected:
  Time_zone *time_zone() override;

 public:
  Item_func_curdate_utc(const POS &pos) : Item_func_curdate(pos) {}
  const char *func_name() const override { return "utc_date"; }
};

/**
  Abstract CURRENT_TIMESTAMP function.

  @sa Item_func_curtime
*/
class Item_func_now : public Item_datetime_func {
  MYSQL_TIME_cache cached_time;

 protected:
  virtual Time_zone *time_zone() = 0;
  type_conversion_status save_in_field_inner(Field *to,
                                             bool no_conversions) override;

 public:
  /**
    Constructor for Item_func_now.
    @param dec_arg  Number of fractional digits.
  */
  Item_func_now(uint8 dec_arg) : Item_datetime_func() { decimals = dec_arg; }
  Item_func_now(const POS &pos, uint8 dec_arg) : Item_datetime_func(pos) {
    decimals = dec_arg;
  }

  bool resolve_type(THD *) override;
  longlong val_date_temporal() override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_packed();
  }
  bool get_date(MYSQL_TIME *res, my_time_flags_t) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.get_time(res);
  }
  String *val_str(String *) override {
    DBUG_ASSERT(fixed == 1);
    return cached_time.val_str(&str_value);
  }
  bool check_function_as_value_generator(uchar *checker_args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return ((func_arg->source == VGS_GENERATED_COLUMN) ||
            (func_arg->source == VGS_CHECK_CONSTRAINT));
  }
};

class Item_func_now_local : public Item_func_now {
 protected:
  Time_zone *time_zone() override;

 public:
  /**
     Stores the query start time in a field, truncating to the field's number
     of fractional second digits.

     @param field The field to store in.
   */
  static void store_in(Field *field);

  Item_func_now_local(uint8 dec_arg) : Item_func_now(dec_arg) {}
  Item_func_now_local(const POS &pos, uint8 dec_arg)
      : Item_func_now(pos, dec_arg) {}

  const char *func_name() const override { return "now"; }
  enum Functype functype() const override { return NOW_FUNC; }
};

class Item_func_now_utc final : public Item_func_now {
  typedef Item_func_now super;

 protected:
  Time_zone *time_zone() override;

 public:
  Item_func_now_utc(const POS &pos, uint8 dec_arg)
      : Item_func_now(pos, dec_arg) {}

  bool itemize(Parse_context *pc, Item **res) override;

  const char *func_name() const override { return "utc_timestamp"; }
};

/**
  SYSDATE() is like NOW(), but always uses the real current time, not the
  query_start(). This matches the Oracle behavior.
*/
class Item_func_sysdate_local final : public Item_datetime_func {
 public:
  Item_func_sysdate_local(uint8 dec_arg) : Item_datetime_func() {
    decimals = dec_arg;
  }
  const char *func_name() const override { return "sysdate"; }
  bool resolve_type(THD *) override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
  /**
    This function is non-deterministic and hence depends on the 'RAND'
    pseudo-table.

    @retval Always RAND_TABLE_BIT
  */
  table_map get_initial_pseudo_tables() const override {
    return RAND_TABLE_BIT;
  }
};

class Item_func_from_days final : public Item_date_func {
 public:
  Item_func_from_days(const POS &pos, Item *a) : Item_date_func(pos, a) {}
  const char *func_name() const override { return "from_days"; }
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return has_date_args() || has_time_args();
  }
};

class Item_func_date_format final : public Item_str_func {
  int fixed_length;
  const bool is_time_format;
  String value;

 public:
  Item_func_date_format(const POS &pos, Item *a, Item *b,
                        bool is_time_format_arg = false)
      : Item_str_func(pos, a, b), is_time_format(is_time_format_arg) {}
  String *val_str(String *str) override;
  const char *func_name() const override {
    return is_time_format ? "time_format" : "date_format";
  }
  bool resolve_type(THD *thd) override;
  uint format_length(const String *format);
  bool eq(const Item *item, bool binary_cmp) const override;
};

class Item_func_from_unixtime final : public Item_datetime_func {
 public:
  Item_func_from_unixtime(const POS &pos, Item *a)
      : Item_datetime_func(pos, a) {}
  const char *func_name() const override { return "from_unixtime"; }
  bool resolve_type(THD *thd) override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
};

/*
  This class represents CONVERT_TZ() function.
  The important fact about this function that it is handled in special way.
  When such function is met in expression time_zone system tables are added
  to global list of tables to open, so later those already opened and locked
  tables can be used during this function calculation for loading time zone
  descriptions.
*/
class Item_func_convert_tz final : public Item_datetime_func {
  /*
    If time zone parameters are constants we are caching objects that
    represent them (we use separate from_tz_cached/to_tz_cached members
    to indicate this fact, since NULL is legal value for from_tz/to_tz
    members.
  */
  bool from_tz_cached, to_tz_cached;
  Time_zone *from_tz, *to_tz;

 public:
  Item_func_convert_tz(const POS &pos, Item *a, Item *b, Item *c)
      : Item_datetime_func(pos, a, b, c),
        from_tz_cached(false),
        to_tz_cached(false) {}
  const char *func_name() const override { return "convert_tz"; }
  bool resolve_type(THD *) override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
  void cleanup() override;
};

class Item_func_sec_to_time final : public Item_time_func {
 public:
  Item_func_sec_to_time(const POS &pos, Item *item)
      : Item_time_func(pos, item) {}
  bool resolve_type(THD *) override {
    set_data_type_time(
        std::min(args[0]->decimals, uint8{DATETIME_MAX_DECIMALS}));
    maybe_null = true;
    return false;
  }
  const char *func_name() const override { return "sec_to_time"; }
  bool get_time(MYSQL_TIME *ltime) override;
};

extern const char *interval_names[];

class Item_date_add_interval final : public Item_temporal_hybrid_func {
  String value;
  bool get_date_internal(MYSQL_TIME *res, my_time_flags_t fuzzy_date);
  bool get_time_internal(MYSQL_TIME *res);

 protected:
  bool val_datetime(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override;

 public:
  const interval_type int_type;  // keep it public
  const bool date_sub_interval;  // keep it public
  Item_date_add_interval(const POS &pos, Item *a, Item *b,
                         interval_type type_arg, bool neg_arg)
      : Item_temporal_hybrid_func(pos, a, b),
        int_type(type_arg),
        date_sub_interval(neg_arg) {}
  /**
     POS-less ctor for post-parse construction with implicit addition to THD's
     free_list (see Item::Item() no-argument ctor).
  */
  Item_date_add_interval(Item *a, Item *b, interval_type type_arg, bool neg_arg)
      : Item_temporal_hybrid_func(a, b),
        int_type(type_arg),
        date_sub_interval(neg_arg) {}
  const char *func_name() const override { return "date_add_interval"; }
  enum Functype functype() const override { return DATEADD_FUNC; }
  bool resolve_type(THD *) override;
  bool eq(const Item *item, bool binary_cmp) const override;
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
};

class Item_extract final : public Item_int_func {
  bool date_value;

 public:
  const interval_type int_type;  // keep it public
  Item_extract(const POS &pos, interval_type type_arg, Item *a)
      : Item_int_func(pos, a), int_type(type_arg) {}
  longlong val_int() override;
  enum Functype functype() const override { return EXTRACT_FUNC; }
  const char *func_name() const override { return "extract"; }
  bool resolve_type(THD *) override;
  bool eq(const Item *item, bool binary_cmp) const override;
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    switch (int_type) {
      case INTERVAL_YEAR:
      case INTERVAL_YEAR_MONTH:
      case INTERVAL_QUARTER:
      case INTERVAL_MONTH:
      /* case INTERVAL_WEEK: Not allowed as partitioning function, bug#57071 */
      case INTERVAL_DAY:
        return !has_date_args();
      case INTERVAL_DAY_HOUR:
      case INTERVAL_DAY_MINUTE:
      case INTERVAL_DAY_SECOND:
      case INTERVAL_DAY_MICROSECOND:
        return !has_datetime_args();
      case INTERVAL_HOUR:
      case INTERVAL_HOUR_MINUTE:
      case INTERVAL_HOUR_SECOND:
      case INTERVAL_MINUTE:
      case INTERVAL_MINUTE_SECOND:
      case INTERVAL_SECOND:
      case INTERVAL_MICROSECOND:
      case INTERVAL_HOUR_MICROSECOND:
      case INTERVAL_MINUTE_MICROSECOND:
      case INTERVAL_SECOND_MICROSECOND:
        return !has_time_args();
      default:
        /*
          INTERVAL_LAST is only an end marker,
          INTERVAL_WEEK depends on default_week_format which is a session
          variable and cannot be used for partitioning. See bug#57071.
        */
        break;
    }
    return true;
  }
};

class Item_typecast_date final : public Item_date_func {
 public:
  Item_typecast_date(Item *a) : Item_date_func(a) { maybe_null = true; }
  Item_typecast_date(const POS &pos, Item *a) : Item_date_func(pos, a) {
    maybe_null = true;
  }

  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  const char *func_name() const override { return "cast_as_date"; }
  enum Functype functype() const override { return TYPECAST_FUNC; }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override;
  const char *cast_type() const { return "date"; }
};

class Item_typecast_time final : public Item_time_func {
  bool detect_precision_from_arg;

 public:
  Item_typecast_time(Item *a) : Item_time_func(a) {
    detect_precision_from_arg = true;
  }
  Item_typecast_time(const POS &pos, Item *a) : Item_time_func(pos, a) {
    detect_precision_from_arg = true;
  }

  Item_typecast_time(const POS &pos, Item *a, uint8 dec_arg)
      : Item_time_func(pos, a) {
    detect_precision_from_arg = false;
    decimals = dec_arg;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  const char *func_name() const override { return "cast_as_time"; }
  enum Functype functype() const override { return TYPECAST_FUNC; }
  bool get_time(MYSQL_TIME *ltime) override;
  const char *cast_type() const { return "time"; }
  bool resolve_type(THD *) override {
    set_data_type_time(detect_precision_from_arg ? args[0]->time_precision()
                                                 : decimals);
    maybe_null = true;
    return false;
  }
};

class Item_typecast_datetime final : public Item_datetime_func {
  bool detect_precision_from_arg;

 public:
  Item_typecast_datetime(Item *a) : Item_datetime_func(a) {
    detect_precision_from_arg = true;
  }
  Item_typecast_datetime(const POS &pos, Item *a) : Item_datetime_func(pos, a) {
    detect_precision_from_arg = true;
  }

  Item_typecast_datetime(const POS &pos, Item *a, uint8 dec_arg)
      : Item_datetime_func(pos, a) {
    detect_precision_from_arg = false;
    decimals = dec_arg;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  const char *func_name() const override { return "cast_as_datetime"; }
  enum Functype functype() const override { return TYPECAST_FUNC; }
  const char *cast_type() const { return "datetime"; }
  bool resolve_type(THD *) override {
    set_data_type_datetime(
        detect_precision_from_arg ? args[0]->datetime_precision() : decimals);
    maybe_null = true;
    return false;
  }
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
};

class Item_func_makedate final : public Item_date_func {
 public:
  Item_func_makedate(const POS &pos, Item *a, Item *b)
      : Item_date_func(pos, a, b) {
    maybe_null = true;
  }
  const char *func_name() const override { return "makedate"; }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override;
};

class Item_func_add_time final : public Item_temporal_hybrid_func {
  const bool is_date;
  int sign;
  bool val_datetime(MYSQL_TIME *time, my_time_flags_t fuzzy_date) override;

 public:
  Item_func_add_time(Item *a, Item *b, bool type_arg, bool neg_arg)
      : Item_temporal_hybrid_func(a, b), is_date(type_arg) {
    sign = neg_arg ? -1 : 1;
  }
  Item_func_add_time(const POS &pos, Item *a, Item *b, bool type_arg,
                     bool neg_arg)
      : Item_temporal_hybrid_func(pos, a, b), is_date(type_arg) {
    sign = neg_arg ? -1 : 1;
  }

  Item_func_add_time(const POS &pos, Item *a, Item *b)
      : Item_func_add_time(pos, a, b, false, false) {}

  bool resolve_type(THD *) override;
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  const char *func_name() const override { return "add_time"; }
};

class Item_func_timediff final : public Item_time_func {
 public:
  Item_func_timediff(const POS &pos, Item *a, Item *b)
      : Item_time_func(pos, a, b) {}
  const char *func_name() const override { return "timediff"; }
  bool resolve_type(THD *) override {
    set_data_type_time(
        std::max(args[0]->time_precision(), args[1]->time_precision()));
    maybe_null = true;
    return false;
  }
  bool get_time(MYSQL_TIME *ltime) override;
};

class Item_func_maketime final : public Item_time_func {
 public:
  Item_func_maketime(const POS &pos, Item *a, Item *b, Item *c)
      : Item_time_func(pos, a, b, c) {
    maybe_null = true;
  }
  bool resolve_type(THD *) override {
    set_data_type_time(
        std::min(args[2]->decimals, uint8{DATETIME_MAX_DECIMALS}));
    return false;
  }
  const char *func_name() const override { return "maketime"; }
  bool get_time(MYSQL_TIME *ltime) override;
};

class Item_func_microsecond final : public Item_int_func {
 public:
  Item_func_microsecond(const POS &pos, Item *a) : Item_int_func(pos, a) {}
  longlong val_int() override;
  const char *func_name() const override { return "microsecond"; }
  enum Functype functype() const override { return MICROSECOND_FUNC; }
  bool resolve_type(THD *) override {
    maybe_null = true;
    return false;
  }
  bool check_partition_func_processor(uchar *) override { return false; }
  bool check_valid_arguments_processor(uchar *) override {
    return !has_time_args();
  }
};

class Item_func_timestamp_diff final : public Item_int_func {
  const interval_type int_type;

 public:
  Item_func_timestamp_diff(const POS &pos, Item *a, Item *b,
                           interval_type type_arg)
      : Item_int_func(pos, a, b), int_type(type_arg) {}
  const char *func_name() const override { return "timestampdiff"; }
  enum Functype functype() const override { return TIMESTAMPDIFF_FUNC; }
  interval_type intervaltype() const { return int_type; }
  longlong val_int() override;
  bool resolve_type(THD *) override {
    maybe_null = true;
    return false;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
};

enum date_time_format {
  USA_FORMAT,
  JIS_FORMAT,
  ISO_FORMAT,
  EUR_FORMAT,
  INTERNAL_FORMAT
};

class Item_func_get_format final : public Item_str_ascii_func {
 public:
  const enum_mysql_timestamp_type type;  // keep it public
  Item_func_get_format(const POS &pos, enum_mysql_timestamp_type type_arg,
                       Item *a)
      : Item_str_ascii_func(pos, a), type(type_arg) {}
  String *val_str_ascii(String *str) override;
  const char *func_name() const override { return "get_format"; }
  bool resolve_type(THD *) override {
    maybe_null = true;
    set_data_type_string(17, default_charset());
    return false;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
};

class Item_func_str_to_date final : public Item_temporal_hybrid_func {
  enum_mysql_timestamp_type cached_timestamp_type;
  void fix_from_format(const char *format, size_t length);

 protected:
  bool val_datetime(MYSQL_TIME *ltime, my_time_flags_t fuzzy_date) override;

 public:
  Item_func_str_to_date(const POS &pos, Item *a, Item *b)
      : Item_temporal_hybrid_func(pos, a, b) {}
  const char *func_name() const override { return "str_to_date"; }
  bool resolve_type(THD *) override;
};

class Item_func_last_day final : public Item_date_func {
 public:
  Item_func_last_day(const POS &pos, Item *a) : Item_date_func(pos, a) {
    maybe_null = true;
  }
  const char *func_name() const override { return "last_day"; }
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
};

class Item_func_internal_update_time final : public Item_datetime_func {
  THD *thd;

 public:
  Item_func_internal_update_time(const POS &pos, PT_item_list *list)
      : Item_datetime_func(pos, list) {}
  enum Functype functype() const override { return DD_INTERNAL_FUNC; }
  const char *func_name() const override { return "internal_update_time"; }
  bool resolve_type(THD *thd) override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
};

class Item_func_internal_check_time final : public Item_datetime_func {
  THD *thd;

 public:
  Item_func_internal_check_time(const POS &pos, PT_item_list *list)
      : Item_datetime_func(pos, list) {}
  enum Functype functype() const override { return DD_INTERNAL_FUNC; }
  const char *func_name() const override { return "internal_check_time"; }
  bool resolve_type(THD *thd) override;
  bool get_date(MYSQL_TIME *res, my_time_flags_t fuzzy_date) override;
};

/* Function prototypes */

bool make_date_time(Date_time_format *format, MYSQL_TIME *l_time,
                    enum_mysql_timestamp_type type, String *str);

#endif /* ITEM_TIMEFUNC_INCLUDED */
