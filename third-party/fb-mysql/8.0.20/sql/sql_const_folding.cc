/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

  @brief Fold constant query expressions. Currently, this is only done at
  optimize time, but should eventually be done as much as possible at prepare
  time. For dynamic parameters, we'd still need to do it at optimize time.

  @defgroup Constant_Folding  Query Optimizer
  @{
*/

#include "sql/sql_const_folding.h"

#include <float.h>                         // DBL_MAX, FLT_MAX
#include <stdint.h>                        // UINT64_MAX
#include <sys/types.h>                     // uint
#include <cstring>                         // memset
#include <utility>                         // swap
#include "decimal.h"                       // E_DEC_FATAL_ERROR
#include "field_types.h"                   // MYSQL_TYPE_DATE
#include "my_dbug.h"                       // DBUG_ASSERT
#include "my_decimal.h"                    // my_decimal, my_decimal_cmp
#include "my_inttypes.h"                   // longlong, ulonglong
#include "my_time.h"                       // TIME_to_longlong_datetime_packed
#include "mysql/udf_registration_types.h"  // INT_RESULT, STRING_RESULT
#include "mysql_time.h"                    // MYSQL_TIME
#include "sql/field.h"                     // Field_real, Field
#include "sql/item.h"                      // Item, Item_field, Item_int
#include "sql/item_cmpfunc.h"              // Item_bool_func2, Item_cond
#include "sql/item_func.h"                 // Item_func, Item_func::LE_FUNC
#include "sql/item_timefunc.h"             // Item_date_literal
#include "sql/sql_class.h"                 // THD
#include "sql/sql_list.h"                  // List_iterator
#include "sql/sql_time.h"                  // my_time_set, actual_decimals
#include "sql/tztime.h"                    // Time_zone, my_tz_UTC
#include "sql_string.h"                    // String
#include "template_utils.h"                // down_cast

/**
  fold_condition uses analyze_field_constant to analyze constants in
  comparisons expressions with a view to fold the expression. The analysis
  will determine the range characteristic of the constant, as represented by
  one value of this set. In the simple case, if the constant is within the range
  of the field to which it is being compared, the value used is RP_INSIDE.
*/
enum Range_placement {
  /**
    The constant is larger than the highest value in the range
  */
  RP_OUTSIDE_HIGH,
  /**
    The constant is lower than the lowest value in the range
  */
  RP_OUTSIDE_LOW,
  /**
    The constant has the value of the lowest value in the range. Only used
    for integer types and the YEAR type.
   */
  RP_ON_MIN,
  /**
    The constant has the value of the highest value in the range. Only used
    for integer types and the YEAR type.
  */
  RP_ON_MAX,
  /**
    The constant has a value within the range, but that is *not* on one of the
    borders if applicable for the type, cf. RP_ON_MIN and RP_ON_MAX.
  */
  RP_INSIDE,
  /**
    The constant has a value within the range, but was truncated. Used
    for DECIMAL if the constant has more precision in the fraction than the
    field and for floating point underflow.
  */
  RP_INSIDE_TRUNCATED,
  /**
    The YEAR type has a discontinuous range {0} U [1901, 2155]. If the constant
    is larger than 0, but less than 1901, we represent that with this value.
  */
  RP_INSIDE_YEAR_HOLE,
  /**
    The constant has been rounded down, i.e.\ to the left on the number line,
    due to restriction on field length (FLOAT or DOUBLE) or because we have a
    FLOAT type and the constant given has a mantissa with more significant
    digits than allowed for FLOAT.
  */
  RP_ROUNDED_DOWN,
  /**
    The constant has been rounded up, i.e.\ to the right on the number line, due
    to restriction on field length (FLOAT or DOUBLE) or because we have a FLOAT
    type and the constant given has a mantissa with more significant digits
    than allowed for FLOAT.
  */
  RP_ROUNDED_UP
};

/**
  Minion of analyze_int_field_constant.
  Round up or down decimal d, then convert constant to int if possible.
  If d has a fraction part, set discount_equal. If it can't be converted to
  int, it is out of range, and we fold by setting place accordingly.

  @param[in]      thd        the session context
  @param[in, out] const_val  the constant we are folding
  @param[in, out] place      the range placement of the constant as analyzed
  @param[in]      f          the field the constant is being compared to
  @param[in]      d          the decimal value of the constant
  @param[in]      up         true if we are to round up, else we round down
  @param[out]     discount_equal
                             set to true of d has a non-zero fraction part

  @return true on error
*/
static bool round_fold_or_convert_dec(THD *thd, Item **const_val,
                                      Range_placement *place, Item_field *f,
                                      my_decimal *d, bool up,
                                      bool *discount_equal) {
  /*
    Round the decimal to next integral value, then try to convert
    that to a longlong
  */
  my_decimal a, b;
  if (my_decimal_floor(E_DEC_FATAL_ERROR, d, &a)) return true;
  if (my_decimal_ceiling(E_DEC_FATAL_ERROR, d, &b)) return true;
  *discount_equal = my_decimal_cmp(&a, &b) != 0;
  longlong ni;
  if (!my_decimal2int(0, /* mask: do not generate error in diagnostic area */
                      up ? &b : &a, f->unsigned_flag, &ni)) {
    Item *ni_item = (f->unsigned_flag ? new (thd->mem_root) Item_uint(ni)
                                      : new (thd->mem_root) Item_int(ni));
    if (ni_item == nullptr) return true;
    thd->change_item_tree(const_val, ni_item);
  } else {
    *place = d->sign() ? RP_OUTSIDE_LOW : RP_OUTSIDE_HIGH;
  }
  return false;
}

/**
  Minion of analyze_int_field_constant.
  Analyze decimal d. Convert decimal constant d to int if d has a zero fraction
  part and is within integer range. If it can't be converted to
  int, it is out of range, and we set place accordingly. If it has a non-zero
  fraction part, we fold. Used for = and <>.

  @param[in]      thd        the session context
  @param[in, out] const_val  the constant we are folding
  @param[in, out] place      the range placement of the constant as analyzed
  @param[in]      f          the field the constant is being compared to
  @param[in]      d          the decimal value of the constant

  @return  true on error
*/
static bool fold_or_convert_dec(THD *thd, Item **const_val,
                                Range_placement *place, Item_field *f,
                                my_decimal *d) {
  my_decimal a, b;
  if (my_decimal_ceiling(E_DEC_FATAL_ERROR, d, &a)) return true;
  if (my_decimal_floor(E_DEC_FATAL_ERROR, d, &b)) return true;
  if (my_decimal_cmp(&a, &b) == 0) {
    // no fraction part, so try to convert to int
    longlong ni;
    if (!my_decimal2int(0 /* mask: want no diagnostics generated */, &a,
                        f->unsigned_flag, &ni)) {
      const auto ni_item =
          (f->unsigned_flag ? new (thd->mem_root) Item_uint(ni)
                            : new (thd->mem_root) Item_int(ni));
      if (ni_item == nullptr) return true;
      thd->change_item_tree(const_val, ni_item);
      return false;
    }
  }
  /*
    Either decimal constant has a fraction, or it's out of all integer
    range, so cannot be true
  */
  *place = RP_OUTSIDE_HIGH;  // well, or low, it doesn't matter for =, <>
  return false;
}

/**
  Minion of analyze_field_constant for integer type fields.

  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param      f         the integer typed field
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param      ft        the function type of the comparison
  @param[out] place     the placement of the const_val relative to
                        the range of f
  @param[out] discount_equal
                        set to true: caller should replace GE with GT or LE
                        with LT.
  @returns   true on error
*/
static bool analyze_int_field_constant(THD *thd, Item_field *f,
                                       Item **const_val, Item_func::Functype ft,
                                       Range_placement *place,
                                       bool *discount_equal) {
  const bool field_unsigned = f->unsigned_flag;
  my_decimal *d = nullptr;
  my_decimal dec;

  switch ((*const_val)->result_type()) {
    case INT_RESULT:
      break;
    case STRING_RESULT: {
      if ((*const_val)->type() == Item::VARBIN_ITEM) {
        /*
          0x digits have STRING_RESULT but are ints in int
          context.
        */
        break;
      }
    }
      // fall-through
    case REAL_RESULT: {
      /*
        Try to convert to decimal. If that fails, we know the constant is out of
        range for integer too. If it can be converted, continue with the decimal
        logic.
      */
      const double v = (*const_val)->val_real();
      int err = double2decimal(v, &dec);

      if (err & E_DEC_OVERFLOW) {
        if (v < 0)
          *place = RP_OUTSIDE_LOW;
        else
          *place = RP_OUTSIDE_HIGH;
        return false;
      }

      if (err & E_DEC_TRUNCATED) {
        /*
          Check for underflow, e.g. 1.7976931348623157E-308 would end up
          as decimal 0.0, which means that the floating point values was
          marginally greater than 0.0, so we "simulate" this by adding 0.1.
          Correspondingly for negative underflow, we subtract 0.1. This is
          OK, because we round later.
        */
        my_decimal n;
        err = int2my_decimal(E_DEC_FATAL_ERROR, 0, false, &n);
        DBUG_ASSERT(err == 0);
        DBUG_ASSERT(my_decimal_cmp(&n, &dec) == 0);
        if (v > 0) {
          // underflow on the positive side
          String s("0.1", thd->charset());
          err = str2my_decimal(E_DEC_FATAL_ERROR, s.ptr(), s.length(),
                               s.charset(), &dec);
          DBUG_ASSERT(err == 0);
        } else {
          String s("-0.1", thd->charset());
          err = str2my_decimal(E_DEC_FATAL_ERROR, s.ptr(), s.length(),
                               s.charset(), &dec);
          DBUG_ASSERT(err == 0);
        }
      }
      d = &dec;
    }
      // fall-through
    case DECIMAL_RESULT: {
      /*
        If out of bounds of longlong, return RP_OUTSIDE_LOW or RP_OUTSIDE_HIGH
        as the case may be. If not, if the decimal has a non-zero fraction,
        equality cannot be true, so just return an RP_OUTSIDE_*.
        For >, >=, < and <=, if we have a non-zero fraction, round up or down
        to closest integer, then proceed with the integer constant logic.
        For equality and a zero fraction convert to integer and proceed with
        the integer constant logic.
      */
      my_decimal d_buff;
      if (d == nullptr) d = (*const_val)->val_decimal(&d_buff);
      if (ft == Item_func::LT_FUNC || ft == Item_func::LE_FUNC) {
        /*
          Round up the decimal to next integral value, then try to convert
          that to a longlong, or short circuit
        */
        if (round_fold_or_convert_dec(thd, const_val, place, f, d, true,
                                      discount_equal))
          return true;
        if (*place != RP_INSIDE) return false;
      } else if (ft == Item_func::GT_FUNC || ft == Item_func::GE_FUNC) {
        /*
          Round down the decimal to next integral value, then try to convert
          that to a longlong
        */
        if (round_fold_or_convert_dec(thd, const_val, place, f, d, false,
                                      discount_equal))
          return true;
        if (*place != RP_INSIDE) return false;
      } else {  // for =, <>
        if (fold_or_convert_dec(thd, const_val, place, f, d)) return true;
        if (*place != RP_INSIDE) return false;
      }
    } break;
    default:
      return false;
  }

  longlong s_max = 0;
  longlong s_min = 0;
  longlong u_max = 0;

  switch (f->field->type()) {
    case MYSQL_TYPE_TINY:
      s_max = INT_MAX8;
      s_min = INT_MIN8;
      u_max = UINT_MAX8;
      break;
    case MYSQL_TYPE_SHORT:
      s_max = INT_MAX16;
      s_min = INT_MIN16;
      u_max = UINT_MAX16;
      break;
    case MYSQL_TYPE_INT24:
      s_max = INT_MAX24;
      s_min = INT_MIN24;
      u_max = UINT_MAX24;
      break;
    case MYSQL_TYPE_LONG:
      s_max = INT_MAX32;
      s_min = INT_MIN32;
      u_max = UINT_MAX32;
      break;
    case MYSQL_TYPE_LONGLONG:
      // special treatment below
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }

  switch (f->field->type()) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG: {
      const longlong v = (*const_val)->val_int();
      const bool value_unsigned = (*const_val)->unsigned_flag;
      if (field_unsigned) {
        /* unsigned field: e.g. for TINYINT, accept [0, 255] */
        if (v < 0 && !value_unsigned) {
          *place = RP_OUTSIDE_LOW;
        } else if (static_cast<ulonglong>(v) > static_cast<ulonglong>(u_max)) {
          *place = RP_OUTSIDE_HIGH;
        } else if (v == u_max) {
          *place = RP_ON_MAX;
        } else if (v == 0) {
          *place = RP_ON_MIN;
        }
      } else {
        /* signed field: e.g. for TINYINT, accept [-128, 127] */
        if (value_unsigned &&
            (static_cast<ulonglong>(v) > static_cast<ulonglong>(s_max))) {
          *place = RP_OUTSIDE_HIGH;
        } else if (v < s_min) {
          *place = RP_OUTSIDE_LOW;
        } else if (v > s_max) {
          *place = RP_OUTSIDE_HIGH;
        } else if (v == s_min) {
          *place = RP_ON_MIN;
        } else if (v == s_max) {
          *place = RP_ON_MAX;
        }
      }
    } break;
    case MYSQL_TYPE_LONGLONG: {
      /*
        Since for 64 bits int we don't have the "luxury" of doing the analysis
        math in a wider int type, the structure is slightly different than the
        checks above.
      */
      const longlong v = (*const_val)->val_int();
      const bool value_unsigned = (*const_val)->unsigned_flag;
      if (field_unsigned) {
        /* unsigned field: accept [0, 18446744073709551615] */
        if (v < 0 && !value_unsigned) {
          *place = RP_OUTSIDE_LOW;
          // No need to check (ulonglong)v > (ulonglong)UINT64_MAX): impossible
        } else if (static_cast<ulonglong>(v) == UINT64_MAX) {
          *place = RP_ON_MAX;
        } else if (v == 0) {
          *place = RP_ON_MIN;
        }
      } else {
        /* signed field: accept [-9223372036854775808 9223372036854775807] */
        if (value_unsigned &&
            (static_cast<ulonglong>(v) > static_cast<ulonglong>(INT_MAX64))) {
          *place = RP_OUTSIDE_HIGH;
          // No need to check v < static_cast<ulonglong>(INT_MIN64): impossible
        } else if (!value_unsigned && v == INT_MIN64) {
          *place = RP_ON_MIN;
        } else if (!value_unsigned && v == INT_MAX64) {
          *place = RP_ON_MAX;
        }
      }
    } break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }

  return false;
}

/**
  Minion of analyze_field_constant for decimal type fields

  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param      f         the decimal typed field
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param      ft        the function type of the comparison
  @param[out] place     the placement of the const_val relative to
                        the range of f
  @param[out] negative  true if the constant is has a (minus) sign
  @returns   true on error
*/
static bool analyze_decimal_field_constant(THD *thd, const Item_field *f,
                                           Item **const_val,
                                           Item_func::Functype ft,
                                           Range_placement *place,
                                           bool *negative) {
  const auto fd = down_cast<const Field_new_decimal *>(f->field);
  const int f_frac = fd->dec;
  const int f_intg = fd->precision - f_frac;
  bool was_string_or_real = false;
  int err;

  Item_result ir = (*const_val)->result_type();

  /*
    If we have a string, it may be anything inside, so assume decimal,
    which also recognizes real constants, btw. The exception is the
    0xnnn numbers, which also have STRING result type, but should be treated
    the same as ints.
  */
  if (ir == STRING_RESULT && (*const_val)->type() != Item::VARBIN_ITEM) {
    was_string_or_real = true;
    ir = DECIMAL_RESULT;
  }

  switch (ir) {
    case STRING_RESULT:
    case INT_RESULT: {
      my_decimal tmp;
      const auto *const d = (*const_val)->val_decimal(&tmp);
      DBUG_ASSERT(decimal_actual_fraction(d) == 0);
      const int actual_intg = decimal_intg(d);

      if (actual_intg > f_intg) {  // overflow
        *place = d->sign() ? RP_OUTSIDE_LOW : RP_OUTSIDE_HIGH;
        break;
      }

      // inside, but convert const to decimal, similar precision as field
      my_decimal tmp_ext;
      if (decimal_round(d, &tmp_ext, f_frac, FLOOR)) return true;
      const auto new_dec = new (thd->mem_root) Item_decimal(&tmp_ext);
      if (new_dec == nullptr) return true;
      thd->change_item_tree(const_val, new_dec);

    } break;
    case REAL_RESULT: {
      my_decimal val_dec;
      double v = (*const_val)->val_real();
      err = double2decimal(v, &val_dec);

      if (err & E_DEC_OVERFLOW) {
        if (v < 0)
          *place = RP_OUTSIDE_LOW;
        else
          *place = RP_OUTSIDE_HIGH;
        return false;
      }

      if (err & E_DEC_TRUNCATED) {
        /*
          Check for underflow, e.g. 1.7976931348623157E-308 would end up
          as decimal 0.0, which means that the floating point values was
          marginally greater that 0.0. So, we convert to 0 and compensate
          in logic for RP_INSIDE_TRUNCATED in fold_condition.
          Similar logic for negative underflow.
        */
        *place = RP_INSIDE_TRUNCATED;
        *negative = v < 0;
        my_decimal tmp;
        err = longlong2decimal(0, &tmp);
        DBUG_ASSERT(err == 0);

        widen_fraction(f_frac, &tmp);
        const auto new_dec = new (thd->mem_root) Item_decimal(&tmp);
        if (new_dec == nullptr) return true;
        thd->change_item_tree(const_val, new_dec);
        return false;
      }
      was_string_or_real = true;
    }
      // fall-thru
    case DECIMAL_RESULT: {
      /*
        Decimal constant can have different range and precision
      */

      // Dictionary info about decimal field:
      // Compute actual (minimal) decimal type of the constant
      my_decimal buff, *d;
      d = (*const_val)->val_decimal(&buff);
      const int actual_frac = decimal_actual_fraction(d);
      const int actual_intg = decimal_intg(d);
      const bool overflow = actual_intg > f_intg;
      const bool truncation = actual_frac > f_frac;

      if (overflow) {
        *place = d->sign() ? RP_OUTSIDE_LOW : RP_OUTSIDE_HIGH;
      } else if (truncation) {
        my_decimal cpy;
        my_decimal2decimal(d, &cpy);

        if (ft == Item_func::GT_FUNC || ft == Item_func::GE_FUNC ||
            ft == Item_func::LT_FUNC || ft == Item_func::LE_FUNC) {
          // adjust precision to same as field
          if (decimal_round(&cpy, &cpy, f_frac, cpy.sign() ? CEILING : FLOOR))
            return true;
          *negative = cpy.sign();
          const auto new_dec = new (thd->mem_root) Item_decimal(&cpy);
          if (new_dec == nullptr) return true;
          thd->change_item_tree(const_val, new_dec);
          *place = RP_INSIDE_TRUNCATED;
        } else {
          *place = RP_OUTSIDE_HIGH;  // =, <>, so outside: convention use high
        }
      } else if (d->frac > f_frac) {
        // truncate zeros
        my_decimal cpy;
        my_decimal2decimal(d, &cpy);
        if (decimal_round(&cpy, &cpy, f_frac, TRUNCATE)) return true;
        const auto new_dec = new (thd->mem_root) Item_decimal(&cpy);
        if (new_dec == nullptr) return true;
        thd->change_item_tree(const_val, new_dec);
      } else if (actual_frac < f_frac) {
        my_decimal cpy;
        my_decimal2decimal(d, &cpy);
        widen_fraction(f_frac, &cpy);

        const auto new_dec = new (thd->mem_root) Item_decimal(&cpy);
        if (new_dec == nullptr) return true;
        thd->change_item_tree(const_val, new_dec);
      } else if (was_string_or_real) {
        // Make a decimal constant instead
        const auto new_dec = new (thd->mem_root) Item_decimal(d);
        if (new_dec == nullptr) return true;
        thd->change_item_tree(const_val, new_dec);
      }
    } break;
    default:
      break;
  }

  return false;
}

/**
  Minion of analyze_field_constant for real type fields

  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param      f         the real (FLOAT, DOUBLE) typed field
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param[out] place     the placement of the const_val relative to
  @returns   true on error
*/
static bool analyze_real_field_constant(THD *thd, Item_field *f,
                                        Item **const_val,
                                        Range_placement *place) {
  switch ((*const_val)->result_type()) {
    case REAL_RESULT:
    case STRING_RESULT:
    case INT_RESULT:
    case DECIMAL_RESULT:
      /*
        Can all be safely converted to a double value. Although we may lose
        precision, we won't overflow. Any constants too large for double will
        have been caught at parsing time.
      */
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
      break;
  }

  double v = (*const_val)->val_real();
  const double orig = v;
  const bool is_float = f->field->type() == MYSQL_TYPE_FLOAT;
  /*
    This check/truncation also handles fixed # decimals digits real types, a
    MySQL extension, e.g. FLOAT(5,2).
  */
  const auto fd = down_cast<Field_real *>(f->field);
  Field_real::Truncate_result err =
      fd->truncate(&v, (is_float ? FLT_MAX : DBL_MAX));
  if (err != Field_real::TR_OK) {
    *place = (err == Field_real::TR_NEGATIVE_OVERFLOW ? RP_OUTSIDE_LOW
                                                      : RP_OUTSIDE_HIGH);
    return false;
  }

  if (!fd->not_fixed) {
    if (is_float) {
      v = (float)v;
      /*
        The number can now have been rounded up or down due to restriction on
        field length or a float orig_v in presence of longer mantissa than
        allowed for float.
      */
      if (v > (float)orig || v < (float)orig) {
        *place = v > orig ? RP_ROUNDED_UP : RP_ROUNDED_DOWN;
      }
    } else {
      if (v > orig || v < orig) {
        *place = v > orig ? RP_ROUNDED_UP : RP_ROUNDED_DOWN;
      }
    }
  }

  /*
    Lastly, convert to double representation.
  */
  if (v != orig || (*const_val)->type() != Item::REAL_ITEM) {
    const auto new_const =
        new (thd->mem_root) Item_float(v, DECIMAL_NOT_SPECIFIED);
    if (new_const == nullptr) return true;
    thd->change_item_tree(const_val, new_const);
  }

  return false;
}

/**
  Minion of analyze_field_constant for YEAR type fields

  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param[out] place     the placement of the const_val relative to
  @returns   true on error
*/
static bool analyze_year_field_constant(THD *thd, Item **const_val,
                                        Range_placement *place) {
  if ((*const_val)->data_type() == MYSQL_TYPE_YEAR) {
    /*
      Decimal, real and string constants have already been converted to int if
      they allowed year values, and these as well as integer constants that are
      allowed year values have been typed as MYSQL_TYPE_YEAR, cf.
      convert_constant_item called during type resolution.
    */
    DBUG_ASSERT((*const_val)->result_type() == INT_RESULT);
    const longlong year = (*const_val)->val_int();

    if (year == 0)
      *place = RP_ON_MIN;
    else if (year == Field_year::MAX_YEAR)
      *place = RP_ON_MAX;
    else
      *place = RP_INSIDE;

    return false;
  }

  // The constant is outside allowed year values, so fold.
  const Item_result ir = (*const_val)->result_type();
  switch (ir) {
    case STRING_RESULT:
    case DECIMAL_RESULT:
    case REAL_RESULT:
    case INT_RESULT: {
      const double year = (*const_val)->val_real();
      if (year > Field_year::MAX_YEAR) {
        *place = RP_OUTSIDE_HIGH;
      } else if (year < 0.0) {
        *place = RP_OUTSIDE_LOW;
      } else if (year > 0.0 && year < Field_year::MIN_YEAR) {
        /*
          These values can be given as constants, but are not allowed to be
          stored in the field, so an = or <> comparison can be folded. For
          other operations, we treat the constants as if it is inside range,
          since a year field may contain 0.
        */
        *place = RP_INSIDE_YEAR_HOLE;
        // Make sure we have an int constant
        if (ir != INT_RESULT) {
          const auto i = new (thd->mem_root) Item_int(static_cast<int>(year));
          if (i == nullptr) return true;
          thd->change_item_tree(const_val, i);
        }
      } else {
        if (ir != INT_RESULT) {
          const auto i = new (thd->mem_root) Item_int(static_cast<int>(year));
          if (i == nullptr) return true;
          thd->change_item_tree(const_val, i);
        }
      }
    } break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
      break;
  }
  return false;
}

/**
  Minion of analyze_field_constant for TIMESTAMP, DATETIME, DATE fields

  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param      f         the field
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param[out] place     the placement of the const_val relative to
  @returns   true on error

*/
static bool analyze_timestamp_field_constant(THD *thd, const Item_field *f,
                                             Item **const_val,
                                             Range_placement *place) {
  const auto rtype = (*const_val)->result_type();
  switch (rtype) {
    case STRING_RESULT:  // This covers both string and TIMESTAMP literals
    case INT_RESULT: {
      MYSQL_TIME ltime =
          my_time_set(0, 0, 0, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATETIME);
      MYSQL_TIME_STATUS status{0, 0, 0};
      if (rtype == STRING_RESULT) {
        String buf, *res = (*const_val)->val_str(&buf);
        /*
          Some wrong values are still compared as DATETIME, e.g. '2018-02-31
          06:14:07' (illegal day in February), while worse values lead to
          comparison as strings (e.g. '2018'). Cf. comment on Bug#27692509. Any
          warnings have already been given earlier, so ignore.
        */
        if (get_mysql_time_from_str_no_warn(thd, res, &ltime, &status)) {
          // Could not fold, so leave untouched.
          return false;
        }

        /*
          A date constant being compared to a timestamp or datetime field is ok,
          convert it to a datetime literal, using 00:00:00 as the time.
          If the field type is DATE, we also use a MYSQL_TIMESTAMP_DATETIME
          constant with zero time part.
        */
        if (ltime.time_type == MYSQL_TIMESTAMP_DATE)
          ltime.time_type = MYSQL_TIMESTAMP_DATETIME;

      } else if (rtype == INT_RESULT) {
        if ((*const_val)->data_type() == MYSQL_TYPE_TIMESTAMP ||
            (*const_val)->data_type() == MYSQL_TYPE_DATETIME ||
            (*const_val)->data_type() == MYSQL_TYPE_DATE) {
          TIME_from_longlong_datetime_packed(&ltime, (*const_val)->val_int());
        } else {
          /*
            The integral constant could not be interpreted as a datetime value,
            the operands will be compared using double.
          */
          return false;
        }
      }

      MYSQL_TIME ltime_utc = ltime;
      const enum_field_types ft = f->field->type();

      if (ft == MYSQL_TYPE_TIMESTAMP) {
        /*
          Convert constant to timeval, if it fits. If not, we are out of
          range for a TIMESTAMP. The timeval is UTC since epoch.
        */
        int warnings = 0;
        struct timeval tm;
        std::memset(&tm, 0, sizeof(tm));
        int zeros = 0;
        zeros += ltime.year == 0;
        zeros += ltime.month == 0;
        zeros += ltime.day == 0;
        if (zeros == 0 || zeros == 3) {  // Cf. NO_ZERO_DATE, NO_ZERO_IN_DATE
          datetime_with_no_zero_in_date_to_timeval(thd, &ltime, &tm, &warnings);
          if ((warnings & MYSQL_TIME_WARN_OUT_OF_RANGE) != 0) {
            /*
              For RP_OUTSIDE_HIGH, this check may not catch case where field
              type has no/fewer fraction digits than the constant. This will
              be caught below.
            */
            *place = ltime.year < 1971 ? RP_OUTSIDE_LOW : RP_OUTSIDE_HIGH;
            return false;
          }
        }  // else zero in date => 0 timeval too

        /*
          Convert timestamp's timeval to UTC ltime and pack it so we can
          compare with min/max, unless it is 0 or has a zero date part (year,
          month or day)
        */
        if (tm.tv_sec != 0) {
          /* '2038-01-19 03:14:07.[999999]' */
          MYSQL_TIME max_timestamp = my_time_set(
              TIMESTAMP_MAX_YEAR, 1, 19, 3, 14, 7,
              max_fraction(
                  down_cast<const Field_temporal *>(f->field)->decimals()),
              false, MYSQL_TIMESTAMP_DATETIME);

          /* '1970-01-01 00:00:01.[000000]' */
          MYSQL_TIME min_timestamp = my_time_set(1970, 1, 1, 0, 0, 1, 0, false,
                                                 MYSQL_TIMESTAMP_DATETIME);

          // We store in UTC, so use as is
          const longlong max_t =
              TIME_to_longlong_datetime_packed(max_timestamp);
          const longlong min_t =
              TIME_to_longlong_datetime_packed(min_timestamp);

          my_tz_UTC->gmt_sec_to_TIME(&ltime_utc, tm);
          const longlong cnst = TIME_to_longlong_datetime_packed(ltime_utc);

          if (cnst > max_t) {
            *place = RP_OUTSIDE_HIGH;
            return false;
          }

          if (cnst < min_t) {
            /*
              A zero ltime (if error in str_to_datetime) before GMT conversion
              ends up as 1970-01-01 00:00:00 which get us here.
            */
            *place = RP_OUTSIDE_LOW;
            return false;
          }
        }  // else: 0 timevalue
      }    // else: not TIMESTAMP field

      /*
        We do not try to truncate the number of decimal digits in the
        constant if it exceeds the number of allowed decimals in the
        type of the field. This could be improved. If we want to try that,
        we could use RP_INSIDE_TRUNCATED. This could lead to folding away of
        =, <>.
      */
      if ((*const_val)->type() == Item::FUNC_ITEM &&
          down_cast<Item_func *>(*const_val)->functype() ==
              Item_func::DATETIME_LITERAL) {
        /* User supplied an ok literal */
      } else {
        Item *i;
        /*
          Make a DATETIME literal, unless the field is a DATE and the constant
          has zero time, in which case we make a DATE literal
        */
        if (ft == MYSQL_TYPE_DATE) {
          ltime.time_type = MYSQL_TIMESTAMP_DATE;
          if (ltime.hour == 0 && ltime.minute == 0 && ltime.second == 0 &&
              ltime.second_part == 0) {
            /* OK, time part is zero, so trivial type change */
          } else {
            /* truncate time part: must adjust operators */
            ltime.hour = 0;
            ltime.minute = 0;
            ltime.second = 0;
            ltime.second_part = 0;
            *place = RP_INSIDE_TRUNCATED;
          }
          i = new (thd->mem_root) Item_date_literal(&ltime);
        } else {
          i = new (thd->mem_root) Item_datetime_literal(
              &ltime, actual_decimals(&ltime), thd->time_zone());
        }
        if (i == nullptr) return true;
        thd->change_item_tree(const_val, i);
      }
    } break;
    case REAL_RESULT:
    case DECIMAL_RESULT:
      /*
        The number could not be interpreted as datetime, so
        compares as double.
      */
      break;
    default:
      break;
  }

  return false;
}

/*
  Minion of analyze_field_constant for TIME fields. Creates a TIME literal
  of a valid TIME constant.
*/
static bool analyze_time_field_constant(THD *thd, Item **const_val) {
  if (!((*const_val)->result_type() == INT_RESULT &&
        (*const_val)->data_type() == MYSQL_TYPE_TIME)) {
    // Not a TIME constant. Compare as string or double.
    return false;
  }

  /*
    An OK TIME constant, represented as Item_time_with_ref.
    Note that excessive decimals have already been rounded, so there is no
    opportunity for folding. This is in contrast to DATETIME/TIMESTAMP
    btw, which retains any excessive decimals digits when comparing.
    Cf. Bug#28320529
  */
  MYSQL_TIME ltime;
  TIME_from_longlong_time_packed(&ltime, (*const_val)->val_time_temporal());
  auto i =
      new (thd->mem_root) Item_time_literal(&ltime, actual_decimals(&ltime));
  if (i == nullptr) return true;
  thd->change_item_tree(const_val, i);
  return false;
}

/**
  Analyze a constant's placement within (or without) the type range of the
  field f. Also normalize the given constant to the type of the field if
  applicable.

  @param      thd       the session context
  @param      f         the field
  @param[out] const_val a pointer to an item tree pointer containing the
                        constant (at execution time). May be modified if
                        we replace or fold the constant.
  @param      func      the function of the comparison
  @param[out] place     the placement of the const_val relative to
                        the range of f
  @param[out] discount_equal
                        set to true: caller should replace GE with GT or LE
                        with LT.
  @param[out] negative  true if the constant (decimal) is has a (minus) sign
  @returns   true on error
*/
static bool analyze_field_constant(THD *thd, Item_field *f, Item **const_val,
                                   Item_func *func, Range_placement *place,
                                   bool *discount_equal, bool *negative) {
  *place = RP_INSIDE;  // a priori

  if ((*const_val)->is_null()) return false;
  if (thd->is_error()) return true;

  const auto ft = func->functype();
  switch (f->data_type()) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
      return analyze_int_field_constant(thd, f, const_val, ft, place,
                                        discount_equal);
    case MYSQL_TYPE_NEWDECIMAL:
      return analyze_decimal_field_constant(thd, f, const_val, ft, place,
                                            negative);
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      return analyze_real_field_constant(thd, f, const_val, place);
    case MYSQL_TYPE_YEAR:
      return analyze_year_field_constant(thd, const_val, place);
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATE:
      return analyze_timestamp_field_constant(thd, f, const_val, place);
    case MYSQL_TYPE_TIME:
      return analyze_time_field_constant(thd, const_val);
    default:
      break;
  }
  return false;
}

/**
  We have found that the constant's value makes the predicate always true or
  false (modulo field nullability), so simplify or remove the condition
  according to this table:
  <pre>
  always_true == true

                   !top_level_item        top_level_item
                ------------------------------------------
     nullable   |  IF(NULL, NULL, 1)  |   IS NOT NULL    |
    !nullable   |  TRUE (1)           |   COND_TRUE      |
                ------------------------------------------

  always_true == false

                   !top_level_item        top_level_item
                ------------------------------------------
     nullable   |  field <> field [*] |   COND_FALSE     |
    !nullable   |  FALSE (0)          |   COND_FALSE     |
                ------------------------------------------

  [*] for the "<=>" operator, we fold to FALSE (0) in this case.
  </pre>

  @param      thd         current session context
  @param      ref_or_field
                          a field (that is being being compared to a constant)
                          or a ref to such a field.
  @param      ft          function type
  @param      always_true true if the predicate is found to be always true
  @param      manifest_result
                          true if we should create an item to represent the
                          truth value (and replace the condition with it)
                          instead of returning nullptr in *cond_value and
                          {Item::COND_TRUE, Item::COND_FALSE} in *cond_value
  @param[out] retcond     a pointer to the condition altered by this method
  @param[out] cond_value  the resulting condition value

  @returns true on error
*/
static bool fold_or_simplify(THD *thd, Item *ref_or_field,
                             Item_func::Functype ft, bool always_true,
                             bool manifest_result, Item **retcond,
                             Item::cond_result *cond_value) {
  Item *i = nullptr;
  const int is_top_level =
      ft == Item_func::MULT_EQUAL_FUNC ||
      down_cast<Item_bool_func2 *>(*retcond)->ignore_unknown();
  if (always_true) {
    if (ref_or_field->maybe_null) {
      if (is_top_level) {
        i = new (thd->mem_root) Item_func_isnotnull(ref_or_field);
      } else {
        Item *arg0 = new (thd->mem_root) Item_func_isnull(ref_or_field);
        if (arg0 == nullptr) return true;
        Item *arg1 = new (thd->mem_root) Item_null();
        if (arg1 == nullptr) return true;
        Item *arg2 = new (thd->mem_root) Item_int(1, 1);
        if (arg2 == nullptr) return true;
        i = new (thd->mem_root) Item_func_if(arg0, arg1, arg2);
      }
    } else {
      if (is_top_level && !manifest_result) {
        *cond_value = Item::COND_TRUE;
        *retcond = nullptr;
        return false;
      }
      i = new (thd->mem_root) Item_func_true();
    }
  } else {
    if (is_top_level && !manifest_result) {
      *cond_value = Item::COND_FALSE;
      *retcond = nullptr;
      return false;
    }

    if (ref_or_field->maybe_null && ft != Item_func::EQUAL_FUNC) {
      i = new (thd->mem_root) Item_func_ne(ref_or_field, ref_or_field);
    } else {
      i = new (thd->mem_root) Item_func_false();
    }
  }

  if (i == nullptr || i->fix_fields(thd, retcond)) return true;
  thd->change_item_tree(retcond, i);
  *cond_value = Item::COND_OK;
  return false;
}

/*
  Call fold_condition on all the function's arguments.

  @param thd    the session state
  @param cond   the function
  @returns true on error
*/
static bool fold_arguments(THD *thd, Item_func *func) {
  for (uint i = 0; i < func->argument_count(); i++) {
    Item::cond_result cv;
    Item **args = func->arguments();
    if (fold_condition(thd, args[i], args + i, &cv, true)) return true;
  }
  func->update_used_tables();
  return false;
}

/**
 Call fold_condition on all the conditions's arguments.

 @param thd    the session state
 @param cond   the condition
 @returns true on error
 */
static bool fold_arguments(THD *thd, Item_cond *cond) {
  List_iterator<Item> li(*cond->argument_list());
  Item *item;

  while ((item = li++)) {
    Item::cond_result cv;
    if (fold_condition(thd, item, li.ref(), &cv, true)) return true;
  }
  cond->update_used_tables();
  return false;
}

/**
  Switch places of low, high, max and min
*/
static Range_placement map_less_to_greater(Range_placement place) {
  switch (place) {
    case RP_OUTSIDE_HIGH:
      place = RP_OUTSIDE_LOW;
      break;
    case RP_OUTSIDE_LOW:
      place = RP_OUTSIDE_HIGH;
      break;
    case RP_ON_MIN:
      place = RP_ON_MAX;
      break;
    case RP_ON_MAX:
      place = RP_ON_MIN;
      break;
    case RP_INSIDE:
    case RP_INSIDE_YEAR_HOLE:
    case RP_INSIDE_TRUNCATED:
    case RP_ROUNDED_DOWN:
    case RP_ROUNDED_UP:
      break;
  }
  return place;
}

/**
  Possibly change comparison operator because we have rounded the constant in
  some direction. Depending on the case, we change:
  <pre>
         <      to        <=
         >      to        >=
         <=     to        <
         >=     to        >
  </pre>
  @param[in]     thd            session thread
  @param[in]     func           the comparison operator
  @param[in]     left_has_field true if &lt;field&gt; &lt;cmp&gt; &lt;const&gt;,
                                false if inverse
  @param[in]     inverse        invert the logic
  @param[in]     ref_or_field   the field or a ref to it
  @param[in]     c              the constant
  @param[in,out] retcond        the resulting condition
  @returns true on error
*/
static bool adjust_cmp_op(THD *thd, Item_func *func, bool left_has_field,
                          bool inverse, Item *ref_or_field, Item *c,
                          Item **retcond) {
  if (inverse) {
    left_has_field = !left_has_field;
    std::swap(ref_or_field, c);
  }

  Item *new_cmp = nullptr;

  if (func->functype() == Item_func::LT_FUNC && left_has_field)
    new_cmp = new (thd->mem_root) Item_func_le(ref_or_field, c);
  else if (func->functype() == Item_func::GT_FUNC && !left_has_field)
    new_cmp = new (thd->mem_root) Item_func_ge(c, ref_or_field);
  else if (func->functype() == Item_func::LE_FUNC && !left_has_field)
    new_cmp = new (thd->mem_root) Item_func_lt(c, ref_or_field);
  else if (func->functype() == Item_func::GE_FUNC && left_has_field)
    new_cmp = new (thd->mem_root) Item_func_gt(ref_or_field, c);
  else
    return false;

  if (new_cmp == nullptr || new_cmp->fix_fields(thd, &new_cmp)) return true;
  thd->change_item_tree(retcond, new_cmp);
  return false;
}

/**
  Replace condition in retcond with "=" (i.e.\ Item_func_eq)
  and set cond_value to Item::COND_OK.

  @param         thd  session context
  @param         ref_or_field
                      the field being compared to a constant in the retcond
  @param         c    the constant being compared to a field in the retcond
  @param[in,out] retcond
                      the comparison condition
  @param[out]    cond_value
                      the resulting condition value
  @returns true on error
*/
static bool simplify_to_equal(THD *thd, Item *ref_or_field, Item *c,
                              Item **retcond, Item::cond_result *cond_value) {
  Item *eq = new (thd->mem_root) Item_func_eq(ref_or_field, c);
  if (eq == nullptr) return true;
  if (eq->fix_fields(thd, &eq)) return true;
  thd->change_item_tree(retcond, eq);
  *cond_value = Item::COND_OK;
  return false;
}

// Main entrypoint for this module. See Doxygen comments in sql_const_folding.h
bool fold_condition(THD *thd, Item *cond, Item **retcond,
                    Item::cond_result *cond_value, bool manifest_result) {
  // A priori result, unless we find otherwise below
  *cond_value = Item::COND_OK;
  *retcond = cond;

  const Item::Type cond_type = cond->type();
  if (!(cond_type == Item::FUNC_ITEM || cond_type == Item::COND_ITEM))
    return false;

  if (cond_type == Item::COND_ITEM) {
    const auto and_or = down_cast<Item_cond *>(cond);
    return fold_arguments(thd, and_or);
  }

  Item_func *const func = down_cast<Item_func *>(cond);
  Item_func::Functype func_type = func->functype();

  switch (func_type) {
    case Item_func::ISNOTNULL_FUNC:
      if (func->arguments()[0]->maybe_null) {
        return fold_arguments(thd, func);
      }

      /* The test can be elided. If top level, drop the condition */
      if (manifest_result) {
        const auto i = new (thd->mem_root) Item_func_true();
        if (i == nullptr) return true;
        thd->change_item_tree(retcond, i);
      } else {
        *cond_value = Item::COND_TRUE;
        *retcond = nullptr;
      }
      return false;
    case Item_func::EQ_FUNC:
    case Item_func::NE_FUNC:
    case Item_func::LT_FUNC:
    case Item_func::LE_FUNC:
    case Item_func::GE_FUNC:
    case Item_func::GT_FUNC:
    case Item_func::EQUAL_FUNC:
    case Item_func::MULT_EQUAL_FUNC:
      break;
    default:
      /* Not a comparison function, so fold its args instead */
      return fold_arguments(thd, func);
  }

  Item **args = nullptr;

  if (func_type != Item_func::MULT_EQUAL_FUNC) {
    args = func->arguments();
  } else {
    /*
      Impedance mispatch between how field, constant is represented for the
      normal comparison functions and multi-equal: we use a scratch two args
      array and update the multi-equal's normal constant later.
    */
    const auto equal = down_cast<Item_equal *>(func);
    // Use first field:  any one will do: they have the same type
    equal->m_const_folding[0] = equal->get_first();
    equal->m_const_folding[1] = equal->get_const();
    args = equal->m_const_folding;
  }

  bool seen_field = false;
  bool seen_constant = false;
  for (int i = 0; i < 2; ++i) {
    if (args[i] == nullptr) continue;  // multi-equal with no constant
    const auto real_type = args[i]->real_item()->type();
    const auto type = args[i]->type();
    if (real_type == Item::FIELD_ITEM) {
      /*
        Using real_item lets us cover fields pointed to by Item_refs for HAVING
        conditions. For conditions on fields from subqueries (also refs),
        temporary file field items are widened to longlong for integer
        types[1], so typically less folding can then happen based on out of
        range constants.
        [1] See `create_tmp_field_from_item' case INT_RESULT.
      */
      seen_field = true;
    } else if (args[i]->const_for_execution() && type != Item::SUBSELECT_ITEM) {
      /*
        Re test on Item::SUBSELECT_ITEM above: we exclude optimize time
        evaluation of constant subqueries for now; since it could still be
        expensive to evaluate and we have no cost model to decide whether
        folding it would save total time spent. It may turn out not to be
        executed at all.
      */
      if (type != Item::FUNC_ITEM) {
        seen_constant = true;
      } else {
        /*
          This "else"-block covers constants that have the shape of
          function calls. We only try to fold two kinds of functions here:
          1) Monadic minus, 2) Item_datetime_literal.
        */
        const auto f = down_cast<Item_func *>(args[i]);
        const auto ft = f->functype();
        if ((ft == Item_func::NEG_FUNC &&
             f->arguments()[0]->basic_const_item()) ||
            ft == Item_func::DATETIME_LITERAL) {
          seen_constant = true;
        }
      }
    }
  }

  if (!(seen_field && seen_constant)) {
    /*
      This comparison function doesn't have the simple form required, so
      instead, try to fold inside its arguments
    */
    return fold_arguments(thd, func);
  }

  const auto arg0_orig = args[0];
  const auto arg1_orig = args[1];

  const bool left_has_field = args[0]->real_item()->type() == Item::FIELD_ITEM;
  const auto ref_or_field = args[!left_has_field];

  Item **c = &args[left_has_field];

  Range_placement place;
  bool discount_eq = false;
  bool negative = false;  // used for decimal constants only

  if (analyze_field_constant(
          thd, down_cast<Item_field *>(args[!left_has_field]->real_item()), c,
          func, &place, &discount_eq, &negative))
    return true; /* purecov: inspected */

  if (discount_eq &&
      (func_type == Item_func::LE_FUNC || func_type == Item_func::GE_FUNC)) {
    bool ignore_unknown = down_cast<Item_bool_func2 *>(cond)->ignore_unknown();
    if (func_type == Item_func::LE_FUNC) {
      if (!(cond = new (thd->mem_root) Item_func_lt(args[0], args[1])))
        return true; /* purecov: inspected */
    } else {
      if (!(cond = new (thd->mem_root) Item_func_gt(args[0], args[1])))
        return true; /* purecov: inspected */
    }
    auto cond_alias = down_cast<Item_bool_func2 *>(cond);
    if (ignore_unknown) cond_alias->apply_is_true();
    if (cond->fix_fields(thd, &cond)) return true;
    func_type = cond_alias->functype();
    thd->change_item_tree(retcond, cond);
    args = cond_alias->arguments();
    c = &args[left_has_field];
  }

  // Fold >, >= handling with <, <=
  if (func_type == Item_func::GT_FUNC || func_type == Item_func::GE_FUNC) {
    place = map_less_to_greater(place);
    func_type = func_type == Item_func::GT_FUNC ? Item_func::LT_FUNC
                                                : Item_func::LE_FUNC;
  }

  switch (func_type) {
    case Item_func::EQ_FUNC:
    case Item_func::EQUAL_FUNC:
    case Item_func::NE_FUNC:
    case Item_func::MULT_EQUAL_FUNC:
      switch (place) {
        case RP_OUTSIDE_HIGH:
        case RP_OUTSIDE_LOW:
        case RP_INSIDE_TRUNCATED:
        case RP_INSIDE_YEAR_HOLE:
        case RP_ROUNDED_DOWN:
        case RP_ROUNDED_UP:
          if (fold_or_simplify(thd, ref_or_field, func_type,
                               func_type == Item_func::NE_FUNC, manifest_result,
                               retcond, cond_value))
            return true; /* purecov: inspected */
          break;
        case RP_INSIDE:
        case RP_ON_MAX:
        case RP_ON_MIN:
          break;
      }
      if (func_type == Item_func::MULT_EQUAL_FUNC && (*retcond != nullptr)) {
        // The constant may have been modified, update the multi-equal
        const auto equal = down_cast<Item_equal *>(func);
        DBUG_ASSERT(equal->m_const_folding[1] != nullptr);  // the constant
        equal->set_const(equal->m_const_folding[1]);
      }
      break;
    case Item_func::LT_FUNC:
    case Item_func::LE_FUNC:
      switch (place) {
        case RP_ROUNDED_DOWN: {  // real fields only
          /*
            Example:
            If we had a float(5,2) and the predicate
                      field < 123.123
            after truncation of the constant we have:
                      field < 123.12
            but what if the field has the value 123.12, i.e. 123.120?
            This is indeed smaller than 123.123, so we need to transform the
            predicate to
                      field <= 123.12
            This guarantees the semantics of MySQL that states we evaluate in
            double precision.
          */
          if (adjust_cmp_op(thd, func, left_has_field, false, ref_or_field, *c,
                            retcond))
            return true; /* purecov: inspected */

        } break;
        case RP_ROUNDED_UP: {  // real fields only
          if (adjust_cmp_op(thd, func, left_has_field, true, ref_or_field, *c,
                            retcond))
            return true; /* purecov: inspected */
        } break;
        case RP_OUTSIDE_HIGH:
          if (fold_or_simplify(thd, ref_or_field, func_type, left_has_field,
                               manifest_result, retcond, cond_value))
            return true; /* purecov: inspected */
          break;
        case RP_OUTSIDE_LOW:
          if (fold_or_simplify(thd, ref_or_field, func_type, !left_has_field,
                               manifest_result, retcond, cond_value))
            return true; /* purecov: inspected */
          break;
        case RP_ON_MIN:
          if (func_type == Item_func::LT_FUNC && left_has_field) {
            if (fold_or_simplify(thd, ref_or_field, func_type, false,
                                 manifest_result, retcond, cond_value))
              return true; /* purecov: inspected */
          } else if (func_type == Item_func::LE_FUNC) {
            if (!left_has_field) {
              if (fold_or_simplify(thd, ref_or_field, func_type, true,
                                   manifest_result, retcond, cond_value))
                return true; /* purecov: inspected */
            } else {
              /*
                E.g. tinyint: f <= -128 (signed), or f <= 0 (unsigned):
                convert to f = <min>
              */
              if (simplify_to_equal(thd, ref_or_field, *c, retcond, cond_value))
                return true; /* purecov: inspected */
            }
          }  // else no folding
          break;
        case RP_INSIDE:
        case RP_INSIDE_YEAR_HOLE:
          break;
        case RP_ON_MAX:
          if (func_type == Item_func::LT_FUNC && !left_has_field) {
            if (fold_or_simplify(thd, ref_or_field, func_type, false,
                                 manifest_result, retcond, cond_value))
              return true; /* purecov: inspected */
          } else if (func_type == Item_func::LE_FUNC) {
            if (left_has_field) {
              if (fold_or_simplify(thd, ref_or_field, func_type, true,
                                   manifest_result, retcond, cond_value))
                return true; /* purecov: inspected */
            } else {
              /*
                E.g. tinyint: 127 <= f (signed), or 255 <= f (unsigned):
                convert to <max>=f
              */
              if (simplify_to_equal(thd, *c, ref_or_field, retcond, cond_value))
                return true; /* purecov: inspected */
            }
          }  // else don't fold
          break;
        case RP_INSIDE_TRUNCATED:
          /*
            The constant now has a smaller absolute value.
            Adjust comparison functions to compensate for this. For example,
            if we have a decimal field f of type DECIMAL(3,1) and compare:
                   f >= 10.13
            we adjust the predicate to
                   f > 10.1
            since we know the value of f has to be either 10.1 or 10.2
            in this "vicinity". If it is 10.1, we need to remove the equal part
            of the comparison to preserve semantics. If the value is 10.2,
            well, this is greater or equal to 10.13 anyway, substituting >
            is also correct. Any other value of f will not be affected by the
            change, so the substitution is correct.
          */
          if (adjust_cmp_op(thd, func, left_has_field, negative, ref_or_field,
                            *c, retcond))
            return true; /* purecov: inspected */
          break;
      }
      break;
    default:
      break;
  }

  /*
    Redo setting of comparison func and caching of constant since we have
    potentially modified it. Note: we can't use our initial determination of
    item type/function type any more - it may be changed - so check again.
  */
  if (*retcond == nullptr || (*retcond)->type() != Item::FUNC_ITEM)
    return false;

  switch (down_cast<Item_func *>(*retcond)->functype()) {
    case Item_func::GE_FUNC:
    case Item_func::GT_FUNC:
    case Item_func::LT_FUNC:
    case Item_func::LE_FUNC:
    case Item_func::EQ_FUNC:
    case Item_func::NE_FUNC:
    case Item_func::EQUAL_FUNC: {
      const auto new_args = down_cast<Item_bool_func2 *>(*retcond)->arguments();
      if (func != *retcond || new_args[0] != arg0_orig ||
          new_args[1] != arg1_orig)
        down_cast<Item_bool_func2 *>(*retcond)->set_cmp_func();
    } break;
    default:
      break;
  }
  return false;
}

/**
  @} (end of group Constant_Folding)
*/
