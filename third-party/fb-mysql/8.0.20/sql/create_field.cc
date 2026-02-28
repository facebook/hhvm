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

#include "sql/create_field.h"

#include "sql/derror.h"
#include "sql/error_handler.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"
#include "sql_string.h"
#include "template_utils.h"

#include <cmath>

// Definition of static constexpr data members in Create_field.
constexpr size_t Create_field::TINYBLOB_MAX_SIZE_IN_BYTES;
constexpr size_t Create_field::BLOB_MAX_SIZE_IN_BYTES;
constexpr size_t Create_field::MEDIUMBLOB_MAX_SIZE_IN_BYTES;
constexpr size_t Create_field::LONGBLOB_MAX_SIZE_IN_BYTES;

/**
    Constructs a column definition from an object representing an actual
    column. This is a reverse-engineering procedure that creates a column
    definition object as produced by the parser (Create_field) from a resolved
    column object (Field).

    @param old_field  The column object from which the column definition is
                      constructed.
    @param orig_field Used for copying default values. This parameter may be
                      NULL, but if present it is used for copying default
                      values.

    Default values are copied into an Item_string unless:
    - The default value is a function.
    - There is no default value.
    - old_field is a BLOB column.
    - old_field has its data pointer improperly initialized.
*/
Create_field::Create_field(Field *old_field, Field *orig_field)
    : hidden(old_field->hidden()),
      field_name(old_field->field_name),
      change(nullptr),
      comment(old_field->comment),
      sql_type(old_field->real_type()),
      decimals(old_field->decimals()),
      flags(old_field->flags),
      auto_flags(old_field->auto_flags),
      charset(old_field->charset()),  // May be NULL ptr
      is_explicit_collation(false),
      geom_type(Field::GEOM_GEOMETRY),
      field(old_field),
      is_nullable(old_field->is_nullable()),
      is_zerofill(false),  // Init to avoid UBSAN warnings
      is_unsigned(false),  // Init to avoid UBSAN warnings
      treat_bit_as_char(
          false),  // Init to avoid valgrind warnings in opt. build
      pack_length_override(0),
      gcol_info(old_field->gcol_info),
      stored_in_db(old_field->stored_in_db),
      m_default_val_expr(old_field->m_default_val_expr),
      is_array(old_field->is_array()),
      m_max_display_width_in_codepoints(old_field->char_length()) {
  switch (sql_type) {
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
      sql_type = blob_type_from_pack_length(old_field->pack_length() -
                                            portable_sizeof_char_ptr);
      break;
    case MYSQL_TYPE_STRING:
      /* Change CHAR -> VARCHAR if dynamic record length */
      if (old_field->type() == MYSQL_TYPE_VAR_STRING)
        sql_type = MYSQL_TYPE_VARCHAR;
      break;
    case MYSQL_TYPE_GEOMETRY: {
      const Field_geom *field_geom = down_cast<const Field_geom *>(old_field);
      geom_type = field_geom->geom_type;
      m_srid = field_geom->get_srid();
      break;
    }
    case MYSQL_TYPE_YEAR:
      m_max_display_width_in_codepoints = 4;  // set default value
      break;
    default:
      break;
  }

  if (flags & (ENUM_FLAG | SET_FLAG))
    interval = down_cast<Field_enum *>(old_field)->typelib;
  else
    interval = nullptr;
  constant_default = nullptr;

  /*
    Copy the default (constant/function) from the column object orig_field, if
    supplied. We do this if all these conditions are met:

    - The column allows a default.

    - The column type is not a BLOB type.

    - The original column (old_field) was properly initialized with a record
      buffer pointer.
  */
  if (!(flags & (NO_DEFAULT_VALUE_FLAG | BLOB_FLAG)) &&
      old_field->ptr != nullptr && orig_field != nullptr) {
    bool default_now = false;
    if (real_type_with_now_as_default(sql_type)) {
      // The SQL type of the new field allows a function default:
      default_now = orig_field->has_insert_default_datetime_value_expression();
      auto_flags = default_now ? Field::DEFAULT_NOW : Field::NONE;
      if (orig_field->has_update_default_datetime_value_expression())
        auto_flags |= Field::ON_UPDATE_NOW;
      if (orig_field->has_insert_default_general_value_expression())
        auto_flags |= Field::GENERATED_FROM_EXPRESSION;
    }
    if (!default_now)  // Give a constant default
    {
      /* Get the value from default_values */
      ptrdiff_t diff = orig_field->table->default_values_offset();
      orig_field->move_field_offset(diff);  // Points now at default_values
      if (!orig_field->is_real_null()) {
        StringBuffer<MAX_FIELD_WIDTH> tmp(charset);
        String *res = orig_field->val_str(&tmp);
        char *pos = sql_strmake(res->ptr(), res->length());
        constant_default = new Item_string(pos, res->length(), charset);
      }
      orig_field->move_field_offset(-diff);  // Back to record[0]
    }
  }
}

/**
  Initialize a column definition object. Column definition objects can be used
  to construct Field objects.

  @param thd                   Session/Thread handle.
  @param fld_name              Column name.
  @param fld_type              Column type.
  @param display_width_in_codepoints The column display width, counted in
                                     number of code points.
  @param fld_decimals          Number of digits to the right of the decimal
                               point (if any.)
  @param fld_type_modifier     Additional type information.
  @param fld_default_value     Column default constant or datetime expression
                               (if any.)
  @param fld_on_update_value   The expression in the ON UPDATE clause.
  @param fld_comment           Column comment.
  @param fld_change            Column change.
  @param fld_interval_list     Interval list (if any.)
  @param fld_charset           Column charset.
  @param has_explicit_collation Column has an explicit COLLATE attribute.
  @param fld_geom_type         Column geometry type (if any.)
  @param fld_gcol_info         Generated column data
  @param fld_default_val_expr  The expression for generating default values
  @param srid                  The SRID specification. This might be null
                               (has_value() may return false).
  @param hidden                Whether this column should be hidden or not.
  @param is_array_arg          Whether the field is a typed array

  @retval
    false on success.
  @retval
    true  on error.
*/

bool Create_field::init(
    THD *thd, const char *fld_name, enum_field_types fld_type,
    const char *display_width_in_codepoints, const char *fld_decimals,
    uint fld_type_modifier, Item *fld_default_value, Item *fld_on_update_value,
    LEX_CSTRING *fld_comment, const char *fld_change,
    List<String> *fld_interval_list, const CHARSET_INFO *fld_charset,
    bool has_explicit_collation, uint fld_geom_type,
    Value_generator *fld_gcol_info, Value_generator *fld_default_val_expr,
    Nullable<gis::srid_t> srid, dd::Column::enum_hidden_type hidden,
    bool is_array_arg) {
  uint sign_len, allowed_type_modifier = 0;
  ulong max_field_charlength = MAX_FIELD_CHARLENGTH;

  DBUG_TRACE;

  DBUG_ASSERT(!(has_explicit_collation && fld_charset == nullptr));

  field = nullptr;
  field_name = fld_name;
  flags = fld_type_modifier;
  is_explicit_collation = (fld_charset != nullptr);

  if (!has_explicit_collation && fld_charset == &my_charset_utf8mb4_0900_ai_ci)
    charset = thd->variables.default_collation_for_utf8mb4;
  else
    charset = fld_charset;

  auto_flags = Field::NONE;
  is_nullable = !(fld_type_modifier & NOT_NULL_FLAG);
  this->hidden = hidden;
  is_array = is_array_arg;

  if (fld_default_value != nullptr &&
      fld_default_value->type() == Item::FUNC_ITEM) {
    // We have a function default for insertions.
    constant_default = nullptr;
    auto_flags |= Field::DEFAULT_NOW;
  } else {
    // No function default for insertions. Either NULL or a constant.
    constant_default = fld_default_value;
  }

  // ON UPDATE CURRENT_TIMESTAMP
  if (fld_on_update_value != nullptr &&
      fld_on_update_value->type() == Item::FUNC_ITEM)
    auto_flags |= Field::ON_UPDATE_NOW;

  // Automatic increment.
  if (fld_type_modifier & AUTO_INCREMENT_FLAG) auto_flags |= Field::NEXT_NUMBER;

  decimals = fld_decimals ? (uint)atoi(fld_decimals) : 0;
  if (is_temporal_real_type(fld_type)) {
    flags |= BINARY_FLAG;
    charset = &my_charset_numeric;
    if (decimals > DATETIME_MAX_DECIMALS) {
      my_error(ER_TOO_BIG_PRECISION, MYF(0), decimals, fld_name,
               DATETIME_MAX_DECIMALS);
      return true;
    }
  } else if (decimals >= DECIMAL_NOT_SPECIFIED) {
    my_error(ER_TOO_BIG_SCALE, MYF(0), decimals, fld_name,
             static_cast<ulong>(DECIMAL_NOT_SPECIFIED - 1));
    return true;
  }

  sql_type = fld_type;
  change = fld_change;
  interval = nullptr;
  geom_type = static_cast<Field::geometry_type>(fld_geom_type);
  interval_list.empty();

  comment = *fld_comment;
  gcol_info = fld_gcol_info;
  stored_in_db = true;
  m_srid = srid;

  if (fld_default_val_expr) {
    constant_default = nullptr;
    auto_flags |= Field::GENERATED_FROM_EXPRESSION;
    m_default_val_expr = fld_default_val_expr;
  }

  // Initialize data for a virtual field or default value expression
  if (gcol_info || m_default_val_expr) {
    if (gcol_info) {
      DBUG_ASSERT(gcol_info->expr_item);
      stored_in_db = gcol_info->get_field_stored();

      /*
        Make a field created for the real type.
        Note that "real" and generated fields differ from each other
        only by Field::gcol_info, which is always 0 for normal columns.
        gcol_info is updated for fields later in procedure open_binary_frm.
      */
      sql_type = fld_type = gcol_info->get_real_type();
      if (pre_validate_value_generator_expr(gcol_info->expr_item, field_name,
                                            VGS_GENERATED_COLUMN))
        return true;
    } else {
      if (pre_validate_value_generator_expr(m_default_val_expr->expr_item,
                                            field_name, VGS_DEFAULT_EXPRESSION))
        return true;
    }
  }

  /*
    Set NO_DEFAULT_VALUE_FLAG if this field doesn't have a default value
    or an expression to generate the default value and
    it is NOT NULL and not an AUTO_INCREMENT field.
  */
  if (!fld_default_value && !fld_default_val_expr &&
      (fld_type_modifier & NOT_NULL_FLAG) &&
      !(fld_type_modifier & AUTO_INCREMENT_FLAG)) {
    /*
      TIMESTAMP columns get implicit DEFAULT value when
      explicit_defaults_for_timestamp is not set.
    */
    if (thd->variables.explicit_defaults_for_timestamp ||
        !is_timestamp_type(fld_type)) {
      flags |= NO_DEFAULT_VALUE_FLAG;
    }
  }

  if (display_width_in_codepoints != nullptr) {
    errno = 0;
    const ulonglong ull_length =
        my_strtoull(display_width_in_codepoints, nullptr, 10);
    if ((errno != 0) || (ull_length > MAX_FIELD_BLOBLENGTH)) {
      my_error(ER_TOO_BIG_DISPLAYWIDTH, MYF(0), fld_name, MAX_FIELD_BLOBLENGTH);
      return true;
    }
    m_max_display_width_in_codepoints = static_cast<size_t>(ull_length);
    m_explicit_display_width = true;

    if (m_max_display_width_in_codepoints == 0)
      display_width_in_codepoints = nullptr; /* purecov: inspected */
  }

  sign_len = fld_type_modifier & UNSIGNED_FLAG ? 0 : 1;

  switch (fld_type) {
    case MYSQL_TYPE_TINY:
      if (!display_width_in_codepoints)
        m_max_display_width_in_codepoints = MAX_TINYINT_WIDTH + sign_len;
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      break;
    case MYSQL_TYPE_SHORT:
      if (!display_width_in_codepoints)
        m_max_display_width_in_codepoints = MAX_SMALLINT_WIDTH + sign_len;
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      break;
    case MYSQL_TYPE_INT24:
      if (!display_width_in_codepoints)
        m_max_display_width_in_codepoints = MAX_MEDIUMINT_WIDTH + sign_len;
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      break;
    case MYSQL_TYPE_LONG:
      if (!display_width_in_codepoints)
        m_max_display_width_in_codepoints = MAX_INT_WIDTH + sign_len;
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      break;
    case MYSQL_TYPE_LONGLONG:
      if (!display_width_in_codepoints)
        m_max_display_width_in_codepoints = MAX_BIGINT_WIDTH;
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      break;
    case MYSQL_TYPE_NULL:
      break;
    case MYSQL_TYPE_NEWDECIMAL: {
      ulong precision = static_cast<ulong>(m_max_display_width_in_codepoints);
      my_decimal_trim(&precision, &decimals);
      m_max_display_width_in_codepoints = precision;
    }
      if (m_max_display_width_in_codepoints > DECIMAL_MAX_PRECISION) {
        my_error(ER_TOO_BIG_PRECISION, MYF(0),
                 static_cast<int>(m_max_display_width_in_codepoints), fld_name,
                 static_cast<ulong>(DECIMAL_MAX_PRECISION));
        return true;
      }
      if (m_max_display_width_in_codepoints < decimals) {
        my_error(ER_M_BIGGER_THAN_D, MYF(0), fld_name);
        return true;
      }
      m_max_display_width_in_codepoints = my_decimal_precision_to_length(
          m_max_display_width_in_codepoints, decimals,
          fld_type_modifier & UNSIGNED_FLAG);
      break;
    case MYSQL_TYPE_VARCHAR:
      // Note that VARCHAR fields with a long length may be converted to blob
      // types in prepare_blob_field()
      max_field_charlength = MAX_FIELD_VARCHARLENGTH;
      break;
    case MYSQL_TYPE_STRING:
      break;
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_JSON:
      if (fld_default_value) {
        /* Allow empty as default value. */
        String str, *res;
        res = fld_default_value->val_str(&str);
        /*
          A default other than '' is always an error, and any non-NULL
          specified default is an error in strict mode.
        */
        if (res->length() || thd->is_strict_sql_mode()) {
          my_error(ER_BLOB_CANT_HAVE_DEFAULT, MYF(0),
                   fld_name); /* purecov: inspected */
          return true;
        } else {
          Strict_error_handler strict_handler;
          if (thd->install_strict_handler())
            thd->push_internal_handler(&strict_handler);

          /*
            Otherwise a default of '' is just a warning.
          */
          push_warning_printf(thd, Sql_condition::SL_WARNING,
                              ER_BLOB_CANT_HAVE_DEFAULT,
                              ER_THD(thd, ER_BLOB_CANT_HAVE_DEFAULT), fld_name);

          if (thd->install_strict_handler()) thd->pop_internal_handler();
        }
        constant_default = nullptr;
      }

      flags |= BLOB_FLAG;
      break;
    case MYSQL_TYPE_GEOMETRY:
      if (fld_default_value) {
        my_error(ER_BLOB_CANT_HAVE_DEFAULT, MYF(0), fld_name);
        return true;
      }
      flags |= BLOB_FLAG;
      break;
    case MYSQL_TYPE_YEAR:
      if (!display_width_in_codepoints ||
          m_max_display_width_in_codepoints != 4)
        m_max_display_width_in_codepoints = 4; /* Default length */
      flags |= ZEROFILL_FLAG | UNSIGNED_FLAG;
      break;
    case MYSQL_TYPE_FLOAT:
      /* change FLOAT(precision) to FLOAT or DOUBLE */
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      if (display_width_in_codepoints && !fld_decimals) {
        size_t tmp_length = m_max_display_width_in_codepoints;
        if (tmp_length > PRECISION_FOR_DOUBLE) {
          my_error(ER_WRONG_FIELD_SPEC, MYF(0), fld_name);
          return true;
        } else if (tmp_length > PRECISION_FOR_FLOAT) {
          sql_type = MYSQL_TYPE_DOUBLE;
          m_max_display_width_in_codepoints = MAX_DOUBLE_STR_LENGTH;
        } else
          m_max_display_width_in_codepoints = MAX_FLOAT_STR_LENGTH;
        decimals = DECIMAL_NOT_SPECIFIED;
        break;
      }
      if (!display_width_in_codepoints && !fld_decimals) {
        m_max_display_width_in_codepoints = MAX_FLOAT_STR_LENGTH;
        decimals = DECIMAL_NOT_SPECIFIED;
      }
      if (m_max_display_width_in_codepoints < decimals &&
          decimals != DECIMAL_NOT_SPECIFIED) {
        my_error(ER_M_BIGGER_THAN_D, MYF(0), fld_name);
        return true;
      }
      break;
    case MYSQL_TYPE_DOUBLE:
      allowed_type_modifier = AUTO_INCREMENT_FLAG;
      if (!display_width_in_codepoints && !fld_decimals) {
        m_max_display_width_in_codepoints = DBL_DIG + 7;
        decimals = DECIMAL_NOT_SPECIFIED;
      }
      if (m_max_display_width_in_codepoints < decimals &&
          decimals != DECIMAL_NOT_SPECIFIED) {
        my_error(ER_M_BIGGER_THAN_D, MYF(0), fld_name);
        return true;
      }
      break;
    case MYSQL_TYPE_TIMESTAMP:
      /* Add flags for TIMESTAMP for 4.0 MYD and 4.0 InnoDB compatibility */
      flags |= ZEROFILL_FLAG | UNSIGNED_FLAG;
      /* Fall through */
    case MYSQL_TYPE_TIMESTAMP2:
      if (display_width_in_codepoints == nullptr) {
        m_max_display_width_in_codepoints =
            MAX_DATETIME_WIDTH + (decimals ? (1 + decimals) : 0);
      } else if (m_max_display_width_in_codepoints != MAX_DATETIME_WIDTH) {
        /*
          We support only even TIMESTAMP lengths less or equal than 14
          and 19 as length of 4.1 compatible representation.  Silently
          shrink it to MAX_DATETIME_COMPRESSED_WIDTH.
        */
        DBUG_ASSERT(MAX_DATETIME_COMPRESSED_WIDTH < UINT_MAX);
        if (m_max_display_width_in_codepoints !=
            UINT_MAX) /* avoid overflow; is safe because of min() */
          m_max_display_width_in_codepoints =
              ((m_max_display_width_in_codepoints + 1) / 2) * 2;
        m_max_display_width_in_codepoints = std::min<size_t>(
            m_max_display_width_in_codepoints, MAX_DATETIME_COMPRESSED_WIDTH);
      }

      /*
        Since we silently rewrite down to MAX_DATETIME_COMPRESSED_WIDTH bytes,
        the parser should not raise errors unless bizzarely large.
       */
      max_field_charlength = UINT_MAX;

      break;
    case MYSQL_TYPE_DATE:
      /* Old date type. */
      sql_type = MYSQL_TYPE_NEWDATE;
      /* fall through */
    case MYSQL_TYPE_NEWDATE:
      m_max_display_width_in_codepoints = MAX_DATE_WIDTH;
      break;
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
      m_max_display_width_in_codepoints =
          MAX_TIME_WIDTH + (decimals ? (1 + decimals) : 0);
      break;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATETIME2:
      m_max_display_width_in_codepoints =
          MAX_DATETIME_WIDTH + (decimals ? (1 + decimals) : 0);
      break;
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM: {
      for (String &it : *fld_interval_list) {
        interval_list.push_back(&it);
      }
      break;
    }
    case MYSQL_TYPE_VAR_STRING:
      DBUG_ASSERT(0); /* Impossible. */
      break;
    case MYSQL_TYPE_BIT: {
      if (!display_width_in_codepoints) {
        my_error(ER_INVALID_FIELD_SIZE, MYF(0), fld_name);
        return true;
      }
      if (m_max_display_width_in_codepoints > MAX_BIT_FIELD_LENGTH) {
        my_error(ER_TOO_BIG_DISPLAYWIDTH, MYF(0), fld_name,
                 static_cast<ulong>(MAX_BIT_FIELD_LENGTH));
        return true;
      }
      break;
    }
    case MYSQL_TYPE_DECIMAL:
    default:
      DBUG_ASSERT(0); /* Was obsolete */
  }

  if (!(flags & BLOB_FLAG) &&
      ((m_max_display_width_in_codepoints > max_field_charlength &&
        fld_type != MYSQL_TYPE_SET && fld_type != MYSQL_TYPE_ENUM &&
        (fld_type != MYSQL_TYPE_VARCHAR || fld_default_value)) ||
       ((m_max_display_width_in_codepoints == 0 && m_explicit_display_width) &&
        fld_type != MYSQL_TYPE_STRING && fld_type != MYSQL_TYPE_VARCHAR &&
        fld_type != MYSQL_TYPE_GEOMETRY))) {
    my_error((fld_type == MYSQL_TYPE_VAR_STRING ||
              fld_type == MYSQL_TYPE_VARCHAR || fld_type == MYSQL_TYPE_STRING)
                 ? ER_TOO_BIG_FIELDLENGTH
                 : ER_TOO_BIG_DISPLAYWIDTH,
             MYF(0), fld_name, max_field_charlength); /* purecov: inspected */
    return true;
  }
  fld_type_modifier &= AUTO_INCREMENT_FLAG;
  if ((~allowed_type_modifier) & fld_type_modifier) {
    my_error(ER_WRONG_FIELD_SPEC, MYF(0), fld_name);
    return true;
  }

  /*
    After all checks were carried out we should be able guarantee that column
    can't have AUTO_INCREMENT and DEFAULT/ON UPDATE CURRENT_TIMESTAMP at the
    same time.
  */
  DBUG_ASSERT(!((auto_flags & (Field::DEFAULT_NOW | Field::ON_UPDATE_NOW |
                               Field::GENERATED_FROM_EXPRESSION)) != 0 &&
                (auto_flags & Field::NEXT_NUMBER) != 0));

  return false; /* success */
}

/**
  Init for a tmp table field. To be extended if need be.
*/
void Create_field::init_for_tmp_table(enum_field_types sql_type_arg,
                                      uint32 length_arg, uint32 decimals_arg,
                                      bool is_nullable_arg,
                                      bool is_unsigned_arg,
                                      uint pack_length_override_arg,
                                      const char *fld_name) {
  DBUG_TRACE;

  field_name = fld_name;
  sql_type = sql_type_arg;
  m_max_display_width_in_codepoints = length_arg;
  auto_flags = Field::NONE;
  interval = nullptr;
  charset = &my_charset_bin;
  geom_type = Field::GEOM_GEOMETRY;

  DBUG_PRINT("enter", ("sql_type: %d, length: %u", sql_type_arg, length_arg));

  /* Init members needed for correct execution of make_field(). */
#ifndef DBUG_OFF
  const uint32 FIELDFLAG_MAX_DEC = 31;
#endif

  switch (sql_type_arg) {
    case MYSQL_TYPE_BIT:
      treat_bit_as_char = true;
      break;
    case MYSQL_TYPE_DATE:
      // Old type
      sql_type = MYSQL_TYPE_NEWDATE;
      break;

    case MYSQL_TYPE_NEWDECIMAL:
      DBUG_ASSERT(decimals_arg <= DECIMAL_MAX_SCALE);
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      DBUG_ASSERT(decimals_arg <= FIELDFLAG_MAX_DEC);
      decimals = decimals_arg;
      break;

    default:
      break;
  }

  is_nullable = is_nullable_arg;

  is_zerofill = false;
  is_unsigned = is_unsigned_arg;

  pack_length_override = pack_length_override_arg;

  gcol_info = nullptr;
  stored_in_db = true;
  m_default_val_expr = nullptr;
}

size_t Create_field::max_display_width_in_codepoints() const {
  if (sql_type == MYSQL_TYPE_ENUM || sql_type == MYSQL_TYPE_SET) {
    DBUG_ASSERT(interval != nullptr);
    DBUG_ASSERT(charset != nullptr);

    size_t max_display_width_in_codepoints = 0;
    for (size_t i = 0; i < interval->count; i++) {
      const char *start = interval->type_names[i];
      const char *end = interval->type_names[i] + interval->type_lengths[i];
      const size_t num_characters =
          charset->cset->numchars(charset, start, end);

      if (sql_type == MYSQL_TYPE_ENUM) {
        // ENUM uses the longest value.
        max_display_width_in_codepoints =
            std::max(max_display_width_in_codepoints, num_characters);
      } else if (sql_type == MYSQL_TYPE_SET) {
        // SET uses the total length of all values, plus a comma between each
        // value.
        max_display_width_in_codepoints += num_characters;
        if (i > 0) {
          max_display_width_in_codepoints++;
        }
      }
    }

    return std::min(max_display_width_in_codepoints,
                    static_cast<size_t>(MAX_FIELD_WIDTH - 1));
  } else if (sql_type == MYSQL_TYPE_TINY_BLOB) {
    return TINYBLOB_MAX_SIZE_IN_BYTES / charset->mbmaxlen;
  } else if (sql_type == MYSQL_TYPE_BLOB && !explicit_display_width()) {
    // For BLOB and TEXT, the user can give a display width explicitly in CREATE
    // TABLE (BLOB(25), TEXT(25)) where the expected behavior is that the server
    // will find the smallest possible BLOB/TEXT type that will fit the given
    // display width. If the user has given an explicit display width, return
    // that instead of the max BLOB size.
    return BLOB_MAX_SIZE_IN_BYTES / charset->mbmaxlen;
  } else if (sql_type == MYSQL_TYPE_MEDIUM_BLOB) {
    return MEDIUMBLOB_MAX_SIZE_IN_BYTES / charset->mbmaxlen;
  } else if (sql_type == MYSQL_TYPE_LONG_BLOB || sql_type == MYSQL_TYPE_JSON ||
             sql_type == MYSQL_TYPE_GEOMETRY) {
    return LONGBLOB_MAX_SIZE_IN_BYTES / charset->mbmaxlen;
  } else {
    return m_max_display_width_in_codepoints;
  }
}

size_t Create_field::max_display_width_in_bytes() const {
  // It might seem unnecessary to have special case for the various BLOB types
  // instead of just using the "else" clause for these types as well. However,
  // that might give us rounding errors for multi-byte character sets. One
  // example is JSON which has the character set utf8mb4_bin.
  // max_display_width_in_codepoints() will return 1073741823 (truncated from
  // 1073741823.75), and multiplying that by four again will give 4294967292
  // which is the wrong result.
  DBUG_ASSERT(charset != nullptr);
  if (is_numeric_type(sql_type) || is_temporal_real_type(sql_type) ||
      sql_type == MYSQL_TYPE_YEAR || sql_type == MYSQL_TYPE_BIT) {
    // Numeric types, temporal types, YEAR or BIT are never multi-byte.
    return max_display_width_in_codepoints();
  } else if (sql_type == MYSQL_TYPE_TINY_BLOB) {
    return TINYBLOB_MAX_SIZE_IN_BYTES;
  } else if (sql_type == MYSQL_TYPE_BLOB && !explicit_display_width()) {
    // For BLOB and TEXT, the user can give a display width (BLOB(25), TEXT(25))
    // where the expected behavior is that the server will find the smallest
    // possible BLOB/TEXT type that will fit the given display width. If the
    // user has given an explicit display width, return that instead of the
    // max BLOB size.
    return BLOB_MAX_SIZE_IN_BYTES;
  } else if (sql_type == MYSQL_TYPE_MEDIUM_BLOB) {
    return MEDIUMBLOB_MAX_SIZE_IN_BYTES;
  } else if (sql_type == MYSQL_TYPE_LONG_BLOB || sql_type == MYSQL_TYPE_JSON ||
             sql_type == MYSQL_TYPE_GEOMETRY) {
    return LONGBLOB_MAX_SIZE_IN_BYTES;
  } else {
    // If the user has given a display width to the TEXT type where the display
    // width is 2^32-1, the below computation will exceed
    // LONGBLOB_MAX_SIZE_IN_BYTES if the character set is multi-byte. So we must
    // ensure that we never return a value greater than
    // LONGBLOB_MAX_SIZE_IN_BYTES.
    std::int64_t display_width = max_display_width_in_codepoints() *
                                 static_cast<std::int64_t>(charset->mbmaxlen);
    return static_cast<size_t>(std::min(
        display_width, static_cast<std::int64_t>(LONGBLOB_MAX_SIZE_IN_BYTES)));
  }
}

size_t Create_field::pack_length(bool dont_override) const {
  if (!dont_override && pack_length_override != 0) return pack_length_override;

  switch (sql_type) {
    case MYSQL_TYPE_SET: {
      return get_set_pack_length(interval == nullptr ? interval_list.elements
                                                     : interval->count);
    }
    case MYSQL_TYPE_ENUM: {
      return get_enum_pack_length(interval == nullptr ? interval_list.elements
                                                      : interval->count);
    }
    case MYSQL_TYPE_NEWDECIMAL: {
      DBUG_ASSERT(decimals <= DECIMAL_MAX_SCALE);
      uint precision = my_decimal_length_to_precision(
          max_display_width_in_bytes(), decimals, (flags & UNSIGNED_FLAG));
      precision = std::min(precision, static_cast<uint>(DECIMAL_MAX_PRECISION));
      return my_decimal_get_binary_size(precision, decimals);
    }
    case MYSQL_TYPE_BIT: {
      if (treat_bit_as_char) {
        return (max_display_width_in_bytes() + 7) / 8;
      } else {
        return max_display_width_in_bytes() / 8;
      }
    }
    default: {
      return calc_pack_length(sql_type, max_display_width_in_bytes());
    }
  }
}

size_t Create_field::key_length() const {
  switch (sql_type) {
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VARCHAR: {
      return std::min(max_display_width_in_bytes(),
                      static_cast<size_t>(MAX_FIELD_BLOBLENGTH));
    }
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET: {
      return pack_length();
    }
    case MYSQL_TYPE_BIT: {
      if (treat_bit_as_char) {
        return pack_length();
      }
      return pack_length() + (max_display_width_in_bytes() & 7 ? 1 : 0);
    }
    default: {
      return pack_length(is_array);
    }
  }
}

bool is_field_for_functional_index(const Create_field *create_field) {
  return create_field->hidden == dd::Column::enum_hidden_type::HT_HIDDEN_SQL;
}
