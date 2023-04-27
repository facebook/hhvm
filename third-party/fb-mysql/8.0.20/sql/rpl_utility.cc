/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_utility.h"

#include <string.h>
#include <iterator>
#include <new>
#include <utility>

#include "lex_string.h"
#include "libbinlogevents/export/binary_log_funcs.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/thr_malloc.h"

struct TYPELIB;

#ifdef MYSQL_SERVER

#include <algorithm>

#include "libbinlogevents/include/binlog_event.h"  // checksum_crv32
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/psi_memory.h"
#include "mysqld_error.h"
#include "sql/create_field.h"
#include "sql/dd/dd.h"          // get_dictionary
#include "sql/dd/dictionary.h"  // is_dd_table_access_allowed
#include "sql/derror.h"         // ER_THD
#include "sql/field.h"          // Field
#include "sql/log.h"
#include "sql/log_event.h"  // Log_event
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // slave_type_conversions_options
#include "sql/psi_memory_key.h"
#include "sql/rpl_rli.h"  // Relay_log_info
#include "sql/rpl_slave.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_list.h"
#include "sql/sql_plugin_ref.h"
#include "sql/sql_tmp_table.h"  // create_tmp_table_from_fields
#include "sql_show.h"           // show_sql_type
#include "sql_string.h"
#include "template_utils.h"  // delete_container_pointers
#include "typelib.h"

using binary_log::checksum_crc32;
using std::max;
using std::min;
using std::unique_ptr;

#endif  // MYSQL_SERVER

/*********************************************************************
 *                   table_def member definitions                    *
 *********************************************************************/

/*
  This function returns the field size in raw bytes based on the type
  and the encoded field data from the master's raw data.
*/
uint32 table_def::calc_field_size(uint col, const uchar *master_data) const {
  uint32 length =
      ::calc_field_size(type(col), master_data, m_field_metadata[col]);
  return length;
}

#if defined(MYSQL_SERVER)
/**
   Function to compare two size_t integers for their relative
   order. Used below.
 */
static int compare(size_t a, size_t b) {
  if (a < b) return -1;
  if (b < a) return 1;
  return 0;
}

/*
  Compare the pack lengths of a source field (on the master) and a
  target field (on the slave).

  @param field    Target field.
  @param type     Source field type.
  @param metadata Source field metadata.

  @retval -1 The length of the source field is smaller than the target field.
  @retval  0 The length of the source and target fields are the same.
  @retval  1 The length of the source field is greater than the target field.
 */
static int compare_lengths(Field *field, enum_field_types source_type,
                           uint16 metadata) {
  DBUG_TRACE;
  size_t const source_length =
      max_display_length_for_field(source_type, metadata);
  size_t const target_length = field->max_display_length();
  DBUG_PRINT("debug", ("source_length: %lu, source_type: %u,"
                       " target_length: %lu, target_type: %u",
                       (unsigned long)source_length, source_type,
                       (unsigned long)target_length, field->real_type()));
  int result = compare(source_length, target_length);
  DBUG_PRINT("result", ("%d", result));
  return result;
}

/**
   Check the order variable and print errors if the order is not
   acceptable according to the current settings.

   @param order  The computed order of the conversion needed.
 */
static bool is_conversion_ok(int order) {
  DBUG_TRACE;
  bool allow_non_lossy, allow_lossy;

  const bool allow_non_truncation =
      slave_type_conversions_options &
      (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_NON_TRUNCATION);

  if (allow_non_truncation) {
    return true;
  }

  allow_non_lossy = slave_type_conversions_options &
                    (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_NON_LOSSY);
  allow_lossy = slave_type_conversions_options &
                (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_LOSSY);

  DBUG_PRINT("enter", ("order: %d, flags:%s%s", order,
                       allow_non_lossy ? " ALL_NON_LOSSY" : "",
                       allow_lossy ? " ALL_LOSSY" : ""));
  if (order < 0 && !allow_non_lossy) {
    /* !!! Add error message saying that non-lossy conversions need to be
     * allowed. */
    return false;
  }

  if (order > 0 && !allow_lossy) {
    /* !!! Add error message saying that lossy conversions need to be allowed.
     */
    return false;
  }

  return true;
}

/**
  Check if the types are criss cross means type1 is MYSQL_TYPE_TIMESTAMP
  and type2 as MYSQL_TYPE_TIMESTAMP2 or vice versa.
*/
inline bool timestamp_cross_check(enum_field_types type1,
                                  enum_field_types type2) {
  return ((type1 == MYSQL_TYPE_TIMESTAMP && type2 == MYSQL_TYPE_TIMESTAMP2) ||
          (type1 == MYSQL_TYPE_TIMESTAMP2 && type2 == MYSQL_TYPE_TIMESTAMP));
}

/**
  Check if the types are criss cross means type1 is MYSQL_TYPE_DATETIME
  and type2 as MYSQL_TYPE_DATETIME or vice versa.
*/
inline bool datetime_cross_check(enum_field_types type1,
                                 enum_field_types type2) {
  return ((type1 == MYSQL_TYPE_DATETIME && type2 == MYSQL_TYPE_DATETIME2) ||
          (type1 == MYSQL_TYPE_DATETIME2 && type2 == MYSQL_TYPE_DATETIME));
}

/**
  Check if the types are criss cross means type1 is MYSQL_TYPE_TIME
  and type2 as MYSQL_TYPE_TIME2 or vice versa.
*/
inline bool time_cross_check(enum_field_types type1, enum_field_types type2) {
  return ((type1 == MYSQL_TYPE_TIME && type2 == MYSQL_TYPE_TIME2) ||
          (type1 == MYSQL_TYPE_TIME2 && type2 == MYSQL_TYPE_TIME));
}

/**
  Check if the field is part of the type mismatch whitelist
  (global var: rbr_column_type_mismatch_whitelist)
*/
static bool is_in_col_whitelist(const Field *field, const Relay_log_info *rli) {
  bool ret = false;
  const auto whitelist_ptr = rli->get_rbr_column_type_mismatch_whitelist();
  if (whitelist_ptr && !whitelist_ptr->empty()) {
    // case: all tables are in the whitelist if '.*' regex is present in the
    // list
    if (whitelist_ptr->count(".*")) return true;
    char fqcn[2 * MAX_ALIAS_NAME + MAX_FIELD_NAME + 2 + 1];
    int cnt = snprintf(fqcn, sizeof(fqcn), "%s.%s.%s", field->table->s->db.str,
                       field->table->s->table_name.str, field->field_name);
    if (cnt >= 0 && cnt < (int)sizeof(fqcn)) {
      ret = whitelist_ptr->count(std::string(fqcn));
    }
  }
  return ret;
}

/**
   Can a type potentially be converted to another type?

   This function check if the types are convertible and what
   conversion is required.

   If conversion is not possible, and error is printed.

   If conversion is possible:

   - *order will be set to -1 if source type is smaller than target
     type and a non-lossy conversion can be required. This includes
     the case where the field types are different but types could
     actually be converted in either direction.

   - *order will be set to 0 if no conversion is required.

   - *order will be set to 1 if the source type is strictly larger
      than the target type and that conversion is potentially lossy.

   @param[in] field    Target field
   @param[in] source_type Source field type
   @param[in] metadata Source field metadata
   @param[in] is_array Whether the source field is a typed array
   @param[in] rli      Relay log info (for error reporting)
   @param[in] mflags   Flags from the table map event
   @param[out] order_var Order between source field and target field

   @return @c true if conversion is possible according to the current
   settings, @c false if conversion is not possible according to the
   current setting.
 */
static bool can_convert_field_to(Field *field, enum_field_types source_type,
                                 uint metadata, bool is_array,
                                 Relay_log_info *rli, uint16 mflags,
                                 int *order_var) {
  DBUG_TRACE;
#ifndef DBUG_OFF
  char field_type_buf[MAX_FIELD_WIDTH];
  String field_type(field_type_buf, sizeof(field_type_buf), &my_charset_latin1);
  field->sql_type(field_type);
  DBUG_PRINT("enter", ("field_type: %s, target_type: %d, source_type: %d, "
                       "source_metadata: 0x%x",
                       field_type.c_ptr_safe(), field->real_type(), source_type,
                       metadata));
#endif
  // Can't convert from scalar to array and vice versa
  if (is_array != field->is_array()) return false;
  bool is_in_whitelist_cached = false;

  /*
    If the real type is the same, we need to check the metadata to
    decide if conversions are allowed.
   */
  if (field->real_type() == source_type) {
    if (metadata == 0)  // Metadata can only be zero if no metadata was provided
    {
      /*
        If there is no metadata, we either have an old event where no
        metadata were supplied, or a type that does not require any
        metadata. In either case, conversion can be done but no
        conversion table is necessary.
       */
      DBUG_PRINT("debug",
                 ("Base types are identical, but there is no metadata"));
      *order_var = 0;
      return true;
    }

    DBUG_PRINT("debug",
               ("Base types are identical, doing field size comparison"));
    if (field->compatible_field_size(metadata, rli, mflags, order_var))
      return is_conversion_ok(*order_var) || is_in_col_whitelist(field, rli);
    else
      return false;
  } else if (is_array) {
    // Can't covert between typed array of different types
    return false;
  } else if (metadata == 0 &&
             (timestamp_cross_check(field->real_type(), source_type) ||
              datetime_cross_check(field->real_type(), source_type) ||
              time_cross_check(field->real_type(), source_type))) {
    /*
      In the above condition, we are taking care
      of case where
      1) Master having old TIME, TIMESTAMP, DATETIME
      and slave have new TIME2, TIMESTAMP2, DATETIME2
      or
      2) Master having new TIMESTAMP2, DATETIME2, TIME2
      with fraction part zero and slave have TIME,
      TIMESTAMP, DATETIME.
      We need second condition, as when we are
      upgrading from 5.5 to 5.6 TIME, TIMESTAMP,
      DATETIME columns are not upgraded to TIME(0),
      TIMESTAMP(0), DATETIME(0).
      So to support these conversion we are putting this
      condition.
    */
    /*
      TS-TODO: conversion from FSP1>FSP2.
      Can do non-lossy conversion
      from old TIME, TIMESTAMP, DATETIME
      to new TIME(0), TIMESTAMP(0), DATETIME(0).
    */
    *order_var = -1;
    return true;
  } else if (!slave_type_conversions_options &&
             !(is_in_whitelist_cached = is_in_col_whitelist(field, rli)))
    return false;

  /*
    Here, from and to will always be different. Since the types are
    different, we cannot use the compatible_field_size() function, but
    have to rely on hard-coded max-sizes for fields.
  */

  DBUG_PRINT("debug", ("Base types are different, checking conversion"));
  switch (source_type)  // Source type (on master)
  {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      switch (field->real_type()) {
        case MYSQL_TYPE_NEWDECIMAL:
          /*
            Then the other type is either FLOAT, DOUBLE, or old style
            DECIMAL, so we require lossy conversion.
          */
          *order_var = 1;
          return is_in_whitelist_cached || is_conversion_ok(*order_var) ||
                 is_in_col_whitelist(field, rli);

        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE: {
          if (source_type == MYSQL_TYPE_NEWDECIMAL ||
              source_type == MYSQL_TYPE_DECIMAL)
            *order_var = 1;  // Always require lossy conversions
          else
            *order_var = compare_lengths(field, source_type, metadata);
          DBUG_ASSERT(*order_var != 0);
          return is_in_whitelist_cached || is_conversion_ok(*order_var) ||
                 is_in_col_whitelist(field, rli);
        }

        default:
          return false;
      }
      break;

    /*
      The length comparison check will do the correct job of comparing
      the field lengths (in bytes) of two integer types.
    */
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
      switch (field->real_type()) {
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
          *order_var = compare_lengths(field, source_type, metadata);
          DBUG_ASSERT(*order_var != 0);
          return is_in_whitelist_cached || is_conversion_ok(*order_var) ||
                 is_in_col_whitelist(field, rli);

        default:
          return false;
      }
      break;

    /*
      Since source and target type is different, and it is not possible
      to convert bit types to anything else, this will return false.
     */
    case MYSQL_TYPE_BIT:
      return false;

    /*
      If all conversions are disabled, it is not allowed to convert
      between these types. Since the TEXT vs. BINARY is distinguished by
      the charset, and the charset is not replicated, we cannot
      currently distinguish between , e.g., TEXT and BLOB.
     */
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
      switch (field->real_type()) {
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
        case MYSQL_TYPE_VARCHAR:
          *order_var = compare_lengths(field, source_type, metadata);
          /*
            Here we know that the types are different, so if the order
            gives that they do not require any conversion, we still need
            to have non-lossy conversion enabled to allow conversion
            between different (string) types of the same length.
           */
          if (*order_var == 0) *order_var = -1;
          return is_in_whitelist_cached || is_conversion_ok(*order_var) ||
                 is_in_col_whitelist(field, rli);

        default:
          return false;
      }
      break;

    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_TYPED_ARRAY:
      return false;
  }
  return false;  // To keep GCC happy
}

/**
  Is the definition compatible with a table when it does not belong to
  the data dictionary?

  This function first finds out whether the table belongs to the data
  dictionary. When not, it will compare the master table with an existing
  table on the slave and see if they are compatible with respect to the
  current settings of @c SLAVE_TYPE_CONVERSIONS.

  If the tables are compatible and conversions are required, @c
  *tmp_table_var will be set to a virtual temporary table with field
  pointers for the fields that require conversions.  This allow simple
  checking of whether a conversion are to be applied or not.

  If tables are compatible, but no conversions are necessary, @c
  *tmp_table_var will be set to NULL.

  @param [in] rli Relay log info, for error reporting.

  @param [in] table Table to compare with

  @param [out] conv_table_var Virtual temporary table for performing
  conversions, if necessary.

  @retval true Master table is compatible with slave table.
  @retval false When the table belongs to the data dictionary or
                master table is not compatible with slave table.
*/
bool table_def::compatible_with(THD *thd, Relay_log_info *rli, TABLE *table,
                                TABLE **conv_table_var) const {
  /*
    Prohibit replication into dictionary internal tables. We know this is
    not DDL (which will be replicated as statements, and rejected by the
    corresponding check for SQL statements), thus 'false' in the call below.
    Also sserting that this is not a DD system thread.
  */
  DBUG_ASSERT(!thd->is_dd_system_thread());
  const dd::Dictionary *dictionary = dd::get_dictionary();
  if (dictionary && !dictionary->is_dd_table_access_allowed(
                        false, false, table->s->db.str, table->s->db.length,
                        table->s->table_name.str)) {
    DBUG_PRINT("debug", ("Access to dictionary table %s.%s is prohibited",
                         table->s->db.str, table->s->table_name.str));
    rli->report(
        ERROR_LEVEL, ER_SERVER_NO_SYSTEM_TABLE_ACCESS,
        ER_THD(thd, ER_SERVER_NO_SYSTEM_TABLE_ACCESS),
        ER_THD_NONCONST(thd, dictionary->table_type_error_code(
                                 table->s->db.str, table->s->table_name.str)),
        table->s->db.str, table->s->table_name.str);
    return false;
  }

  /*
    We only check the initial columns for the tables.

    If column names are logged by the master, we check all
    the columns present in master.
  */
  Replicated_columns_view fields{table, Replicated_columns_view::INBOUND, thd};
  uint const cols_to_check = min<ulong>(fields.filtered_size(), size());
  TABLE *tmp_table = nullptr;
  uint removed_cols = 0;

  for (auto it = fields.begin(); it.filtered_pos() < cols_to_check; ++it) {
    size_t col = it.filtered_pos();
    Field *const field = have_column_names() ? find_field_in_table_sef(
                                                   table, get_column_name(col))
                                             : *it;
    if (!field) {
      // This column is removed on slave
      ++removed_cols;
      continue;
    }
    int order;
    if (can_convert_field_to(field, type(col), field_metadata(col),
                             is_array(col), rli, m_flags, &order)) {
      DBUG_PRINT("debug", ("Checking column %lu -"
                           " field '%s' can be converted - order: %d",
                           static_cast<long unsigned int>(col),
                           field->field_name, order));
      DBUG_ASSERT(order >= -1 && order <= 1);

      /*
        If order is not 0, a conversion is required, so we need to set
        up the conversion table.
       */
      if (order != 0 && tmp_table == nullptr) {
        /*
          This will create the full table with all fields. This is
          necessary to ge the correct field lengths for the record.
        */
        tmp_table = create_conversion_table(thd, rli, table);
        if (tmp_table == nullptr) return false;
        /*
          Clear all fields up to, but not including, this column.
          Note the slave conversion table doesn't have deleted columns. This
          needs to be taken care of when clearing fields.
        */
        for (unsigned int i = 0; i < col - removed_cols; ++i)
          tmp_table->field[i] = nullptr;
      }

      if (order == 0 && tmp_table != nullptr)
        tmp_table->field[col - removed_cols] = nullptr;
    } else {
      DBUG_PRINT("debug",
                 ("Checking column %lu -"
                  " field '%s' can not be converted",
                  static_cast<long unsigned int>(col), field->field_name));
      DBUG_ASSERT(col < size() &&
                  (col < table->s->fields || have_column_names()));
      DBUG_ASSERT(table->s->db.str && table->s->table_name.str);
      const char *db_name = table->s->db.str;
      const char *tbl_name = table->s->table_name.str;
      char source_buf[MAX_FIELD_WIDTH];
      char target_buf[MAX_FIELD_WIDTH];
      String field_sql_type;
      enum loglevel report_level = INFORMATION_LEVEL;
      String source_type(source_buf, sizeof(source_buf), &my_charset_latin1);
      String target_type(target_buf, sizeof(target_buf), &my_charset_latin1);
      show_sql_type(type(col), is_array(col), field_metadata(col),
                    &source_type);
      field->sql_type(target_type);
      if (!ignored_error_code(ER_SERVER_SLAVE_CONVERSION_FAILED)) {
        report_level = ERROR_LEVEL;
        thd->is_slave_error = true;
      } else if (log_error_verbosity >= 2)
        report_level = WARNING_LEVEL;

      if (field->has_charset() && (field->type() == MYSQL_TYPE_VARCHAR ||
                                   field->type() == MYSQL_TYPE_STRING)) {
        field_sql_type.append((field->type() == MYSQL_TYPE_VARCHAR) ? "varchar"
                                                                    : "char");
        const CHARSET_INFO *cs = field->charset();
        size_t length = cs->cset->snprintf(
            cs, target_type.ptr(), target_type.alloced_length(),
            "%s(%u(bytes) %s)", field_sql_type.c_ptr_safe(),
            field->field_length, field->charset()->csname);
        target_type.length(length);
      } else
        field->sql_type(target_type);

      if (report_level != INFORMATION_LEVEL)
        rli->report(report_level, ER_SERVER_SLAVE_CONVERSION_FAILED,
                    ER_THD(thd, ER_SERVER_SLAVE_CONVERSION_FAILED), col,
                    db_name, tbl_name, source_type.c_ptr_safe(),
                    target_type.c_ptr_safe());
      return false;
    }
  }

  *conv_table_var = tmp_table;
  return true;
}

/**
  Create a conversion table.

  If the function is unable to create the conversion table, an error
  will be printed and NULL will be returned.

  @return Pointer to conversion table, or NULL if unable to create
  conversion table.
 */

TABLE *table_def::create_conversion_table(THD *thd, Relay_log_info *rli,
                                          TABLE *target_table) const {
  DBUG_TRACE;

  List<Create_field> field_list;
  TABLE *conv_table = nullptr;
  /*
    At slave, columns may differ. So we should create
    min(columns@master, columns@slave) columns in the
    conversion table.
  */
  uint const cols_to_create = have_column_names()
                                  ? size()
                                  : min<ulong>(target_table->s->fields, size());

  int unsigned_override = -1;

  // Check if slave_type_conversions contains ALL_UNSIGNED
  if (slave_type_conversions_options &
      (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_UNSIGNED)) {
    unsigned_override = 1;
  }

  // Check if slave_type_conversions contains ALL_SIGNED
  if ((slave_type_conversions_options &
       (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_SIGNED))) {
    unsigned_override = 0;
  }

  for (uint col = 0; col < cols_to_create; ++col) {
    Field *const slave_field =
        have_column_names()
            ? find_field_in_table_sef(target_table, get_column_name(col))
            : target_table->field[col];

    if (!slave_field) {
      // This column is removed on slave
      continue;
    }
    Create_field *field_def = new (thd->mem_root) Create_field();
    if (field_list.push_back(field_def)) return nullptr;

    uint decimals = 0;
    TYPELIB *interval = nullptr;
    uint pack_length_override = 0;  // 0 => NA. Only assigned below when needed.
    enum_field_types field_type = type(col);
    uint32 max_length =
        max_display_length_for_field(field_type, field_metadata(col));

    switch (field_type) {
      uint precision;
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_SET:
        interval = static_cast<Field_enum *>(slave_field)->typelib;
        /*
          Number of elements in interval on master and slave might differ.
          Use pack length from binary log instead of one calculated from
          number of interval elements on slave.
        */
        pack_length_override = field_metadata(col) & 0x00ff;
        break;

      case MYSQL_TYPE_NEWDECIMAL:
        /*
          The display length of a DECIMAL type is not the same as the
          length that should be supplied to make_field, so we correct
          the length here.
         */
        precision = field_metadata(col) >> 8;
        decimals = field_metadata(col) & 0x00ff;
        max_length = my_decimal_precision_to_length(precision, decimals, false);
        break;

      case MYSQL_TYPE_DECIMAL:
        LogErr(ERROR_LEVEL, ER_RPL_INCOMPATIBLE_DECIMAL_IN_RBR,
               target_table->s->db.str, target_table->s->table_name.str,
               target_table->field[col]->field_name);
        goto err;

      case MYSQL_TYPE_BLOB:
        /*
          Blobs are binlogged as MYSQL_TYPE_BLOB, even when pack_length
          != 2. Need the exact blob type for the call to
          Create_field::init_for_tmp_table() below. Note that
          pack_length is NOT assigned to pack_length_override here, as
          this should only be used when the pack_length cannot be
          derived from the exact type, i.e. for ENUM and SET (see
          above).
        */
        field_type = blob_type_from_pack_length(field_metadata(col) & 0x00ff);
        break;

      default:
        break;
    }

    const bool unsigned_flag =
        unsigned_override != -1 ? unsigned_override : is_unsigned(col);

    DBUG_PRINT(
        "debug",
        ("sql_type: %d, target_field: '%s', max_length: %d, decimals: %d,"
         " maybe_null: %d, unsigned_flag: %d",
         binlog_type(col), slave_field->field_name, max_length, decimals, true,
         unsigned_flag));
    field_def->init_for_tmp_table(field_type, max_length, decimals,
                                  true,           // maybe_null
                                  unsigned_flag,  // unsigned_flag
                                  pack_length_override);
    field_def->charset = slave_field->charset();
    field_def->interval = interval;
    field_def->field_name = slave_field->field_name;
  }

  conv_table = DBUG_EVALUATE_IF(
      "simulate_out_of_memory_while_creating_temp_table_for_conversion",
      nullptr, create_tmp_table_from_fields(thd, field_list));
err:
  if (conv_table == nullptr) {
    enum loglevel report_level = INFORMATION_LEVEL;
    if (!ignored_error_code(ER_SLAVE_CANT_CREATE_CONVERSION)) {
      report_level = ERROR_LEVEL;
      thd->is_slave_error = true;
    } else if (log_error_verbosity >= 2)
      report_level = WARNING_LEVEL;

    if (report_level != INFORMATION_LEVEL)
      rli->report(report_level, ER_SLAVE_CANT_CREATE_CONVERSION,
                  ER_THD(thd, ER_SLAVE_CANT_CREATE_CONVERSION),
                  target_table->s->db.str, target_table->s->table_name.str);
  }
  return conv_table;
}

bool table_def::use_column_names(TABLE *table) {
  // case: no col names, just return
  if (!have_column_names()) return false;

  // case: return cached value
  if (m_slave_schema_is_different != -1) return m_slave_schema_is_different;

  // case: schema is different because the number of cols differ
  if (table->s->fields != size()) {
    m_slave_schema_is_different = 1;
    return true;
  }

  // check if the cols are in same order
  for (uint i = 0; i < size(); ++i) {
    const char *col_name = get_column_name(i);
    Field *const field = find_field_in_table_sef(table, col_name);
    if (!field || field->field_index != i) {
      m_slave_schema_is_different = 1;
      return true;
    }
  }
  m_slave_schema_is_different = 0;
  return false;
}
#endif /* MYSQL_SERVER */

/**
  Decode field metadata from a char buffer (serialized form) into an int
  (packed form).

  @note On little-endian platforms (e.g Intel) this function effectively
  inverts order of bytes compared to what Field::save_field_metadata()
  writes. E.g for MYSQL_TYPE_NEWDECIMAL save_field_metadata writes precision
  into the first byte and decimals into the second, this function puts
  precision into the second byte and decimals into the first. This layout
  is expected by replication code that reads metadata in the uint form.
  Due to this design feature show_sql_type() can't correctly print
  immediate output of save_field_metadata(), this function have to be used
  as translator.

  @param buffer Field metadata, in the character stream form produced by
                save_field_metadata.
  @param binlog_type The type of the field, in the form returned by
                      Field::binlog_type and stored in Table_map_log_event.
  @retval pair where:
  - the first component is the length of the metadata within 'buffer',
    i.e., how much the buffer pointer should move forward in order to skip it.
  - the second component is pair containing:
    - the metadata, encoded as an 'uint', in the form required by e.g.
      show_sql_type.
    - bool indicating whether the field is array (true) or a scalar (false)
*/

std::pair<my_off_t, std::pair<uint, bool>> read_field_metadata(
    const uchar *buffer, enum_field_types binlog_type) {
  bool is_array = false;
  uint metadata = 0;
  uint index = 0;
  if (binlog_type == MYSQL_TYPE_TYPED_ARRAY) {
    binlog_type = static_cast<enum_field_types>(buffer[index++]);
    is_array = true;
  }
  switch (binlog_type) {
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_JSON: {
      /*
        These types store a single byte.
      */
      metadata = buffer[index++];
      break;
    }
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_STRING: {
      metadata = buffer[index++] << 8U;  // real_type
      metadata += buffer[index++];       // pack or field length
      break;
    }
    case MYSQL_TYPE_BIT: {
      metadata = buffer[index++];
      metadata += (buffer[index++] << 8U);
      break;
    }
    case MYSQL_TYPE_VARCHAR: {
      /*
        These types store two bytes.
      */
      if (is_array) {
        metadata = uint3korr(buffer + index);
        index = index + 3;
      } else {
        metadata = uint2korr(buffer + index);
        index = index + 2;
      }
      break;
    }
    case MYSQL_TYPE_NEWDECIMAL: {
      metadata = buffer[index++] << 8U;  // precision
      metadata += buffer[index++];       // decimals
      break;
    }
    default:
      metadata = 0;
      break;
  }
  return std::make_pair(index, std::make_pair(metadata, is_array));
}

PSI_memory_key key_memory_table_def_memory;

table_def::table_def(unsigned char *types, ulong size, uchar *field_metadata,
                     int metadata_size, uchar *null_bitmap, uint16 flags,
                     const uchar *column_names, unsigned long column_names_size,
                     const uchar *sign_bits)
    : m_size(size),
      m_type(nullptr),
      m_field_metadata_size(metadata_size),
      m_field_metadata(nullptr),
      m_null_bits(nullptr),
      m_flags(flags),
      m_memory(nullptr),
      m_json_column_count(-1),
      m_is_array(nullptr),
      m_sign_bits(nullptr) {
  m_memory = (uchar *)my_multi_malloc(
      key_memory_table_def_memory, MYF(MY_WME), &m_type, size,
      &m_field_metadata, size * sizeof(uint), &m_is_array, size * sizeof(bool),
      &m_null_bits, (size + 7) / 8, &m_sign_bits, (size + 7) / 8, nullptr);

  memset(m_field_metadata, 0, size * sizeof(uint));
  memset(m_is_array, 0, size * sizeof(bool));

  // -1 means that we haven't computed the value yet
  // see @table_def::use_column_names()
  m_slave_schema_is_different = -1;

  if (m_type)
    memcpy(m_type, types, size);
  else
    m_size = 0;
  /*
    Extract the data from the table map into the field metadata array
    iff there is field metadata. The variable metadata_size will be
    0 if we are replicating from an older version server since no field
    metadata was written to the table map. This can also happen if
    there were no fields in the master that needed extra metadata.
  */
  if (m_size && metadata_size) {
    int index = 0;
    for (unsigned int i = 0; i < m_size; i++) {
      std::pair<my_off_t, std::pair<uint, bool>> pack = read_field_metadata(
          static_cast<const uchar *>(field_metadata + index), binlog_type(i));
      // Update type of the typed array
      if (binlog_type(i) == MYSQL_TYPE_TYPED_ARRAY)
        m_type[i] = static_cast<enum_field_types>(field_metadata[index]);
      // Fill in read metadata
      m_field_metadata[i] = pack.second.first;
      m_is_array[i] = pack.second.second;
      index += pack.first;
      DBUG_ASSERT(index <= metadata_size);
    }
  }
  if (m_size && null_bitmap) memcpy(m_null_bits, null_bitmap, (m_size + 7) / 8);
  memset(m_sign_bits, 0, (m_size + 7) / 8);
  if (m_size && sign_bits) memcpy(m_sign_bits, sign_bits, (m_size + 7) / 8);

  if (column_names && column_names_size) {
    // store column names in to an array and indices in a map
    for (uint i = 0; i < m_size; i++) {
      uint length = (uint)*column_names;
      if (!length || length + 1 > column_names_size) {
        for (auto &col : m_column_names) my_free(col);
        m_column_names.clear();
        break;
      }
      // memory allocated by this malloc is freed in
      // the class destructor.
      char *str = (char *)my_malloc(PSI_NOT_INSTRUMENTED, length, MYF(0));
      strncpy(str, (const char *)column_names + 1, length);
      str[length - 1] = 0;
      m_column_indices[str] = i;
      m_column_names.push_back(str);
      column_names += length + 1;
      column_names_size -= length + 1;
    }
  }
}

table_def::~table_def() {
  my_free(m_memory);
  for (auto &col : m_column_names) {
    my_free(col);
  }
#ifndef DBUG_OFF
  m_type = nullptr;
  m_size = 0;
#endif
}

#ifdef MYSQL_SERVER

#define HASH_ROWS_POS_SEARCH_INVALID -1

/**
  Utility methods for handling row based operations.
 */

void hash_slave_rows_free_entry::operator()(HASH_ROW_ENTRY *entry) const {
  DBUG_TRACE;
  if (entry) {
    if (entry->preamble) {
      entry->preamble->~HASH_ROW_PREAMBLE();
      my_free(entry->preamble);
    }
    if (entry->positions) my_free(entry->positions);
    my_free(entry);
  }
}

bool Hash_slave_rows::is_empty(void) { return m_hash.empty(); }

/**
   Hashing commodity structures and functions.
 */

bool Hash_slave_rows::init(void) { return false; }

bool Hash_slave_rows::deinit(void) {
  DBUG_TRACE;
  m_hash.clear();
  return false;
}

int Hash_slave_rows::size() { return m_hash.size(); }

HASH_ROW_ENTRY *Hash_slave_rows::make_entry() {
  return make_entry(nullptr, nullptr);
}

HASH_ROW_ENTRY *Hash_slave_rows::make_entry(const uchar *bi_start,
                                            const uchar *bi_ends) {
  DBUG_TRACE;

  HASH_ROW_ENTRY *entry = (HASH_ROW_ENTRY *)my_malloc(
      key_memory_HASH_ROW_ENTRY, sizeof(HASH_ROW_ENTRY), MYF(0));
  HASH_ROW_PREAMBLE *preamble = (HASH_ROW_PREAMBLE *)my_malloc(
      key_memory_HASH_ROW_ENTRY, sizeof(HASH_ROW_PREAMBLE), MYF(0));
  HASH_ROW_POS *pos = (HASH_ROW_POS *)my_malloc(key_memory_HASH_ROW_ENTRY,
                                                sizeof(HASH_ROW_POS), MYF(0));

  if (!entry || !preamble || !pos) goto err;

  /**
     Filling in the preamble.
   */
  new (preamble) HASH_ROW_PREAMBLE();
  preamble->hash_value = 0;
  preamble->search_state = m_hash.end();
  preamble->is_search_state_inited = false;

  /**
     Filling in the positions.
   */
  pos->bi_start = bi_start;
  pos->bi_ends = bi_ends;

  /**
    Filling in the entry
   */
  entry->preamble = preamble;
  entry->positions = pos;

  return entry;

err:
  DBUG_PRINT("info", ("Hash_slave_rows::make_entry - malloc error"));
  if (entry) my_free(entry);
  if (preamble) {
    preamble->~HASH_ROW_PREAMBLE();
    my_free(preamble);
  }
  if (pos) my_free(pos);
  return nullptr;
}

bool Hash_slave_rows::put(TABLE *table, MY_BITMAP *cols,
                          HASH_ROW_ENTRY *entry) {
  DBUG_TRACE;

  HASH_ROW_PREAMBLE *preamble = entry->preamble;

  /**
     Skip blobs and BIT fields from key calculation.
     Handle X bits.
     Handle nulled fields.
     Handled fields not signaled.
  */
  preamble->hash_value = make_hash_key(table, cols);

  m_hash.emplace(preamble->hash_value,
                 unique_ptr<HASH_ROW_ENTRY, hash_slave_rows_free_entry>(entry));
  DBUG_PRINT("debug",
             ("Added record to hash with key=%u", preamble->hash_value));
  return false;
}

HASH_ROW_ENTRY *Hash_slave_rows::get(TABLE *table, MY_BITMAP *cols) {
  DBUG_TRACE;
  uint key;
  HASH_ROW_ENTRY *entry = nullptr;

  key = make_hash_key(table, cols);

  DBUG_PRINT("debug", ("Looking for record with key=%u in the hash.", key));

  const auto it = m_hash.find(key);
  if (it != m_hash.end()) {
    DBUG_PRINT("debug", ("Found record with key=%u in the hash.", key));

    /**
       Save the search state in case we need to go through entries for
       the given key.
    */
    entry = it->second.get();
    entry->preamble->search_state = it;
    entry->preamble->is_search_state_inited = true;
  }

  return entry;
}

bool Hash_slave_rows::next(HASH_ROW_ENTRY **entry) {
  DBUG_TRACE;
  DBUG_ASSERT(*entry);

  if (*entry == nullptr) return true;

  HASH_ROW_PREAMBLE *preamble = (*entry)->preamble;

  if (!preamble->is_search_state_inited) return true;

  uint key = preamble->hash_value;
  const auto it = std::next(preamble->search_state);

  /*
    Invalidate search for current preamble, because it is going to be
    used in the search below (and search state is used in a
    one-time-only basis).
   */
  preamble->search_state = m_hash.end();
  preamble->is_search_state_inited = false;

  DBUG_PRINT("debug",
             ("Looking for record with key=%u in the hash (next).", key));

  if (it != m_hash.end() && it->first == key) {
    DBUG_PRINT("debug", ("Found record with key=%u in the hash (next).", key));
    *entry = it->second.get();
    preamble = (*entry)->preamble;

    /**
       Save the search state for next iteration (if any).
     */
    preamble->search_state = it;
    preamble->is_search_state_inited = true;
  } else {
    *entry = nullptr;
  }

  return false;
}

bool Hash_slave_rows::del(HASH_ROW_ENTRY *entry) {
  DBUG_TRACE;
  DBUG_ASSERT(entry);

  erase_specific_element(&m_hash, entry->preamble->hash_value, entry);
  return false;
}

uint Hash_slave_rows::make_hash_key(TABLE *table, MY_BITMAP *cols) {
  DBUG_TRACE;
  ha_checksum crc = 0L;

  uchar *record = table->record[0];
  uchar saved_x = 0, saved_filler = 0;

  if (table->s->null_bytes > 0) {
    /*
      If we have an X bit then we need to take care of it.
    */
    if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD)) {
      saved_x = record[0];
      record[0] |= 1U;
    }

    /*
      If (last_null_bit_pos == 0 && null_bytes > 1), then:
      X bit (if any) + N nullable fields + M Field_bit fields = 8 bits
      Ie, the entire byte is used.
    */
    if (table->s->last_null_bit_pos > 0) {
      saved_filler = record[table->s->null_bytes - 1];
      record[table->s->null_bytes - 1] |=
          256U - (1U << table->s->last_null_bit_pos);
    }
  }

  /*
    We can only checksum the bytes if all fields have been signaled
    in the before image. Otherwise, unpack_row will not have set the
    null_flags correctly (because it only unpacks those fields and
    their flags that were actually in the before image).

    @c record_compare, as it also skips null_flags if the read_set
    was not marked completely.
   */
  if (bitmap_is_set_all(cols) && cols->n_bits == table->s->fields) {
    crc = checksum_crc32(crc, table->null_flags, table->s->null_bytes);
    DBUG_PRINT("debug", ("make_hash_entry: hash after null_flags: %u", crc));
  }

  for (Field **ptr = table->field; *ptr && ((*ptr)->field_index < cols->n_bits);
       ptr++) {
    Field *f = (*ptr);

    /*
      Field is set in the read_set and is isn't NULL.
     */
    if (bitmap_is_set(cols, f->field_index) &&
        !f->is_virtual_gcol() &&  // Avoid virtual generated columns on hashes
        !f->is_null()) {
      /*
        BLOB and VARCHAR have pointers in their field, we must convert
        to string; GEOMETRY and JSON are implemented on top of BLOB.
        BIT may store its data among NULL bits, convert as well.
      */
      switch (f->type()) {
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_JSON:
        case MYSQL_TYPE_BIT: {
          String tmp;
          f->val_str(&tmp);
          crc = checksum_crc32(crc, pointer_cast<const uchar *>(tmp.ptr()),
                               tmp.length());
          break;
        }
        default:
          crc = checksum_crc32(crc, f->ptr, f->data_length());
          break;
      }
#ifndef DBUG_OFF
      String tmp;
      f->val_str(&tmp);
      DBUG_PRINT("debug", ("make_hash_entry: hash after field %s=%s: %u",
                           f->field_name, tmp.c_ptr_safe(), crc));
#endif
    }
  }

  /*
    Restore the saved bytes.

    TODO[record format ndb]: Remove this code once NDB returns the
    correct record format.
  */
  if (table->s->null_bytes > 0) {
    if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD))
      record[0] = saved_x;

    if (table->s->last_null_bit_pos)
      record[table->s->null_bytes - 1] = saved_filler;
  }

  DBUG_PRINT("debug", ("Created key=%u", crc));
  return crc;
}

#endif

#if defined(MYSQL_SERVER)

Deferred_log_events::Deferred_log_events()
    : m_array(key_memory_table_def_memory) {}

Deferred_log_events::~Deferred_log_events() { m_array.clear(); }

int Deferred_log_events::add(Log_event *ev) {
  m_array.push_back(ev);
  ev->worker = nullptr;  // to mark event busy avoiding deletion
  return 0;
}

bool Deferred_log_events::is_empty() { return m_array.empty(); }

bool Deferred_log_events::execute(Relay_log_info *rli) {
  bool res = false;

  DBUG_ASSERT(rli->deferred_events_collecting);

  rli->deferred_events_collecting = false;
  for (Log_event **it = m_array.begin(); !res && it != m_array.end(); ++it) {
    Log_event *ev = *it;
    res = ev->apply_event(rli);
  }
  rli->deferred_events_collecting = true;
  return res;
}

void Deferred_log_events::rewind() {
  /*
    Reset preceding Query log event events which execution was
    deferred because of slave side filtering.
  */
  delete_container_pointers(m_array);
  m_array.shrink_to_fit();
}

std::string replace_all_in_str(std::string from, std::string find,
                               std::string replace) {
  std::string to{from.data()};
  if (to.length() == 0) {
    return to;
  }

  size_t start{0};
  while ((start = to.find(find, start)) != std::string::npos) {
    to.replace(start, find.size(), replace);
    start += replace.length();
  }

  return to;
}

#endif

#ifdef MYSQL_SERVER
THD_instance_guard::THD_instance_guard(THD *thd)
    : m_is_locally_initialized{thd == nullptr} {
  if (this->m_is_locally_initialized) {
    this->m_target = new THD;
    this->m_target->thread_stack = (char *)&this->m_target;
    this->m_target->store_globals();
    this->m_target->security_context()->skip_grants();
  } else {
    this->m_target = thd;
  }
}

THD_instance_guard::~THD_instance_guard() {
  if (this->m_is_locally_initialized) {
    delete this->m_target;
  }
}

THD_instance_guard::operator THD *() { return this->m_target; }

bool evaluate_command_row_only_restrictions(THD *thd) {
  LEX *const lex = thd->lex;

  switch (lex->sql_command) {
    case SQLCOM_UPDATE:
    case SQLCOM_INSERT:
    case SQLCOM_INSERT_SELECT:
    case SQLCOM_DELETE:
    case SQLCOM_LOAD:
    case SQLCOM_REPLACE:
    case SQLCOM_REPLACE_SELECT:
    case SQLCOM_DELETE_MULTI:
    case SQLCOM_UPDATE_MULTI: {
      return true;
    }
    case SQLCOM_CREATE_TABLE: {
      return (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE);
    }
    case SQLCOM_DROP_TABLE: {
      return (lex->drop_temporary);
    }
    default:
      break;
  }

  return false;
}

#endif  // MYSQL_SERVER
