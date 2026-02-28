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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/field.h"

#include <float.h>
#include <stddef.h>

#include "m_ctype.h"
#include "my_config.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <algorithm>
#include <cmath>   // isnan
#include <memory>  // unique_ptr

#include "decimal.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_byteorder.h"
#include "my_compare.h"
#include "my_dbug.h"
#include "my_double2ulonglong.h"
#include "my_sqlcommand.h"
#include "myisampack.h"
#include "sql/create_field.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/types/table.h"
#include "sql/dd_table_share.h"     // dd_get_old_field_type
#include "sql/derror.h"             // ER_THD
#include "sql/filesort.h"           // change_double_for_sort
#include "sql/gis/rtree_support.h"  // get_mbr_from_store
#include "sql/gis/srid.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_json_func.h"  // ensure_utf8mb4
#include "sql/item_timefunc.h"   // Item_func_now_local
#include "sql/json_binary.h"     // json_binary::serialize
#include "sql/json_diff.h"       // Json_diff_vector
#include "sql/json_dom.h"        // Json_dom, Json_wrapper
#include "sql/key.h"
#include "sql/log_event.h"  // class Table_map_log_event
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // log_10
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/rpl_rli.h"                // Relay_log_info
#include "sql/rpl_slave.h"              // rpl_master_has_bug
#include "sql/spatial.h"                // Geometry
#include "sql/sql_class.h"              // THD
#include "sql/sql_exception_handler.h"  // handle_std_exception
#include "sql/sql_lex.h"
#include "sql/sql_time.h"       // str_to_datetime_with_warn
#include "sql/sql_tmp_table.h"  // create_tmp_field
#include "sql/srs_fetcher.h"
#include "sql/strfunc.h"  // find_type2
#include "sql/system_variables.h"
#include "sql/transaction_info.h"
#include "sql/tztime.h"      // Time_zone
#include "template_utils.h"  // pointer_cast
#include "typelib.h"

namespace dd {
class Spatial_reference_system;
}  // namespace dd

using std::max;
using std::min;

#define FLAGSTR(V, F) ((V) & (F) ? #F " " : "")

// Maximum allowed exponent value for converting string to decimal
#define MAX_EXPONENT 1024

/**
  Static variables
*/
const char field_separator = ',';
uchar Field::dummy_null_buffer = ' ';

#define DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE FLOATING_POINT_BUFFER
#define LONGLONG_TO_STRING_CONVERSION_BUFFER_SIZE 128
#define DECIMAL_TO_STRING_CONVERSION_BUFFER_SIZE 128
#define BLOB_PACK_LENGTH_TO_MAX_LENGH(arg) \
  ((ulong)((1LL << std::min(arg, 4U) * 8) - 1LL))

/*
  Rules for merging different types of fields in UNION

  NOTE: to avoid 256*256 table, gap in table types numeration is skiped
  following #defines describe that gap and how to canculate number of fields
  and index of field in thia array.
*/
#define FIELDTYPE_TEAR_FROM (MYSQL_TYPE_BIT + 1)
#define FIELDTYPE_TEAR_TO (MYSQL_TYPE_JSON - 1)
#define FIELDTYPE_NUM (FIELDTYPE_TEAR_FROM + (255 - FIELDTYPE_TEAR_TO))

namespace {
/**
   Predicate to determine if a field type change prevents alter
   from being done inplace.

   @param from - existing Field object.
   @param to   - Create_field object describing new version of field.

   @return true if alter cannot be done inplace due to specified
   condition, false otherwise.
*/
bool sql_type_prevents_inplace(const Field &from, const Create_field &to) {
  return to.sql_type != from.real_type();
}

/**
   Predicate to determine if a length change prevents alter from being
   done inplace. Length cannot decrease and cannot cross the 256 byte
   row format barrier.

   @param from - existing Field object.
   @param to   - Create_field object describing new version of field.

   @return true if alter cannot be done inplace due to specified
   condition, false otherwise.
*/
bool length_prevents_inplace(const Field &from, const Create_field &to) {
  DBUG_PRINT("inplace", ("stmt: %s", current_thd->query().str));

  DBUG_PRINT("inplace", ("from: field_length:%u, pack_length():%u, "
                         "row_pack_length():%u, max_display_length():%u",
                         from.field_length, from.pack_length(),
                         from.row_pack_length(), from.max_display_length()));

  DBUG_PRINT("inplace", ("to: length:%zu, pack_length:%zu",
                         to.max_display_width_in_bytes(), to.pack_length()));

  return (
      to.pack_length() < from.pack_length() ||
      (to.max_display_width_in_bytes() >= 256 && from.row_pack_length() < 256));
}

/**
   Predicate to determine if a charset change prevents alter from being
   done inplace.

   For changes other than the following, we can immediately reject using
   the inplace algorithm:

      - Changing collation while keeping the charset.
      - Changing any charset to the binary charset.
      - Changing utf8mb3 to utf8mb4.

   @note The changes listed above are potentially acceptable if the field
   is not indexed in the target table. This information is not available
   here, and is checked later in fill_alter_inplace_info().

   @note ASCII cannot be converted to UTF-8 inplace because inserting
   non-ascii values into an ASCII column only trigger a warning not an
   error.

   @param from - existing Field object.
   @param to   - Create_field object describing new version of field.

   @return true if alter cannot be done inplace due to specified
   condition, false otherwise.
*/
bool charset_prevents_inplace(const Field_str &from, const Create_field &to) {
  if (my_charset_same(to.charset, from.charset()) ||
      my_charset_same(to.charset, &my_charset_bin)) {
    return false;
  }
  return (0 != strcmp(to.charset->csname, MY_UTF8MB4) ||
          0 != strcmp(from.charset()->csname, MY_UTF8MB3));
}

/**
   Predicate to determine if the difference between a Field and the
   new Create_field prevents alter from being done
   inplace. Convenience wrapper for the preceeding predicates.

   @param from - existing Field object.
   @param to   - Create_field object describing new version of field.

   @return true if alter cannot be done inplace due to specified
   condition, false otherwise.
*/
bool change_prevents_inplace(const Field_str &from, const Create_field &to) {
  return sql_type_prevents_inplace(from, to) ||
         length_prevents_inplace(from, to) ||
         charset_prevents_inplace(from, to);
}
}  // namespace

inline int field_type2index(enum_field_types field_type) {
  field_type = real_type_to_type(field_type);
  DBUG_ASSERT(field_type < FIELDTYPE_TEAR_FROM ||
              field_type > FIELDTYPE_TEAR_TO);
  return (field_type < FIELDTYPE_TEAR_FROM
              ? field_type
              : ((int)FIELDTYPE_TEAR_FROM) + (field_type - FIELDTYPE_TEAR_TO) -
                    1);
}

static enum_field_types field_types_merge_rules[FIELDTYPE_NUM][FIELDTYPE_NUM] =
    {
        /* MYSQL_TYPE_DECIMAL -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_DECIMAL, MYSQL_TYPE_DECIMAL,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_TINY -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_TINY,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_TINY, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_SHORT -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_SHORT,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_SHORT, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_SHORT,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_LONG -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_LONG, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_FLOAT -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_FLOAT,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_FLOAT,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_FLOAT,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_DOUBLE -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_NULL -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_TINY,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_TIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_BIT,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_JSON,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_ENUM,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_SET, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_GEOMETRY},
        /* MYSQL_TYPE_TIMESTAMP -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_TIMESTAMP, MYSQL_TYPE_TIMESTAMP,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_LONGLONG -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_INT24 -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_INT24, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_INT24, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL    MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_DATE -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_TIME -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_TIME, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_TIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_DATETIME -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_YEAR -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_YEAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_YEAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_NEWDATE -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_DATETIME,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_DATETIME, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_VARCHAR -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_BIT -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_BIT, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONGLONG, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_LONGLONG,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_BIT,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_JSON -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_JSON, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_JSON,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_NEWDECIMAL -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_DOUBLE, MYSQL_TYPE_DOUBLE,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_NEWDECIMAL,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_ENUM -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_ENUM, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_SET -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_SET, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_TINY_BLOB -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_TINY_BLOB},
        /* MYSQL_TYPE_MEDIUM_BLOB -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_MEDIUM_BLOB},
        /* MYSQL_TYPE_LONG_BLOB -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_LONG_BLOB},
        /* MYSQL_TYPE_BLOB -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_BLOB, MYSQL_TYPE_BLOB},
        /* MYSQL_TYPE_VAR_STRING -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR},
        /* MYSQL_TYPE_STRING -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_STRING, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_STRING,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_STRING,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_STRING, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_STRING},
        /* MYSQL_TYPE_GEOMETRY -> */
        {// MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
         MYSQL_TYPE_GEOMETRY, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_BIT          <16>-<244>
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_JSON
         MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
         MYSQL_TYPE_VARCHAR, MYSQL_TYPE_TINY_BLOB,
         // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
         MYSQL_TYPE_MEDIUM_BLOB, MYSQL_TYPE_LONG_BLOB,
         // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
         MYSQL_TYPE_BLOB, MYSQL_TYPE_VARCHAR,
         // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
         MYSQL_TYPE_STRING, MYSQL_TYPE_GEOMETRY}};

bool pre_validate_value_generator_expr(Item *expression, const char *name,
                                       Value_generator_source source) {
  enum error_type { ER_GENERATED_ROW, ER_NAMED_FUNCTION, MAX_ERROR };
  int error_code_map[][MAX_ERROR] = {
      // Generated column
      {ER_GENERATED_COLUMN_ROW_VALUE,
       ER_GENERATED_COLUMN_NAMED_FUNCTION_IS_NOT_ALLOWED},
      // Default expression
      {ER_DEFAULT_VAL_GENERATED_ROW_VALUE,
       ER_DEFAULT_VAL_GENERATED_NAMED_FUNCTION_IS_NOT_ALLOWED},
      // Check constraint
      {ER_CHECK_CONSTRAINT_ROW_VALUE,
       ER_CHECK_CONSTRAINT_NAMED_FUNCTION_IS_NOT_ALLOWED}};

  // ROW values are not allowed
  if (expression->type() == Item::ROW_ITEM) {
    my_error(error_code_map[source][ER_GENERATED_ROW], MYF(0), name);
    return true;
  }

  Check_function_as_value_generator_parameters checker_args(
      error_code_map[source][ER_NAMED_FUNCTION], source);

  if (expression->walk(&Item::check_function_as_value_generator,
                       enum_walk::SUBQUERY_POSTFIX,
                       pointer_cast<uchar *>(&checker_args))) {
    my_error(checker_args.err_code, MYF(0), name,
             checker_args.banned_function_name);
    return true;
  }

  return false;
}

/**
  Set field to temporary value NULL.
*/
void Field::set_tmp_null() {
  m_is_tmp_null = true;

  m_check_for_truncated_fields_saved =
      table ? table->in_use->check_for_truncated_fields
            : current_thd->check_for_truncated_fields;
}

uint Field::is_equal(const Create_field *new_field) const {
  return (real_type() == new_field->sql_type);
}
/**
  Return type of which can carry value of both given types in UNION result.

  @param a  type for merging
  @param b  type for merging

  @return
    type of field
*/

enum_field_types Field::field_type_merge(enum_field_types a,
                                         enum_field_types b) {
  return field_types_merge_rules[field_type2index(a)][field_type2index(b)];
}

static Item_result field_types_result_type[FIELDTYPE_NUM] = {
    // MYSQL_TYPE_DECIMAL      MYSQL_TYPE_TINY
    DECIMAL_RESULT, INT_RESULT,
    // MYSQL_TYPE_SHORT        MYSQL_TYPE_LONG
    INT_RESULT, INT_RESULT,
    // MYSQL_TYPE_FLOAT        MYSQL_TYPE_DOUBLE
    REAL_RESULT, REAL_RESULT,
    // MYSQL_TYPE_NULL         MYSQL_TYPE_TIMESTAMP
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_LONGLONG     MYSQL_TYPE_INT24
    INT_RESULT, INT_RESULT,
    // MYSQL_TYPE_DATE         MYSQL_TYPE_TIME
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_DATETIME     MYSQL_TYPE_YEAR
    STRING_RESULT, INT_RESULT,
    // MYSQL_TYPE_NEWDATE      MYSQL_TYPE_VARCHAR
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_BIT          <16>-<244>
    INT_RESULT,
    // MYSQL_TYPE_JSON
    STRING_RESULT,
    // MYSQL_TYPE_NEWDECIMAL   MYSQL_TYPE_ENUM
    DECIMAL_RESULT, STRING_RESULT,
    // MYSQL_TYPE_SET          MYSQL_TYPE_TINY_BLOB
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_MEDIUM_BLOB  MYSQL_TYPE_LONG_BLOB
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_BLOB         MYSQL_TYPE_VAR_STRING
    STRING_RESULT, STRING_RESULT,
    // MYSQL_TYPE_STRING       MYSQL_TYPE_GEOMETRY
    STRING_RESULT, STRING_RESULT};

/**
  Convert Field::geometry_type to the corresponding Geometry::wkbType.

  @param t The geometry_type to convert

  @return The corresponding Geometry::wkbType, or
          Geometry::wkb_invalid_type if there's not suitable type.
*/
static Geometry::wkbType geometry_type_to_wkb_type(Field::geometry_type t) {
  switch (t) {
    case Field::GEOM_GEOMETRY:
      return Geometry::wkb_invalid_type;
    case Field::GEOM_POINT:
      return Geometry::wkb_point;
    case Field::GEOM_LINESTRING:
      return Geometry::wkb_linestring;
    case Field::GEOM_POLYGON:
      return Geometry::wkb_polygon;
    case Field::GEOM_MULTIPOINT:
      return Geometry::wkb_multipoint;
    case Field::GEOM_MULTILINESTRING:
      return Geometry::wkb_multilinestring;
    case Field::GEOM_MULTIPOLYGON:
      return Geometry::wkb_multipolygon;
    case Field::GEOM_GEOMETRYCOLLECTION:
      return Geometry::wkb_geometrycollection;
    default:
      DBUG_ASSERT(0);
      return Geometry::wkb_invalid_type;
  }
}

/*
  Test if the given string contains important data:
  not spaces for character string,
  or any data for binary string.

  SYNOPSIS
    test_if_important_data()
    cs          Character set
    str         String to test
    strend      String end

  RETURN
    false - If string does not have important data
    true  - If string has some important data
*/

static bool test_if_important_data(const CHARSET_INFO *cs, const char *str,
                                   const char *strend) {
  if (cs != &my_charset_bin)
    str += cs->cset->scan(cs, str, strend, MY_SEQ_SPACES);
  return (str < strend);
}

/**
   Function to compare two unsigned integers for their relative order.
   Used below. In an anonymous namespace to not clash with definitions
   in other files.
 */

namespace {

int compare(unsigned int a, unsigned int b) {
  if (a < b) return -1;
  if (b < a) return 1;
  return 0;
}

}  // namespace

/**
  Detect Item_result by given field type of UNION merge result.

  @param field_type  given field type

  @return
    Item_result (type of internal MySQL expression result)
*/

Item_result Field::result_merge_type(enum_field_types field_type) {
  return field_types_result_type[field_type2index(field_type)];
}

/*****************************************************************************
  Static help functions
*****************************************************************************/

/**
  Output a warning for erroneous conversion of strings to numerical
  values. For use with ER_TRUNCATED_WRONG_VALUE[_FOR_FIELD]

  @param thd         THD object
  @param str         pointer to string that failed to be converted
  @param length      length of string
  @param cs          charset for string
  @param typestr     string describing type converted to
  @param error       error value to output
  @param field_name  (for *_FOR_FIELD) name of field
  @param row_num     (for *_FOR_FIELD) row number
 */
static void push_numerical_conversion_warning(
    THD *thd, const char *str, uint length, const CHARSET_INFO *cs,
    const char *typestr, int error, const char *field_name = "UNKNOWN",
    ulong row_num = 0) {
  char buf[std::max(std::max(DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE,
                             LONGLONG_TO_STRING_CONVERSION_BUFFER_SIZE),
                    DECIMAL_TO_STRING_CONVERSION_BUFFER_SIZE)];

  String tmp(buf, sizeof(buf), cs);
  tmp.copy(str, length, cs);
  push_warning_printf(thd, Sql_condition::SL_WARNING, error,
                      ER_THD_NONCONST(thd, error), typestr, tmp.c_ptr(),
                      field_name, row_num);
}

/**
  Emits a warning for the decimal conversion error. May modify
  dec_value if there was conversion overflow or bad number.

  @param field             Field to operate on
  @param dec_error         decimal library return code
                           (E_DEC_* see include/decimal.h)
  @param [in,out] dec_value Decimal value returned by conversion function.
  @param from              Value converted from
  @param length            Length of 'from'
  @param charset_arg       Charset of 'from'
*/
static void set_decimal_warning(Field_new_decimal *field, int dec_error,
                                my_decimal *dec_value, const char *from,
                                size_t length,
                                const CHARSET_INFO *charset_arg) {
  switch (dec_error) {
    case E_DEC_TRUNCATED:
      field->set_warning(Sql_condition::SL_NOTE, WARN_DATA_TRUNCATED, 1);
      break;
    case E_DEC_OVERFLOW:
      field->set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE,
                         1);
      field->set_value_on_overflow(dec_value, dec_value->sign());
      break;
    case E_DEC_BAD_NUM:
      ErrConvString errmsg(from, length, charset_arg);
      const Diagnostics_area *da = field->table->in_use->get_stmt_da();
      push_warning_printf(
          field->table->in_use, Sql_condition::SL_WARNING,
          ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
          ER_THD(field->table->in_use, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD),
          "decimal", errmsg.ptr(), field->field_name,
          da->current_row_for_condition());
      my_decimal_set_zero(dec_value);
  }
}

/**
  Copy string with optional character set conversion.

  This calls helper function well_formed_copy_nchars to copy string
  with optional character set convertion. Specially, it checks if
  the ASCII code point exceeds code range. If YES, it allows the
  input but raises a warning.

  @param to_cs                    Character set of "to" string
  @param to                       Store result here
  @param to_length                Maxinum length of "to" string
  @param from_cs                  From character set
  @param from                     Copy from here
  @param from_length              Length of from string
  @param nchars                   Copy not more that nchars characters
  @param well_formed_error_pos    Return position when "from" is not well
                                  formed or NULL otherwise.
  @param cannot_convert_error_pos Return position where a not convertable
                                  character met, or NULL otherwise.
  @param from_end_pos             Return position where scanning of "from"
                                  string stopped.

  @retval length of bytes copied to 'to'
*/

static size_t field_well_formed_copy_nchars(
    const CHARSET_INFO *to_cs, char *to, size_t to_length,
    const CHARSET_INFO *from_cs, const char *from, size_t from_length,
    size_t nchars, const char **well_formed_error_pos,
    const char **cannot_convert_error_pos, const char **from_end_pos) {
  size_t res = well_formed_copy_nchars(
      to_cs, to, to_length, from_cs, from, from_length, nchars,
      well_formed_error_pos, cannot_convert_error_pos, from_end_pos);
  /*
   If the code point is out of ascii range, we only give user a warning
   in 5.7. Need to change to give a ERROR in future version.
  */
  if ((to_cs->state & MY_CS_PUREASCII) && *well_formed_error_pos != nullptr) {
    char tmp[32];
    *well_formed_error_pos = nullptr;
    convert_to_printable(tmp, sizeof(tmp), from, from_length, from_cs, 6);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_INVALID_CHARACTER_STRING,
        ER_THD(current_thd, ER_INVALID_CHARACTER_STRING), "ascii", tmp);
  }
  return res;
}

/**
  Check whether a field type can be partially indexed by a key.

  This is a static method, rather than a virtual function, because we need
  to check the type of a non-Field in mysql_alter_table().

  @param type  field type

  @retval
    true  Type can have a prefixed key
  @retval
    false Type can not have a prefixed key
*/

bool Field::type_can_have_key_part(enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_GEOMETRY:
      return true;
    default:
      return false;
  }
}

/**
  Numeric fields base class constructor.
*/
Field_num::Field_num(uchar *ptr_arg, uint32 len_arg, uchar *null_ptr_arg,
                     uchar null_bit_arg, uchar auto_flags_arg,
                     const char *field_name_arg, uint8 dec_arg, bool zero_arg,
                     bool unsigned_arg)
    : Field(ptr_arg, len_arg, null_ptr_arg, null_bit_arg, auto_flags_arg,
            field_name_arg),
      dec(dec_arg),
      zerofill(zero_arg) {
  unsigned_flag = unsigned_arg;
  if (zerofill) flags |= ZEROFILL_FLAG;
  if (unsigned_flag) flags |= UNSIGNED_FLAG;
}

void Field_num::prepend_zeros(String *value) const {
  int diff;
  if ((diff = (int)(field_length - value->length())) > 0) {
    const bool error = value->mem_realloc(field_length);
    if (!error) {
      memmove(value->ptr() + field_length - value->length(), value->ptr(),
              value->length());
      memset(value->ptr(), '0', diff);
      value->length(field_length);
    }
  }
}

/**
  Test if given number is a int.

  @todo
    Make this multi-byte-character safe

  @param cs		Character set
  @param str		String to test
  @param length         Length of 'str'
  @param int_end	Pointer to char after last used digit
  @param error          Return code from strntoull10rnd()

  @note
    This is called after one has called strntoull10rnd() function.

  @return TYPE_OK, TYPE_ERR_BAD_VALUE or TYPE_WARN_TRUNCATED
*/

type_conversion_status Field_num::check_int(const CHARSET_INFO *cs,
                                            const char *str, size_t length,
                                            const char *int_end, int error) {
  /* Test if we get an empty string or wrong integer */
  if (str == int_end || error == MY_ERRNO_EDOM) {
    ErrConvString err(str, length, cs);
    push_warning_printf(
        table->in_use, Sql_condition::SL_WARNING,
        ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
        ER_THD(table->in_use, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD), "integer",
        err.ptr(), field_name,
        table->in_use->get_stmt_da()->current_row_for_condition());
    return TYPE_ERR_BAD_VALUE;
  }
  /* Test if we have garbage at the end of the given string. */
  if (test_if_important_data(cs, int_end, str + length)) {
    set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    return TYPE_WARN_TRUNCATED;
  }
  return TYPE_OK;
}

/*
  Conver a string to an integer then check bounds.

  SYNOPSIS
    Field_num::get_int
    cs            Character set
    from          String to convert
    len           Length of the string
    rnd           OUT longlong value
    unsigned_max  max unsigned value
    signed_min    min signed value
    signed_max    max signed value

  DESCRIPTION
    The function calls strntoull10rnd() to get an integer value then
    check bounds and errors returned. In case of any error a warning
    is raised.

  @return TYPE_OK, TYPE_WARN_OUT_OF_RANGE, TYPE_ERR_BAD_VALUE or
          TYPE_WARN_TRUNCATED
*/

type_conversion_status Field_num::get_int(const CHARSET_INFO *cs,
                                          const char *from, size_t len,
                                          longlong *rnd, ulonglong unsigned_max,
                                          longlong signed_min,
                                          longlong signed_max) {
  const char *end;
  int error;

  *rnd = (longlong)cs->cset->strntoull10rnd(cs, from, len, unsigned_flag, &end,
                                            &error);
  if (unsigned_flag) {
    if ((((ulonglong)*rnd > unsigned_max) && (*rnd = (longlong)unsigned_max)) ||
        error == MY_ERRNO_ERANGE)
      goto out_of_range;
  } else {
    if (*rnd < signed_min) {
      *rnd = signed_min;
      goto out_of_range;
    } else if (*rnd > signed_max) {
      *rnd = signed_max;
      goto out_of_range;
    }
  }
  if (table->in_use->check_for_truncated_fields != 0)
    return check_int(cs, from, len, end, error);

  return TYPE_OK;

out_of_range:
  set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
  return TYPE_WARN_OUT_OF_RANGE;
}

/*
  This is a generic method which is executed only for
  Field_short, Field_medium, Field_long, Field_longlong and Field_tiny.

  The other field types that come from Field_num override this method:
  Field_real (common parent for Field_decimal, Field_float, Field_double),
  Field_new_decimal, Field_year.
*/
type_conversion_status Field_num::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  longlong nr = propagate_datetime_overflow(
      current_thd, [&](int *w) { return TIME_to_ulonglong_round(*ltime, w); });
  return store(ltime->neg ? -nr : nr, false);
}

/**
  Process decimal library return codes and issue warnings for overflow and
  truncation.

  @param op_result  decimal library return code (E_DEC_* see include/decimal.h)

  @retval 0 No error or some other errors except overflow
  @retval 1 There was overflow
*/

bool Field::warn_if_overflow(int op_result) {
  if (op_result == E_DEC_OVERFLOW) {
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return true;
  }
  if (op_result == E_DEC_TRUNCATED) {
    set_warning(Sql_condition::SL_NOTE, WARN_DATA_TRUNCATED, 1);
    /* We return 0 here as this is not a critical issue */
  }
  return false;
}

/**
  Interpret field value as an integer but return the result as a string.

  This is used for printing bit_fields as numbers while debugging.
*/

String *Field::val_int_as_str(String *val_buffer, bool unsigned_val) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = &my_charset_bin;
  size_t length;
  longlong value = val_int();

  if (val_buffer->alloc(MY_INT64_NUM_DECIMAL_DIGITS)) return nullptr;
  length = (*cs->cset->longlong10_to_str)(cs, val_buffer->ptr(),
                                          MY_INT64_NUM_DECIMAL_DIGITS,
                                          unsigned_val ? 10 : -10, value);
  val_buffer->length(length);
  return val_buffer;
}

/// This is used as a table name when the table structure is not set up
Field::Field(uchar *ptr_arg, uint32 length_arg, uchar *null_ptr_arg,
             uchar null_bit_arg, uchar auto_flags_arg,
             const char *field_name_arg)
    : ptr(ptr_arg),
      m_hidden(dd::Column::enum_hidden_type::HT_VISIBLE),
      m_null_ptr(null_ptr_arg),
      m_is_tmp_nullable(false),
      m_is_tmp_null(false),
      m_check_for_truncated_fields_saved(CHECK_FIELD_IGNORE),
      table(nullptr),
      orig_table(nullptr),
      table_name(nullptr),
      field_name(field_name_arg),
      field_length(length_arg),
      null_bit(null_bit_arg),
      auto_flags(auto_flags_arg),
      is_created_from_null_item(false),
      m_indexed(false),
      m_warnings_pushed(0),
      gcol_info(nullptr),
      stored_in_db(true),
      unsigned_flag(false),
      m_default_val_expr(nullptr)

{
  flags = is_nullable() ? 0 : NOT_NULL_FLAG;
  comment.str = "";
  comment.length = 0;
  field_index = 0;
}

/**
  Check NOT NULL constraint on the field after temporary nullability is
  disabled.

  @param mysql_errno Warning to report.

  @return TYPE_OK if the value is Ok, or corresponding error code from
  the type_conversion_status enum.
*/
type_conversion_status Field::check_constraints(int mysql_errno) {
  /*
    Ensure that Field::check_constraints() is called only when temporary
    nullability is disabled.
  */

  DBUG_ASSERT(!is_tmp_nullable());

  if (is_nullable()) return TYPE_OK;  // If the field is nullable, we're Ok.

  if (!m_is_tmp_null) return TYPE_OK;  // If the field was not NULL, we're Ok.

  // The field has been set to NULL.

  /*
    If the field is of AUTO_INCREMENT, and the next number
    has been assigned to it, we're Ok.
  */

  if (this == table->next_number_field) return TYPE_OK;

  switch (m_check_for_truncated_fields_saved) {
    case CHECK_FIELD_WARN:
      set_warning(Sql_condition::SL_WARNING, mysql_errno, 1);
      /* fall through */
    case CHECK_FIELD_IGNORE:
      return TYPE_OK;
    case CHECK_FIELD_ERROR_FOR_NULL:
      my_error(ER_BAD_NULL_ERROR, MYF(0), field_name);
      return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
  }

  DBUG_ASSERT(0);  // impossible
  my_error(ER_BAD_NULL_ERROR, MYF(0), field_name);
  return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
}

/**
  Set field to value NULL.

  @param row_offset    This is the offset between the row being updated
                       and table->record[0]
*/
void Field::set_null(ptrdiff_t row_offset) {
  if (is_nullable()) {
    DBUG_ASSERT(m_null_ptr != &dummy_null_buffer);
    m_null_ptr[row_offset] |= null_bit;
  } else if (is_tmp_nullable()) {
    set_tmp_null();
  }
}

/**
  Set field to value NOT NULL.

  @param row_offset    This is the offset between the row being updated
                       and table->record[0]
*/
void Field::set_notnull(ptrdiff_t row_offset) {
  if (is_nullable()) {
    DBUG_ASSERT(m_null_ptr != &dummy_null_buffer);
    m_null_ptr[row_offset] &= (uchar)~null_bit;
  } else if (is_tmp_nullable()) {
    reset_tmp_null();
  }
}

void Field::hash(ulong *nr, ulong *nr2) const {
  if (is_null()) {
    *nr ^= (*nr << 1) | 1;
  } else {
    uint len = pack_length();
    const CHARSET_INFO *cs = sort_charset();
    uint64 tmp1 = *nr;
    uint64 tmp2 = *nr2;
    cs->coll->hash_sort(cs, ptr, len, &tmp1, &tmp2);

    // NOTE: This truncates to 32-bit on Windows, to keep on-disk stability.
    *nr = static_cast<ulong>(tmp1);
    *nr2 = static_cast<ulong>(tmp2);
  }
}

void Field::copy_data(ptrdiff_t src_record_offset) {
  memcpy(ptr, ptr + src_record_offset, pack_length());

  if (is_nullable()) {
    // Set to NULL if the source record is NULL, otherwise set to NOT-NULL.
    DBUG_ASSERT(m_null_ptr != &dummy_null_buffer);
    m_null_ptr[0] = (m_null_ptr[0] & ~null_bit) |
                    (m_null_ptr[src_record_offset] & null_bit);
  } else if (is_tmp_nullable())
    m_is_tmp_null = false;
}

bool Field::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  char buff[MAX_FIELD_WIDTH];
  String tmp(buff, sizeof(buff), charset());
  String *res = val_str(&tmp);
  return res ? protocol->store(res) : protocol->store_null();
}

/**
   Check to see if field size is compatible with destination.

   This method is used in row-based replication to verify that the
   slave's field size is less than or equal to the master's field
   size. The encoded field metadata (from the master or source) is
   decoded and compared to the size of this field (the slave or
   destination).

   The comparison is made so that if the source data (from the master)
   is less than the target data (on the slave), -1 is returned in
   <code>*order_var</code>. This implies that a conversion is
   necessary, but that it is lossy and can result in truncation of the
   value.

   If the source data is strictly greater than the target data, 1 is
   returned in <code>*order_var</code>. This implies that the source
   type can is contained in the target type and that a conversion is
   necessary but is non-lossy.

   If no conversion is required to fit the source type in the target
   type, 0 is returned in <code>*order_var</code>.

   @param   field_metadata   Encoded size in field metadata
   @param   order_var        Pointer to variable where the order
                             between the source field and this field
                             will be returned.

   @return @c true if this field's size is compatible with the
   master's field size, @c false otherwise.
*/
bool Field::compatible_field_size(uint field_metadata, Relay_log_info *, uint16,
                                  int *order_var) const {
  uint const source_size = pack_length_from_metadata(field_metadata);
  uint const destination_size = row_pack_length();
  DBUG_PRINT("debug", ("real_type: %d, source_size: %u, destination_size: %u",
                       real_type(), source_size, destination_size));
  *order_var = compare(source_size, destination_size);
  return true;
}

type_conversion_status Field::store(const char *to, size_t length,
                                    const CHARSET_INFO *cs,
                                    enum_check_fields check_level) {
  enum_check_fields old_check_level = table->in_use->check_for_truncated_fields;
  table->in_use->check_for_truncated_fields = check_level;
  const type_conversion_status res = store(to, length, cs);
  table->in_use->check_for_truncated_fields = old_check_level;
  return res;
}

uchar *Field::pack(uchar *to, const uchar *from, uint max_length,
                   bool low_byte_first MY_ATTRIBUTE((unused))) const {
  uint32 length = std::min(pack_length(), max_length);
  memcpy(to, from, length);
  return to + length;
}

/**
   Unpack a field from row data.

   This method is used to unpack a field from a master whose size of
   the field is less than that of the slave.

   The <code>param_data</code> parameter is a two-byte integer (stored
   in the least significant 16 bits of the unsigned integer) usually
   consisting of two parts: the real type in the most significant byte
   and a original pack length in the least significant byte.

   The exact layout of the <code>param_data</code> field is given by
   the <code>Table_map_log_event::save_field_metadata()</code>.

   This is the default method for unpacking a field. It just copies
   the memory block in byte order (of original pack length bytes or
   length of field, whichever is smaller).

   @param   to         Destination of the data
   @param   from       Source of the data
   @param   param_data Real type and original pack length of the field
                       data

   @param low_byte_first
   If this flag is @c true, all composite entities (e.g., lengths)
   should be unpacked in little-endian format; otherwise, the entities
   are unpacked in native order.

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field::unpack(uchar *to, const uchar *from, uint param_data,
                           bool low_byte_first MY_ATTRIBUTE((unused))) {
  uint length = pack_length();
  int from_type = 0;
  /*
    If from length is > 255, it has encoded data in the upper bits. Need
    to mask it out.
  */
  if (param_data > 255) {
    from_type = (param_data & 0xff00) >> 8U;  // real_type.
    param_data = param_data & 0x00ff;         // length.
  }

  if ((param_data == 0) || (length == param_data) ||
      (from_type != real_type())) {
    memcpy(to, from, length);
    return from + length;
  }

  uint len = (param_data && (param_data < length)) ? param_data : length;

  memcpy(to, from, param_data > length ? length : len);
  return from + len;
}

/**
  Appends the UNSIGNED and ZEROFILL attributes to a String if a Field_num has
  these attributes.

  @param field the field with the attributes to append
  @param[in,out] res the String to append to
*/
static void append_zerofill_and_unsigned(const Field_num *field, String *res) {
  if (field->unsigned_flag) res->append(STRING_WITH_LEN(" unsigned"));
  if (field->zerofill) res->append(STRING_WITH_LEN(" zerofill"));
}

/// Writes an integer type specification to a string.
static void integer_sql_type(const Field_num *field, const char *type_name,
                             String *res) {
  res->length(0);
  res->append(type_name);
  if (field->zerofill) res->append_parenthesized(field->field_length);
  append_zerofill_and_unsigned(field, res);
}

void Field::make_send_field(Send_field *field) const {
  if (orig_table && orig_table->s->db.str && *orig_table->s->db.str) {
    field->db_name = orig_table->s->db.str;
    if (orig_table->pos_in_table_list &&
        orig_table->pos_in_table_list->schema_table)
      field->org_table_name =
          (orig_table->pos_in_table_list->schema_table->table_name);
    else
      field->org_table_name = orig_table->s->table_name.str;
  } else
    field->org_table_name = field->db_name = "";
  if (orig_table && orig_table->alias) {
    field->table_name = orig_table->alias;
    field->org_col_name = field_name;
  } else {
    field->table_name = "";
    field->org_col_name = "";
  }
  field->col_name = field_name;
  field->charsetnr = charset()->number;
  field->length = field_length;
  field->type = type();
  field->flags = table->is_nullable() ? (flags & ~NOT_NULL_FLAG) : flags;
  field->decimals = decimals();
  field->field = false;
}

/**
  Conversion from decimal to longlong. Checks overflow and returns
  correct value (min/max) in case of overflow.

  @param val             value to be converted
  @param unsigned_flag   type of integer to which we convert val
  @param has_overflow    true if there is overflow

  @return
    value converted from val
*/
longlong Field::convert_decimal2longlong(const my_decimal *val,
                                         bool unsigned_flag,
                                         bool *has_overflow) {
  if (unsigned_flag && val->sign()) {
    // Converting a signed decimal to unsigned int
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    *has_overflow = true;
    return 0;
  }

  longlong val_ll;
  int conversion_error =
      my_decimal2int(E_DEC_ERROR & ~E_DEC_OVERFLOW & ~E_DEC_TRUNCATED, val,
                     unsigned_flag, &val_ll);

  if (warn_if_overflow(conversion_error)) {
    *has_overflow = true;
    if (unsigned_flag) return ULLONG_MAX;

    return (val->sign() ? LLONG_MIN : LLONG_MAX);
  }

  return val_ll;
}

/**
  Storing decimal in integer fields.

  @param val       value for storing

  @note
    This method is used by all integer fields, real/decimal redefine it

  @retval TYPE_OK   Storage of value went fine without warnings or errors
  @retval !TYPE_OK  Warning/error as indicated by type_conversion_status enum
                    value
*/
type_conversion_status Field_num::store_decimal(const my_decimal *val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  bool has_overflow = false;
  longlong i = convert_decimal2longlong(val, unsigned_flag, &has_overflow);
  const type_conversion_status res = store(i, unsigned_flag);
  return has_overflow ? TYPE_WARN_OUT_OF_RANGE : res;
}

/**
  Return decimal value of integer field.

  @param decimal_value     buffer for storing decimal value

  @note
    This method is used by all integer fields, real/decimal redefine it.
    All longlong values fit in our decimal buffer which cal store 8*9=72
    digits of integer number

  @return
    pointer to decimal buffer with value of field
*/

my_decimal *Field_num::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_ASSERT(result_type() == INT_RESULT);
  longlong nr = val_int();
  int2my_decimal(E_DEC_FATAL_ERROR, nr, unsigned_flag, decimal_value);
  return decimal_value;
}

bool Field_num::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) const {
  DBUG_ASSERT(result_type() == INT_RESULT);
  return my_longlong_to_datetime_with_warn(val_int(), ltime, fuzzydate);
}

bool Field_num::get_time(MYSQL_TIME *ltime) const {
  DBUG_ASSERT(result_type() == INT_RESULT);
  return my_longlong_to_time_with_warn(val_int(), ltime);
}

Field_str::Field_str(uchar *ptr_arg, uint32 len_arg, uchar *null_ptr_arg,
                     uchar null_bit_arg, uchar auto_flags_arg,
                     const char *field_name_arg,
                     const CHARSET_INFO *charset_arg)
    : Field(ptr_arg, len_arg, null_ptr_arg, null_bit_arg, auto_flags_arg,
            field_name_arg) {
  field_charset = charset_arg;
  if (charset_arg->state & MY_CS_BINSORT) flags |= BINARY_FLAG;
  field_derivation = DERIVATION_IMPLICIT;
  char_length_cache = char_length();
}

void Field_str::make_send_field(Send_field *field) const {
  Field::make_send_field(field);
  field->decimals = 0;
}

/**
  Decimal representation of Field_str.

  @param d         value for storing

  @note
    Field_str is the base class for fields implemeting
    [VAR]CHAR, VAR[BINARY], BLOB/TEXT, GEOMETRY, JSON.
    String value should be converted to floating point value according
    our rules, so we use double to store value of decimal in string.

  @todo
    use decimal2string?

  @retval
    0     OK
  @retval
    !=0  error
*/

type_conversion_status Field_str::store_decimal(const my_decimal *d) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  double val;
  /* TODO: use decimal2string? */
  int err = my_decimal2double(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW, d, &val);
  warn_if_overflow(err);
  const type_conversion_status res = store(val);

  return (err != E_DEC_OK) ? decimal_err_to_type_conv_status(err) : res;
}

bool Field::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) const {
  char buff[MAX_DATE_STRING_REP_LENGTH];
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  return !(res = val_str(&tmp)) ||
         str_to_datetime_with_warn(res, ltime, fuzzydate);
}

bool Field::get_time(MYSQL_TIME *ltime) const {
  char buff[MAX_DATE_STRING_REP_LENGTH];
  String tmp(buff, sizeof(buff), &my_charset_bin), *res;
  return !(res = val_str(&tmp)) || str_to_time_with_warn(res, ltime);
}

bool Field::get_timestamp(struct timeval *tm, int *warnings) const {
  MYSQL_TIME ltime;
  DBUG_ASSERT(!is_null());
  return get_date(&ltime, TIME_FUZZY_DATE) ||
         datetime_to_timeval(current_thd, &ltime, tm, warnings);
}

/**
  This is called when storing a date in a string.

  @note
    Needs to be changed if/when we want to support different time formats.
*/

type_conversion_status Field::store_time(MYSQL_TIME *ltime, uint8 dec_arg) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  char buff[MAX_DATE_STRING_REP_LENGTH];
  uint length = my_TIME_to_str(*ltime, buff,
                               std::min(dec_arg, uint8{DATETIME_MAX_DECIMALS}));
  /* Avoid conversion when field character set is ASCII compatible */
  return store(
      buff, length,
      (charset()->state & MY_CS_NONASCII) ? &my_charset_latin1 : charset());
}

bool Field::optimize_range(uint idx, uint part) const {
  return table->file->index_flags(idx, part, true) & HA_READ_RANGE;
}

Field *Field::new_field(MEM_ROOT *root, TABLE *new_table) const {
  Field *tmp = clone(root);
  if (tmp == nullptr) return nullptr;

  if (tmp->table && tmp->table->is_nullable()) tmp->flags &= ~NOT_NULL_FLAG;
  tmp->table = new_table;
  tmp->key_start.init(0);
  tmp->part_of_key.init(0);
  tmp->part_of_prefixkey.init(0);
  tmp->part_of_sortkey.init(0);
  tmp->m_indexed = false;
  /*
    todo: We should never alter auto_flags after an object is constructed,
    and the member should be made const. But a lot of code depends upon this
    hack, and some flags are completely unrelated so we can never be quite
    sure which parts of the server will break.
  */
  tmp->auto_flags = Field::NONE;
  tmp->flags &= (NOT_NULL_FLAG | BLOB_FLAG | UNSIGNED_FLAG | ZEROFILL_FLAG |
                 BINARY_FLAG | ENUM_FLAG | SET_FLAG | NOT_SECONDARY_FLAG);
  tmp->reset_fields();
  return tmp;
}

Field *Field::new_key_field(MEM_ROOT *root, TABLE *new_table, uchar *new_ptr,
                            uchar *new_null_ptr, uint new_null_bit) const {
  Field *tmp = new_field(root, new_table);
  if (tmp != nullptr) {
    tmp->ptr = new_ptr;
    tmp->m_null_ptr = new_null_ptr;
    tmp->null_bit = new_null_bit;
  }
  return tmp;
}

void Field::evaluate_insert_default_function() {
  if (has_insert_default_datetime_value_expression())
    Item_func_now_local::store_in(this);
  // evaluate and store the values generated by default expression
  else if (has_insert_default_general_value_expression())
    m_default_val_expr->expr_item->save_in_field(this, false);
}

void Field::evaluate_update_default_function() {
  if (has_update_default_datetime_value_expression())
    Item_func_now_local::store_in(this);
}

/****************************************************************************
  Field_null, a field that always return NULL
****************************************************************************/

void Field_null::sql_type(String &res) const {
  res.set_ascii(STRING_WITH_LEN("null"));
}

/****************************************************************************
  Functions for the Field_decimal class
  This is an number stored as a pre-space (or pre-zero) string
****************************************************************************/

type_conversion_status Field_decimal::reset() {
  Field_decimal::store(STRING_WITH_LEN("0"), &my_charset_bin);
  return TYPE_OK;
}

void Field_decimal::overflow(bool negative) {
  uint len = field_length;
  uchar *to = ptr, filler = '9';

  set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
  if (negative) {
    if (!unsigned_flag) {
      /* Put - sign as a first digit so we'll have -999..999 or 999..999 */
      *to++ = '-';
      len--;
    } else {
      filler = '0';  // Fill up with 0
      if (!zerofill) {
        /*
          Handle unsigned integer without zerofill, in which case
          the number should be of format '   0' or '   0.000'
        */
        uint whole_part = field_length - (dec ? dec + 2 : 1);
        // Fill with spaces up to the first digit
        memset(to, ' ', whole_part);
        to += whole_part;
        len -= whole_part;
        // The main code will also handle the 0 before the decimal point
      }
    }
  }
  memset(to, filler, len);
  if (dec) ptr[field_length - dec - 1] = '.';
  return;
}

type_conversion_status Field_decimal::store(const char *from_arg, size_t len,
                                            const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  char buff[STRING_BUFFER_USUAL_SIZE];
  String tmp(buff, sizeof(buff), &my_charset_bin);
  const uchar *from = pointer_cast<const uchar *>(from_arg);

  /* Convert character set if the old one is multi uchar */
  if (cs->mbmaxlen > 1) {
    uint dummy_errors;
    tmp.copy(from_arg, len, cs, &my_charset_bin, &dummy_errors);
    from = (uchar *)tmp.ptr();
    len = tmp.length();
  }

  const uchar *end = from + len;
  /* The pointer where the field value starts (i.e., "where to write") */
  uchar *to = ptr;
  uint tmp_dec, tmp_uint;
  /*
    The sign of the number : will be 0 (means positive but sign not
    specified), '+' or '-'
  */
  uchar sign_char = 0;
  /* The pointers where prezeros start and stop */
  const uchar *pre_zeros_from, *pre_zeros_end;
  /* The pointers where digits at the left of '.' start and stop */
  const uchar *int_digits_from, *int_digits_end;
  /* The pointers where digits at the right of '.' start and stop */
  const uchar *frac_digits_from, *frac_digits_end;
  /* The sign of the exponent : will be 0 (means no exponent), '+' or '-' */
  char expo_sign_char = 0;
  uint exponent = 0;  // value of the exponent
  /*
    Pointers used when digits move from the left of the '.' to the
    right of the '.' (explained below)
  */
  const uchar *int_digits_tail_from = nullptr;
  /* Number of 0 that need to be added at the left of the '.' (1E3: 3 zeros) */
  uint int_digits_added_zeros = 0;
  /*
    Pointer used when digits move from the right of the '.' to the left
    of the '.'
  */
  const uchar *frac_digits_head_end = nullptr;
  /* Number of 0 that need to be added at the right of the '.' (for 1E-3) */
  uint frac_digits_added_zeros = 0;
  uchar *pos, *tmp_left_pos, *tmp_right_pos;
  /* Pointers that are used as limits (begin and end of the field buffer) */
  uchar *left_wall, *right_wall;
  uchar tmp_char;
  /*
    To remember if table->in_use->num_truncated_fields has already
    been incremented, to do that only once
  */
  bool has_incremented_num_truncated_fields = false;

  /*
    There are three steps in this function :
    - parse the input string
    - modify the position of digits around the decimal dot '.'
      according to the exponent value (if specified)
    - write the formatted number
  */

  if ((tmp_dec = dec)) tmp_dec++;

  /* skip pre-space */
  while (from != end && my_isspace(&my_charset_bin, *from)) from++;
  if (from == end) {
    set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    has_incremented_num_truncated_fields = true;
  } else if (*from == '+' || *from == '-')  // Found some sign ?
  {
    sign_char = *from++;
    /*
      We allow "+" for unsigned decimal unless defined different
      Both options allowed as one may wish not to have "+" for unsigned numbers
      because of data processing issues
    */
    if (unsigned_flag) {
      if (sign_char == '-') {
        Field_decimal::overflow(true);
        return TYPE_WARN_OUT_OF_RANGE;
      }
    }
  }

  pre_zeros_from = from;
  for (; from != end && *from == '0'; from++)
    ;  // Read prezeros
  pre_zeros_end = int_digits_from = from;
  /* Read non zero digits at the left of '.'*/
  for (; from != end && my_isdigit(&my_charset_bin, *from); from++)
    ;
  int_digits_end = from;
  if (from != end && *from == '.')  // Some '.' ?
    from++;
  frac_digits_from = from;
  /* Read digits at the right of '.' */
  for (; from != end && my_isdigit(&my_charset_bin, *from); from++)
    ;
  frac_digits_end = from;
  // Some exponentiation symbol ?
  if (from != end && (*from == 'e' || *from == 'E')) {
    from++;
    if (from != end && (*from == '+' || *from == '-'))  // Some exponent sign ?
      expo_sign_char = *from++;
    else
      expo_sign_char = '+';
    /*
      Read digits of the exponent and compute its value.  We must care about
      'exponent' overflow, because as unsigned arithmetic is "modulo", big
      exponents will become small (e.g. 1e4294967296 will become 1e0, and the
      field will finally contain 1 instead of its max possible value).
    */
    for (; from != end && my_isdigit(&my_charset_bin, *from); from++) {
      exponent = 10 * exponent + (*from - '0');
      if (exponent > MAX_EXPONENT) break;
    }
  }

  /*
    We only have to generate warnings if check_for_truncated_fields is set.
    This is to avoid extra checks of the number when they are not needed.
    Even if this flag is not set, it's OK to increment warnings, if
    it makes the code easer to read.
  */

  if (table->in_use->check_for_truncated_fields) {
    // Skip end spaces
    for (; from != end && my_isspace(&my_charset_bin, *from); from++)
      ;
    if (from != end)  // If still something left, warn
    {
      set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
      has_incremented_num_truncated_fields = true;
    }
  }

  /*
    Now "move" digits around the decimal dot according to the exponent value,
    and add necessary zeros.
    Examples :
    - 1E+3 : needs 3 more zeros at the left of '.' (int_digits_added_zeros=3)
    - 1E-3 : '1' moves at the right of '.', and 2 more zeros are needed
    between '.' and '1'
    - 1234.5E-3 : '234' moves at the right of '.'
    These moves are implemented with pointers which point at the begin
    and end of each moved segment. Examples :
    - 1234.5E-3 : before the code below is executed, the int_digits part is
    from '1' to '4' and the frac_digits part from '5' to '5'. After the code
    below, the int_digits part is from '1' to '1', the frac_digits_head
    part is from '2' to '4', and the frac_digits part from '5' to '5'.
    - 1234.5E3 : before the code below is executed, the int_digits part is
    from '1' to '4' and the frac_digits part from '5' to '5'. After the code
    below, the int_digits part is from '1' to '4', the int_digits_tail
    part is from '5' to '5', the frac_digits part is empty, and
    int_digits_added_zeros=2 (to make 1234500).
  */

  /*
     Below tmp_uint cannot overflow with small enough MAX_EXPONENT setting,
     as int_digits_added_zeros<=exponent<4G and
     (int_digits_end-int_digits_from)<=max_allowed_packet<=2G and
     (frac_digits_from-int_digits_tail_from)<=max_allowed_packet<=2G
  */

  if (!expo_sign_char)
    tmp_uint = tmp_dec + (uint)(int_digits_end - int_digits_from);
  else if (expo_sign_char == '-') {
    tmp_uint = min(exponent, (uint)(int_digits_end - int_digits_from));
    frac_digits_added_zeros = exponent - tmp_uint;
    int_digits_end -= tmp_uint;
    frac_digits_head_end = int_digits_end + tmp_uint;
    tmp_uint = tmp_dec + (uint)(int_digits_end - int_digits_from);
  } else  // (expo_sign_char=='+')
  {
    tmp_uint = min(exponent, (uint)(frac_digits_end - frac_digits_from));
    int_digits_added_zeros = exponent - tmp_uint;
    int_digits_tail_from = frac_digits_from;
    frac_digits_from = frac_digits_from + tmp_uint;
    /*
      We "eat" the heading zeros of the
      int_digits.int_digits_tail.int_digits_added_zeros concatenation
      (for example 0.003e3 must become 3 and not 0003)
    */
    if (int_digits_from == int_digits_end) {
      /*
        There was nothing in the int_digits part, so continue
        eating int_digits_tail zeros
      */
      for (; int_digits_tail_from != frac_digits_from &&
             *int_digits_tail_from == '0';
           int_digits_tail_from++)
        ;
      if (int_digits_tail_from == frac_digits_from) {
        // there were only zeros in int_digits_tail too
        int_digits_added_zeros = 0;
      }
    }
    tmp_uint = (uint)(tmp_dec + (int_digits_end - int_digits_from) +
                      (uint)(frac_digits_from - int_digits_tail_from) +
                      int_digits_added_zeros);
  }

  /*
    Now write the formated number

    First the digits of the int_% parts.
    Do we have enough room to write these digits ?
    If the sign is defined and '-', we need one position for it
  */

  if (field_length < tmp_uint + (int)(sign_char == '-')) {
    // too big number, change to max or min number
    Field_decimal::overflow(sign_char == '-');
    return TYPE_WARN_OUT_OF_RANGE;
  }

  /*
    Tmp_left_pos is the position where the leftmost digit of
    the int_% parts will be written
  */
  tmp_left_pos = pos = to + (uint)(field_length - tmp_uint);

  // Write all digits of the int_% parts
  while (int_digits_from != int_digits_end) *pos++ = *int_digits_from++;

  if (expo_sign_char == '+') {
    while (int_digits_tail_from != frac_digits_from)
      *pos++ = *int_digits_tail_from++;
    while (int_digits_added_zeros-- > 0) *pos++ = '0';
  }
  /*
    Note the position where the rightmost digit of the int_% parts has been
    written (this is to later check if the int_% parts contained nothing,
    meaning an extra 0 is needed).
  */
  tmp_right_pos = pos;

  /*
    Step back to the position of the leftmost digit of the int_% parts,
    to write sign and fill with zeros or blanks or prezeros.
  */
  pos = tmp_left_pos - 1;
  if (zerofill) {
    left_wall = to - 1;
    while (pos > left_wall)  // Fill with zeros
      *pos-- = '0';
  } else {
    left_wall = to + (sign_char != 0) - 1;
    if (!expo_sign_char)  // If exponent was specified, ignore prezeros
    {
      for (; pos > left_wall && pre_zeros_from != pre_zeros_end;
           pre_zeros_from++)
        *pos-- = '0';
    }
    if (pos == tmp_right_pos - 1)
      *pos-- = '0';  // no 0 has ever been written, so write one
    left_wall = to - 1;
    if (sign_char && pos != left_wall) {
      /* Write sign if possible (it is if sign is '-') */
      *pos-- = sign_char;
    }
    while (pos != left_wall) *pos-- = ' ';  // fill with blanks
  }

  /*
    Write digits of the frac_% parts ;
    Depending on table->in_use->count_cutted_fields, we may also want
    to know if some non-zero tail of these parts will
    be truncated (for example, 0.002->0.00 will generate a warning,
    while 0.000->0.00 will not)
    (and 0E1000000000 will not, while 1E-1000000000 will)
  */

  pos = to + (uint)(field_length - tmp_dec);  // Calculate post to '.'
  right_wall = to + field_length;
  if (pos != right_wall) *pos++ = '.';

  if (expo_sign_char == '-') {
    while (frac_digits_added_zeros-- > 0) {
      if (pos == right_wall) {
        if (table->in_use->check_for_truncated_fields &&
            !has_incremented_num_truncated_fields)
          break;  // Go on below to see if we lose non zero digits
        return TYPE_OK;
      }
      *pos++ = '0';
    }
    while (int_digits_end != frac_digits_head_end) {
      tmp_char = *int_digits_end++;
      if (pos == right_wall) {
        if (tmp_char != '0')  // Losing a non zero digit ?
        {
          if (!has_incremented_num_truncated_fields)
            set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
          return TYPE_OK;
        }
        continue;
      }
      *pos++ = tmp_char;
    }
  }

  for (; frac_digits_from != frac_digits_end;) {
    tmp_char = *frac_digits_from++;
    if (pos == right_wall) {
      if (tmp_char != '0')  // Losing a non zero digit ?
      {
        if (!has_incremented_num_truncated_fields) {
          /*
            This is a note, not a warning, as we don't want to abort
            when we cut decimals in strict mode
          */
          set_warning(Sql_condition::SL_NOTE, WARN_DATA_TRUNCATED, 1);
        }
        return TYPE_OK;
      }
      continue;
    }
    *pos++ = tmp_char;
  }

  while (pos != right_wall) *pos++ = '0';  // Fill with zeros at right of '.'
  return TYPE_OK;
}

type_conversion_status Field_decimal::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  if (unsigned_flag && nr < 0) {
    overflow(true);
    return TYPE_WARN_OUT_OF_RANGE;
  }

  if (!std::isfinite(nr))  // Handle infinity as special case
  {
    overflow(nr < 0.0);
    return TYPE_WARN_OUT_OF_RANGE;
  }

  size_t i;
  size_t length;
  uchar fyllchar, *to;
  char buff[DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE];

  fyllchar = zerofill ? '0' : ' ';
  length = my_fcvt(nr, dec, buff, nullptr);

  if (length > field_length) {
    overflow(nr < 0.0);
    return TYPE_WARN_OUT_OF_RANGE;
  } else {
    to = ptr;
    for (i = field_length - length; i-- > 0;) *to++ = fyllchar;
    memcpy(to, buff, length);
    return TYPE_OK;
  }
}

type_conversion_status Field_decimal::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  char buff[22];
  uint length, int_part;
  char fyllchar;
  uchar *to;

  if (nr < 0 && unsigned_flag && !unsigned_val) {
    overflow(true);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  length = (uint)(longlong10_to_str(nr, buff, unsigned_val ? 10 : -10) - buff);
  int_part = field_length - (dec ? dec + 1 : 0);

  if (length > int_part) {
    overflow(!unsigned_val && nr < 0L); /* purecov: inspected */
    return TYPE_WARN_OUT_OF_RANGE;
  }

  fyllchar = zerofill ? '0' : ' ';
  to = ptr;
  for (uint i = int_part - length; i-- > 0;) *to++ = fyllchar;
  memcpy(to, buff, length);
  if (dec) {
    to[length] = '.';
    memset(to + length + 1, '0', dec);
  }
  return TYPE_OK;
}

double Field_decimal::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int not_used;
  const char *end_not_used;
  return my_strntod(&my_charset_bin, pointer_cast<const char *>(ptr),
                    field_length, &end_not_used, &not_used);
}

longlong Field_decimal::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int not_used;
  if (unsigned_flag)
    return my_strntoull(&my_charset_bin, pointer_cast<const char *>(ptr),
                        field_length, 10, NULL, &not_used);
  return my_strntoll(&my_charset_bin, pointer_cast<const char *>(ptr),
                     field_length, 10, NULL, &not_used);
}

String *Field_decimal::val_str(String *val_buffer MY_ATTRIBUTE((unused)),
                               String *val_ptr) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const uchar *str;
  size_t tmp_length;

  for (str = ptr; *str == ' '; str++)
    ;
  val_ptr->set_charset(&my_charset_numeric);
  tmp_length = (size_t)(str - ptr);
  if (field_length < tmp_length)  // Error in data
    val_ptr->length(0);
  else
    val_ptr->set_ascii(pointer_cast<const char *>(str),
                       field_length - tmp_length);
  return val_ptr;
}

/**
  Should be able to handle at least the following fixed decimal formats:
  5.00 , -1.0,  05,  -05, +5 with optional pre/end space
*/

int Field_decimal::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  const uchar *end;
  int swap = 0;
  /* First remove prefixes '0', ' ', and '-' */
  for (end = a_ptr + field_length;
       a_ptr != end &&
       (*a_ptr == *b_ptr || ((my_isspace(&my_charset_bin, *a_ptr) ||
                              *a_ptr == '+' || *a_ptr == '0') &&
                             (my_isspace(&my_charset_bin, *b_ptr) ||
                              *b_ptr == '+' || *b_ptr == '0')));
       a_ptr++, b_ptr++) {
    if (*a_ptr == '-')  // If both numbers are negative
      swap = -1 ^ 1;    // Swap result
  }
  if (a_ptr == end) return 0;
  if (*a_ptr == '-') return -1;
  if (*b_ptr == '-') return 1;

  while (a_ptr != end) {
    if (*a_ptr++ != *b_ptr++)
      return swap ^ (a_ptr[-1] < b_ptr[-1] ? -1 : 1);  // compare digits
  }
  return 0;
}

size_t Field_decimal::make_sort_key(uchar *to, size_t length) const {
  uchar *str, *end;
  for (str = ptr, end = ptr + length;
       str != end &&
       ((my_isspace(&my_charset_bin, *str) || *str == '+' || *str == '0'));
       str++)
    *to++ = ' ';
  if (str == end) return length; /* purecov: inspected */

  if (*str == '-') {
    *to++ = 1;  // Smaller than any number
    str++;
    while (str != end)
      if (my_isdigit(&my_charset_bin, *str))
        *to++ = (char)('9' - *str++);
      else
        *to++ = *str++;
  } else
    memcpy(to, str, (uint)(end - str));
  return length;
}

void Field_decimal::sql_type(String &res) const {
  const CHARSET_INFO *cs = res.charset();
  uint tmp = field_length;
  if (!unsigned_flag) tmp--;
  if (dec) tmp--;
  res.length(cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                "decimal(%d,%d)", tmp, dec));
  append_zerofill_and_unsigned(this, &res);
}

/****************************************************************************
** Field_new_decimal
****************************************************************************/

Field_new_decimal::Field_new_decimal(uchar *ptr_arg, uint32 len_arg,
                                     uchar *null_ptr_arg, uchar null_bit_arg,
                                     uchar auto_flags_arg,
                                     const char *field_name_arg, uint8 dec_arg,
                                     bool zero_arg, bool unsigned_arg)
    : Field_num(ptr_arg, len_arg, null_ptr_arg, null_bit_arg, auto_flags_arg,
                field_name_arg, dec_arg, zero_arg, unsigned_arg) {
  precision =
      std::min(my_decimal_length_to_precision(len_arg, dec_arg, unsigned_arg),
               uint(DECIMAL_MAX_PRECISION));
  DBUG_ASSERT((precision <= DECIMAL_MAX_PRECISION) &&
              (dec <= DECIMAL_MAX_SCALE));
  bin_size = my_decimal_get_binary_size(precision, dec);
}

Field_new_decimal::Field_new_decimal(uint32 len_arg, bool is_nullable_arg,
                                     const char *name, uint8 dec_arg,
                                     bool unsigned_arg)
    : Field_num(nullptr, len_arg,
                is_nullable_arg ? &dummy_null_buffer : nullptr, 0, NONE, name,
                dec_arg, false, unsigned_arg) {
  precision =
      std::min(my_decimal_length_to_precision(len_arg, dec_arg, unsigned_arg),
               uint(DECIMAL_MAX_PRECISION));
  DBUG_ASSERT((precision <= DECIMAL_MAX_PRECISION) &&
              (dec <= DECIMAL_MAX_SCALE));
  bin_size = my_decimal_get_binary_size(precision, dec);
}

Field *Field_new_decimal::create_from_item(const Item *item) {
  uint8 dec = item->decimals;
  uint8 intg = item->decimal_precision() - dec;
  uint32 len = item->max_char_length();

  DBUG_ASSERT(item->result_type() == DECIMAL_RESULT);

  /*
    Trying to put too many digits overall in a DECIMAL(prec,dec)
    will always throw a warning. We must limit dec to
    DECIMAL_MAX_SCALE however to prevent an assert() later.
  */

  if (dec > 0) {
    signed int overflow;

    dec = min<int>(dec, DECIMAL_MAX_SCALE);

    /*
      If the value still overflows the field with the corrected dec,
      we'll throw out decimals rather than integers. This is still
      bad and of course throws a truncation warning.
      +1: for decimal point
      */

    const int required_length =
        my_decimal_precision_to_length(intg + dec, dec, item->unsigned_flag);

    overflow = required_length - len;

    if (overflow > 0)
      dec = max(0, dec - overflow);  // too long, discard fract
    else
      /* Corrected value fits. */
      len = required_length;
  }
  return new (*THR_MALLOC) Field_new_decimal(
      len, item->maybe_null, item->item_name.ptr(), dec, item->unsigned_flag);
}

type_conversion_status Field_new_decimal::reset() {
  store_value(&decimal_zero);
  return TYPE_OK;
}

/**
  Generate max/min decimal value in case of overflow.

  @param decimal_value     buffer for value
  @param sign              sign of value which caused overflow
*/

void Field_new_decimal::set_value_on_overflow(my_decimal *decimal_value,
                                              bool sign) const {
  DBUG_TRACE;
  max_my_decimal(decimal_value, precision, decimals());
  if (sign) {
    if (unsigned_flag)
      my_decimal_set_zero(decimal_value);
    else
      decimal_value->sign(true);
  }
}

/**
  Store decimal value in the binary buffer.

  Checks if decimal_value fits into field size.
  If it does, stores the decimal in the buffer using binary format.
  Otherwise sets maximal number that can be stored in the field.

  @param decimal_value   my_decimal

  @retval
    0 ok
  @retval
    1 error
*/
type_conversion_status Field_new_decimal::store_value(
    const my_decimal *decimal_value) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  DBUG_TRACE;
#ifndef DBUG_OFF
  {
    char dbug_buff[DECIMAL_MAX_STR_LENGTH + 2];
    DBUG_PRINT("enter",
               ("value: %s", dbug_decimal_as_string(dbug_buff, decimal_value)));
  }
#endif

  /* check that we do not try to write negative value in unsigned field */
  if (unsigned_flag && decimal_value->sign()) {
    DBUG_PRINT("info", ("unsigned overflow"));
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    error = TYPE_WARN_OUT_OF_RANGE;
    decimal_value = &decimal_zero;
  }
#ifndef DBUG_OFF
  {
    char dbug_buff[DECIMAL_MAX_STR_LENGTH + 2];
    DBUG_PRINT("info",
               ("saving with precision %d  scale: %d  value %s", (int)precision,
                (int)dec, dbug_decimal_as_string(dbug_buff, decimal_value)));
  }
#endif

  int err = my_decimal2binary(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW,
                              decimal_value, ptr, precision, dec);
  if (warn_if_overflow(err)) {
    my_decimal buff;
    DBUG_PRINT("info", ("overflow"));
    set_value_on_overflow(&buff, decimal_value->sign());
    my_decimal2binary(E_DEC_FATAL_ERROR, &buff, ptr, precision, dec);
  }
  DBUG_EXECUTE("info", print_decimal_buff(decimal_value, ptr, bin_size););
  return (err != E_DEC_OK) ? decimal_err_to_type_conv_status(err) : error;
}

type_conversion_status Field_new_decimal::store(
    const char *from, size_t length, const CHARSET_INFO *charset_arg) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  my_decimal decimal_value;
  DBUG_TRACE;

  int err =
      str2my_decimal(E_DEC_FATAL_ERROR & ~(E_DEC_OVERFLOW | E_DEC_BAD_NUM),
                     from, length, charset_arg, &decimal_value);

  if (err != 0 && !table->in_use->lex->is_ignore() &&
      table->in_use->install_strict_handler()) {
    ErrConvString errmsg(from, length, charset_arg);
    const Diagnostics_area *da = table->in_use->get_stmt_da();
    push_warning_printf(
        table->in_use, Sql_condition::SL_WARNING,
        ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
        ER_THD(table->in_use, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD), "decimal",
        errmsg.ptr(), field_name, da->current_row_for_condition());
    if (table->in_use->is_strict_sql_mode())
      return decimal_err_to_type_conv_status(err);
  }

  if (err != 0)
    set_decimal_warning(this, err, &decimal_value, from, length, charset_arg);

#ifndef DBUG_OFF
  char dbug_buff[DECIMAL_MAX_STR_LENGTH + 2];
  DBUG_PRINT("enter",
             ("value: %s", dbug_decimal_as_string(dbug_buff, &decimal_value)));
#endif

  type_conversion_status store_stat = store_value(&decimal_value);
  return err != 0 ? decimal_err_to_type_conv_status(err) : store_stat;
}

type_conversion_status store_internal_with_error_check(Field_new_decimal *field,
                                                       int err,
                                                       my_decimal *value) {
  type_conversion_status stat = TYPE_OK;
  if (err == E_DEC_OVERFLOW) {
    field->set_value_on_overflow(value, value->sign());
    stat = TYPE_WARN_OUT_OF_RANGE;
  } else if (err == E_DEC_TRUNCATED) {
    stat = TYPE_NOTE_TRUNCATED;
  }
  uint cond_count = field->table->in_use->get_stmt_da()->cond_count();
  type_conversion_status store_stat = field->store_value(value);
  if (store_stat != TYPE_OK)
    return store_stat;
  else if (err != 0 &&
           (field->table->in_use->get_stmt_da()->cond_count() == cond_count)) {
    /* Only issue a warning if store_value doesn't issue an warning */
    field->warn_if_overflow(err);
  }
  return stat;
}

/**
  @todo
  Fix following when double2my_decimal when double2decimal
  will return E_DEC_TRUNCATED always correctly
*/

type_conversion_status Field_new_decimal::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  DBUG_TRACE;
  my_decimal decimal_value;

  int conv_err = double2my_decimal(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW, nr,
                                   &decimal_value);
  return store_internal_with_error_check(this, conv_err, &decimal_value);
}

type_conversion_status Field_new_decimal::store(longlong nr,
                                                bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  DBUG_TRACE;
  my_decimal decimal_value;
  int conv_err = int2my_decimal(E_DEC_FATAL_ERROR & ~E_DEC_OVERFLOW, nr,
                                unsigned_val, &decimal_value);
  return store_internal_with_error_check(this, conv_err, &decimal_value);
}

type_conversion_status Field_new_decimal::store_decimal(
    const my_decimal *decimal_value) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  return store_value(decimal_value);
}

type_conversion_status Field_new_decimal::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  my_decimal decimal_value;
  return store_value(date2my_decimal(ltime, &decimal_value));
}

double Field_new_decimal::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  double dbl;
  my_decimal decimal_value;
  my_decimal2double(E_DEC_FATAL_ERROR, val_decimal(&decimal_value), &dbl);
  return dbl;
}

longlong Field_new_decimal::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  longlong i;
  my_decimal decimal_value;
  my_decimal2int(E_DEC_FATAL_ERROR, val_decimal(&decimal_value), unsigned_flag,
                 &i);
  return i;
}

my_decimal *Field_new_decimal::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_TRACE;
  binary2my_decimal(E_DEC_FATAL_ERROR, ptr, decimal_value, precision, dec,
                    m_keep_precision);
  DBUG_EXECUTE("info", print_decimal_buff(decimal_value, ptr, bin_size););
  return decimal_value;
}

String *Field_new_decimal::val_str(
    String *val_buffer, String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  my_decimal decimal_value;
  uint fixed_precision = zerofill ? precision : 0;
  my_decimal2string(E_DEC_FATAL_ERROR, val_decimal(&decimal_value),
                    fixed_precision, dec, val_buffer);
  val_buffer->set_charset(&my_charset_numeric);
  return val_buffer;
}

bool Field_new_decimal::get_date(MYSQL_TIME *ltime,
                                 my_time_flags_t fuzzydate) const {
  my_decimal buf, *decimal_value = val_decimal(&buf);
  if (!decimal_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
    return true;
  }
  return my_decimal_to_datetime_with_warn(decimal_value, ltime, fuzzydate);
}

bool Field_new_decimal::get_time(MYSQL_TIME *ltime) const {
  my_decimal buf, *decimal_value = val_decimal(&buf);
  if (!decimal_value) {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    return true;
  }
  return my_decimal_to_time_with_warn(decimal_value, ltime);
}

int Field_new_decimal::cmp(const uchar *a, const uchar *b) const {
  return memcmp(a, b, bin_size);
}

size_t Field_new_decimal::make_sort_key(uchar *buff, size_t length) const {
  memcpy(buff, ptr, min(length, static_cast<size_t>(bin_size)));
  return length;
}

void Field_new_decimal::sql_type(String &str) const {
  const CHARSET_INFO *cs = str.charset();
  str.length(cs->cset->snprintf(cs, str.ptr(), str.alloced_length(),
                                "decimal(%d,%d)", precision, (int)dec));
  append_zerofill_and_unsigned(this, &str);
}

/**
   Save the field metadata for new decimal fields.

   Saves the precision in the first byte and decimals() in the second
   byte of the field metadata array at index of *metadata_ptr and
   *(metadata_ptr + 1).

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_new_decimal::do_save_field_metadata(uchar *metadata_ptr) const {
  *metadata_ptr = precision;
  *(metadata_ptr + 1) = decimals();
  return 2;
}

/**
   Returns the number of bytes field uses in row-based replication
   row packed size.

   This method is used in row-based replication to determine the number
   of bytes that the field consumes in the row record format. This is
   used to skip fields in the master that do not exist on the slave.

   @param   field_metadata   Encoded size in field metadata

   @returns The size of the field based on the field metadata.
*/
uint Field_new_decimal::pack_length_from_metadata(uint field_metadata) const {
  uint const source_precision = (field_metadata >> 8U) & 0x00ff;
  uint const source_decimal = field_metadata & 0x00ff;
  uint const source_size =
      my_decimal_get_binary_size(source_precision, source_decimal);
  return (source_size);
}

/**
   Check to see if field size is compatible with destination.

   This method is used in row-based replication to verify that the slave's
   field size is less than or equal to the master's field size. The
   encoded field metadata (from the master or source) is decoded and compared
   to the size of this field (the slave or destination).

   @param   field_metadata   Encoded size in field metadata
   @param   order_var        Pointer to variable where the order
                             between the source field and this field
                             will be returned.

   @return @c true
*/
bool Field_new_decimal::compatible_field_size(uint field_metadata,
                                              Relay_log_info *, uint16,
                                              int *order_var) const {
  uint const source_precision = (field_metadata >> 8U) & 0x00ff;
  uint const source_decimal = field_metadata & 0x00ff;
  int order = compare(source_precision, precision);
  *order_var = order != 0 ? order : compare(source_decimal, dec);
  return true;
}

uint Field_new_decimal::is_equal(const Create_field *new_field) const {
  return (
      (new_field->sql_type == real_type()) &&
      ((new_field->flags & UNSIGNED_FLAG) == (uint)(flags & UNSIGNED_FLAG)) &&
      ((new_field->flags & AUTO_INCREMENT_FLAG) ==
       (uint)(flags & AUTO_INCREMENT_FLAG)) &&
      (new_field->max_display_width_in_bytes() == max_display_length()) &&
      (new_field->decimals == dec));
}

/**
   Unpack a decimal field from row data.

   This method is used to unpack a decimal or numeric field from a master
   whose size of the field is less than that of the slave.

   @param   to         Destination of the data
   @param   from       Source of the data
   @param   param_data Precision (upper) and decimal (lower) values
   @param   low_byte_first See Field::unpack.

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field_new_decimal::unpack(uchar *to, const uchar *from,
                                       uint param_data, bool low_byte_first) {
  if (param_data == 0)
    return Field::unpack(to, from, param_data, low_byte_first);

  uint from_precision = (param_data & 0xff00) >> 8U;
  uint from_decimal = param_data & 0x00ff;
  uint length = pack_length();
  uint from_pack_len = my_decimal_get_binary_size(from_precision, from_decimal);
  uint len = (param_data && (from_pack_len < length)) ? from_pack_len : length;
  if ((from_pack_len && (from_pack_len < length)) ||
      (from_precision < precision) || (from_decimal < decimals())) {
    /*
      If the master's data is smaller than the slave, we need to convert
      the binary to decimal then resize the decimal converting it back to
      a decimal and write that to the raw data buffer.
    */
    decimal_digit_t dec_buf[DECIMAL_MAX_PRECISION];
    decimal_t dec_val;
    dec_val.len = from_precision;
    dec_val.buf = dec_buf;
    /*
      Note: bin2decimal does not change the length of the field. So it is
      just the first step the resizing operation. The second step does the
      resizing using the precision and decimals from the slave.
    */
    bin2decimal(from, &dec_val, from_precision, from_decimal);
    decimal2bin(&dec_val, to, precision, decimals());
  } else
    memcpy(to, from, len);  // Sizes are the same, just copy the data.
  return from + len;
}

bool Field_new_decimal::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  my_decimal dec_value;
  return protocol->store_decimal(val_decimal(&dec_value),
                                 zerofill ? precision : 0, dec);
}

/****************************************************************************
** tiny int
****************************************************************************/

type_conversion_status Field_tiny::store(const char *from, size_t len,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  longlong rnd;

  const type_conversion_status error =
      get_int(cs, from, len, &rnd, 255, -128, 127);
  ptr[0] = unsigned_flag ? (char)(ulonglong)rnd : (char)rnd;
  return error;
}

type_conversion_status Field_tiny::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  nr = rint(nr);
  if (unsigned_flag) {
    if (nr < 0.0) {
      *ptr = 0;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > 255.0) {
      *ptr = (char)255;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      *ptr = static_cast<unsigned char>(nr);
  } else {
    if (nr < -128.0) {
      *ptr = (char)-128;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > 127.0) {
      *ptr = 127;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      *ptr = (char)(int)nr;
  }
  return error;
}

type_conversion_status Field_tiny::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;

  if (unsigned_flag) {
    if (nr < 0 && !unsigned_val) {
      *ptr = 0;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if ((ulonglong)nr > (ulonglong)255) {
      *ptr = (char)255;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      *ptr = (char)nr;
  } else {
    if (nr < 0 && unsigned_val) nr = 256;  // Generate overflow
    if (nr < -128) {
      *ptr = (char)-128;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > 127) {
      *ptr = 127;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      *ptr = (char)nr;
  }
  return error;
}

double Field_tiny::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int tmp = unsigned_flag ? (int)ptr[0] : (int)((signed char *)ptr)[0];
  return (double)tmp;
}

longlong Field_tiny::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int tmp = unsigned_flag ? (int)ptr[0] : (int)((signed char *)ptr)[0];
  return (longlong)tmp;
}

String *Field_tiny::val_str(String *val_buffer,
                            String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = &my_charset_numeric;
  uint length;
  uint mlength = max(field_length + 1, 5 * cs->mbmaxlen);
  val_buffer->alloc(mlength);
  char *to = val_buffer->ptr();

  if (unsigned_flag)
    length = (uint)cs->cset->long10_to_str(cs, to, mlength, 10, (long)*ptr);
  else
    length = (uint)cs->cset->long10_to_str(cs, to, mlength, -10,
                                           (long)*((signed char *)ptr));

  val_buffer->length(length);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(cs);
  return val_buffer;
}

bool Field_tiny::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  return protocol->store_tiny(Field_tiny::val_int(),
                              zerofill ? field_length : 0);
}

int Field_tiny::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  signed char a, b;
  a = (signed char)a_ptr[0];
  b = (signed char)b_ptr[0];
  if (unsigned_flag)
    return ((uchar)a < (uchar)b) ? -1 : ((uchar)a > (uchar)b) ? 1 : 0;
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_tiny::make_sort_key(uchar *to,
                                 size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 1);
  if (unsigned_flag)
    *to = *ptr;
  else
    to[0] = (char)(ptr[0] ^ (uchar)128); /* Revers signbit */
  return 1;
}

void Field_tiny::sql_type(String &res) const {
  if (field_length == 1 && !unsigned_flag && !zerofill) {
    // Print TINYINT(1) since connectors use this to indicate BOOLEAN
    res.length(0);
    res.append("tinyint(1)");
  } else {
    integer_sql_type(this, "tinyint", &res);
  }
}

/****************************************************************************
 Field type short int (2 byte)
****************************************************************************/

type_conversion_status Field_short::store(const char *from, size_t len,
                                          const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int store_tmp;
  longlong rnd;

  const type_conversion_status error =
      get_int(cs, from, len, &rnd, UINT_MAX16, INT_MIN16, INT_MAX16);
  store_tmp = unsigned_flag ? (int)(ulonglong)rnd : (int)rnd;
  if (table->s->db_low_byte_first)
    int2store(ptr, store_tmp);
  else
    shortstore(ptr, (short)store_tmp);
  return error;
}

type_conversion_status Field_short::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  int16 res;
  nr = rint(nr);
  if (unsigned_flag) {
    if (nr < 0) {
      res = 0;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (double)UINT_MAX16) {
      res = (int16)UINT_MAX16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int16)(uint16)nr;
  } else {
    if (nr < (double)INT_MIN16) {
      res = INT_MIN16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (double)INT_MAX16) {
      res = INT_MAX16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int16)(int)nr;
  }
  if (table->s->db_low_byte_first)
    int2store(ptr, res);
  else
    shortstore(ptr, res);
  return error;
}

type_conversion_status Field_short::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  int16 res;

  if (unsigned_flag) {
    if (nr < 0L && !unsigned_val) {
      res = 0;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if ((ulonglong)nr > (ulonglong)UINT_MAX16) {
      res = (int16)UINT_MAX16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int16)(uint16)nr;
  } else {
    if (nr < 0 && unsigned_val) nr = UINT_MAX16 + 1;  // Generate overflow

    if (nr < INT_MIN16) {
      res = INT_MIN16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (longlong)INT_MAX16) {
      res = INT_MAX16;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int16)nr;
  }
  if (table->s->db_low_byte_first)
    int2store(ptr, res);
  else
    shortstore(ptr, res);
  return error;
}

double Field_short::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  short j;
  if (table->s->db_low_byte_first)
    j = sint2korr(ptr);
  else
    j = shortget(ptr);
  return unsigned_flag ? (double)(unsigned short)j : (double)j;
}

longlong Field_short::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  short j;
  if (table->s->db_low_byte_first)
    j = sint2korr(ptr);
  else
    j = shortget(ptr);
  return unsigned_flag ? (longlong)(unsigned short)j : (longlong)j;
}

String *Field_short::val_str(String *val_buffer,
                             String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = &my_charset_numeric;
  uint length;
  uint mlength = max(field_length + 1, 7 * cs->mbmaxlen);
  val_buffer->alloc(mlength);
  char *to = val_buffer->ptr();
  short j;
  if (table->s->db_low_byte_first)
    j = sint2korr(ptr);
  else
    j = shortget(ptr);

  if (unsigned_flag)
    length =
        (uint)cs->cset->long10_to_str(cs, to, mlength, 10, (long)(uint16)j);
  else
    length = (uint)cs->cset->long10_to_str(cs, to, mlength, -10, (long)j);
  val_buffer->length(length);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(cs);
  return val_buffer;
}

bool Field_short::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  return protocol->store_short(Field_short::val_int(),
                               zerofill ? field_length : 0);
}

int Field_short::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  short a, b;
  if (table->s->db_low_byte_first) {
    a = sint2korr(a_ptr);
    b = sint2korr(b_ptr);
  } else {
    a = shortget(a_ptr);
    b = shortget(b_ptr);
  }

  if (unsigned_flag)
    return ((unsigned short)a < (unsigned short)b)
               ? -1
               : ((unsigned short)a > (unsigned short)b) ? 1 : 0;
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_short::make_sort_key(uchar *to,
                                  size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 2);
#ifdef WORDS_BIGENDIAN
  if (!table->s->db_low_byte_first) {
    if (unsigned_flag)
      to[0] = ptr[0];
    else
      to[0] = (char)(ptr[0] ^ 128); /* Revers signbit */
    to[1] = ptr[1];
  } else
#endif
  {
    if (unsigned_flag)
      to[0] = ptr[1];
    else
      to[0] = (char)(ptr[1] ^ 128); /* Revers signbit */
    to[1] = ptr[0];
  }
  return 2;
}

void Field_short::sql_type(String &res) const {
  integer_sql_type(this, "smallint", &res);
}

/****************************************************************************
  Field type medium int (3 byte)
****************************************************************************/

type_conversion_status Field_medium::store(const char *from, size_t len,
                                           const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int store_tmp;
  longlong rnd;

  const type_conversion_status error =
      get_int(cs, from, len, &rnd, UINT_MAX24, INT_MIN24, INT_MAX24);
  store_tmp = unsigned_flag ? (int)(ulonglong)rnd : (int)rnd;
  int3store(ptr, store_tmp);
  return error;
}

type_conversion_status Field_medium::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  nr = rint(nr);
  if (unsigned_flag) {
    if (nr < 0) {
      int3store(ptr, 0);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr >= (double)(long)(1L << 24)) {
      uint32 tmp = (uint32)(1L << 24) - 1L;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      int3store(ptr, (uint32)nr);
  } else {
    if (nr < (double)INT_MIN24) {
      long tmp = (long)INT_MIN24;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (double)INT_MAX24) {
      long tmp = (long)INT_MAX24;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      int3store(ptr, (long)nr);
  }
  return error;
}

type_conversion_status Field_medium::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;

  if (unsigned_flag) {
    if (nr < 0 && !unsigned_val) {
      int3store(ptr, 0);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if ((ulonglong)nr >= (ulonglong)(long)(1L << 24)) {
      long tmp = (long)(1L << 24) - 1L;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      int3store(ptr, (uint32)nr);
  } else {
    if (nr < 0 && unsigned_val)
      nr = (ulonglong)(long)(1L << 24);  // Generate overflow

    if (nr < (longlong)INT_MIN24) {
      long tmp = (long)INT_MIN24;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (longlong)INT_MAX24) {
      long tmp = (long)INT_MAX24;
      int3store(ptr, tmp);
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      int3store(ptr, (long)nr);
  }
  return error;
}

double Field_medium::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  long j = unsigned_flag ? (long)uint3korr(ptr) : sint3korr(ptr);
  return (double)j;
}

longlong Field_medium::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  long j = unsigned_flag ? (long)uint3korr(ptr) : sint3korr(ptr);
  return (longlong)j;
}

String *Field_medium::val_str(String *val_buffer,
                              String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = &my_charset_numeric;
  uint length;
  uint mlength = max(field_length + 1, 10 * cs->mbmaxlen);
  val_buffer->alloc(mlength);
  char *to = val_buffer->ptr();
  long j = unsigned_flag ? (long)uint3korr(ptr) : sint3korr(ptr);

  length = (uint)cs->cset->long10_to_str(cs, to, mlength, -10, j);
  val_buffer->length(length);
  if (zerofill) prepend_zeros(val_buffer); /* purecov: inspected */
  val_buffer->set_charset(cs);
  return val_buffer;
}

bool Field_medium::send_to_protocol(Protocol *protocol) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (is_null()) return protocol->store_null();
  return protocol->store_long(Field_medium::val_int(),
                              zerofill ? field_length : 0);
}

int Field_medium::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  long a, b;
  if (unsigned_flag) {
    a = uint3korr(a_ptr);
    b = uint3korr(b_ptr);
  } else {
    a = sint3korr(a_ptr);
    b = sint3korr(b_ptr);
  }
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_medium::make_sort_key(uchar *to,
                                   size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 3);
  if (unsigned_flag)
    to[0] = ptr[2];
  else
    to[0] = (uchar)(ptr[2] ^ 128); /* Revers signbit */
  to[1] = ptr[1];
  to[2] = ptr[0];
  return 3;
}

void Field_medium::sql_type(String &res) const {
  integer_sql_type(this, "mediumint", &res);
}

/****************************************************************************
** long int
****************************************************************************/

type_conversion_status Field_long::store(const char *from, size_t len,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  long store_tmp;
  longlong rnd;

  const type_conversion_status error =
      get_int(cs, from, len, &rnd, UINT_MAX32, INT_MIN32, INT_MAX32);
  store_tmp = unsigned_flag ? (long)(ulonglong)rnd : (long)rnd;
  if (table->s->db_low_byte_first)
    int4store(ptr, store_tmp);
  else
    longstore(ptr, store_tmp);
  return error;
}

type_conversion_status Field_long::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  int32 res;
  nr = rint(nr);
  if (unsigned_flag) {
    if (nr < 0) {
      res = 0;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (double)UINT_MAX32) {
      res = UINT_MAX32;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int32)(ulong)nr;
  } else {
    if (nr < (double)INT_MIN32) {
      res = (int32)INT_MIN32;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (double)INT_MAX32) {
      res = (int32)INT_MAX32;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int32)(longlong)nr;
  }
  if (error)
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);

  if (table->s->db_low_byte_first)
    int4store(ptr, res);
  else
    longstore(ptr, res);
  return error;
}

/**
  Store a longlong in the field

  @param nr            the value to store
  @param unsigned_val  whether or not 'nr' should be interpreted as
                       signed or unsigned. E.g., if 'nr' has all bits
                       set it is interpreted as -1 if unsigned_val is
                       false and ULLONG_MAX if unsigned_val is true.
*/
type_conversion_status Field_long::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  int32 res;

  if (unsigned_flag) {
    if (nr < 0 && !unsigned_val) {
      res = 0;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if ((ulonglong)nr >= (1LL << 32)) {
      res = (int32)(uint32)~0L;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int32)(uint32)nr;
  } else {
    if (nr < 0 && unsigned_val) {
      nr = ((longlong)INT_MAX32) + 1;  // Generate overflow
      error = TYPE_WARN_OUT_OF_RANGE;
    }
    if (nr < (longlong)INT_MIN32) {
      res = (int32)INT_MIN32;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr > (longlong)INT_MAX32) {
      res = (int32)INT_MAX32;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (int32)nr;
  }
  if (error)
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);

  if (table->s->db_low_byte_first)
    int4store(ptr, res);
  else
    longstore(ptr, res);
  return error;
}

double Field_long::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int32 j;
  if (table->s->db_low_byte_first)
    j = sint4korr(ptr);
  else
    j = longget(ptr);
  return unsigned_flag ? (double)(uint32)j : (double)j;
}

longlong Field_long::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int32 j;
  /* See the comment in Field_long::store(long long) */
  DBUG_ASSERT(table->in_use == current_thd);
  if (table->s->db_low_byte_first)
    j = sint4korr(ptr);
  else
    j = longget(ptr);
  return unsigned_flag ? (longlong)(uint32)j : (longlong)j;
}

String *Field_long::val_str(String *val_buffer,
                            String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = &my_charset_numeric;
  size_t length;
  uint mlength = max(field_length + 1, 12 * cs->mbmaxlen);
  val_buffer->alloc(mlength);
  char *to = val_buffer->ptr();
  int32 j;
  if (table->s->db_low_byte_first)
    j = sint4korr(ptr);
  else
    j = longget(ptr);

  if (unsigned_flag)
    length = cs->cset->long10_to_str(cs, to, mlength, 10, (long)(uint32)j);
  else
    length = cs->cset->long10_to_str(cs, to, mlength, -10, (long)j);
  val_buffer->length(length);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(cs);
  return val_buffer;
}

bool Field_long::send_to_protocol(Protocol *protocol) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (is_null()) return protocol->store_null();
  return protocol->store_long(Field_long::val_int(),
                              zerofill ? field_length : 0);
}

int Field_long::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  int32 a, b;
  if (table->s->db_low_byte_first) {
    a = sint4korr(a_ptr);
    b = sint4korr(b_ptr);
  } else {
    a = longget(a_ptr);
    b = longget(b_ptr);
  }
  if (unsigned_flag)
    return ((uint32)a < (uint32)b) ? -1 : ((uint32)a > (uint32)b) ? 1 : 0;
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_long::make_sort_key(uchar *to,
                                 size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 4);
#ifdef WORDS_BIGENDIAN
  if (!table->s->db_low_byte_first) {
    if (unsigned_flag)
      to[0] = ptr[0];
    else
      to[0] = (char)(ptr[0] ^ 128); /* Reverse sign bit */
    to[1] = ptr[1];
    to[2] = ptr[2];
    to[3] = ptr[3];
  } else
#endif
  {
    if (unsigned_flag)
      to[0] = ptr[3];
    else
      to[0] = (char)(ptr[3] ^ 128); /* Reverse sign bit */
    to[1] = ptr[2];
    to[2] = ptr[1];
    to[3] = ptr[0];
  }
  return 4;
}

void Field_long::sql_type(String &res) const {
  integer_sql_type(this, "int", &res);
}

/****************************************************************************
 Field type longlong int (8 bytes)
****************************************************************************/

type_conversion_status Field_longlong::store(const char *from, size_t len,
                                             const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int conv_err = 0;
  type_conversion_status error = TYPE_OK;
  const char *end;
  ulonglong tmp;

  tmp = cs->cset->strntoull10rnd(cs, from, len, unsigned_flag, &end, &conv_err);
  if (conv_err == MY_ERRNO_ERANGE) {
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    error = TYPE_WARN_OUT_OF_RANGE;
  } else if (table->in_use->check_for_truncated_fields &&
             check_int(cs, from, len, end, conv_err))
    error = TYPE_WARN_OUT_OF_RANGE;
  else
    error = TYPE_OK;

  if (table->s->db_low_byte_first)
    int8store(ptr, tmp);
  else
    longlongstore(ptr, tmp);
  return error;
}

type_conversion_status Field_longlong::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  longlong res;

  nr = rint(nr);
  if (unsigned_flag) {
    if (nr < 0) {
      res = 0;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr >= (double)ULLONG_MAX) {
      res = ~(longlong)0;
      error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (longlong)double2ulonglong(nr);
  } else {
    if (nr <= (double)LLONG_MIN) {
      res = LLONG_MIN;
      if (nr < (double)LLONG_MIN) error = TYPE_WARN_OUT_OF_RANGE;
    } else if (nr >= (double)(ulonglong)LLONG_MAX) {
      res = LLONG_MAX;
      if (nr > (double)LLONG_MAX) error = TYPE_WARN_OUT_OF_RANGE;
    } else
      res = (longlong)nr;
  }
  if (error)
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);

  if (table->s->db_low_byte_first)
    int8store(ptr, res);
  else
    longlongstore(ptr, res);
  return error;
}

type_conversion_status Field_longlong::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;

  if (nr < 0)  // Only possible error
  {
    /*
      if field is unsigned and value is signed (< 0) or
      if field is signed and value is unsigned we have an overflow
    */
    if (unsigned_flag != unsigned_val) {
      nr = unsigned_flag ? (ulonglong)0 : (ulonglong)LLONG_MAX;
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
      error = TYPE_WARN_OUT_OF_RANGE;
    }
  }

  if (table->s->db_low_byte_first)
    int8store(ptr, nr);
  else
    longlongstore(ptr, nr);
  return error;
}

double Field_longlong::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  longlong j;
  if (table->s->db_low_byte_first)
    j = sint8korr(ptr);
  else
    j = longlongget(ptr);
  if (unsigned_flag) {
    return ulonglong2double(static_cast<ulonglong>(j));
  }
  return static_cast<double>(j);
}

longlong Field_longlong::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (table->s->db_low_byte_first)
    return sint8korr(ptr);
  else
    return longlongget(ptr);
}

String *Field_longlong::val_str(String *val_buffer,
                                String *val_ptr MY_ATTRIBUTE((unused))) const {
  const CHARSET_INFO *cs = &my_charset_numeric;
  uint length;
  uint mlength = max(field_length + 1, 22 * cs->mbmaxlen);
  val_buffer->alloc(mlength);
  char *to = val_buffer->ptr();
  longlong j;
  if (table->s->db_low_byte_first)
    j = sint8korr(ptr);
  else
    j = longlongget(ptr);

  length = (uint)(cs->cset->longlong10_to_str)(cs, to, mlength,
                                               unsigned_flag ? 10 : -10, j);
  val_buffer->length(length);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(cs);
  return val_buffer;
}

bool Field_longlong::send_to_protocol(Protocol *protocol) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (is_null()) return protocol->store_null();
  return protocol->store_longlong(Field_longlong::val_int(), unsigned_flag,
                                  zerofill ? field_length : 0);
}

int Field_longlong::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  longlong a, b;
  if (table->s->db_low_byte_first) {
    a = sint8korr(a_ptr);
    b = sint8korr(b_ptr);
  } else {
    a = longlongget(a_ptr);
    b = longlongget(b_ptr);
  }
  if (unsigned_flag)
    return ((ulonglong)a < (ulonglong)b)
               ? -1
               : ((ulonglong)a > (ulonglong)b) ? 1 : 0;
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_longlong::make_sort_key(uchar *to, size_t length) const {
  DBUG_ASSERT(length == PACK_LENGTH);
#ifdef WORDS_BIGENDIAN
  if (table == NULL || !table->s->db_low_byte_first)
    copy_integer<true>(to, length, ptr, PACK_LENGTH, unsigned_flag);
  else
#endif
    copy_integer<false>(to, length, ptr, PACK_LENGTH, unsigned_flag);
  return PACK_LENGTH;
}

void Field_longlong::sql_type(String &res) const {
  integer_sql_type(this, "bigint", &res);
}

/*
  Floating-point numbers
 */

uchar *Field_real::pack(uchar *to, const uchar *from, uint max_length,
                        bool low_byte_first) const {
  DBUG_TRACE;
#ifdef WORDS_BIGENDIAN
  if (low_byte_first != table->s->db_low_byte_first) {
    unsigned len = std::min(pack_length(), max_length);
    for (unsigned i = 0; i < len; ++i) {
      to[i] = from[pack_length() - i - 1];
    }
    return to + pack_length();
  } else
#endif
    return Field::pack(to, from, max_length, low_byte_first);
}

const uchar *Field_real::unpack(uchar *to, const uchar *from, uint param_data,
                                bool low_byte_first) {
  DBUG_TRACE;
#ifdef WORDS_BIGENDIAN
  if (low_byte_first != table->s->db_low_byte_first) {
    const uchar *dptr = from + pack_length();
    while (dptr-- > from) *to++ = *dptr;
    return from + pack_length();
  } else
#endif
    return Field::unpack(to, from, param_data, low_byte_first);
}

type_conversion_status Field_real::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  double nr = TIME_to_double(*ltime);
  return store(ltime->neg ? -nr : nr);
}

/****************************************************************************
  single precision float
****************************************************************************/

type_conversion_status Field_float::store(const char *from, size_t len,
                                          const CHARSET_INFO *cs) {
  int conv_error;
  type_conversion_status err = TYPE_OK;
  const char *end;
  double nr = my_strntod(cs, from, len, &end, &conv_error);
  if (conv_error || (!len || ((uint)(end - from) != len &&
                              table->in_use->check_for_truncated_fields))) {
    set_warning(Sql_condition::SL_WARNING,
                (conv_error ? ER_WARN_DATA_OUT_OF_RANGE : WARN_DATA_TRUNCATED),
                1);
    err = conv_error ? TYPE_WARN_OUT_OF_RANGE : TYPE_WARN_TRUNCATED;
  }
  Field_float::store(nr);
  return err;
}

type_conversion_status Field_float::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  const type_conversion_status error =
      truncate(&nr, FLT_MAX) ? TYPE_WARN_OUT_OF_RANGE : TYPE_OK;

  float j = (float)nr;

  if (table->s->db_low_byte_first)
    float4store(ptr, j);
  else
    floatstore(ptr, j);
  return error;
}

type_conversion_status Field_float::store(longlong nr, bool unsigned_val) {
  return Field_float::store(unsigned_val ? ulonglong2double((ulonglong)nr)
                                         : (double)nr);
}

double Field_float::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (table->s->db_low_byte_first)
    return double{float4get(ptr)};
  else
    return double{floatget(ptr)};
}

longlong Field_float::val_int() const {
  float j;
  if (table->s->db_low_byte_first)
    j = float4get(ptr);
  else
    j = floatget(ptr);
  return (longlong)rint(j);
}

String *Field_float::val_str(String *val_buffer,
                             String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_ASSERT(!zerofill || field_length <= MAX_FIELD_CHARLENGTH);
  float nr;
  if (table && table->s->db_low_byte_first)
    nr = float4get(ptr);
  else
    nr = floatget(ptr);

  uint to_length = 70;
  if (val_buffer->alloc(to_length)) {
    my_error(ER_OUT_OF_RESOURCES, MYF(0));
    return val_buffer;
  }

  char *to = val_buffer->ptr();
  size_t len;

  if (dec >= DECIMAL_NOT_SPECIFIED)
    len = my_gcvt(nr, MY_GCVT_ARG_FLOAT, to_length - 1, to, nullptr);
  else {
    /*
      We are safe here because the buffer length is 70, and
      fabs(float) < 10^39, dec < DECIMAL_NOT_SPECIFIED. So the resulting string
      will be not longer than 69 chars + terminating '\0'.
    */
    len = my_fcvt(nr, dec, to, nullptr);
  }
  val_buffer->length((uint)len);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(&my_charset_numeric);
  return val_buffer;
}

int Field_float::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  float a, b;
  if (table->s->db_low_byte_first) {
    a = float4get(a_ptr);
    b = float4get(b_ptr);
  } else {
    a = floatget(a_ptr);
    b = floatget(b_ptr);
  }
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_float::make_sort_key(uchar *to,
                                  size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == sizeof(float));
  float nr;
  if (table->s->db_low_byte_first)
    nr = float4get(ptr);
  else
    nr = floatget(ptr);

  /*
    -0.0 and +0.0 compare identically, so make sure they use exactly the same
    bit pattern.
  */
  if (nr == 0.0f) nr = 0.0f;

  /*
    Positive floats sort exactly as ints; negative floats need
    bit flipping. The bit flipping sets the upper bit to 0
    unconditionally, so put 1 in there for positive numbers
    (so they sort later for our unsigned comparison).
    NOTE: This does not sort infinities or NaN correctly.
  */
  int32 nr_int;
  memcpy(&nr_int, &nr, sizeof(nr));
  nr_int = (nr_int ^ (nr_int >> 31)) | ((~nr_int) & 0x80000000);
  store32be(to, nr_int);

  return sizeof(float);
}

bool Field_float::send_to_protocol(Protocol *protocol) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (is_null()) return protocol->store_null();
  return protocol->store_float(static_cast<float>(Field_float::val_real()), dec,
                               zerofill ? field_length : 0);
}

/**
   Save the field metadata for float fields.

   Saves the pack length in the first byte.

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_float::do_save_field_metadata(uchar *metadata_ptr) const {
  *metadata_ptr = pack_length();
  return 1;
}

void Field_float::sql_type(String &res) const {
  if (dec == DECIMAL_NOT_SPECIFIED) {
    res.set_ascii(STRING_WITH_LEN("float"));
  } else {
    const CHARSET_INFO *cs = res.charset();
    res.length(cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                  "float(%d,%d)", (int)field_length, dec));
  }
  append_zerofill_and_unsigned(this, &res);
}

/****************************************************************************
  double precision floating point numbers
****************************************************************************/

type_conversion_status Field_double::store(const char *from, size_t len,
                                           const CHARSET_INFO *cs) {
  int conv_error;
  type_conversion_status error = TYPE_OK;
  const char *end;
  double nr = my_strntod(cs, from, len, &end, &conv_error);
  if (conv_error != 0 || len == 0 ||
      (((uint)(end - from) != len &&
        table->in_use->check_for_truncated_fields))) {
    set_warning(Sql_condition::SL_WARNING,
                (conv_error ? ER_WARN_DATA_OUT_OF_RANGE : WARN_DATA_TRUNCATED),
                1);
    error = conv_error ? TYPE_WARN_OUT_OF_RANGE : TYPE_WARN_TRUNCATED;
  }
  Field_double::store(nr);
  return error;
}

type_conversion_status Field_double::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  const type_conversion_status error =
      truncate(&nr, DBL_MAX) ? TYPE_WARN_OUT_OF_RANGE : TYPE_OK;

  if (table->s->db_low_byte_first)
    float8store(ptr, nr);
  else
    doublestore(ptr, nr);
  return error;
}

type_conversion_status Field_double::store(longlong nr, bool unsigned_val) {
  return Field_double::store(unsigned_val ? ulonglong2double((ulonglong)nr)
                                          : (double)nr);
}

/**
  If a field has fixed length, truncate the double argument pointed to by 'nr'
  appropriately.
  Also ensure that the argument is within [min_value; max_value] where
  min_value == 0 if unsigned_flag is set, else -max_value.

  @param[in,out] nr         the real number (FLOAT or DOUBLE) to be truncated
  @param[in]     max_value  the maximum (absolute) value of the real type

  @returns truncation result
*/

Field_real::Truncate_result Field_real::truncate(double *nr, double max_value) {
  if (std::isnan(*nr)) {
    *nr = 0;
    set_null();
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TR_POSITIVE_OVERFLOW;
  } else if (unsigned_flag && *nr < 0) {
    *nr = 0;
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TR_NEGATIVE_OVERFLOW;
  }

  if (!not_fixed) {
    double orig_max_value = max_value;
    uint order = field_length - dec;
    uint step = array_elements(log_10) - 1;
    max_value = 1.0;
    for (; order > step; order -= step) max_value *= log_10[step];
    max_value *= log_10[order];
    max_value -= 1.0 / log_10[dec];
    max_value = std::min(max_value, orig_max_value);

    /* Check for infinity so we don't get NaN in calculations */
    if (!std::isinf(*nr)) {
      double tmp = rint((*nr - floor(*nr)) * log_10[dec]) / log_10[dec];
      *nr = floor(*nr) + tmp;
    }
  }

  if (*nr < -max_value) {
    *nr = -max_value;
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TR_NEGATIVE_OVERFLOW;
  } else if (*nr > max_value) {
    *nr = max_value;
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TR_POSITIVE_OVERFLOW;
  }

  return TR_OK;
}

type_conversion_status Field_real::store_decimal(const my_decimal *dm) {
  double dbl;
  my_decimal2double(E_DEC_FATAL_ERROR, dm, &dbl);
  return store(dbl);
}

double Field_double::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (table->s->db_low_byte_first)
    return float8get(ptr);
  else
    return doubleget(ptr);
}

longlong Field_double::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  double j;
  longlong res;
  if (table->s->db_low_byte_first)
    j = float8get(ptr);
  else
    j = doubleget(ptr);
  /* Check whether we fit into longlong range */
  if (j <= (double)LLONG_MIN) {
    res = (longlong)LLONG_MIN;
    goto warn;
  }
  if (j >= (double)(ulonglong)LLONG_MAX) {
    res = (longlong)LLONG_MAX;
    goto warn;
  }
  return (longlong)rint(j);

warn : {
  char buf[DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE];
  String tmp(buf, sizeof(buf), &my_charset_latin1), *str;
  str = val_str(&tmp, nullptr);
  ErrConvString err(str);
  push_warning_printf(
      current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
      ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "INTEGER", err.ptr());
}
  return res;
}

my_decimal *Field_real::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  double2my_decimal(E_DEC_FATAL_ERROR, val_real(), decimal_value);
  return decimal_value;
}

bool Field_real::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) const {
  return my_double_to_datetime_with_warn(val_real(), ltime, fuzzydate);
}

bool Field_real::get_time(MYSQL_TIME *ltime) const {
  return my_double_to_time_with_warn(val_real(), ltime);
}

String *Field_double::val_str(String *val_buffer,
                              String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_ASSERT(!zerofill || field_length <= MAX_FIELD_CHARLENGTH);
  double nr;
  if (table && table->s->db_low_byte_first)
    nr = float8get(ptr);
  else
    nr = doubleget(ptr);
  uint to_length = DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE;
  if (val_buffer->alloc(to_length)) {
    my_error(ER_OUT_OF_RESOURCES, MYF(0));
    return val_buffer;
  }

  char *to = val_buffer->ptr();
  size_t len;

  if (dec >= DECIMAL_NOT_SPECIFIED)
    len = my_gcvt(nr, MY_GCVT_ARG_DOUBLE, to_length - 1, to, nullptr);
  else
    len = my_fcvt(nr, dec, to, nullptr);

  val_buffer->length((uint)len);
  if (zerofill) prepend_zeros(val_buffer);
  val_buffer->set_charset(&my_charset_numeric);
  return val_buffer;
}

bool Field_double::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  return protocol->store_double(Field_double::val_real(), dec,
                                zerofill ? field_length : 0);
}

int Field_double::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  double a, b;
  if (table->s->db_low_byte_first) {
    a = float8get(a_ptr);
    b = float8get(b_ptr);
  } else {
    a = doubleget(a_ptr);
    b = doubleget(b_ptr);
  }
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/* The following should work for IEEE */

size_t Field_double::make_sort_key(uchar *to, size_t length) const {
  DBUG_ASSERT(length == sizeof(double));
  double nr;
  if (table->s->db_low_byte_first)
    nr = float8get(ptr);
  else
    nr = doubleget(ptr);
  if (length < 8) {
    uchar buff[8];
    change_double_for_sort(nr, buff);
    memcpy(to, buff, length);
  } else
    change_double_for_sort(nr, to);
  return sizeof(double);
}

/**
   Save the field metadata for double fields.

   Saves the pack length in the first byte of the field metadata array
   at index of *metadata_ptr.

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_double::do_save_field_metadata(uchar *metadata_ptr) const {
  *metadata_ptr = pack_length();
  return 1;
}

void Field_double::sql_type(String &res) const {
  const CHARSET_INFO *cs = res.charset();
  if (dec == DECIMAL_NOT_SPECIFIED) {
    res.set_ascii(STRING_WITH_LEN("double"));
  } else {
    res.length(cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                  "double(%d,%d)", (int)field_length, dec));
  }
  append_zerofill_and_unsigned(this, &res);
}

/****************************************************************************
** Common code for all temporal data types: DATE, DATETIME, TIMESTAMP, TIME
*****************************************************************************/

my_time_flags_t Field_temporal::date_flags() const {
  return date_flags(table ? table->in_use : current_thd);
}

uint Field_temporal::is_equal(const Create_field *new_field) const {
  return new_field->sql_type == real_type() &&
         new_field->decimals == decimals();
}

my_decimal *Field_temporal::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_ASSERT(decimals() == 0);
  int2my_decimal(E_DEC_FATAL_ERROR, val_int(), false, decimal_value);
  return decimal_value;
}

/**
  Set warnings from a warning vector.
  Note, multiple warnings can be set at the same time.

  @param str       Value.
  @param warnings  Warning vector.

  @retval false  Function reported warning
  @retval true   Function reported error

  @note STRICT mode can convert warnings to error.
*/
bool Field_temporal::set_warnings(const ErrConvString &str, int warnings) {
  bool truncate_incremented = false;
  enum_mysql_timestamp_type ts_type = field_type_to_timestamp_type(type());

  if (warnings & MYSQL_TIME_WARN_TRUNCATED) {
    if (set_datetime_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED,
                             str, ts_type, !truncate_incremented))
      return true;
    truncate_incremented = true;
  }
  if (warnings & (MYSQL_TIME_WARN_OUT_OF_RANGE | MYSQL_TIME_WARN_ZERO_DATE |
                  MYSQL_TIME_WARN_ZERO_IN_DATE)) {
    if (set_datetime_warning(Sql_condition::SL_WARNING,
                             ER_WARN_DATA_OUT_OF_RANGE, str, ts_type,
                             !truncate_incremented))
      return true;
    truncate_incremented = true;
  }
  if (warnings & MYSQL_TIME_WARN_INVALID_TIMESTAMP) {
    if (set_datetime_warning(Sql_condition::SL_WARNING,
                             ER_WARN_INVALID_TIMESTAMP, str, ts_type,
                             !truncate_incremented))
      return true;
    truncate_incremented = true;
  }
  if ((warnings & MYSQL_TIME_NOTE_TRUNCATED) &&
      !(warnings & MYSQL_TIME_WARN_TRUNCATED)) {
    if (set_datetime_warning(Sql_condition::SL_NOTE, WARN_DATA_TRUNCATED, str,
                             ts_type, !truncate_incremented))
      return true;
  }
  return false;
}

type_conversion_status Field_temporal::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int warnings = 0;
  MYSQL_TIME ltime;
  type_conversion_status error =
      convert_number_to_TIME(nr, unsigned_val, 0, &ltime, &warnings);
  if (error == TYPE_OK || error == TYPE_NOTE_TRUNCATED)
    error = store_internal(&ltime, &warnings);
  else {
    DBUG_ASSERT(warnings != 0);  // Must be set by convert_number_to_TIME

    if (warnings & (MYSQL_TIME_WARN_ZERO_DATE | MYSQL_TIME_WARN_ZERO_IN_DATE) &&
        !current_thd->is_strict_sql_mode())
      error = TYPE_NOTE_TIME_TRUNCATED;
  }

  if (warnings && set_warnings(ErrConvString(nr, unsigned_val), warnings))
    return TYPE_ERR_BAD_VALUE;

  return error;
}

type_conversion_status Field_temporal::store_lldiv_t(const lldiv_t *lld,
                                                     int *warnings) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error;
  MYSQL_TIME ltime;
  error = convert_number_to_TIME(lld->quot, false, static_cast<int>(lld->rem),
                                 &ltime, warnings);
  if (error == TYPE_OK || error == TYPE_NOTE_TRUNCATED)
    error = store_internal_adjust_frac(&ltime, warnings);
  else if (!*warnings) {
    DBUG_ASSERT(warnings != nullptr);  // Must be set by convert_number_to_TIME
    if (((*warnings & MYSQL_TIME_WARN_ZERO_DATE) != 0 ||
         (*warnings & MYSQL_TIME_WARN_ZERO_IN_DATE) != 0) &&
        !current_thd->is_strict_sql_mode())
      error = TYPE_NOTE_TIME_TRUNCATED;
  }

  return error;
}

type_conversion_status Field_temporal::store_decimal(
    const my_decimal *decimal) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  lldiv_t lld;
  int warnings = 0;
  /* Pass 0 in the first argument, not to produce warnings automatically */
  my_decimal2lldiv_t(0, decimal, &lld);
  const type_conversion_status error = store_lldiv_t(&lld, &warnings);
  if (warnings && set_warnings(ErrConvString(decimal), warnings))
    return TYPE_ERR_BAD_VALUE;

  return error;
}

type_conversion_status Field_temporal::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int warnings = 0;
  lldiv_t lld;
  double2lldiv_t(nr, &lld);
  const type_conversion_status error = store_lldiv_t(&lld, &warnings);
  if (warnings && set_warnings(ErrConvString(nr), warnings))
    return TYPE_ERR_BAD_VALUE;

  return error;
}

/**
  Store string into a date/time/datetime field.

  @param str      Date/time string
  @param  len     Length of the string
  @param  cs      Character set of the string

  @retval TYPE_OK   Storage of value went fine without warnings or errors
  @retval !TYPE_OK  Warning/error as indicated by type_conversion_status enum
                    value
*/
type_conversion_status Field_temporal::store(const char *str, size_t len,
                                             const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  MYSQL_TIME ltime;
  MYSQL_TIME_STATUS status;
  if (convert_str_to_TIME(str, len, cs, &ltime, &status)) {
    /*
      When convert_str_to_TIME() returns error, ltime has been set to
      0 so there's nothing to store in the field.
    */
    reset();
    if (status.warnings &
            (MYSQL_TIME_WARN_ZERO_DATE | MYSQL_TIME_WARN_ZERO_IN_DATE) &&
        !current_thd->is_strict_sql_mode())
      error = TYPE_NOTE_TIME_TRUNCATED;
    else
      error = TYPE_ERR_BAD_VALUE;
  } else {
    error = time_warning_to_type_conversion_status(status.warnings);
    const type_conversion_status tmp_error =
        store_internal_adjust_frac(&ltime, &status.warnings);

    // Return the most serious error of the two, see type_conversion_status
    if (tmp_error > error) error = tmp_error;
  }
  if (status.warnings &&
      set_warnings(ErrConvString(str, len, cs), status.warnings))
    return TYPE_ERR_BAD_VALUE;

  return error;
}

/**

  @param nr The datetime value specified as "number", see number_to_datetime()
  for details on this format.

  @param [out] ltime A MYSQL_TIME struct where the result is stored.
  @param warnings Truncation warning code, see was_cut in number_to_datetime().

  @retval -1    Timestamp with wrong values.
  @retval other DATETIME as integer in YYYYMMDDHHMMSS format.
*/
longlong Field_temporal::convert_number_to_datetime(longlong nr, bool,
                                                    MYSQL_TIME *ltime,
                                                    int *warnings) {
  /*
    Note, number_to_datetime can return a result different from nr:
    e.g. 111111 -> 20111111000000
  */
  longlong tmp = number_to_datetime(nr, ltime, date_flags(), warnings);
  if (tmp == -1LL) reset();
  return tmp;
}

/****************************************************************************
** Common code for temporal data types with date: DATE, DATETIME, TIMESTAMP
*****************************************************************************/

bool Field_temporal_with_date::get_internal_check_zero(
    MYSQL_TIME *ltime, my_time_flags_t fuzzydate) const {
  if (get_date_internal(ltime)) /* '0000-00-00' */
  {
    DBUG_ASSERT(type() == MYSQL_TYPE_TIMESTAMP);
    if (fuzzydate & TIME_NO_ZERO_DATE) return true;
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME);
  }
  return false;
}

longlong Field_temporal_with_date::val_date_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime) ? 0
                                   : TIME_to_longlong_datetime_packed(ltime);
}

longlong Field_temporal_with_date::val_time_temporal() const {
  /*
    There are currently no tests covering this method,
    as DATETIME seems to always superseed over TIME in comparison.
  */
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime) ? 0 : TIME_to_longlong_time_packed(ltime);
}

/**
  Convert a number in format YYMMDDhhmmss to string.
  Straight coded to avoid problem with slow longlong arithmetic and sprintf.

  @param[out] pos      pointer to convert to.
  @param      tmp      number with datetime value.
*/
static inline int my_datetime_number_to_str(char *pos, longlong tmp) {
  long part1 = (long)(tmp / 1000000LL);
  long part2 = (long)(tmp - (ulonglong)part1 * 1000000LL);
  int part3;
  pos += MAX_DATETIME_WIDTH; /* Start from the end */
  *pos-- = 0;
  *pos-- = (char)('0' + (char)(part2 % 10)); /* Seconds */
  part2 /= 10;
  *pos-- = (char)('0' + (char)(part2 % 10));
  part3 = (int)(part2 / 10);
  *pos-- = ':';
  *pos-- = (char)('0' + (char)(part3 % 10)); /* Minutes */
  part3 /= 10;
  *pos-- = (char)('0' + (char)(part3 % 10));
  part3 /= 10;
  *pos-- = ':';
  *pos-- = (char)('0' + (char)(part3 % 10)); /* Hours */
  part3 /= 10;
  *pos-- = (char)('0' + (char)part3);
  *pos-- = ' ';
  *pos-- = (char)('0' + (char)(part1 % 10)); /* Day */
  part1 /= 10;
  *pos-- = (char)('0' + (char)(part1 % 10));
  part1 /= 10;
  *pos-- = '-';
  *pos-- = (char)('0' + (char)(part1 % 10)); /* Month */
  part1 /= 10;
  *pos-- = (char)('0' + (char)(part1 % 10));
  part3 = (int)(part1 / 10);
  *pos-- = '-';
  *pos-- = (char)('0' + (char)(part3 % 10)); /* Year */
  part3 /= 10;
  *pos-- = (char)('0' + (char)(part3 % 10));
  part3 /= 10;
  *pos-- = (char)('0' + (char)(part3 % 10));
  part3 /= 10;
  *pos = (char)('0' + (char)part3);
  return MAX_DATETIME_WIDTH;
}

String *Field_temporal_with_date::val_str(String *val_buffer, String *) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  val_buffer->alloc(field_length + 1);
  val_buffer->set_charset(&my_charset_numeric);
  if (get_date_internal(&ltime)) {
    val_buffer->set_ascii(my_zero_datetime6, field_length);
    return val_buffer;
  }
  make_datetime((Date_time_format *)nullptr, &ltime, val_buffer, dec);
  return val_buffer;
}

type_conversion_status Field_temporal_with_date::convert_number_to_TIME(
    longlong nr, bool unsigned_val, int nanoseconds, MYSQL_TIME *ltime,
    int *warnings) {
  if (nr < 0 || nanoseconds < 0) {
    reset();
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return TYPE_WARN_OUT_OF_RANGE;
  }

  if (convert_number_to_datetime(nr, unsigned_val, ltime, warnings) == -1LL)
    return TYPE_ERR_BAD_VALUE;

  if (ltime->time_type == MYSQL_TIMESTAMP_DATE && nanoseconds) {
    *warnings |= MYSQL_TIME_WARN_TRUNCATED;
    return TYPE_NOTE_TRUNCATED;
  }

  ltime->second_part = 0;
  if (propagate_datetime_overflow(current_thd, warnings,
                                  datetime_add_nanoseconds_adjust_frac(
                                      ltime, nanoseconds, warnings,
                                      (date_flags() & TIME_FRAC_TRUNCATE)))) {
    reset();
    return TYPE_WARN_OUT_OF_RANGE;
  }
  return TYPE_OK;
}

type_conversion_status Field_temporal_with_date::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error;
  int warnings = 0;

  switch (ltime->time_type)  // TS-TODO: split into separate methods?
  {
    case MYSQL_TIMESTAMP_DATETIME:
    case MYSQL_TIMESTAMP_DATETIME_TZ:
    case MYSQL_TIMESTAMP_DATE:
      if (check_date(*ltime, non_zero_date(*ltime), date_flags(), &warnings)) {
        DBUG_ASSERT(warnings &
                    (MYSQL_TIME_WARN_OUT_OF_RANGE | MYSQL_TIME_WARN_ZERO_DATE |
                     MYSQL_TIME_WARN_ZERO_IN_DATE));

        error = time_warning_to_type_conversion_status(warnings);
        reset();
      } else {
        error = store_internal_adjust_frac(ltime, &warnings);
      }
      break;
    case MYSQL_TIMESTAMP_TIME: {
      /* Convert TIME to DATETIME */
      THD *thd = table ? table->in_use : current_thd;
      MYSQL_TIME ltime2;
      time_to_datetime(thd, ltime, &ltime2);
      error = store_internal_adjust_frac(&ltime2, &warnings);
      break;
    }
    case MYSQL_TIMESTAMP_NONE:
    case MYSQL_TIMESTAMP_ERROR:
    default:
      warnings |= MYSQL_TIME_WARN_TRUNCATED;
      reset();
      error = TYPE_WARN_TRUNCATED;
  }

  if (warnings && set_warnings(ErrConvString(ltime, decimals()), warnings))
    return TYPE_ERR_BAD_VALUE;

  return error;
}

bool Field_temporal_with_date::convert_str_to_TIME(const char *str, size_t len,
                                                   const CHARSET_INFO *cs,
                                                   MYSQL_TIME *ltime,
                                                   MYSQL_TIME_STATUS *status) {
  return propagate_datetime_overflow(
      current_thd, &status->warnings,
      str_to_datetime(cs, str, len, ltime, date_flags(), status));
}

bool Field_temporal_with_date::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  MYSQL_TIME ltime;
  if (get_date_internal(&ltime)) {
    // Only MYSQL_TYPE_TIMESTAMP can return an error in get_date_internal()
    DBUG_ASSERT(type() == MYSQL_TYPE_TIMESTAMP);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_DATETIME);
  }
  return protocol->store_datetime(ltime, dec);
}

type_conversion_status Field_temporal_with_date::store_internal_adjust_frac(
    MYSQL_TIME *ltime, int *warnings) {
  if (propagate_datetime_overflow(
          current_thd, warnings,
          my_datetime_adjust_frac(ltime, dec, warnings,
                                  (date_flags() & TIME_FRAC_TRUNCATE)))) {
    reset();
    return time_warning_to_type_conversion_status(*warnings);
  } else
    return store_internal(ltime, warnings);
}

/**
  Validate date value stored in the field.

  Now we check whether date value is zero or has zero in date or not and sets
  warning/error message appropriately(depending on the sql_mode).
*/
type_conversion_status Field_temporal_with_date::validate_stored_val(THD *) {
  MYSQL_TIME ltime;
  type_conversion_status error = TYPE_OK;
  int warnings = 0;

  if (is_real_null()) return error;

  memset(&ltime, 0, sizeof(MYSQL_TIME));
  get_date_internal(&ltime);
  if (check_date(ltime, non_zero_date(ltime), date_flags(), &warnings))
    error = time_warning_to_type_conversion_status(warnings);

  if (warnings) {
    ltime.time_type = field_type_to_timestamp_type(type());
    if (set_warnings(ErrConvString(&ltime, dec), warnings))
      return TYPE_ERR_BAD_VALUE;
  }

  return error;
}

/****************************************************************************
** Common code for data types with date and time: DATETIME, TIMESTAMP
*****************************************************************************/

void Field_temporal_with_date_and_time::store_timestamp(
    const struct timeval *tm) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  if (!my_time_fraction_remainder(tm->tv_usec, decimals())) {
    store_timestamp_internal(tm);
    return;
  }
  struct timeval tm2 = *tm;
  my_timeval_round(&tm2, decimals());
  store_timestamp_internal(&tm2);
}

bool Field_temporal_with_date_and_time::convert_TIME_to_timestamp(
    THD *thd, const MYSQL_TIME *ltime, struct timeval *tm, int *warnings) {
  /*
    No need to do check_date(TIME_NO_ZERO_IN_DATE),
    because it has been done earlier in
    store_time(), number_to_datetime() or str_to_datetime().
  */
  if (datetime_with_no_zero_in_date_to_timeval(thd, ltime, tm, warnings)) {
    tm->tv_sec = tm->tv_usec = 0;
    return true;
  }
  return false;
}

void Field_temporal_with_date_and_time::init_timestamp_flags() {
  if (auto_flags != NONE && (!(auto_flags & GENERATED_FROM_EXPRESSION))) {
    /*
      This TIMESTAMP column is hereby quietly assumed to have an insert or
      update default function.
    */
    flags |= TIMESTAMP_FLAG;
    if (auto_flags & ON_UPDATE_NOW) flags |= ON_UPDATE_NOW_FLAG;
  }
}

/****************************************************************************
** Common code for DATETIME(N) and TIMESTAMP(N)
*****************************************************************************/

double Field_temporal_with_date_and_timef::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime) ? 0 : TIME_to_double_datetime(ltime);
}

longlong Field_temporal_with_date_and_timef::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime)
             ? 0
             : propagate_datetime_overflow(current_thd, [&](int *w) {
                 return TIME_to_ulonglong_datetime_round(ltime, w);
               });
}

my_decimal *Field_temporal_with_date_and_timef::val_decimal(
    my_decimal *dec_arg) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  if (get_date_internal(&ltime)) {
    // Only MYSQL_TYPE_TIMESTAMP can return an error in get_date_internal()
    DBUG_ASSERT(type() == MYSQL_TYPE_TIMESTAMP);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_DATETIME);
  }
  return date2my_decimal(&ltime, dec_arg);
}

/**
  TIMESTAMP type columns hold date and time values in the range 1970-01-01
  00:00:01 UTC to 2038-01-01 00:00:00 UTC, stored as number of seconds since
  the start of the Unix Epoch (1970-01-01 00:00:01 UTC.)

  TIMESTAMP columns can be automatically set on row updates to and/or have
  CURRENT_TIMESTAMP as default value for inserts.
  We use flags Field::auto_flags member to control this behavior.
*/
Field_timestamp::Field_timestamp(uchar *ptr_arg, uint32, uchar *null_ptr_arg,
                                 uchar null_bit_arg, uchar auto_flags_arg,
                                 const char *field_name_arg)
    : Field_temporal_with_date_and_time(ptr_arg, null_ptr_arg, null_bit_arg,
                                        auto_flags_arg, field_name_arg, 0) {
  init_timestamp_flags();
  /* For 4.0 MYD and 4.0 InnoDB compatibility */
  flags |= ZEROFILL_FLAG | UNSIGNED_FLAG;
}

Field_timestamp::Field_timestamp(bool is_nullable_arg,
                                 const char *field_name_arg)
    : Field_temporal_with_date_and_time(
          nullptr, is_nullable_arg ? &dummy_null_buffer : nullptr, 0, NONE,
          field_name_arg, 0) {
  init_timestamp_flags();
  /* For 4.0 MYD and 4.0 InnoDB compatibility */
  flags |= ZEROFILL_FLAG | UNSIGNED_FLAG;
}

my_time_flags_t Field_timestamp::date_flags(const THD *thd) const {
  /* We don't want to store invalid or fuzzy datetime values in TIMESTAMP */
  my_time_flags_t date_flags = TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
    date_flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags |= TIME_FRAC_TRUNCATE;

  return date_flags;
}

type_conversion_status Field_timestamp::store_internal(const MYSQL_TIME *ltime,
                                                       int *warnings) {
  THD *thd = table ? table->in_use : current_thd;
  struct timeval tm;
  convert_TIME_to_timestamp(thd, ltime, &tm, warnings);
  const type_conversion_status error =
      time_warning_to_type_conversion_status(*warnings);
  store_timestamp_internal(&tm);
  return error;
}

/**
  Get a value from record, without checking fuzzy date flags.
  @retval true  - if timestamp is 0, ltime is not touched in this case.
  @retval false - if timestamp is non-zero.
*/
bool Field_timestamp::get_date_internal(MYSQL_TIME *ltime) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  uint32 temp;
  THD *thd = table ? table->in_use : current_thd;
  if (table && table->s->db_low_byte_first)
    temp = uint4korr(ptr);
  else
    temp = ulongget(ptr);
  if (!temp) return true;
  thd->time_zone()->gmt_sec_to_TIME(ltime, (my_time_t)temp);
  return false;
}

/**
   Get TIMESTAMP field value as seconds since begging of Unix Epoch
*/
bool Field_timestamp::get_timestamp(struct timeval *tm, int *) const {
  if (is_null()) return true;
  tm->tv_usec = 0;
  if (table && table->s->db_low_byte_first) {
    tm->tv_sec = sint4korr(ptr);
    return false;
  }
  tm->tv_sec = longget(ptr);
  return false;
}

void Field_timestamp::store_timestamp_internal(const struct timeval *tm) {
  if (table && table->s->db_low_byte_first)
    int4store(ptr, tm->tv_sec);
  else
    longstore(ptr, (uint32)tm->tv_sec);
}

type_conversion_status Field_timestamp::store_packed(longlong nr) {
  /* Make sure the stored value was previously properly rounded or truncated */
  DBUG_ASSERT((my_packed_time_get_frac_part(nr) %
               (int)log_10_int[DATETIME_MAX_DECIMALS - decimals()]) == 0);
  MYSQL_TIME ltime;
  TIME_from_longlong_datetime_packed(&ltime, nr);
  return Field_timestamp::store_time(&ltime, 0);
}

longlong Field_timestamp::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime) ? 0 : TIME_to_ulonglong_datetime(ltime);
}

bool Field_timestamp::get_date(MYSQL_TIME *ltime,
                               my_time_flags_t fuzzydate) const {
  /* Don't do check_fuzzy_date() as month and year are never 0 for timestamp */
  return get_internal_check_zero(ltime, fuzzydate);
}

int Field_timestamp::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  int32 a, b;
  if (table && table->s->db_low_byte_first) {
    a = sint4korr(a_ptr);
    b = sint4korr(b_ptr);
  } else {
    a = longget(a_ptr);
    b = longget(b_ptr);
  }
  return ((uint32)a < (uint32)b) ? -1 : ((uint32)a > (uint32)b) ? 1 : 0;
}

size_t Field_timestamp::make_sort_key(
    uchar *to, size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 4);
#ifdef WORDS_BIGENDIAN
  if (!table || !table->s->db_low_byte_first) {
    to[0] = ptr[0];
    to[1] = ptr[1];
    to[2] = ptr[2];
    to[3] = ptr[3];
  } else
#endif
  {
    to[0] = ptr[3];
    to[1] = ptr[2];
    to[2] = ptr[1];
    to[3] = ptr[0];
  }
  return 4;
}

void Field_timestamp::sql_type(String &res) const {
  res.set_ascii(STRING_WITH_LEN("timestamp"));
}

type_conversion_status Field_timestamp::validate_stored_val(THD *thd) {
  /*
    While deprecating "TIMESTAMP with implicit DEFAULT value", we can
    remove this function implementation and depend directly on
    "Field_temporal_with_date::validate_stored_val"
  */
  if (!thd->variables.explicit_defaults_for_timestamp) return TYPE_OK;

  return (Field_temporal_with_date::validate_stored_val(thd));
}

/****************************************************************************
** timestamp(N) type
** In string context: YYYY-MM-DD HH:MM:SS.FFFFFF
** In number context: YYYYMMDDHHMMSS.FFFFFF
** Stored as a 7 byte value
****************************************************************************/
Field_timestampf::Field_timestampf(uchar *ptr_arg, uchar *null_ptr_arg,
                                   uchar null_bit_arg, uchar auto_flags_arg,
                                   const char *field_name_arg, uint8 dec_arg)
    : Field_temporal_with_date_and_timef(ptr_arg, null_ptr_arg, null_bit_arg,
                                         auto_flags_arg, field_name_arg,
                                         dec_arg) {
  init_timestamp_flags();
}

Field_timestampf::Field_timestampf(bool is_nullable_arg,
                                   const char *field_name_arg, uint8 dec_arg)
    : Field_temporal_with_date_and_timef(
          nullptr, is_nullable_arg ? &dummy_null_buffer : nullptr, 0, NONE,
          field_name_arg, dec_arg) {
  if (auto_flags & ON_UPDATE_NOW) flags |= ON_UPDATE_NOW_FLAG;
}

my_time_flags_t Field_timestampf::date_flags(const THD *thd) const {
  /* We don't want to store invalid or fuzzy datetime values in TIMESTAMP */
  my_time_flags_t date_flags = TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
    date_flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags |= TIME_FRAC_TRUNCATE;

  return date_flags;
}

type_conversion_status Field_timestampf::reset() {
  memset(ptr, 0, pack_length());
  return TYPE_OK;
}

void Field_timestampf::store_timestamp_internal(const struct timeval *tm) {
  my_timestamp_to_binary(tm, ptr, dec);
}

type_conversion_status Field_timestampf::store_internal(const MYSQL_TIME *ltime,
                                                        int *warnings) {
  THD *thd = table ? table->in_use : current_thd;
  struct timeval tm;
  convert_TIME_to_timestamp(thd, ltime, &tm, warnings);
  const type_conversion_status error =
      time_warning_to_type_conversion_status(*warnings);
  store_timestamp_internal(&tm);
  return error;
}

type_conversion_status Field_timestampf::store_packed(longlong nr) {
  MYSQL_TIME ltime;
  TIME_from_longlong_datetime_packed(&ltime, nr);
  return Field_timestampf::store_time(&ltime, dec);
}

bool Field_timestampf::get_date(MYSQL_TIME *ltime,
                                my_time_flags_t fuzzydate) const {
  /* Don't do check_fuzzy_date() as month and year are never 0 for timestamp */
  return get_internal_check_zero(ltime, fuzzydate);
}

void Field_timestampf::sql_type(String &res) const {
  if (dec == 0) {
    res.set_ascii(STRING_WITH_LEN("timestamp"));
    return;
  }
  const CHARSET_INFO *cs = res.charset();
  res.length(cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                "timestamp(%d)", dec));
}

bool Field_timestampf::get_date_internal(MYSQL_TIME *ltime) const {
  THD *thd = table ? table->in_use : current_thd;
  struct timeval tm;
  my_timestamp_from_binary(&tm, ptr, dec);
  if (tm.tv_sec == 0) return true;
  thd->time_zone()->gmt_sec_to_TIME(ltime, tm);
  return false;
}

bool Field_timestampf::get_timestamp(struct timeval *tm, int *) const {
  THD *thd = table ? table->in_use : current_thd;
  thd->time_zone_used = true;
  DBUG_ASSERT(!is_null());
  my_timestamp_from_binary(tm, ptr, dec);
  return false;
}

type_conversion_status Field_timestampf::validate_stored_val(THD *thd) {
  /*
    While deprecating "TIMESTAMP with implicit DEFAULT value", we can
    remove this function implementation and depend directly on
    "Field_temporal_with_date::validate_stored_val"
  */
  if (!thd->variables.explicit_defaults_for_timestamp) return TYPE_OK;

  return (Field_temporal_with_date::validate_stored_val(thd));
}

/****************************************************************************
** TIME and TIME(N) common methods
****************************************************************************/

bool Field_time_common::convert_str_to_TIME(const char *str, size_t len,
                                            const CHARSET_INFO *cs,
                                            MYSQL_TIME *ltime,
                                            MYSQL_TIME_STATUS *status) {
  return str_to_time(cs, str, len, ltime, date_flags(), status);
}

type_conversion_status Field_time_common::convert_number_to_TIME(
    longlong nr, bool unsigned_val, int nanoseconds, MYSQL_TIME *ltime,
    int *warnings) {
  if (unsigned_val && nr < 0) {
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    set_max_time(ltime, false);
    store_internal(ltime, warnings);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  if (number_to_time(nr, ltime, warnings)) {
    store_internal(ltime, warnings);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  /*
    Both number_to_time() call and negative nanoseconds value
    affect ltime->neg, hence "|=" to combine them:
  */
  if ((ltime->neg |= (nanoseconds < 0))) nanoseconds = -nanoseconds;
  ltime->second_part = 0;

  bool error = time_add_nanoseconds_adjust_frac(
      ltime, nanoseconds, warnings, (date_flags() & TIME_FRAC_TRUNCATE));
  return error ? time_warning_to_type_conversion_status(*warnings) : TYPE_OK;
}

type_conversion_status Field_time_common::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  /* Check if seconds or minutes are out of range */
  if (ltime->second >= 60 || ltime->minute >= 60) {
    if (set_warnings(ErrConvString(ltime, decimals()),
                     MYSQL_TIME_WARN_OUT_OF_RANGE))
      return TYPE_ERR_BAD_VALUE;
    reset();
    return TYPE_WARN_OUT_OF_RANGE;
  }
  int warnings = 0;
  return store_internal_adjust_frac(ltime, &warnings);
}

type_conversion_status Field_time_common::store_internal_adjust_frac(
    MYSQL_TIME *ltime, int *warnings) {
  if (my_time_adjust_frac(ltime, dec, (date_flags() & TIME_FRAC_TRUNCATE)))
    return TYPE_WARN_OUT_OF_RANGE;

  return store_internal(ltime, warnings);
}

String *Field_time_common::val_str(
    String *val_buffer, String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  val_buffer->alloc(MAX_DATE_STRING_REP_LENGTH);
  val_buffer->set_charset(&my_charset_numeric);
  if (get_time(&ltime)) {
    DBUG_ASSERT(0);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_TIME);
  }
  make_time((Date_time_format *)nullptr, &ltime, val_buffer, dec);
  return val_buffer;
}

/**
  For a column for TIME type, get_date() takes the time
  value of the field, adds current date to it and returns
  the result as a DATETIME value.
*/

bool Field_time_common::get_date(MYSQL_TIME *ltime, my_time_flags_t) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME tm;
  if (get_time(&tm)) {
    DBUG_ASSERT(0);
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
  }
  time_to_datetime(table ? table->in_use : current_thd, &tm, ltime);
  return false;
}

longlong Field_time_common::val_date_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME time, datetime;
  if (get_time(&time)) {
    DBUG_ASSERT(0);  // Field_time*::get_time should not fail
    return 0;
  }
  time_to_datetime(table ? table->in_use : current_thd, &time, &datetime);
  return TIME_to_longlong_datetime_packed(datetime);
}

bool Field_time_common::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  MYSQL_TIME ltime;
  if (get_time(&ltime)) {
    DBUG_ASSERT(0);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_TIME);
  }
  return protocol->store_time(ltime, dec);
}

my_time_flags_t Field_time_common::date_flags(const THD *thd) const {
  my_time_flags_t date_flags = 0;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags = TIME_FRAC_TRUNCATE;

  return date_flags;
}

/****************************************************************************
** time type
** In string context: HH:MM:SS
** In number context: HHMMSS
** Stored as a 3 byte unsigned int
****************************************************************************/

type_conversion_status Field_time::store_internal(const MYSQL_TIME *ltime,
                                                  int *) {
  long tmp = ((ltime->month ? 0 : ltime->day * 24L) + ltime->hour) * 10000L +
             (ltime->minute * 100 + ltime->second);
  if (ltime->neg) tmp = -tmp;
  int3store(ptr, tmp);
  return TYPE_OK;
}

type_conversion_status Field_time::store_packed(longlong nr) {
  MYSQL_TIME ltime;
  TIME_from_longlong_time_packed(&ltime, nr);
  return Field_time::store_time(&ltime, 0);
}

longlong Field_time::val_time_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_time(&ltime) ? 0 : TIME_to_longlong_time_packed(ltime);
}

longlong Field_time::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  return (longlong)sint3korr(ptr);
}

bool Field_time::get_time(MYSQL_TIME *ltime) const {
  long tmp = (long)sint3korr(ptr);
  if ((ltime->neg = tmp < 0)) tmp = -tmp;
  ltime->year = ltime->month = ltime->day = 0;
  TIME_set_hhmmss(ltime, tmp);
  ltime->second_part = 0;
  ltime->time_type = MYSQL_TIMESTAMP_TIME;
  return false;
}

int Field_time::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  int32 a, b;
  a = sint3korr(a_ptr);
  b = sint3korr(b_ptr);
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_time::make_sort_key(uchar *to,
                                 size_t length MY_ATTRIBUTE((unused))) const {
  DBUG_ASSERT(length == 3);
  to[0] = (uchar)(ptr[2] ^ 128);
  to[1] = ptr[1];
  to[2] = ptr[0];
  return 3;
}

void Field_time::sql_type(String &res) const {
  res.set_ascii(STRING_WITH_LEN("time"));
}

/****************************************************************************
** time type with fsp
** In string context: HH:MM:SS.FFFFFF
** In number context: HHMMSS.FFFFFF
****************************************************************************/

longlong Field_timef::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  if (get_time(&ltime)) {
    DBUG_ASSERT(0);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_TIME);
  }
  longlong tmp = (longlong)TIME_to_ulonglong_time_round(ltime);
  return ltime.neg ? -tmp : tmp;
}

my_decimal *Field_timef::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  if (get_time(&ltime)) {
    DBUG_ASSERT(0);
    set_zero_time(&ltime, MYSQL_TIMESTAMP_TIME);
  }
  return time2my_decimal(&ltime, decimal_value);
}

double Field_timef::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  if (get_time(&ltime)) {
    DBUG_ASSERT(0);
    return 0;
  }
  double tmp = TIME_to_double_time(ltime);
  return ltime.neg ? -tmp : tmp;
}

void Field_timef::sql_type(String &res) const {
  if (dec == 0) {
    res.set_ascii(STRING_WITH_LEN("time"));
    return;
  }
  const CHARSET_INFO *cs = res.charset();
  res.length(
      cs->cset->snprintf(cs, res.ptr(), res.alloced_length(), "time(%d)", dec));
}

type_conversion_status Field_timef::reset() { return store_packed(0); }

type_conversion_status Field_timef::store_packed(longlong nr) {
  my_time_packed_to_binary(nr, ptr, dec);
  return TYPE_OK;
}

longlong Field_timef::val_time_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  return my_time_packed_from_binary(ptr, dec);
}

type_conversion_status Field_timef::store_internal(const MYSQL_TIME *ltime,
                                                   int *warnings) {
  type_conversion_status rc =
      store_packed(TIME_to_longlong_time_packed(*ltime));
  if (rc == TYPE_OK && non_zero_date(*ltime)) {
    /*
      The DATE part got lost; we warn, like in Field_newdate::store_internal,
      and trigger some code in get_mm_leaf()
      (see err==TYPE_NOTE_TIME_TRUNCATED there).
    */
    *warnings |= MYSQL_TIME_NOTE_TRUNCATED;
    rc = TYPE_NOTE_TIME_TRUNCATED;
  }
  return rc;
}

bool Field_timef::get_time(MYSQL_TIME *ltime) const {
  longlong tmp = val_time_temporal();
  TIME_from_longlong_time_packed(ltime, tmp);
  return false;
}

/****************************************************************************
** year type
** Save in a byte the year 0, 1901->2155
****************************************************************************/

type_conversion_status Field_year::store(const char *from, size_t len,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  const char *end;
  int conv_error;
  type_conversion_status ret = TYPE_OK;
  longlong nr = cs->cset->strntoull10rnd(cs, from, len, 0, &end, &conv_error);

  if (nr < 0 || (nr >= 100 && nr < MIN_YEAR) || nr > MAX_YEAR ||
      conv_error == MY_ERRNO_ERANGE) {
    *ptr = 0;
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TYPE_WARN_OUT_OF_RANGE;
  }

  if (conv_error) ret = TYPE_ERR_BAD_VALUE;

  if (table->in_use->check_for_truncated_fields)
    ret = check_int(cs, from, len, end, conv_error);

  if (ret != TYPE_OK) {
    if (ret == TYPE_ERR_BAD_VALUE) /* empty or incorrect string */
    {
      *ptr = 0;  // Invalid date
      return ret;
    }
    ret = TYPE_WARN_OUT_OF_RANGE;
  }

  if (nr != 0 || len != 4) {
    if (nr < YY_PART_YEAR)
      nr += 100;  // 2000 - 2069
    else if (nr > 1900)
      nr -= 1900;
  }
  *ptr = (char)(uchar)nr;
  return ret;
}

type_conversion_status Field_year::store(double nr) {
  if (nr < 0.0 || nr > MAX_YEAR) {
    (void)Field_year::store((longlong)-1, false);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  return Field_year::store((longlong)nr, false);
}

type_conversion_status Field_year::store_time(
    MYSQL_TIME *ltime, uint8 dec_arg MY_ATTRIBUTE((unused))) {
  if (ltime->time_type != MYSQL_TIMESTAMP_DATETIME &&
      ltime->time_type != MYSQL_TIMESTAMP_DATE) {
    /* Convert time to datetime, then store year of the result */
    THD *thd = table ? table->in_use : current_thd;
    MYSQL_TIME ltime2;
    time_to_datetime(thd, ltime, &ltime2);
    return store(ltime2.year, false);
  }
  return store(ltime->year, false);
}

type_conversion_status Field_year::store(longlong nr, bool) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  if (nr < 0 || (nr >= 100 && nr < MIN_YEAR) || nr > MAX_YEAR) {
    *ptr = 0;
    set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  if (nr != 0)  // 0000 -> 0
  {
    if (nr < YY_PART_YEAR)
      nr += 100;  // 2000 - 2069
    else if (nr > 1900)
      nr -= 1900;
  }
  *ptr = (char)(uchar)nr;
  return TYPE_OK;
}

bool Field_year::send_to_protocol(Protocol *protocol) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  if (is_null()) return protocol->store_null();
  // YEAR is always ZEROFILL. Always zero-pad values up to 4 digits.
  DBUG_ASSERT(zerofill);
  ulonglong tmp = Field_year::val_int();
  return protocol->store_short(tmp, field_length);
}

double Field_year::val_real() const { return (double)Field_year::val_int(); }

longlong Field_year::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  DBUG_ASSERT(field_length == 4);
  int tmp = (int)ptr[0];
  if (tmp != 0) tmp += 1900;
  return (longlong)tmp;
}

String *Field_year::val_str(String *val_buffer, String *) const {
  DBUG_ASSERT(field_length == 4);
  val_buffer->length(0);
  const longlong year = val_int();
  // YEAR is always ZEROFILL. Always zero-pad values up to 4 digits.
  DBUG_ASSERT(zerofill);
  if (year == 0)
    val_buffer->fill(field_length, '0');
  else  // If year != 0, year is always 4 digits
    val_buffer->append_longlong(year);
  val_buffer->set_charset(&my_charset_numeric);
  return val_buffer;
}

void Field_year::sql_type(String &res) const {
  res.length(0);
  res.append(STRING_WITH_LEN("year"));
}

/****************************************************************************
** The new date type
** Stored as 3 bytes
** In number context: YYYYMMDD
****************************************************************************/

my_time_flags_t Field_newdate::date_flags(const THD *thd) const {
  my_time_flags_t date_flags = TIME_FUZZY_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
    date_flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_IN_DATE)
    date_flags |= TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_INVALID_DATES)
    date_flags |= TIME_INVALID_DATES;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags |= TIME_FRAC_TRUNCATE;

  return date_flags;
}

type_conversion_status Field_newdate::store_internal(const MYSQL_TIME *ltime,
                                                     int *warnings) {
  my_date_to_binary(ltime, ptr);
  if (non_zero_time(*ltime)) {
    *warnings |= MYSQL_TIME_NOTE_TRUNCATED;
    return TYPE_NOTE_TIME_TRUNCATED;
  }
  return TYPE_OK;
}

bool Field_newdate::get_date_internal(MYSQL_TIME *ltime) const {
  uint32 tmp = uint3korr(ptr);
  ltime->day = tmp & 31;
  ltime->month = (tmp >> 5) & 15;
  ltime->year = (tmp >> 9);
  ltime->time_type = MYSQL_TIMESTAMP_DATE;
  ltime->hour = ltime->minute = ltime->second = ltime->second_part =
      ltime->neg = false;
  ltime->time_zone_displacement = 0;
  return false;
}

type_conversion_status Field_newdate::store_packed(longlong nr) {
  int warnings = 0;
  MYSQL_TIME ltime;
  TIME_from_longlong_date_packed(&ltime, nr);
  return store_internal(&ltime, &warnings);
}

bool Field_newdate::send_to_protocol(Protocol *protocol) const {
  if (is_null()) return protocol->store_null();
  MYSQL_TIME ltime;
  get_date(&ltime, 0);
  return protocol->store_date(ltime);
}

longlong Field_newdate::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  ulong j = uint3korr(ptr);
  j = (j % 32L) + (j / 32L % 16L) * 100L + (j / (16L * 32L)) * 10000L;
  return (longlong)j;
}

longlong Field_newdate::val_date_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  MYSQL_TIME ltime;
  return get_date_internal(&ltime) ? 0 : TIME_to_longlong_date_packed(ltime);
}

longlong Field_newdate::val_time_temporal() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  return 0;
}

String *Field_newdate::val_str(String *val_buffer,
                               String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  val_buffer->alloc(field_length);
  val_buffer->length(field_length);
  uint32 tmp = uint3korr(ptr);
  int part;
  char *pos = val_buffer->ptr() + 10;

  /* Open coded to get more speed */
  *pos-- = 0;  // End NULL
  part = (int)(tmp & 31);
  *pos-- = (char)('0' + part % 10);
  *pos-- = (char)('0' + part / 10);
  *pos-- = '-';
  part = (int)(tmp >> 5 & 15);
  *pos-- = (char)('0' + part % 10);
  *pos-- = (char)('0' + part / 10);
  *pos-- = '-';
  part = (int)(tmp >> 9);
  *pos-- = (char)('0' + part % 10);
  part /= 10;
  *pos-- = (char)('0' + part % 10);
  part /= 10;
  *pos-- = (char)('0' + part % 10);
  part /= 10;
  *pos = (char)('0' + part);
  val_buffer->set_charset(&my_charset_numeric);
  return val_buffer;
}

bool Field_newdate::get_date(MYSQL_TIME *ltime,
                             my_time_flags_t fuzzydate) const {
  return get_internal_check_zero(ltime, fuzzydate) ||
         check_fuzzy_date(*ltime, fuzzydate);
}

int Field_newdate::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  uint32 a, b;
  a = uint3korr(a_ptr);
  b = uint3korr(b_ptr);
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_newdate::make_sort_key(
    uchar *to, size_t length MY_ATTRIBUTE((unused))) const {
  memset(to, 0, length);
  to[0] = ptr[2];
  to[1] = ptr[1];
  to[2] = ptr[0];
  return 3;
}

void Field_newdate::sql_type(String &res) const {
  res.set_ascii(STRING_WITH_LEN("date"));
}

/****************************************************************************
** datetime type
** In string context: YYYY-MM-DD HH:MM:DD
** In number context: YYYYMMDDHHMMDD
** Stored as a 8 byte unsigned int. Should sometimes be change to a 6 byte int.
****************************************************************************/

my_time_flags_t Field_datetime::date_flags(const THD *thd) const {
  my_time_flags_t date_flags = TIME_FUZZY_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
    date_flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_IN_DATE)
    date_flags |= TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_INVALID_DATES)
    date_flags |= TIME_INVALID_DATES;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags |= TIME_FRAC_TRUNCATE;

  return date_flags;
}

void Field_datetime::store_timestamp_internal(const timeval *tm) {
  MYSQL_TIME mysql_time;
  THD *thd = current_thd;
  thd->variables.time_zone->gmt_sec_to_TIME(&mysql_time, *tm);
  thd->time_zone_used = true;
  int error = 0;
  store_internal(&mysql_time, &error);
}

/**
  Store a DATETIME in a 8-byte integer to record.

  @param table  Table
  @param tmp    The number, in YYYYMMDDhhmmss format
  @param ptr    Where to store to
*/
static inline type_conversion_status datetime_store_internal(TABLE *table,
                                                             ulonglong tmp,
                                                             uchar *ptr) {
  if (table && table->s->db_low_byte_first)
    int8store(ptr, tmp);
  else
    longlongstore(ptr, tmp);
  return TYPE_OK;
}

/**
  Read a DATETIME from record to a 8-byte integer

  @param table  Table
  @param ptr    Where to read from
  @retval       An integer in format YYYYMMDDhhmmss
*/
static inline longlong datetime_get_internal(TABLE *table, uchar *ptr) {
  if (table && table->s->db_low_byte_first)
    return sint8korr(ptr);
  else
    return longlongget(ptr);
}

bool Field_datetime::get_date_internal(MYSQL_TIME *ltime) const {
  longlong tmp = datetime_get_internal(table, ptr);
  ltime->time_type = MYSQL_TIMESTAMP_DATETIME;
  ltime->neg = false;
  ltime->second_part = 0;
  TIME_set_yymmdd(ltime, (uint)(tmp / 1000000LL));
  TIME_set_hhmmss(ltime, (uint)(tmp % 1000000LL));
  ltime->time_zone_displacement = 0;
  return false;
}

type_conversion_status Field_datetime::store_internal(const MYSQL_TIME *ltime,
                                                      int *) {
  ulonglong tmp = TIME_to_ulonglong_datetime(*ltime);
  return datetime_store_internal(table, tmp, ptr);
}

type_conversion_status Field_datetime::store(longlong nr, bool unsigned_val) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  MYSQL_TIME ltime;
  int warnings;
  type_conversion_status error = TYPE_OK;
  longlong tmp =
      convert_number_to_datetime(nr, unsigned_val, &ltime, &warnings);
  if (tmp == -1LL)
    error = TYPE_ERR_BAD_VALUE;
  else {
    error = time_warning_to_type_conversion_status(warnings);
    datetime_store_internal(table, tmp, ptr);
  }
  if (warnings && set_warnings(ErrConvString(nr, unsigned_val), warnings))
    error = TYPE_ERR_BAD_VALUE;
  return error;
}

type_conversion_status Field_datetime::store_packed(longlong nr) {
  MYSQL_TIME ltime;
  TIME_from_longlong_datetime_packed(&ltime, nr);
  return Field_datetime::store_time(&ltime, 0);
}

longlong Field_datetime::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  return datetime_get_internal(table, ptr);
}

/*
  We don't reuse the parent method for performance purposes,
  to avoid convertion from number to MYSQL_TIME.
  Using my_datetime_number_to_str() instead of my_datetime_to_str().
*/
String *Field_datetime::val_str(String *val_buffer,
                                String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  val_buffer->alloc(field_length + 1);
  val_buffer->set_charset(&my_charset_numeric);
  val_buffer->length(MAX_DATETIME_WIDTH);
  longlong tmp = datetime_get_internal(table, ptr);
  val_buffer->length(my_datetime_number_to_str(val_buffer->ptr(), tmp));
  return val_buffer;
}

bool Field_datetime::get_date(MYSQL_TIME *ltime,
                              my_time_flags_t fuzzydate) const {
  return get_internal_check_zero(ltime, fuzzydate) ||
         check_fuzzy_date(*ltime, fuzzydate);
}

int Field_datetime::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  longlong a, b;
  if (table && table->s->db_low_byte_first) {
    a = sint8korr(a_ptr);
    b = sint8korr(b_ptr);
  } else {
    a = longlongget(a_ptr);
    b = longlongget(b_ptr);
  }
  return ((ulonglong)a < (ulonglong)b) ? -1
                                       : ((ulonglong)a > (ulonglong)b) ? 1 : 0;
}

size_t Field_datetime::make_sort_key(uchar *to, size_t length) const {
  DBUG_ASSERT(length == PACK_LENGTH);
#ifdef WORDS_BIGENDIAN
  if (!table || !table->s->db_low_byte_first)
    copy_integer<true>(to, length, ptr, PACK_LENGTH, true);
  else
#endif
    copy_integer<false>(to, length, ptr, PACK_LENGTH, true);
  return PACK_LENGTH;
}

void Field_datetime::sql_type(String &res) const {
  res.set_ascii(STRING_WITH_LEN("datetime"));
}

/****************************************************************************
** datetimef type
** In string context: YYYY-MM-DD HH:MM:DD.FFFFFF
** In number context: YYYYMMDDHHMMDD.FFFFFF
** Stored as a 8 byte value.
****************************************************************************/

my_time_flags_t Field_datetimef::date_flags(const THD *thd) const {
  my_time_flags_t date_flags = TIME_FUZZY_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE)
    date_flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_NO_ZERO_IN_DATE)
    date_flags |= TIME_NO_ZERO_IN_DATE;
  if (thd->variables.sql_mode & MODE_INVALID_DATES)
    date_flags |= TIME_INVALID_DATES;
  if (thd->variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL)
    date_flags |= TIME_FRAC_TRUNCATE;

  return date_flags;
}

void Field_datetimef::store_timestamp_internal(const timeval *tm) {
  MYSQL_TIME mysql_time;
  THD *thd = current_thd;
  thd->variables.time_zone->gmt_sec_to_TIME(&mysql_time, *tm);
  thd->time_zone_used = true;
  int warnings = 0;
  store_internal(&mysql_time, &warnings);
}

bool Field_datetimef::get_date(MYSQL_TIME *ltime,
                               my_time_flags_t fuzzydate) const {
  return get_internal_check_zero(ltime, fuzzydate) ||
         check_fuzzy_date(*ltime, fuzzydate);
}

void Field_datetimef::sql_type(String &res) const {
  if (dec == 0) {
    res.set_ascii(STRING_WITH_LEN("datetime"));
    return;
  }
  const CHARSET_INFO *cs = res.charset();
  res.length(cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                "datetime(%d)", dec));
}

bool Field_datetimef::get_date_internal(MYSQL_TIME *ltime) const {
  TIME_from_longlong_datetime_packed(ltime, val_date_temporal());
  return false;
}

type_conversion_status Field_datetimef::store_internal(const MYSQL_TIME *ltime,
                                                       int *) {
  /*
    If time zone displacement information is present in "ltime"
    - adjust the value to UTC based on the time zone
    - convert to the local time zone
  */
  MYSQL_TIME temp_t = *ltime;
  adjust_time_zone_displacement(current_thd->time_zone(), &temp_t);
  store_packed(TIME_to_longlong_datetime_packed(temp_t));

  return TYPE_OK;
}

type_conversion_status Field_datetimef::reset() {
  store_packed(0);
  return TYPE_OK;
}

longlong Field_datetimef::val_date_temporal() const {
  return my_datetime_packed_from_binary(ptr, dec);
}

type_conversion_status Field_datetimef::store_packed(longlong nr) {
  my_datetime_packed_to_binary(nr, ptr, dec);
  return TYPE_OK;
}

/****************************************************************************
** string type
** A string may be varchar or binary
****************************************************************************/

/**
  Report "not well formed" or "cannot convert" error
  after storing a character string info a field.

  As of version 5.0 both cases return the same error:

      "Invalid string value: 'xxx' for column 't' at row 1"

  Future versions will possibly introduce a new error message:

      "Cannot convert character string: 'xxx' for column 't' at row 1"

  @param  well_formed_error_pos      position of the first non-wellformed
                                     character in the source string
  @param  cannot_convert_error_pos   position of the first non-convertable
                                     character in the source string
  @param  from_end_pos               position where conversion stopped in
                                     the source string
  @param  end                        end of the source string
  @param  count_spaces               treat trailing spaces as important data
  @param  cs                         character set of the string

  @return TYPE_OK, TYPE_NOTE_TRUNCATED, TYPE_WARN_TRUNCATED,
          TYPE_WARN_INVALID_STRING

*/

type_conversion_status Field_longstr::check_string_copy_error(
    const char *well_formed_error_pos, const char *cannot_convert_error_pos,
    const char *from_end_pos, const char *end, bool count_spaces,
    const CHARSET_INFO *cs) {
  const char *pos;
  char tmp[32];
  THD *thd = table->in_use;

  if (!(pos = well_formed_error_pos) && !(pos = cannot_convert_error_pos))
    return report_if_important_data(from_end_pos, end, count_spaces);

  convert_to_printable(tmp, sizeof(tmp), pos, (end - pos), cs, 6);

  push_warning_printf(
      thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
      ER_THD(thd, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD), "string", tmp,
      field_name, thd->get_stmt_da()->current_row_for_condition());

  if (well_formed_error_pos != nullptr) return TYPE_WARN_INVALID_STRING;

  return TYPE_WARN_TRUNCATED;
}

/*
  Check if we lost any important data and send a truncation error/warning

  SYNOPSIS
    Field_longstr::report_if_important_data()
    pstr                     - Truncated rest of string
    end                      - End of truncated string
    count_spaces             - Treat traling spaces as important data

  RETURN VALUES
    TYPE_OK    - None was truncated
    != TYPE_OK - Some bytes were truncated

  NOTE
    Check if we lost any important data (anything in a binary string,
    or any non-space in others). If only trailing spaces was lost,
    send a truncation note, otherwise send a truncation error.
    Silently ignore traling spaces if the count_space parameter is false.
*/

type_conversion_status Field_longstr::report_if_important_data(
    const char *pstr, const char *end, bool count_spaces) {
  if (pstr < end)  // String is truncated
  {
    if (test_if_important_data(field_charset, pstr, end)) {
      THD *thd = table->in_use;
      thd->really_error_partial_strict = thd->variables.error_partial_strict;

      // Warning should only be written when check_for_truncated_fields is set
      if (thd->check_for_truncated_fields) {
        if (!thd->lex->is_ignore() &&
            (thd->is_strict_sql_mode() || thd->really_error_partial_strict))
          set_warning(Sql_condition::SL_WARNING, ER_DATA_TOO_LONG, 1);
        else
          set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
      }

      thd->really_error_partial_strict = false;
      return TYPE_WARN_TRUNCATED;
    } else if (count_spaces) {
      // If we lost only spaces then produce a NOTE, not a WARNING
      if (table->in_use->check_for_truncated_fields) {
        set_warning(Sql_condition::SL_NOTE, WARN_DATA_TRUNCATED, 1);
      }
      return TYPE_NOTE_TRUNCATED;
    }
  }
  return TYPE_OK;
}

/* Copy a string and fill with space */

type_conversion_status Field_string::store(const char *from, size_t length,
                                           const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  size_t copy_length;
  const char *well_formed_error_pos;
  const char *cannot_convert_error_pos;
  const char *from_end_pos;

  /* See the comment for Field_long::store(long long) */
  DBUG_ASSERT(table->in_use == current_thd);

  copy_length = field_well_formed_copy_nchars(
      field_charset, (char *)ptr, field_length, cs, from, length,
      field_length / field_charset->mbmaxlen, &well_formed_error_pos,
      &cannot_convert_error_pos, &from_end_pos);

  /* Append spaces if the string was shorter than the field. */
  if (copy_length < field_length)
    field_charset->cset->fill(field_charset, (char *)ptr + copy_length,
                              field_length - copy_length,
                              field_charset->pad_char);

  return check_string_copy_error(well_formed_error_pos,
                                 cannot_convert_error_pos, from_end_pos,
                                 from + length, false, cs);
}

/**
  Store double value in Field_string or Field_varstring.

  Pretty prints double number into field_length characters buffer.

  @param nr            number
*/

type_conversion_status Field_str::store(double nr) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  char buff[DOUBLE_TO_STRING_CONVERSION_BUFFER_SIZE];
  uint local_char_length = field_length / charset()->mbmaxlen;
  size_t length = 0;
  bool error = (local_char_length == 0);

  // my_gcvt() requires width > 0, and we may have a CHAR(0) column.
  if (!error)
    length = my_gcvt(nr, MY_GCVT_ARG_DOUBLE, local_char_length, buff, &error);

  if (error) {
    if (!table->in_use->lex->is_ignore() && table->in_use->is_strict_sql_mode())
      set_warning(Sql_condition::SL_WARNING, ER_DATA_TOO_LONG, 1);
    else
      set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
  }
  return store(buff, length, &my_charset_numeric);
}

/**
  Check whether generated columns' expressions are the same.

  @param field  A new field to compare against

  @return true means the same, otherwise not.
*/

bool Field::gcol_expr_is_equal(const Create_field *field) const {
  DBUG_ASSERT(is_gcol() && field->is_gcol());
  return gcol_info->expr_item->eq(field->gcol_info->expr_item, true);
}

uint Field_str::is_equal(const Create_field *new_field) const {
  if (change_prevents_inplace(*this, *new_field)) {
    return IS_EQUAL_NO;
  }

  size_t new_char_len = new_field->max_display_width_in_codepoints();
  if (new_char_len != char_length()  // Changed char len cannot be done
                                     // inplace due to padding
  ) {
    return IS_EQUAL_NO;
  }

  if (new_field->charset == field_charset) {
    return IS_EQUAL_YES;
  }

  return IS_EQUAL_PACK_LENGTH;
}

type_conversion_status Field_string::store(longlong nr, bool unsigned_val) {
  char buff[64];
  size_t l;
  const CHARSET_INFO *cs = charset();
  l = (cs->cset->longlong10_to_str)(cs, buff, sizeof(buff),
                                    unsigned_val ? 10 : -10, nr);
  return Field_string::store(buff, l, cs);
}

type_conversion_status Field_longstr::store_decimal(const my_decimal *d) {
  StringBuffer<DECIMAL_MAX_STR_LENGTH + 1> str(&my_charset_numeric);
  my_decimal2string(E_DEC_FATAL_ERROR, d, &str);
  return store(str.ptr(), str.length(), str.charset());
}

uint32 Field_longstr::max_data_length() const {
  return field_length + (field_length > 255 ? 2 : 1);
}

double Field_string::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int error;
  const char *end;
  const CHARSET_INFO *cs = charset();
  double result;

  result = my_strntod(cs, (char *)ptr, field_length, &end, &error);
  if ((error ||
       (field_length != (uint32)(end - (char *)ptr) &&
        !check_if_only_end_space(cs, end, (char *)ptr + field_length)))) {
    size_t length =
        cs->cset->lengthsp(cs, pointer_cast<const char *>(ptr), field_length);
    ErrConvString err((char *)ptr, length, cs);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "DOUBLE", err.ptr());
  }
  return result;
}

longlong Field_string::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int error;
  const char *end;
  const CHARSET_INFO *cs = charset();
  longlong result;

  result = my_strntoll(cs, (char *)ptr, field_length, 10, &end, &error);
  if ((error ||
       (field_length != (uint32)(end - (char *)ptr) &&
        !check_if_only_end_space(cs, end, (char *)ptr + field_length)))) {
    size_t length =
        cs->cset->lengthsp(cs, pointer_cast<const char *>(ptr), field_length);
    ErrConvString err((char *)ptr, length, cs);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "INTEGER", err.ptr());
  }
  return result;
}

String *Field_string::val_str(String *val_buffer MY_ATTRIBUTE((unused)),
                              String *val_ptr) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  /* See the comment for Field_long::store(long long) */
  DBUG_ASSERT(table->in_use == current_thd);
  size_t length;
  if (table->in_use->variables.sql_mode & MODE_PAD_CHAR_TO_FULL_LENGTH)
    length = my_charpos(field_charset, ptr, ptr + field_length,
                        field_length / field_charset->mbmaxlen);
  else
    length = field_charset->cset->lengthsp(field_charset, (const char *)ptr,
                                           field_length);
  val_ptr->set((const char *)ptr, length, field_charset);
  return val_ptr;
}

my_decimal *Field_string::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = charset();
  int err = str2my_decimal(E_DEC_FATAL_ERROR, (char *)ptr, field_length, cs,
                           decimal_value);
  if (err) {
    size_t length =
        cs->cset->lengthsp(cs, pointer_cast<const char *>(ptr), field_length);
    ErrConvString errmsg((char *)ptr, length, cs);
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
        ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "DECIMAL", errmsg.ptr());
  }

  return decimal_value;
}

struct Check_field_param {
  const Field *field;
};

static bool check_field_for_37426(const void *param_arg) {
  const Check_field_param *param =
      static_cast<const Check_field_param *>(param_arg);
  DBUG_ASSERT(param->field->real_type() == MYSQL_TYPE_STRING);
  DBUG_PRINT("debug",
             ("Field %s - type: %d, size: %d", param->field->field_name,
              param->field->real_type(), param->field->row_pack_length()));
  return param->field->row_pack_length() > 255;
}

bool Field_string::compatible_field_size(uint field_metadata,
                                         Relay_log_info *rli_arg, uint16 mflags,
                                         int *order_var) const {
  const Check_field_param check_param = {this};
  if (!is_mts_worker(rli_arg->info_thd) &&
      rpl_master_has_bug(rli_arg, 37426, true, check_field_for_37426,
                         &check_param))
    return false;  // Not compatible field sizes
  return Field::compatible_field_size(field_metadata, rli_arg, mflags,
                                      order_var);
}

int Field_string::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  size_t a_len, b_len;

  if (field_charset->mbmaxlen != 1) {
    uint char_len = field_length / field_charset->mbmaxlen;
    a_len = my_charpos(field_charset, a_ptr, a_ptr + field_length, char_len);
    b_len = my_charpos(field_charset, b_ptr, b_ptr + field_length, char_len);
  } else
    a_len = b_len = field_length;

  if (field_charset->pad_attribute == NO_PAD &&
      !(table->in_use->variables.sql_mode & MODE_PAD_CHAR_TO_FULL_LENGTH)) {
    /*
      Our CHAR default behavior is to strip spaces. For PAD SPACE collations,
      this doesn't matter, for but NO PAD, we need to do it ourselves here.
    */
    a_len = field_charset->cset->lengthsp(field_charset, (const char *)a_ptr,
                                          a_len);
    b_len = field_charset->cset->lengthsp(field_charset, (const char *)b_ptr,
                                          b_len);
  }

  return field_charset->coll->strnncollsp(field_charset, a_ptr, a_len, b_ptr,
                                          b_len);
}

size_t Field_string::make_sort_key(uchar *to, size_t length) const {
  /*
    We don't store explicitly how many bytes long this string is.
    Find out by calling charpos, since just using field_length
    could give strnxfrm a buffer with more than char_length() code
    points, which is not allowed.

    The min() is because charpos() is allowed to return a value past
    the end of the string for “end of string”.
  */
  size_t input_length = std::min<size_t>(
      field_length,
      field_charset->cset->charpos(
          field_charset, pointer_cast<const char *>(ptr),
          pointer_cast<const char *>(ptr) + field_length, char_length()));

  if (field_charset->pad_attribute == NO_PAD &&
      !(table->in_use->variables.sql_mode & MODE_PAD_CHAR_TO_FULL_LENGTH)) {
    /*
      Our CHAR default behavior is to strip spaces. For PAD SPACE collations,
      this doesn't matter, for but NO PAD, we need to do it ourselves here.
    */
    input_length = field_charset->cset->lengthsp(
        field_charset, (const char *)ptr, input_length);
  }

  DBUG_ASSERT(char_length_cache == char_length());
  size_t tmp MY_ATTRIBUTE((unused)) = field_charset->coll->strnxfrm(
      field_charset, to, length, char_length_cache, ptr, input_length,
      MY_STRXFRM_PAD_TO_MAXLEN);
  DBUG_ASSERT(tmp == length);
  return length;
}

void Field_string::sql_type(String &res) const {
  THD *thd = table->in_use;
  const CHARSET_INFO *cs = res.charset();
  size_t length;

  length = cs->cset->snprintf(
      cs, res.ptr(), res.alloced_length(), "%s(%d)",
      ((type() == MYSQL_TYPE_VAR_STRING && !thd->variables.new_mode)
           ? (has_charset() ? "varchar" : "varbinary")
           : (has_charset() ? "char" : "binary")),
      (int)field_length / charset()->mbmaxlen);
  res.length(length);
}

uchar *Field_string::pack(uchar *to, const uchar *from, uint max_length,
                          bool) const {
  uint length = my_charpos(field_charset, from, from + field_length,
                           field_length / field_charset->mbmaxlen);
  uint length_bytes = (field_length > 255) ? 2 : 1;

  /*
     TODO: change charset interface to add a new function that does
           the following or add a flag to lengthsp to do it itself
           (this is for not packing padding adding bytes in BINARY
           fields).
  */
  if (key_type() != HA_KEYTYPE_BINARY) {
    if (field_charset->mbmaxlen == 1) {
      while (length && from[length - 1] == field_charset->pad_char) length--;
    } else
      length = field_charset->cset->lengthsp(field_charset, (const char *)from,
                                             length);
  }

  if (max_length < length_bytes)
    length = 0;
  else if (length > max_length - length_bytes)
    length = max_length - length_bytes;

  /* Length always stored little-endian */
  if (max_length >= 1) {
    *to++ = length & 0xFF;
    if (length_bytes == 2 && max_length >= 2) *to++ = (length >> 8) & 0xFF;
  }

  /* Store bytes of string */
  memcpy(to, from, length);

  return to + length;
}

/**
   Unpack a string field from row data.

   This method is used to unpack a string field from a master whose size
   of the field is less than that of the slave. Note that there can be a
   variety of field types represented with this class. Certain types like
   ENUM or SET are processed differently. Hence, the upper byte of the
   @c param_data argument contains the result of field->real_type() from
   the master.

   @note For information about how the length is packed, see @c
   Field_string::do_save_field_metadata

   @param   to         Destination of the data
   @param   from       Source of the data
   @param   param_data Real type (upper) and length (lower) values
   @param   low_byte_first Unused.

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field_string::unpack(uchar *to, const uchar *from, uint param_data,
                                  bool low_byte_first MY_ATTRIBUTE((unused))) {
  uint from_length, length;

  /*
    Compute the declared length of the field on the master. This is
    used to decide if one or two bytes should be read as length.
   */
  if (param_data)
    from_length = (((param_data >> 4) & 0x300) ^ 0x300) + (param_data & 0x00ff);
  else
    from_length = field_length;

  DBUG_PRINT("debug", ("param_data: 0x%x, field_length: %u, from_length: %u",
                       param_data, field_length, from_length));
  /*
    Compute the actual length of the data by reading one or two bits
    (depending on the declared field length on the master).
   */
  if (from_length > 255) {
    length = uint2korr(from);
    from += 2;
  } else
    length = (uint)*from++;

  memcpy(to, from, length);
  // Pad the string with the pad character of the fields charset
  field_charset->cset->fill(field_charset, (char *)to + length,
                            field_length - length, field_charset->pad_char);
  return from + length;
}

/**
   Save the field metadata for string fields.

   Saves the real type in the first byte and the field length in the
   second byte of the field metadata array at index of *metadata_ptr and
   *(metadata_ptr + 1).

   @note In order to be able to handle lengths exceeding 255 and be
   backwards-compatible with pre-5.1.26 servers, an extra two bits of
   the length has been added to the metadata in such a way that if
   they are set, a new unrecognized type is generated.  This will
   cause pre-5.1-26 servers to stop due to a field type mismatch,
   while new servers will be able to extract the extra bits. If the
   length is <256, there will be no difference and both a new and an
   old server will be able to handle it.

   @note The extra two bits are added to bits 13 and 14 of the
   parameter data (with 1 being the least siginficant bit and 16 the
   most significant bit of the word) by xoring the extra length bits
   with the real type.  Since all allowable types have 0xF as most
   significant bits of the metadata word, lengths <256 will not affect
   the real type at all, while all other values will result in a
   non-existant type in the range 17-244.

   @see Field_string::unpack

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_string::do_save_field_metadata(uchar *metadata_ptr) const {
  DBUG_ASSERT(field_length < 1024);
  DBUG_ASSERT((real_type() & 0xF0) == 0xF0);
  DBUG_PRINT("debug",
             ("field_length: %u, real_type: %u", field_length, real_type()));
  *metadata_ptr = (real_type() ^ ((field_length & 0x300) >> 4));
  *(metadata_ptr + 1) = field_length & 0xFF;
  return 2;
}

uint Field_string::max_packed_col_length() const {
  const uint max_length = pack_length();
  return (max_length > 255 ? 2 : 1) + max_length;
}

size_t Field_string::get_key_image(uchar *buff, size_t length,
                                   imagetype) const {
  size_t bytes =
      my_charpos(field_charset, (char *)ptr, (char *)ptr + field_length,
                 length / field_charset->mbmaxlen);
  memcpy(buff, ptr, bytes);
  if (bytes < length)
    field_charset->cset->fill(field_charset, (char *)buff + bytes,
                              length - bytes, field_charset->pad_char);
  return bytes;
}

/****************************************************************************
  VARCHAR type
  Data in field->ptr is stored as:
    1 or 2 bytes length-prefix-header  (from Field_varstring::length_bytes)
    data

  NOTE:
  When VARCHAR is stored in a key (for handler::index_read() etc) it's always
  stored with a 2 byte prefix. (Just like blob keys).

  Normally length_bytes is calculated as (field_length < 256 : 1 ? 2)
  The exception is if there is a prefix key field that is part of a long
  VARCHAR, in which case field_length for this may be 1 but the length_bytes
  is 2.
****************************************************************************/

/**
   Save the field metadata for varstring fields.

   Saves the field length in the first byte. Note: may consume
   2 bytes. Caller must ensure second byte is contiguous with
   first byte (e.g. array index 0,1).

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_varstring::do_save_field_metadata(uchar *metadata_ptr) const {
  DBUG_ASSERT(field_length <= 65535);
  int2store((char *)metadata_ptr, field_length);
  return 2;
}

type_conversion_status Field_varstring::store(const char *from, size_t length,
                                              const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  size_t copy_length;
  const char *well_formed_error_pos;
  const char *cannot_convert_error_pos;
  const char *from_end_pos;

  copy_length = field_well_formed_copy_nchars(
      field_charset, (char *)ptr + length_bytes, field_length, cs, from, length,
      field_length / field_charset->mbmaxlen, &well_formed_error_pos,
      &cannot_convert_error_pos, &from_end_pos);

  if (length_bytes == 1)
    *ptr = (uchar)copy_length;
  else
    int2store(ptr, static_cast<uint16>(copy_length));

  return check_string_copy_error(well_formed_error_pos,
                                 cannot_convert_error_pos, from_end_pos,
                                 from + length, true, cs);
}

type_conversion_status Field_varstring::store(longlong nr, bool unsigned_val) {
  char buff[64];
  uint length;
  length = (uint)(field_charset->cset->longlong10_to_str)(
      field_charset, buff, sizeof(buff), (unsigned_val ? 10 : -10), nr);
  return Field_varstring::store(buff, length, field_charset);
}

double Field_varstring::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int error;
  const char *end;
  double result;
  const CHARSET_INFO *cs = charset();

  uint length = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
  result = my_strntod(cs, (char *)ptr + length_bytes, length, &end, &error);

  if ((error || (length != (uint)(end - (char *)ptr + length_bytes) &&
                 !check_if_only_end_space(
                     cs, end, (char *)ptr + length_bytes + length)))) {
    push_numerical_conversion_warning(current_thd, (char *)ptr + length_bytes,
                                      length, cs, "DOUBLE",
                                      ER_TRUNCATED_WRONG_VALUE);
  }
  return result;
}

longlong Field_varstring::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int error;
  const char *end;
  const CHARSET_INFO *cs = charset();

  uint length = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
  longlong result =
      my_strntoll(cs, (char *)ptr + length_bytes, length, 10, &end, &error);

  if ((error || (length != (uint)(end - (char *)ptr + length_bytes) &&
                 !check_if_only_end_space(
                     cs, end, (char *)ptr + length_bytes + length)))) {
    push_numerical_conversion_warning(current_thd, (char *)ptr + length_bytes,
                                      length, cs, "INTEGER",
                                      ER_TRUNCATED_WRONG_VALUE);
  }
  return result;
}

String *Field_varstring::val_str(String *val_buffer MY_ATTRIBUTE((unused)),
                                 String *val_ptr) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  uint length = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
  val_ptr->set((const char *)ptr + length_bytes, length, field_charset);
  return val_ptr;
}

my_decimal *Field_varstring::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  const CHARSET_INFO *cs = charset();
  uint length = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
  int error = str2my_decimal(E_DEC_FATAL_ERROR, (char *)ptr + length_bytes,
                             length, cs, decimal_value);

  if (error) {
    push_numerical_conversion_warning(current_thd, (char *)ptr + length_bytes,
                                      length, cs, "DECIMAL",
                                      ER_TRUNCATED_WRONG_VALUE);
  }
  return decimal_value;
}

int Field_varstring::cmp_max(const uchar *a_ptr, const uchar *b_ptr,
                             uint max_len) const {
  uint a_length, b_length;
  int diff;

  if (length_bytes == 1) {
    a_length = (uint)*a_ptr;
    b_length = (uint)*b_ptr;
  } else {
    a_length = uint2korr(a_ptr);
    b_length = uint2korr(b_ptr);
  }
  a_length = std::min(a_length, max_len);
  b_length = std::min(b_length, max_len);
  diff = field_charset->coll->strnncollsp(field_charset, a_ptr + length_bytes,
                                          a_length, b_ptr + length_bytes,
                                          b_length);
  return diff;
}

/**
  @note
    varstring and blob keys are ALWAYS stored with a 2 byte length prefix
*/

int Field_varstring::key_cmp(const uchar *key_ptr, uint max_key_length) const {
  uint length = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
  uint local_char_length = max_key_length / field_charset->mbmaxlen;

  local_char_length =
      my_charpos(field_charset, ptr + length_bytes, ptr + length_bytes + length,
                 local_char_length);
  length = std::min(length, local_char_length);
  return field_charset->coll->strnncollsp(field_charset, ptr + length_bytes,
                                          length, key_ptr + HA_KEY_BLOB_LENGTH,
                                          uint2korr(key_ptr));
}

/**
  Compare to key segments (always 2 byte length prefix).

  @note
    This is used only to compare key segments created for index_read().
    (keys are created and compared in key.cc)
*/

int Field_varstring::key_cmp(const uchar *a, const uchar *b) const {
  return field_charset->coll->strnncollsp(field_charset, a + HA_KEY_BLOB_LENGTH,
                                          uint2korr(a), b + HA_KEY_BLOB_LENGTH,
                                          uint2korr(b));
}

size_t Field_varstring::make_sort_key(uchar *to, size_t length) const {
  size_t input_bytes = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);

  const int flags =
      (field_charset->pad_attribute == NO_PAD) ? 0 : MY_STRXFRM_PAD_TO_MAXLEN;

  DBUG_ASSERT(char_length_cache == char_length());
  return field_charset->coll->strnxfrm(field_charset, to, length,
                                       char_length_cache, ptr + length_bytes,
                                       input_bytes, flags);
}

enum ha_base_keytype Field_varstring::key_type() const {
  enum ha_base_keytype res;

  if (binary())
    res = length_bytes == 1 ? HA_KEYTYPE_VARBINARY1 : HA_KEYTYPE_VARBINARY2;
  else
    res = length_bytes == 1 ? HA_KEYTYPE_VARTEXT1 : HA_KEYTYPE_VARTEXT2;
  return res;
}

void Field_varstring::sql_type(String &res) const {
  const CHARSET_INFO *cs = res.charset();
  size_t length;

  length = cs->cset->snprintf(cs, res.ptr(), res.alloced_length(), "%s(%d)",
                              (has_charset() ? "varchar" : "varbinary"),
                              (int)field_length / charset()->mbmaxlen);
  res.length(length);
}

uint32 Field_varstring::data_length(ptrdiff_t row_offset) const {
  return length_bytes == 1 ? (uint32) * (ptr + row_offset)
                           : uint2korr(ptr + row_offset);
}

/*
  Functions to create a packed row.
  Here the number of length bytes are depending on the given max_length
*/

uchar *Field_varstring::pack(uchar *to, const uchar *from, uint max_length,
                             bool) const {
  uint length = length_bytes == 1 ? (uint)*from : uint2korr(from);
  if (max_length < length_bytes)
    length = 0;
  else if (length > max_length - length_bytes)
    length = max_length - length_bytes;

  /* Length always stored little-endian */
  if (max_length >= 1) {
    *to++ = length & 0xFF;
    if (length_bytes == 2 && max_length >= 2) *to++ = (length >> 8) & 0xFF;
  }

  /* Store bytes of string */
  memcpy(to, from + length_bytes, length);
  return to + length;
}

/**
   Unpack a varstring field from row data.

   This method is used to unpack a varstring field from a master
   whose size of the field is less than that of the slave.

   @note
   The string length is always packed little-endian.

   @param   to         Destination of the data
   @param   from       Source of the data
   @param   param_data Length bytes from the master's field data
   @param   low_byte_first Unused.

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field_varstring::unpack(
    uchar *to, const uchar *from, uint param_data,
    bool low_byte_first MY_ATTRIBUTE((unused))) {
  uint length;
  uint l_bytes = (param_data && (param_data < field_length))
                     ? (param_data <= 255) ? 1 : 2
                     : length_bytes;
  if (l_bytes == 1) {
    to[0] = *from++;
    length = to[0];
    if (length_bytes == 2) to[1] = 0;
  } else /* l_bytes == 2 */
  {
    length = uint2korr(from);
    to[0] = *from++;
    to[1] = *from++;
  }
  if (length) memcpy(to + length_bytes, from, length);
  return from + length;
}

size_t Field_varstring::get_key_image(uchar *buff, size_t length,
                                      imagetype) const {
  /*
    If NULL, data bytes may have been left random by the storage engine, so
    don't try to read them.
  */
  uint f_length =
      is_null() ? 0 : (length_bytes == 1 ? (uint)*ptr : uint2korr(ptr));
  uint local_char_length = length / field_charset->mbmaxlen;
  uchar *pos = ptr + length_bytes;
  local_char_length =
      my_charpos(field_charset, pos, pos + f_length, local_char_length);
  f_length = std::min(f_length, local_char_length);
  /* Key is always stored with 2 bytes */
  int2store(buff, f_length);
  memcpy(buff + HA_KEY_BLOB_LENGTH, pos, f_length);
  if (f_length < length) {
    /*
      Must clear this as we do a memcmp in opt_range.cc to detect
      identical keys
    */
    memset(buff + HA_KEY_BLOB_LENGTH + f_length, 0, (length - f_length));
  }
  return HA_KEY_BLOB_LENGTH + f_length;
}

void Field_varstring::set_key_image(const uchar *buff, size_t length) {
  length = uint2korr(buff);  // Real length is here
  (void)Field_varstring::store((const char *)buff + HA_KEY_BLOB_LENGTH, length,
                               field_charset);
}

int Field_varstring::cmp_binary(const uchar *a_ptr, const uchar *b_ptr,
                                uint32 max_length) const {
  uint32 a_length, b_length;

  if (length_bytes == 1) {
    a_length = (uint)*a_ptr;
    b_length = (uint)*b_ptr;
  } else {
    a_length = uint2korr(a_ptr);
    b_length = uint2korr(b_ptr);
  }
  a_length = std::min(a_length, max_length);
  b_length = std::min(b_length, max_length);
  if (a_length != b_length) return 1;
  return memcmp(a_ptr + length_bytes, b_ptr + length_bytes, a_length);
}

Field *Field_varstring::new_field(MEM_ROOT *root, TABLE *new_table) const {
  Field_varstring *res =
      down_cast<Field_varstring *>(Field::new_field(root, new_table));
  if (res) res->length_bytes = length_bytes;
  return res;
}

Field *Field_varstring::new_key_field(MEM_ROOT *root, TABLE *new_table,
                                      uchar *new_ptr, uchar *new_null_ptr,
                                      uint new_null_bit) const {
  Field_varstring *res;
  if ((res = (Field_varstring *)Field::new_key_field(
           root, new_table, new_ptr, new_null_ptr, new_null_bit))) {
    /* Keys length prefixes are always packed with 2 bytes */
    res->length_bytes = 2;
  }
  return res;
}

uint Field_varstring::is_equal(const Create_field *new_field) const {
  if (change_prevents_inplace(*this, *new_field)) {
    return IS_EQUAL_NO;
  }

  if (new_field->charset == field_charset &&
      new_field->pack_length() == pack_length()) {
    return IS_EQUAL_YES;
  }

  return IS_EQUAL_PACK_LENGTH;
}

void Field_varstring::hash(ulong *nr, ulong *nr2) const {
  if (is_null()) {
    *nr ^= (*nr << 1) | 1;
  } else {
    uint len = length_bytes == 1 ? (uint)*ptr : uint2korr(ptr);
    const CHARSET_INFO *cs = charset();
    uint64 tmp1 = *nr;
    uint64 tmp2 = *nr2;
    cs->coll->hash_sort(cs, ptr + length_bytes, len, &tmp1, &tmp2);

    // NOTE: This truncates to 32-bit on Windows, to keep on-disk stability.
    *nr = static_cast<ulong>(tmp1);
    *nr2 = static_cast<ulong>(tmp2);
  }
}

/****************************************************************************
** blob type
** A blob is saved as a length and a pointer. The length is stored in the
** packlength slot and may be from 1-4.
****************************************************************************/

Field_blob::Field_blob(uint32 packlength_arg)
    : Field_longstr(nullptr, 0, &dummy_null_buffer, 0, NONE, "temp",
                    system_charset_info),
      packlength(packlength_arg),
      m_keep_old_value(false) {}

Field_blob::Field_blob(uchar *ptr_arg, uchar *null_ptr_arg, uchar null_bit_arg,
                       uchar auto_flags_arg, const char *field_name_arg,
                       TABLE_SHARE *share, uint blob_pack_length,
                       const CHARSET_INFO *cs)
    : Field_longstr(ptr_arg, BLOB_PACK_LENGTH_TO_MAX_LENGH(blob_pack_length),
                    null_ptr_arg, null_bit_arg, auto_flags_arg, field_name_arg,
                    cs),
      packlength(blob_pack_length),
      m_keep_old_value(false) {
  DBUG_ASSERT(blob_pack_length <=
              4);  // Only pack lengths 1-4 supported currently
  flags |= BLOB_FLAG;
  share->blob_fields++;
  /* TODO: why do not fill table->s->blob_field array here? */
}

void store_blob_length(uchar *i_ptr, uint i_packlength, uint32 i_number,
                              bool low_byte_first) {
  switch (i_packlength) {
    case 1:
      i_ptr[0] = (uchar)i_number;
      break;
    case 2:
      if (low_byte_first)
        int2store(i_ptr, (unsigned short)i_number);
      else
        shortstore(i_ptr, (unsigned short)i_number);
      break;
    case 3:
      int3store(i_ptr, i_number);
      break;
    case 4:
      if (low_byte_first)
        int4store(i_ptr, i_number);
      else
        longstore(i_ptr, i_number);
  }
}

uint32 Field_blob::get_length(const uchar *pos, uint packlength_arg,
                              bool low_byte_first) {
  switch (packlength_arg) {
    case 1:
      return (uint32)pos[0];
    case 2: {
      if (low_byte_first)
        return uint2korr(pos);
      else
        return ushortget(pos);
    }
    case 3:
      return uint3korr(pos);
    case 4: {
      if (low_byte_first)
        return uint4korr(pos);
      else
        return ulongget(pos);
    }
  }
  /* When expanding this, see also MAX_FIELD_BLOBLENGTH. */
  return 0;  // Impossible
}

/**
  Store a blob value to memory storage.
  @param      from         the string value to store.
  @param      length       length of the string value.
  @param      cs           character set of the string value.
  @param      max_length   Cut at this length safely (multibyte aware).
*/
type_conversion_status Field_blob::store_to_mem(const char *from, size_t length,
                                                const CHARSET_INFO *cs,
                                                size_t max_length,
                                                Blob_mem_storage *) {
  /*
    We don't need to support escaping or character set conversions here,
    because store_to_mem() is currently called only when we process
    queries having GROUP_CONCAT with ORDER BY or DISTINCT,
    hence some assersions:
  */
  DBUG_ASSERT(field_charset == cs);
  DBUG_ASSERT(length <= max_data_length());

  if (length > max_length) {
    int well_formed_error;
    length = cs->cset->well_formed_len(cs, from, from + max_length, length,
                                       &well_formed_error);
    table->blob_storage->set_truncated_value(true);
  }
  char *tmp;
  if (!(tmp = table->blob_storage->store(from, length))) {
    memset(ptr, 0, Field_blob::pack_length());
    return TYPE_ERR_OOM;
  }
  store_ptr_and_length(tmp, length);
  return TYPE_OK;
}

type_conversion_status Field_blob::store_internal(const char *from,
                                                  size_t length,
                                                  const CHARSET_INFO *cs) {
  size_t new_length;
  char buff[STRING_BUFFER_USUAL_SIZE], *tmp;
  String tmpstr(buff, sizeof(buff), &my_charset_bin);

  /*
    If the 'from' address is in the range of the temporary 'value'-
    object we need to copy the content to a different location or it will be
    invalidated when the 'value'-object is reallocated to make room for
    the new character set.
  */
  if (from >= value.ptr() && from <= value.ptr() + value.length()) {
    /*
      If content of the 'from'-address is cached in the 'value'-object
      it is possible that the content needs a character conversion.
    */
    if (!String::needs_conversion_on_storage(length, cs, field_charset)) {
      store_ptr_and_length(from, length);
      return TYPE_OK;
    }
    if (tmpstr.copy(from, length, cs)) goto oom_error;
    from = tmpstr.ptr();
  }

  new_length = min<size_t>(max_data_length(), field_charset->mbmaxlen * length);

  if (value.alloc(new_length)) goto oom_error;

  tmp = value.ptr();

  {
    const char *well_formed_error_pos;
    const char *cannot_convert_error_pos;
    const char *from_end_pos;
    /*
      "length" is OK as "nchars" argument to well_formed_copy_nchars as this
      is never used to limit the length of the data. The cut of long data
      is done with the new_length value.
    */
    size_t copy_length = field_well_formed_copy_nchars(
        field_charset, tmp, new_length, cs, from, length, length,
        &well_formed_error_pos, &cannot_convert_error_pos, &from_end_pos);

    store_ptr_and_length(tmp, copy_length);
    return check_string_copy_error(well_formed_error_pos,
                                   cannot_convert_error_pos, from_end_pos,
                                   from + length, true, cs);
  }

oom_error:
  /* Fatal OOM error */
  memset(ptr, 0, Field_blob::pack_length());
  return TYPE_ERR_OOM;
}

type_conversion_status Field_blob::store(const char *from, size_t length,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;

  if (table->blob_storage)  // GROUP_CONCAT with ORDER BY | DISTINCT
    return store_to_mem(from, length, cs,
                        table->in_use->variables.group_concat_max_len,
                        table->blob_storage);

  return store_internal(from, length, cs);
}

type_conversion_status Field_blob::store(double nr) {
  const CHARSET_INFO *cs = charset();
  value.set_real(nr, DECIMAL_NOT_SPECIFIED, cs);
  return Field_blob::store(value.ptr(), value.length(), cs);
}

type_conversion_status Field_blob::store(longlong nr, bool unsigned_val) {
  const CHARSET_INFO *cs = charset();
  value.set_int(nr, unsigned_val, cs);
  return Field_blob::store(value.ptr(), value.length(), cs);
}

type_conversion_status Field_blob::store(const Field *from) {
  from->val_str(&value);

  /*
    Copy value if copy_blobs is set, or source is part of the table's
    writeset.
  */
  if (table->copy_blobs || (!value.is_alloced() && from->is_updatable()))
    value.copy();

  return store(value.ptr(), value.length(), from->charset());
}

double Field_blob::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  const char *blob = pointer_cast<const char *>(get_ptr());
  if (blob == nullptr) return 0.0;

  int not_used;
  const char *end_not_used;
  return my_strntod(charset(), blob, get_length(ptr), &end_not_used, &not_used);
}

longlong Field_blob::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  const char *blob = pointer_cast<const char *>(get_ptr());
  if (blob == nullptr) return 0;

  int not_used;
  return my_strntoll(charset(), blob, get_length(ptr), 10, nullptr, &not_used);
}

String *Field_blob::val_str(String *, String *val_ptr) const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  const char *blob = pointer_cast<const char *>(get_ptr());
  if (blob == nullptr)
    val_ptr->set("", 0, charset());  // A bit safer than ->length(0)
  else
    val_ptr->set(blob, get_length(ptr), charset());
  return val_ptr;
}

my_decimal *Field_blob::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  size_t length;
  const char *blob = pointer_cast<const char *>(get_ptr());
  if (!blob) {
    blob = "";
    length = 0;
  } else
    length = get_length(ptr);

  str2my_decimal(E_DEC_FATAL_ERROR, blob, length, charset(), decimal_value);
  return decimal_value;
}

int Field_blob::cmp(const uchar *a, uint32 a_length, const uchar *b,
                    uint32 b_length) const {
  return field_charset->coll->strnncollsp(field_charset, a, a_length, b,
                                          b_length);
}

int Field_blob::cmp_max(const uchar *a_ptr, const uchar *b_ptr,
                        uint max_length) const {
  const uchar *blob1 = get_blob_data(a_ptr + packlength);
  const uchar *blob2 = get_blob_data(b_ptr + packlength);
  uint32 a_len = min(get_length(a_ptr), max_length);
  uint32 b_len = min(get_length(b_ptr), max_length);
  return Field_blob::cmp(blob1, a_len, blob2, b_len);
}

int Field_blob::cmp_binary(const uchar *a_ptr, const uchar *b_ptr,
                           uint32 max_length) const {
  const uchar *a = get_blob_data(a_ptr + packlength);
  const uchar *b = get_blob_data(b_ptr + packlength);
  uint32 a_length = min(get_length(a_ptr), max_length);
  uint32 b_length = min(get_length(b_ptr), max_length);
  const uint32 min_a_b = min(a_length, b_length);
  uint diff = min_a_b == 0 ? 0 : memcmp(a, b, min_a_b);  // memcmp(a, b, 0) == 0
  return diff ? diff : (int)(a_length - b_length);
}

/* The following is used only when comparing a key */

size_t Field_blob::get_key_image(uchar *buff, size_t length,
                                 imagetype type_arg) const {
  uint32 blob_length = get_length();
  const uchar *const blob = get_ptr();

  if (type_arg == itMBR) {
    const uint image_length = SIZEOF_STORED_DOUBLE * 4;

    if (blob_length < SRID_SIZE) {
      memset(buff, 0, image_length);
      return image_length;
    }
    gis::srid_t srid = uint4korr(blob);
    const dd::Spatial_reference_system *srs = nullptr;
    dd::cache::Dictionary_client::Auto_releaser m_releaser(
        current_thd->dd_client());
    Srs_fetcher fetcher(current_thd);
    if (srid != 0) fetcher.acquire(srid, &srs);
    if (get_mbr_from_store(srs, blob, blob_length, 2,
                           pointer_cast<double *>(buff), &srid)) {
      memset(buff, 0, image_length);
    } else {
      // get_mbr_from_store returns the MBR in machine byte order, but buff
      // should always be in little-endian order.
      float8store(buff, *pointer_cast<double *>(buff));
      float8store(buff + 8, *(pointer_cast<double *>(buff) + 1));
      float8store(buff + 16, *(pointer_cast<double *>(buff) + 2));
      float8store(buff + 24, *(pointer_cast<double *>(buff) + 3));
    }

    return image_length;
  }

  uint local_char_length = length / field_charset->mbmaxlen;
  local_char_length =
      my_charpos(field_charset, blob, blob + blob_length, local_char_length);
  blob_length = std::min(blob_length, local_char_length);

  if ((uint32)length > blob_length) {
    /*
      Must clear this as we do a memcmp in opt_range.cc to detect
      identical keys
    */
    memset(buff + HA_KEY_BLOB_LENGTH + blob_length, 0, (length - blob_length));
    length = (uint)blob_length;
  }
  int2store(buff, static_cast<uint16>(length));
  if (length > 0) memcpy(buff + HA_KEY_BLOB_LENGTH, blob, length);
  return HA_KEY_BLOB_LENGTH + length;
}

void Field_blob::set_key_image(const uchar *buff, size_t length) {
  length = uint2korr(buff);
  (void)Field_blob::store((const char *)buff + HA_KEY_BLOB_LENGTH, length,
                          field_charset);
}

int Field_blob::key_cmp(const uchar *key_ptr, uint max_key_length) const {
  uint32 blob_length = get_length(ptr);
  const uchar *blob1 = get_ptr();
  const CHARSET_INFO *cs = charset();
  uint local_char_length = max_key_length / cs->mbmaxlen;
  local_char_length =
      my_charpos(cs, blob1, blob1 + blob_length, local_char_length);
  blob_length = min(blob_length, local_char_length);
  return Field_blob::cmp(blob1, blob_length, key_ptr + HA_KEY_BLOB_LENGTH,
                         uint2korr(key_ptr));
}

int Field_blob::key_cmp(const uchar *a, const uchar *b) const {
  return Field_blob::cmp(a + HA_KEY_BLOB_LENGTH, uint2korr(a),
                         b + HA_KEY_BLOB_LENGTH, uint2korr(b));
}

/**
   Save the field metadata for blob fields.

   Saves the pack length in the first byte of the field metadata array
   at index of *metadata_ptr.

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_blob::do_save_field_metadata(uchar *metadata_ptr) const {
  DBUG_TRACE;
  *metadata_ptr = pack_length_no_ptr();
  DBUG_PRINT("debug", ("metadata: %u (pack_length_no_ptr)", *metadata_ptr));
  return 1;
}

uint32 Field_blob::sort_length() const { return 0xFFFFFFFFu; }

size_t Field_blob::make_sort_key(uchar *to, size_t length) const {
  static const uchar EMPTY_BLOB[1] = {0};
  uint32 blob_length = get_length();

  const int flags =
      (field_charset->pad_attribute == NO_PAD) ? 0 : MY_STRXFRM_PAD_TO_MAXLEN;

  const uchar *blob = blob_length > 0 ? get_ptr() : EMPTY_BLOB;

  return field_charset->coll->strnxfrm(field_charset, to, length, length, blob,
                                       blob_length, flags);
}

void Field_blob::sql_type(String &res) const {
  const char *str;
  uint length;
  switch (packlength) {
    default:
      str = "tiny";
      length = 4;
      break;
    case 2:
      str = "";
      length = 0;
      break;
    case 3:
      str = "medium";
      length = 6;
      break;
    case 4:
      str = "long";
      length = 4;
      break;
  }
  res.set_ascii(str, length);
  if (charset() == &my_charset_bin)
    res.append(STRING_WITH_LEN("blob"));
  else {
    res.append(STRING_WITH_LEN("text"));
  }
}

bool Field_blob::copy() {
  const uint32 length = get_length();
  if (value.copy(pointer_cast<const char *>(get_ptr()), length, charset())) {
    Field_blob::reset();
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), length);
    return true;
  }
  DBUG_ASSERT(value.length() == length);
  set_ptr(length, pointer_cast<const uchar *>(value.ptr()));
  return false;
}

uchar *Field_blob::pack(uchar *to, const uchar *from, uint max_length,
                        bool low_byte_first) const {
  uint32 length = get_length(from);  // Length of from string

  /*
    Store max length, which will occupy packlength bytes.
  */
  uchar len_buf[4];
  DBUG_ASSERT(packlength <= sizeof(len_buf));
  store_blob_length(len_buf, packlength, length, low_byte_first);

  if (packlength >= max_length) {
    memcpy(to, len_buf, max_length);
    return to + max_length;
  }

  memcpy(to, len_buf, packlength);

  /*
    Store the actual blob data, which will occupy 'length' bytes.
   */
  uint32 store_length = min(length, max_length - packlength);
  if (store_length > 0) {
    memcpy(to + packlength, get_blob_data(from + packlength), store_length);
  }

  return to + packlength + store_length;
}

uchar *Field_blob::pack_with_metadata_bytes(uchar *to, const uchar *from,
                                            uint max_length) const {
  uint32 length = get_length(from);  // Length of from string

  /*
    Store max length, which will occupy packlength bytes. If the max
    length given is smaller than the actual length of the blob, we
    just store the initial bytes of the blob.
   */
  store_blob_length(to, packlength, min(length, max_length),
                    /*low_byte_first=*/true);

  /*
    Store the actual blob data, which will occupy 'length' bytes.
   */
  if (length > 0) {
    memcpy(to + packlength, get_blob_data(from + packlength), length);
  }
  return to + packlength + length;
}

/**
   Unpack a blob field from row data.

   This method is used to unpack a blob field from a master whose size of
   the field is less than that of the slave. Note: This method is included
   to satisfy inheritance rules, but is not needed for blob fields. It
   simply is used as a pass-through to the original unpack() method for
   blob fields.

   @param   from       Source of the data
   @param   param_data The "metadata", as stored in the Table_map_log_event
                       for this field. This metadata is the number of bytes
                       used to represent the length of the blob (1, 2, 3, or
                       4).
   @param low_byte_first If true, the length should be unpacked in
   little-endian format, otherwise in the machine's native order.

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field_blob::unpack(uchar *, const uchar *from, uint param_data,
                                bool low_byte_first) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("from: %p;"
                       " param_data: %u; low_byte_first: %d",
                       from, param_data, low_byte_first));
  uint const master_packlength =
      param_data > 0 ? param_data & 0xFF : packlength;
  uint32 const length = get_length(from, master_packlength, low_byte_first);
  DBUG_DUMP("packed", from, length + master_packlength);
  bitmap_set_bit(table->write_set, field_index);
  Field_blob::store(pointer_cast<const char *>(from) + master_packlength,
                    length, field_charset);
  DBUG_DUMP("field", ptr, pack_length() /* len bytes + ptr bytes */);
  DBUG_DUMP("value", get_ptr(), length /* the blob value length */);
  return from + master_packlength + length;
}

uint Field_blob::max_packed_col_length() const {
  switch (packlength) {
    case 1:
    case 2:
    case 3:
      return packlength + (1u << (packlength * 8)) - 1;
    case 4:
      return UINT_MAX;
    default:
      DBUG_ASSERT(false);
      return UINT_MAX;
  }
}

uint Field_blob::is_equal(const Create_field *new_field) const {
  // Can't use change_prevents_inplace() here as it uses
  // sql_type_prevents_inplace() which checks real_type(), and
  // Field_blob::real_type() does NOT return the actual blob type as
  // one could expect, so we have to check against
  // get_blob_type_from_length(max_data_length() instead.
  // length_prevents_inplace() is less strict than pack_length
  // equality so would be redundant here.
  if (new_field->sql_type != get_blob_type_from_length(max_data_length()) ||
      new_field->pack_length() != pack_length() ||
      charset_prevents_inplace(*this, *new_field)) {
    return IS_EQUAL_NO;
  }

  if (new_field->charset == field_charset) {
    return IS_EQUAL_YES;
  }

  return IS_EQUAL_PACK_LENGTH;
}

void Field_geom::sql_type(String &res) const {
  const CHARSET_INFO *cs = &my_charset_latin1;
  switch (geom_type) {
    case GEOM_POINT:
      res.set(STRING_WITH_LEN("point"), cs);
      break;
    case GEOM_LINESTRING:
      res.set(STRING_WITH_LEN("linestring"), cs);
      break;
    case GEOM_POLYGON:
      res.set(STRING_WITH_LEN("polygon"), cs);
      break;
    case GEOM_MULTIPOINT:
      res.set(STRING_WITH_LEN("multipoint"), cs);
      break;
    case GEOM_MULTILINESTRING:
      res.set(STRING_WITH_LEN("multilinestring"), cs);
      break;
    case GEOM_MULTIPOLYGON:
      res.set(STRING_WITH_LEN("multipolygon"), cs);
      break;
    case GEOM_GEOMETRYCOLLECTION:
      res.set(STRING_WITH_LEN("geomcollection"), cs);
      break;
    default:
      res.set(STRING_WITH_LEN("geometry"), cs);
  }
}

type_conversion_status Field_geom::store(double) {
  my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
  return TYPE_ERR_BAD_VALUE;
}

type_conversion_status Field_geom::store(longlong, bool) {
  my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
  return TYPE_ERR_BAD_VALUE;
}

type_conversion_status Field_geom::store_decimal(const my_decimal *) {
  my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
  return TYPE_ERR_BAD_VALUE;
}

type_conversion_status Field_geom::store(const char *from, size_t length,
                                         const CHARSET_INFO *cs) {
  if (length < SRID_SIZE + WKB_HEADER_SIZE + sizeof(uint32)) {
    memset(ptr, 0, Field_blob::pack_length());
    my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
    return TYPE_ERR_BAD_VALUE;
  }

  return Field_blob::store(from, length, cs);
}

type_conversion_status Field_geom::store_internal(const char *from,
                                                  size_t length,
                                                  const CHARSET_INFO *cs) {
  // Check that the given WKB
  // 1. is at least 13 bytes long (length of GEOMETRYCOLLECTION EMPTY)
  // 2. isn't marked as bad geometry data
  // 3. isn't shorter than empty geometrycollection
  // 4. is a valid geometry type
  // 5. is well formed
  if (length < 13 ||                                                   // 1
      from == Geometry::bad_geometry_data.ptr() ||                     // 2
      length < SRID_SIZE + WKB_HEADER_SIZE + sizeof(uint32) ||         // 3
      !Geometry::is_valid_geotype(uint4korr(from + SRID_SIZE + 1)) ||  // 4
      !Geometry::is_well_formed(from, length,                          // 5
                                geometry_type_to_wkb_type(geom_type),
                                Geometry::wkb_ndr)) {
    memset(ptr, 0, Field_blob::pack_length());
    my_error(ER_CANT_CREATE_GEOMETRY_OBJECT, MYF(0));
    return TYPE_ERR_BAD_VALUE;
  }

  /*
    Check that the SRID of the geometry matches the expected SRID for this
    field
  */
  if (get_srid().has_value()) {
    gis::srid_t geometry_srid = uint4korr(from);
    if (geometry_srid != get_srid().value()) {
      memset(ptr, 0, Field_blob::pack_length());
      my_error(ER_WRONG_SRID_FOR_COLUMN, MYF(0), field_name, geometry_srid,
               get_srid().value());
      return TYPE_ERR_BAD_VALUE;
    }
  }

  if (table->copy_blobs || length <= MAX_FIELD_WIDTH) {  // Must make a copy
    value.copy(from, length, cs);
    from = value.ptr();
  }

  store_ptr_and_length(from, length);
  return TYPE_OK;
}

uint Field_geom::is_equal(const Create_field *new_field) const {
  return new_field->sql_type == real_type() &&
         new_field->geom_type == get_geometry_type() &&
         new_field->charset == field_charset &&
         new_field->pack_length() == pack_length();
}

/**
  Get the type of this field (json).
  @param str  the string that receives the type
*/
void Field_json::sql_type(String &str) const {
  str.set_ascii(STRING_WITH_LEN("json"));
}

/// Create a shallow clone of this field in the specified MEM_ROOT.
Field_json *Field_json::clone(MEM_ROOT *mem_root) const {
  DBUG_ASSERT(type() == MYSQL_TYPE_JSON);
  return new (mem_root) Field_json(*this);
}

/**
  Check if a new field is compatible with this one.
  @param new_field  the new field
  @return true if new_field is compatible with this field, false otherwise
*/
uint Field_json::is_equal(const Create_field *new_field) const {
  // All JSON fields are compatible with each other.
  return (new_field->sql_type == real_type());
}

/**
  Store data in this JSON field.

  JSON data is usually stored using store(Field_json*) or store_json(), so this
  function will only be called if non-JSON data is attempted stored in a JSON
  field. This results in an error in most cases.

  It will attempt to parse the string (unless it's binary) as JSON text, and
  store a binary representation of JSON document if the string could be parsed.

  Note that we override store() and not store_internal() because
  Field_blob::store() contains logic that bypasses store_internal() in
  some cases we care about. In particular:

  - When supplied an empty string, we want to raise a JSON syntax
    error instead of silently inserting an empty byte string.

  - When called from GROUP_CONCAT with ORDER BY or DISTINCT, we want
    to do the same data conversion as usual, whereas
    Field_blob::store() jumps directly to Field_blob::store_to_mem()
    with the unprocessed input data.

  @param from   the start of the data to be stored
  @param length the length of the data
  @param cs     the character set of the data
  @return zero on success, non-zero on failure
*/
type_conversion_status Field_json::store(const char *from, size_t length,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;

  /*
    First clear the field so that it doesn't contain garbage if we
    return with an error. Some callers continue for a while even after
    an error has been raised, and they could get into trouble if the
    field contains garbage.
  */
  reset();

  const char *s;
  size_t ss;
  String v(from, length, cs);

  if (ensure_utf8mb4(v, &value, &s, &ss, true)) {
    return TYPE_ERR_BAD_VALUE;
  }

  const char *parse_err;
  size_t err_offset;
  std::unique_ptr<Json_dom> dom(
      Json_dom::parse(s, ss, false, &parse_err, &err_offset));

  if (dom.get() == nullptr) {
    if (parse_err != nullptr) {
      // Syntax error.
      invalid_text(parse_err, err_offset);
    }
    return TYPE_ERR_BAD_VALUE;
  }

  if (json_binary::serialize(table->in_use, dom.get(), &value))
    return TYPE_ERR_BAD_VALUE;

  return store_binary(value.ptr(), value.length());
}

/**
  Helper function for raising an error when trying to store a value
  into a JSON column, and that value needs to be cast to JSON before
  it can be stored.
*/
type_conversion_status Field_json::unsupported_conversion() {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  invalid_text("not a JSON text, may need CAST", 0);
  return TYPE_ERR_BAD_VALUE;
}

/**
  Store the provided JSON binary data in this field.

  @param[in] data    pointer to JSON binary data
  @param[in] length  the length of the binary data
  @return zero on success, non-zero on failure
*/
type_conversion_status Field_json::store_binary(const char *data,
                                                size_t length) {
  /*
    We expect that a valid binary representation of a JSON document is
    passed to us.
  */
  DBUG_ASSERT(json_binary::parse_binary(data, length).is_valid());

  if (length > UINT_MAX32) {
    /* purecov: begin inspected */
    my_error(ER_JSON_VALUE_TOO_BIG, MYF(0));
    return TYPE_ERR_BAD_VALUE;
    /* purecov: end */
  }

  return Field_blob::store(data, length, field_charset);
}

/// Store a double in a JSON field. Will raise an error for now.
type_conversion_status Field_json::store(double) {
  return unsupported_conversion();
}

/// Store an integer in a JSON field. Will raise an error for now.
type_conversion_status Field_json::store(longlong, bool) {
  return unsupported_conversion();
}

/// Store a decimal in a JSON field. Will raise an error for now.
type_conversion_status Field_json::store_decimal(const my_decimal *) {
  return unsupported_conversion();
}

/// Store a TIME value in a JSON field. Will raise an error for now.
type_conversion_status Field_json::store_time(MYSQL_TIME *, uint8) {
  return unsupported_conversion();
}

/**
  Store a JSON value as binary.

  @param json  the JSON value to store
  @return zero on success, non-zero otherwise
*/
type_conversion_status Field_json::store_json(const Json_wrapper *json) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;

  /*
    We want to serialize the JSON value directly into Field_blob::value if
    possible, so that we don't have to copy it there afterwards.

    If the Json_wrapper is pointing to a document that already lives in
    Field_blob::value, it isn't safe to do so, since the source buffer and
    destination buffer is the same. This can happen if an UPDATE statement
    updates the same JSON column twice and the second update of the column
    reads data that the first update wrote to the output buffer. For example:

      UPDATE t SET json_col = <something>, json_col = json_col->'$.path'

    In that case, we serialize into a temporary string, which is later copied
    into Field_blob::value by store_binary().
  */
  StringBuffer<STRING_BUFFER_USUAL_SIZE> tmpstr;
  String *buffer = json->is_binary_backed_by(&value) ? &tmpstr : &value;

  if (json->to_binary(table->in_use, buffer)) return TYPE_ERR_BAD_VALUE;

  return store_binary(buffer->ptr(), buffer->length());
}

/**
  Copy the contents of a non-null JSON field into this field.

  @param[in] field the field to copy data from
  @return zero on success, non-zero on failure
*/
type_conversion_status Field_json::store(const Field_json *field) {
  /*
    The callers of this function have already checked for null, so we
    don't need to handle it here for now. Assert that field is not
    null.
  */
  DBUG_ASSERT(!field->is_null());

  String tmp;
  String *s = field->Field_blob::val_str(&tmp, &tmp);
  return store_binary(s->ptr(), s->length());
}

bool Field_json::val_json(Json_wrapper *wr) const {
  DBUG_TRACE;
  ASSERT_COLUMN_MARKED_FOR_READ;

  String tmp;
  String *s = Field_blob::val_str(&tmp, &tmp);

  json_binary::Value v(json_binary::parse_binary(s->ptr(), s->length()));
  if (v.type() == json_binary::Value::ERROR) {
    /* purecov: begin inspected */
    my_error(ER_INVALID_JSON_BINARY_DATA, MYF(0));
    return true;
    /* purecov: end */
  }

  *wr = Json_wrapper(v);
  return false;
}

longlong Field_json::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  Json_wrapper wr;
  if (val_json(&wr)) return 0; /* purecov: inspected */

  return wr.coerce_int(field_name);
}

double Field_json::val_real() const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  Json_wrapper wr;
  if (val_json(&wr)) return 0.0; /* purecov: inspected */

  return wr.coerce_real(field_name);
}

String *Field_json::val_str(String *buf1,
                            String *buf2 MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  /*
    Per contract of Field::val_str(String*,String*), buf1 should be
    used if the value needs to be converted to string, and buf2 should
    be used if the string value is already known. We need to convert,
    so use buf1.
  */
  buf1->length(0);

  Json_wrapper wr;
  if (val_json(&wr) || wr.to_string(buf1, true, field_name)) buf1->length(0);

  return buf1;
}

my_decimal *Field_json::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  Json_wrapper wr;
  if (val_json(&wr)) {
    /* purecov: begin inspected */
    my_decimal_set_zero(decimal_value);
    return decimal_value;
    /* purecov: end */
  }

  return wr.coerce_decimal(decimal_value, field_name);
}

bool Field_json::pack_diff(uchar **to, ulonglong value_format) const {
  DBUG_TRACE;

  const Json_diff_vector *diff_vector;
  get_diff_vector_and_length(value_format, &diff_vector);
  if (diff_vector == nullptr) return true;

  // We know the caller has allocated enough space, but we don't
  // know how much it is.  So just say that it is large, to
  // suppress bounds checks.
  String to_string((char *)*to, 0xffffFFFF, &my_charset_bin);
  to_string.length(0);
  if (diff_vector->write_binary(&to_string))
    // write_binary only returns true (error) in case it failed to
    // allocate memory. But now we know it will not try to
    // allocate.
    DBUG_ASSERT(0); /* purecov: inspected */

  // It should not have reallocated.
  DBUG_ASSERT(*to == (uchar *)to_string.ptr());

  *to += to_string.length();
  return false;
}

bool Field_json::is_before_image_equal_to_after_image() const {
  ptrdiff_t row_offset = table->record[1] - table->record[0];
  if (get_length(row_offset) != get_length()) return false;
  if (cmp(ptr, ptr + row_offset) != 0) return false;
  return true;
}

longlong Field_json::get_diff_vector_and_length(
    ulonglong value_options, const Json_diff_vector **diff_vector_p) const {
  DBUG_TRACE;

  if (is_null()) {
    DBUG_PRINT("info", ("Returning 0 because field is NULL"));
    if (diff_vector_p) *diff_vector_p = nullptr;
    return 0;
  }

  longlong length_of_full_format = get_length();
  longlong length = -1;
  const Json_diff_vector *diff_vector = nullptr;
  // Is the partial update smaller than the full update?
  DBUG_PRINT("info", ("length_of_full_format=%lu",
                      (unsigned long)length_of_full_format));

  // Is partial JSON updates enabled at all?
  if ((value_options & PARTIAL_JSON_UPDATES) == 0) {
    DBUG_PRINT("info", ("Using full JSON format because "
                        "binlog_row_value_format does not include "
                        "PARTIAL_JSON"));
    length = length_of_full_format;
  } else {
    // Was the optimizer able to compute a partial update?
    diff_vector = table->get_logical_diffs(this);
    if (diff_vector == nullptr) {
      // Are before-image and after-image equal?
      if (is_before_image_equal_to_after_image()) {
        DBUG_PRINT("info", ("Using empty Json_diff_vector because before-image "
                            "is different from after-image."));
        diff_vector = &Json_diff_vector::EMPTY_JSON_DIFF_VECTOR;
        length = diff_vector->binary_length();
      } else {
        DBUG_PRINT("info",
                   ("Using full JSON format because there is no diff vector "
                    "and before-image is different from after-image."));
        length = length_of_full_format;
      }
    } else {
      longlong length_of_diff_vector = diff_vector->binary_length();
      longlong length_of_empty_diff_vector =
          Json_diff_vector::EMPTY_JSON_DIFF_VECTOR.binary_length();
      DBUG_PRINT("info", ("length_of_diff_vector=%lu diff_vector->size()=%u",
                          (unsigned long)length_of_diff_vector,
                          (uint)diff_vector->size()));

      // If the vector is empty, no need to do the expensive comparison
      // between before-image and after-image.
      if (length_of_diff_vector == length_of_empty_diff_vector) {
        DBUG_PRINT("info",
                   ("Using empty Json_diff_vector provided by optimizer."));
        length = length_of_diff_vector;
      }
      // Are the before-image and the after-image equal? (This can
      // happen despite having a nonempty diff vector, in case
      // optimizer does not detect the equality)
      else if (is_before_image_equal_to_after_image()) {
        DBUG_PRINT("info", ("Using Json_diff_vector::EMPTY_JSON_DIFF_VECTOR "
                            "because the before-image equals the after-image "
                            "but the diff vector provided by the optimizer "
                            "is non-empty."));
        diff_vector = &Json_diff_vector::EMPTY_JSON_DIFF_VECTOR;
        length = length_of_diff_vector;
      }
      // Is the diff vector better than the full format?
      else if (length_of_diff_vector < length_of_full_format) {
        DBUG_PRINT("info", ("Using non-empty Json_diff_vector provided by "
                            "optimizer because it is smaller than "
                            "full format."));
        length = length_of_diff_vector;
      } else {
        DBUG_PRINT("info",
                   ("Using full JSON format because diff vector was not "
                    "smaller."));
        diff_vector = nullptr;
        length = length_of_full_format;
      }
    }
  }

  /*
    Can be equal to zero in the corner case where user inserted NULL
    value in a JSON NOT NULL column in non-strict mode.  The server
    will store this as a zero-length non-NULL object, and interpret it
    as a JSON 'null' literal.
  */
  DBUG_ASSERT(length >= 0);

  if (diff_vector_p != nullptr) *diff_vector_p = diff_vector;
  return length;
}

bool Field_json::unpack_diff(const uchar **from) {
  DBUG_TRACE;

  // Use a temporary mem_root so that the thread does not hold the
  // memory for the Json_diff_vector until the end of the statement.
  int memory_page_size = my_getpagesize();
  MEM_ROOT mem_root(key_memory_Slave_applier_json_diff_vector,
                    memory_page_size);

  Json_diff_vector diff_vector{Json_diff_vector::allocator_type(&mem_root)};

  // The caller should have verified that the buffer at 'from' is
  // sufficiently big to hold the whole diff_vector.
  if (diff_vector.read_binary(pointer_cast<const char **>(from), table,
                              field_name))
    return true;

  // Apply
  switch (apply_json_diffs(this, &diff_vector)) {
    case enum_json_diff_status::REJECTED:
      my_error(ER_COULD_NOT_APPLY_JSON_DIFF, MYF(0),
               (int)table->s->table_name.length, table->s->table_name.str,
               field_name);
      return true;
    case enum_json_diff_status::ERROR:
      return true; /* purecov: inspected */
    case enum_json_diff_status::SUCCESS:
      break;
  }
  return false;
}

bool Field_json::get_date(MYSQL_TIME *ltime, my_time_flags_t) const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  Json_wrapper wr;
  bool result = val_json(&wr) || wr.coerce_date(ltime, field_name);
  if (result)
    set_zero_time(ltime, MYSQL_TIMESTAMP_DATETIME); /* purecov: inspected */
  return result;
}

bool Field_json::get_time(MYSQL_TIME *ltime) const {
  ASSERT_COLUMN_MARKED_FOR_READ;

  Json_wrapper wr;
  bool result = val_json(&wr) || wr.coerce_time(ltime, field_name);
  if (result)
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME); /* purecov: inspected */
  return result;
}

int Field_json::cmp_binary(const uchar *a_ptr, const uchar *b_ptr,
                           uint32 /* max_length */) const {
  const char *a = pointer_cast<char *>(get_blob_data(a_ptr + packlength));
  const char *b = pointer_cast<char *>(get_blob_data(b_ptr + packlength));
  uint32 a_length = get_length(a_ptr);
  uint32 b_length = get_length(b_ptr);
  Json_wrapper aw(json_binary::parse_binary(a, a_length));
  Json_wrapper bw(json_binary::parse_binary(b, b_length));
  return aw.compare(bw);
}

size_t Field_json::make_sort_key(uchar *to, size_t length) const {
  Json_wrapper wr;
  if (val_json(&wr)) {
    return 0;
  }
  return wr.make_sort_key(to, length);
}

ulonglong Field_json::make_hash_key(ulonglong hash_val) const {
  Json_wrapper wr;
  if (val_json(&wr)) return hash_val; /* purecov: inspected */
  return wr.make_hash_key(hash_val);
}

const char *Field_json::get_binary(ptrdiff_t row_offset) const {
  return pointer_cast<char *>(get_blob_data(ptr + packlength + row_offset));
}

/****************************************************************************
** enum type.
** This is a string which only can have a selection of different values.
** If one uses this string in a number context one gets the type number.
****************************************************************************/

enum ha_base_keytype Field_enum::key_type() const {
  switch (packlength) {
    default:
      return HA_KEYTYPE_BINARY;
    case 2:
      return HA_KEYTYPE_USHORT_INT;
    case 3:
      return HA_KEYTYPE_UINT24;
    case 4:
      return HA_KEYTYPE_ULONG_INT;
    case 8:
      return HA_KEYTYPE_ULONGLONG;
  }
}

void Field_enum::store_type(ulonglong value) {
  switch (packlength) {
    case 1:
      ptr[0] = (uchar)value;
      break;
    case 2:
      if (table->s->db_low_byte_first)
        int2store(ptr, (unsigned short)value);
      else
        shortstore(ptr, (unsigned short)value);
      break;
    case 3:
      int3store(ptr, (long)value);
      break;
    case 4:
      if (table->s->db_low_byte_first)
        int4store(ptr, value);
      else
        longstore(ptr, (long)value);
      break;
    case 8:
      if (table->s->db_low_byte_first)
        int8store(ptr, value);
      else
        longlongstore(ptr, value);
      break;
  }
}

/**
  @note
    Storing a empty string in a enum field gives a warning
    (if there isn't a empty value in the enum)
*/

type_conversion_status Field_enum::store(const char *from, size_t length,
                                         const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int err = 0;
  type_conversion_status ret = TYPE_OK;
  char buff[STRING_BUFFER_USUAL_SIZE];
  String tmpstr(buff, sizeof(buff), &my_charset_bin);

  /* Convert character set if necessary */
  if (String::needs_conversion_on_storage(length, cs, field_charset)) {
    uint dummy_errors;
    tmpstr.copy(from, length, cs, field_charset, &dummy_errors);
    from = tmpstr.ptr();
    length = tmpstr.length();
  }

  /* Remove end space */
  length = field_charset->cset->lengthsp(field_charset, from, length);
  uint tmp = find_type2(typelib, from, length, field_charset);
  if (!tmp) {
    if (length < 6)  // Can't be more than 99999 enums
    {
      /* This is for reading numbers with LOAD DATA INFILE */
      const char *end;
      tmp = (uint)my_strntoul(cs, from, length, 10, &end, &err);
      if (err || end != from + length || tmp > typelib->count) {
        tmp = 0;
        set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
        ret = TYPE_WARN_TRUNCATED;
      }
      if (!table->in_use->check_for_truncated_fields) ret = TYPE_OK;
    } else
      set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
  }
  store_type((ulonglong)tmp);
  return ret;
}

type_conversion_status Field_enum::store(double nr) {
  return Field_enum::store((longlong)nr, false);
}

type_conversion_status Field_enum::store(longlong nr, bool) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  if ((ulonglong)nr > typelib->count || nr == 0) {
    set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    if (nr != 0 || table->in_use->check_for_truncated_fields) {
      nr = 0;
      error = TYPE_WARN_TRUNCATED;
    }
  }
  store_type((ulonglong)(uint)nr);
  return error;
}

double Field_enum::val_real() const { return (double)Field_enum::val_int(); }

my_decimal *Field_enum::val_decimal(my_decimal *decimal_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int2my_decimal(E_DEC_FATAL_ERROR, val_int(), false, decimal_value);
  return decimal_value;
}

// Utility function used by ::val_int() and ::cmp()
static longlong enum_val_int(const uchar *ptr, uint packlength,
                             bool low_byte_first) {
  switch (packlength) {
    case 1:
      return ptr[0];
    case 2: {
      if (low_byte_first)
        return uint2korr(ptr);
      else
        return ushortget(ptr);
    }
    case 3:
      return uint3korr(ptr);
    case 4: {
      if (low_byte_first)
        return uint4korr(ptr);
      else
        return ulongget(ptr);
    }
    case 8: {
      if (low_byte_first)
        return sint8korr(ptr);
      else
        return longlongget(ptr);
    }
  }
  return 0;  // impossible
}

longlong Field_enum::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  return enum_val_int(ptr, packlength, table->s->db_low_byte_first);
}

/**
   Save the field metadata for enum fields.

   Saves the real type in the first byte and the pack length in the
   second byte of the field metadata array at index of *metadata_ptr and
   *(metadata_ptr + 1).

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_enum::do_save_field_metadata(uchar *metadata_ptr) const {
  *metadata_ptr = real_type();
  *(metadata_ptr + 1) = pack_length();
  return 2;
}

String *Field_enum::val_str(String *val_buffer MY_ATTRIBUTE((unused)),
                            String *val_ptr) const {
  uint tmp = (uint)Field_enum::val_int();
  if (!tmp || tmp > typelib->count)
    val_ptr->set("", 0, field_charset);
  else
    val_ptr->set(typelib->type_names[tmp - 1], typelib->type_lengths[tmp - 1],
                 field_charset);
  return val_ptr;
}

int Field_enum::cmp(const uchar *a_ptr, const uchar *b_ptr) const {
  const longlong a =
      enum_val_int(a_ptr, packlength, table->s->db_low_byte_first);
  const longlong b =
      enum_val_int(b_ptr, packlength, table->s->db_low_byte_first);
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

size_t Field_enum::make_sort_key(uchar *to, size_t length) const {
#ifdef WORDS_BIGENDIAN
  if (!table->s->db_low_byte_first)
    copy_integer<true>(to, length, ptr, packlength, true);
  else
#endif
    copy_integer<false>(to, length, ptr, packlength, true);
  return length;
}

void Field_enum::sql_type(String &res) const {
  char buffer[255];
  String enum_item(buffer, sizeof(buffer), res.charset());

  res.length(0);
  res.append(STRING_WITH_LEN("enum("));

  bool flag = false;
  uint *len = typelib->type_lengths;
  for (const char **pos = typelib->type_names; *pos; pos++, len++) {
    uint dummy_errors;
    if (flag) res.append(',');
    /* convert to res.charset() == utf8, then quote */
    enum_item.copy(*pos, *len, charset(), res.charset(), &dummy_errors);

    const CHARSET_INFO *cs = res.charset();
    int well_formed_error = 42;
#ifndef DBUG_OFF
    size_t wl =
#endif
        cs->cset->well_formed_len(cs, enum_item.ptr(),
                                  enum_item.ptr() + enum_item.length(),
                                  enum_item.length(), &well_formed_error);
    DBUG_ASSERT(wl <= enum_item.length());
    if (well_formed_error) {
      // Append the hex literal instead
      res.append("x'");
      char b[6];
      const char *eip = enum_item.ptr();
      for (size_t i = 0; i < enum_item.length(); ++i) {
        unsigned char v = static_cast<unsigned char>(eip[i]);
        snprintf(b, sizeof(b), "%x", v);
        res.append(b);
      }
      res.append("'");
    } else {
      append_unescaped(&res, enum_item.ptr(), enum_item.length());
    }
    flag = true;
  }
  res.append(')');
}

Field *Field_enum::new_field(MEM_ROOT *root, TABLE *new_table) const {
  Field_enum *res = down_cast<Field_enum *>(Field::new_field(root, new_table));
  if (res) res->typelib = copy_typelib(root, typelib);
  return res;
}

/*
   set type.
   This is a string which can have a collection of different values.
   Each string value is separated with a ','.
   For example "One,two,five"
   If one uses this string in a number context one gets the bits as a longlong
   number.
*/

type_conversion_status Field_set::store(const char *from, size_t length,
                                        const CHARSET_INFO *cs) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  bool got_warning = false;
  int err = 0;
  type_conversion_status ret = TYPE_OK;
  const char *not_used;
  uint not_used2;
  char buff[STRING_BUFFER_USUAL_SIZE];
  String tmpstr(buff, sizeof(buff), &my_charset_bin);

  /* Convert character set if necessary */
  if (String::needs_conversion_on_storage(length, cs, field_charset)) {
    uint dummy_errors;
    tmpstr.copy(from, length, cs, field_charset, &dummy_errors);
    from = tmpstr.ptr();
    length = tmpstr.length();
  }
  ulonglong tmp = find_set(typelib, from, length, field_charset, &not_used,
                           &not_used2, &got_warning);
  if (!tmp && length && length < 22) {
    /* This is for reading numbers with LOAD DATA INFILE */
    const char *end;
    tmp = my_strntoull(cs, from, length, 10, &end, &err);
    if (err || end != from + length ||
        (typelib->count < 64 && tmp >= (1ULL << typelib->count))) {
      tmp = 0;
      set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
      ret = TYPE_WARN_TRUNCATED;
    }
  } else if (got_warning)
    set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
  store_type(tmp);
  return ret;
}

type_conversion_status Field_set::store(longlong nr, bool) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  type_conversion_status error = TYPE_OK;
  ulonglong max_nr;

  if (sizeof(ulonglong) * 8 <= typelib->count)
    max_nr = ULLONG_MAX;
  else
    max_nr = (1ULL << typelib->count) - 1;

  if ((ulonglong)nr > max_nr) {
    nr &= max_nr;
    set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    error = TYPE_WARN_TRUNCATED;
  }
  store_type((ulonglong)nr);
  return error;
}

String *Field_set::val_str(String *val_buffer,
                           String *val_ptr MY_ATTRIBUTE((unused))) const {
  ulonglong tmp = (ulonglong)Field_enum::val_int();
  uint bitnr = 0;

  /*
    Some callers expect *val_buffer to contain the result,
    so we assign to it, rather than doing 'return &empty_set_string.
  */
  *val_buffer = empty_set_string;
  if (tmp == 0) {
    return val_buffer;
  }

  val_buffer->set_charset(field_charset);
  val_buffer->length(0);

  while (tmp && bitnr < typelib->count) {
    if (tmp & 1) {
      if (val_buffer->length())
        val_buffer->append(&field_separator, 1, &my_charset_latin1);
      String str(typelib->type_names[bitnr], typelib->type_lengths[bitnr],
                 field_charset);
      val_buffer->append(str);
    }
    tmp >>= 1;
    bitnr++;
  }
  return val_buffer;
}

void Field_set::sql_type(String &res) const {
  char buffer[255];
  String set_item(buffer, sizeof(buffer), res.charset());

  res.length(0);
  res.append(STRING_WITH_LEN("set("));

  bool flag = false;
  uint *len = typelib->type_lengths;
  for (const char **pos = typelib->type_names; *pos; pos++, len++) {
    uint dummy_errors;
    if (flag) res.append(',');
    /* convert to res.charset() == utf8, then quote */
    set_item.copy(*pos, *len, charset(), res.charset(), &dummy_errors);
    append_unescaped(&res, set_item.ptr(), set_item.length());
    flag = true;
  }
  res.append(')');
}

/**
  @retval
    true   if the fields are equally defined
  @retval
    false  if the fields are unequally defined
*/

bool Field::eq_def(const Field *field) const {
  if (real_type() != field->real_type() || charset() != field->charset() ||
      pack_length() != field->pack_length())
    return false;
  return true;
}

/**
  Compare the first t1::count type names.

  @return true if the type names of t1 match those of t2. false otherwise.
*/

static bool compare_type_names(const CHARSET_INFO *charset, TYPELIB *t1,
                               TYPELIB *t2) {
  for (uint i = 0; i < t1->count; i++)
    if (my_strnncoll(charset, (const uchar *)t1->type_names[i],
                     t1->type_lengths[i], (const uchar *)t2->type_names[i],
                     t2->type_lengths[i]))
      return false;
  return true;
}

/**
  @return
  returns 1 if the fields are equally defined
*/

bool Field_enum::eq_def(const Field *field) const {
  TYPELIB *values;

  if (!Field::eq_def(field)) return false;

  values = down_cast<const Field_enum *>(field)->typelib;

  /* Definition must be strictly equal. */
  if (typelib->count != values->count) return false;

  return compare_type_names(field_charset, typelib, values);
}

/**
  Check whether two fields can be considered 'equal' for table
  alteration purposes. Fields are equal if they retain the same
  pack length and if new members are added to the end of the list.

  @return IS_EQUAL_YES if fields are compatible.
          IS_EQUAL_NO otherwise.
*/

uint Field_enum::is_equal(const Create_field *new_field) const {
  /*
    The fields are compatible if they have the same flags,
    type, charset and have the same underlying length.
  */
  if (change_prevents_inplace(*this, *new_field)) {
    DBUG_PRINT("inplace", ("change_prevents_inplace() -> IS_EQUAL_NO"));
    return IS_EQUAL_NO;
  }
  // Assert - either the charset is unchanged or the change is
  // inplace-compatible.

  TYPELIB *new_typelib = new_field->interval;

  /*
    Changing the definition of an ENUM or SET column by adding a new
    enumeration or set members to the end of the list of valid member
    values only alters table metadata and not table data.
  */
  if (typelib->count > new_typelib->count) {
    DBUG_PRINT("inplace", ("(typelib->count > new_typelib->count) -> "
                           "IS_EQUAL_NO"));
    return IS_EQUAL_NO;
  }

  /* Check whether there are modification before the end. Since we
     know that the charset change (if there is one) is
     inplace-compatible, it is safe to perform the comparison using the
     new charset.
  */
  if (!compare_type_names(new_field->charset, typelib, new_typelib)) {
    DBUG_PRINT("inplace", ("!compare_type_names() -> IS_EQUAL_NO"));
    return IS_EQUAL_NO;
  }

  DBUG_PRINT("inplace",
             ("Field_enum::is_equal() -> %s",
              (new_field->pack_length() == pack_length() ? "IS_EQUAL_YES"
                                                         : "IS_EQUAL_NO")));

  // Can't return IS_EQUAL_PACK_LENGTH here as using inplace leads to
  // assert when trying to insert set elements that use bits outside
  // the original length:
  // Assertion failure: rem0rec.cc:408:len <= fixed_len thread 140301224261376
  return (new_field->pack_length() == pack_length() ? IS_EQUAL_YES
                                                    : IS_EQUAL_NO);
}

uchar *Field_enum::pack(uchar *to, const uchar *from, uint max_length,
                        bool low_byte_first) const {
  DBUG_TRACE;
  DBUG_PRINT("debug", ("packlength: %d", packlength));
  DBUG_DUMP("from", from, packlength);

  switch (packlength) {
    case 1:
      if (max_length > 0) *to = *from;
      return to + 1;
    case 2:
      return pack_int16(to, from, max_length, low_byte_first);
    case 3:
      return pack_int24(to, from, max_length, low_byte_first);
    case 4:
      return pack_int32(to, from, max_length, low_byte_first);
    case 8:
      return pack_int64(to, from, max_length, low_byte_first);
    default:
      DBUG_ASSERT(0);
  }
  MY_ASSERT_UNREACHABLE();
  return nullptr;
}

const uchar *Field_enum::unpack(uchar *to, const uchar *from, uint,
                                bool low_byte_first) {
  DBUG_TRACE;
  DBUG_PRINT("debug", ("packlength: %d", packlength));
  DBUG_DUMP("from", from, packlength);

  switch (packlength) {
    case 1:
      *to = *from;
      return from + 1;

    case 2:
      return unpack_int16(to, from, low_byte_first);
    case 3:
      return unpack_int24(to, from, low_byte_first);
    case 4:
      return unpack_int32(to, from, low_byte_first);
    case 8:
      return unpack_int64(to, from, low_byte_first);
    default:
      DBUG_ASSERT(0);
  }
  MY_ASSERT_UNREACHABLE();
  return nullptr;
}

/**
  @return
  returns true if the fields are equally defined
*/
bool Field_num::eq_def(const Field *field) const {
  if (!Field::eq_def(field)) return false;
  const Field_num *from_num = down_cast<const Field_num *>(field);

  if (unsigned_flag != from_num->unsigned_flag ||
      (zerofill && !from_num->zerofill && !zero_pack()) || dec != from_num->dec)
    return false;
  return true;
}

/**
  Check whether two numeric fields can be considered 'equal' for table
  alteration purposes. Fields are equal if they are of the same type
  and retain the same pack length.
*/

uint Field_num::is_equal(const Create_field *new_field) const {
  return (
      (new_field->sql_type == real_type()) &&
      ((new_field->flags & UNSIGNED_FLAG) == (uint)(flags & UNSIGNED_FLAG)) &&
      ((new_field->flags & AUTO_INCREMENT_FLAG) ==
       (uint)(flags & AUTO_INCREMENT_FLAG)) &&
      (new_field->pack_length() == pack_length()));
}

/*
  Bit field.

  We store the first 0 - 6 uneven bits among the null bits
  at the start of the record. The rest bytes are stored in
  the record itself.

  For example:

  CREATE TABLE t1 (a int, b bit(17), c bit(21) not null, d bit(8));
  We would store data  as follows in the record:

  Byte        Bit
  1           7 - reserve for delete
              6 - null bit for 'a'
              5 - null bit for 'b'
              4 - first (high) bit of 'b'
              3 - first (high) bit of 'c'
              2 - second bit of 'c'
              1 - third bit of 'c'
              0 - forth bit of 'c'
  2           7 - firth bit of 'c'
              6 - null bit for 'd'
  3 - 6       four bytes for 'a'
  7 - 8       two bytes for 'b'
  9 - 10      two bytes for 'c'
  11          one byte for 'd'
*/

Field_bit::Field_bit(uchar *ptr_arg, uint32 len_arg, uchar *null_ptr_arg,
                     uchar null_bit_arg, uchar *bit_ptr_arg, uchar bit_ofs_arg,
                     uchar auto_flags_arg, const char *field_name_arg)
    : Field(ptr_arg, len_arg, null_ptr_arg, null_bit_arg, auto_flags_arg,
            field_name_arg),
      bit_ptr(bit_ptr_arg),
      bit_ofs(bit_ofs_arg),
      bit_len(len_arg & 7),
      bytes_in_rec(len_arg / 8) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("ptr_arg: %p, null_ptr_arg: %p, len_arg: %u, bit_len: "
                       "%u, bytes_in_rec: %u",
                       ptr_arg, null_ptr_arg, len_arg, bit_len, bytes_in_rec));
  flags |= UNSIGNED_FLAG;
  /*
    Ensure that Field::eq() can distinguish between two different bit fields.
    (two bit fields that are not null, may have same ptr and m_null_ptr)
  */
  if (!null_ptr_arg) null_bit = bit_ofs_arg;
}

void Field_bit::hash(ulong *nr, ulong *nr2) const {
  if (is_null()) {
    *nr ^= (*nr << 1) | 1;
  } else {
    const CHARSET_INFO *cs = &my_charset_bin;
    longlong value = Field_bit::val_int();
    uchar tmp[8];
    mi_int8store(tmp, value);

    uint64 tmp1 = *nr;
    uint64 tmp2 = *nr2;
    cs->coll->hash_sort(cs, tmp, 8, &tmp1, &tmp2);

    // NOTE: This truncates to 32-bit on Windows, to keep on-disk stability.
    *nr = static_cast<ulong>(tmp1);
    *nr2 = static_cast<ulong>(tmp2);
  }
}

Field *Field_bit::new_key_field(MEM_ROOT *root, TABLE *new_table,
                                uchar *new_ptr, uchar *new_null_ptr,
                                uint new_null_bit) const {
  Field_bit *res;
  if ((res = (Field_bit *)Field::new_key_field(root, new_table, new_ptr,
                                               new_null_ptr, new_null_bit))) {
    /* Move bits normally stored in null_pointer to new_ptr */
    res->bit_ptr = new_ptr;
    res->bit_ofs = 0;
    if (bit_len) res->ptr++;  // Store rest of data here
  }
  return res;
}

uint Field_bit::is_equal(const Create_field *new_field) const {
  return (new_field->sql_type == real_type() &&
          new_field->max_display_width_in_bytes() == max_display_length());
}

type_conversion_status Field_bit::reset() {
  memset(ptr, 0, bytes_in_rec);
  if (bit_ptr && (bit_len > 0))  // reset odd bits among null bits
    clr_rec_bits(bit_ptr, bit_ofs, bit_len);
  return TYPE_OK;
}

type_conversion_status Field_bit::store(const char *from, size_t length,
                                        const CHARSET_INFO *) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int delta;

  for (; length && !*from; from++, length--)
    ;  // skip left 0's
  delta = bytes_in_rec - static_cast<int>(length);

  /*
   *from should probably be treated like uint here see BUG#13727586
   */
  if (delta < -1 || (delta == -1 && (uchar)*from > ((1 << bit_len) - 1)) ||
      (!bit_len && delta < 0)) {
    set_rec_bits((1 << bit_len) - 1, bit_ptr, bit_ofs, bit_len);
    memset(ptr, 0xff, bytes_in_rec);
    if (table->in_use->is_strict_sql_mode())
      set_warning(Sql_condition::SL_WARNING, ER_DATA_TOO_LONG, 1);
    else
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  /* delta is >= -1 here */
  if (delta > 0) {
    if (bit_len) clr_rec_bits(bit_ptr, bit_ofs, bit_len);
    memset(ptr, 0, delta);
    memcpy(ptr + delta, from, length);
  } else if (delta == 0) {
    if (bit_len) clr_rec_bits(bit_ptr, bit_ofs, bit_len);
    memcpy(ptr, from, length);
  } else {
    if (bit_len) {
      set_rec_bits((uchar)*from, bit_ptr, bit_ofs, bit_len);
      from++;
    }
    memcpy(ptr, from, bytes_in_rec);
  }
  return TYPE_OK;
}

type_conversion_status Field_bit::store(double nr) {
  return Field_bit::store((longlong)nr, false);
}

type_conversion_status Field_bit::store(longlong nr, bool) {
  char buf[8];

  mi_int8store(buf, nr);
  return store(buf, 8, nullptr);
}

type_conversion_status Field_bit::store_decimal(const my_decimal *val) {
  bool has_overflow = false;
  longlong i = convert_decimal2longlong(val, true, &has_overflow);
  type_conversion_status res = store(i, true);
  return has_overflow ? TYPE_WARN_OUT_OF_RANGE : res;
}

double Field_bit::val_real() const { return (double)Field_bit::val_int(); }

longlong Field_bit::val_int() const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  ulonglong bits = 0;
  if (bit_len) {
    bits = get_rec_bits(bit_ptr, bit_ofs, bit_len);
    bits <<= (bytes_in_rec * 8);
  }

  switch (bytes_in_rec) {
    case 0:
      return bits;
    case 1:
      return bits | (ulonglong)ptr[0];
    case 2:
      return bits | mi_uint2korr(ptr);
    case 3:
      return bits | mi_uint3korr(ptr);
    case 4:
      return bits | mi_uint4korr(ptr);
    case 5:
      return bits | mi_uint5korr(ptr);
    case 6:
      return bits | mi_uint6korr(ptr);
    case 7:
      return bits | mi_uint7korr(ptr);
    default:
      return mi_uint8korr(ptr + bytes_in_rec - sizeof(longlong));
  }
}

String *Field_bit::val_str(String *val_buffer,
                           String *val_ptr MY_ATTRIBUTE((unused))) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  char buff[sizeof(longlong)];
  uint length = min<uint>(pack_length(), sizeof(longlong));
  ulonglong bits = val_int();
  mi_int8store(buff, bits);

  val_buffer->alloc(length);
  memcpy(val_buffer->ptr(), buff + 8 - length, length);
  val_buffer->length(length);
  val_buffer->set_charset(&my_charset_bin);
  return val_buffer;
}

my_decimal *Field_bit::val_decimal(my_decimal *deciaml_value) const {
  ASSERT_COLUMN_MARKED_FOR_READ;
  int2my_decimal(E_DEC_FATAL_ERROR, val_int(), true, deciaml_value);
  return deciaml_value;
}

/*
  Compare two bit fields using pointers within the record.
  SYNOPSIS
    cmp_max()
    a                 Pointer to field->ptr in first record
    b                 Pointer to field->ptr in second record
  DESCRIPTION
    This method is used from key_rec_cmp used by merge sorts used
    by partitioned index read and later other similar places.
    The a and b pointer must be pointers to the field in a record
    (not the table->record[0] necessarily)
*/
int Field_bit::cmp_max(const uchar *a, const uchar *b, uint) const {
  ptrdiff_t a_diff = a - ptr;
  ptrdiff_t b_diff = b - ptr;
  if (bit_len) {
    int flag;
    uchar bits_a = get_rec_bits(bit_ptr + a_diff, bit_ofs, bit_len);
    uchar bits_b = get_rec_bits(bit_ptr + b_diff, bit_ofs, bit_len);
    if ((flag = (int)(bits_a - bits_b))) return flag;
  }
  return memcmp(a, b, pack_length());
}

int Field_bit::key_cmp(const uchar *str, uint length) const {
  if (bit_len) {
    int flag;
    uchar bits = get_rec_bits(bit_ptr, bit_ofs, bit_len);
    if ((flag = (int)(bits - *str))) return flag;
    str++;
    length--;
  }
  return memcmp(ptr, str, length);
}

int Field_bit::cmp_offset(ptrdiff_t row_offset) const {
  if (bit_len) {
    int flag;
    uchar bits_a = get_rec_bits(bit_ptr, bit_ofs, bit_len);
    uchar bits_b = get_rec_bits(bit_ptr + row_offset, bit_ofs, bit_len);
    if ((flag = (int)(bits_a - bits_b))) return flag;
  }
  return memcmp(ptr, ptr + row_offset, bytes_in_rec);
}

size_t Field_bit::get_key_image(uchar *buff, size_t length, imagetype) const {
  if (bit_len) {
    uchar bits = get_rec_bits(bit_ptr, bit_ofs, bit_len);
    *buff++ = bits;
    length--;
  }
  size_t data_length = min(length, static_cast<size_t>(bytes_in_rec));
  memcpy(buff, ptr, data_length);
  return data_length + 1;
}

/**
   Save the field metadata for bit fields.

   Saves the bit length in the first byte and bytes in record in the
   second byte of the field metadata array at index of *metadata_ptr and
   *(metadata_ptr + 1).

   @param   metadata_ptr   First byte of field metadata

   @returns number of bytes written to metadata_ptr
*/
int Field_bit::do_save_field_metadata(uchar *metadata_ptr) const {
  DBUG_TRACE;
  DBUG_PRINT("debug", ("bit_len: %d, bytes_in_rec: %d", bit_len, bytes_in_rec));
  /*
    Since this class and Field_bit_as_char have different ideas of
    what should be stored here, we compute the values of the metadata
    explicitly using the field_length.
   */
  metadata_ptr[0] = field_length % 8;
  metadata_ptr[1] = field_length / 8;
  return 2;
}

/**
   Returns the number of bytes field uses in row-based replication
   row packed size.

   This method is used in row-based replication to determine the number
   of bytes that the field consumes in the row record format. This is
   used to skip fields in the master that do not exist on the slave.

   @param   field_metadata   Encoded size in field metadata

   @returns The size of the field based on the field metadata.
*/
uint Field_bit::pack_length_from_metadata(uint field_metadata) const {
  uint const from_len = (field_metadata >> 8U) & 0x00ff;
  uint const from_bit_len = field_metadata & 0x00ff;
  uint const source_size = from_len + ((from_bit_len > 0) ? 1 : 0);
  return (source_size);
}

/**
   Check to see if field size is compatible with destination.

   This method is used in row-based replication to verify that the slave's
   field size is less than or equal to the master's field size. The
   encoded field metadata (from the master or source) is decoded and compared
   to the size of this field (the slave or destination).

   @param   field_metadata   Encoded size in field metadata
   @param   mflags           Flags from the table map event for the table.
   @param   order_var        Pointer to variable where the order
                             between the source field and this field
                             will be returned.

   @return @c true
*/
bool Field_bit::compatible_field_size(uint field_metadata,
                                      Relay_log_info *MY_ATTRIBUTE((unused)),
                                      uint16 mflags, int *order_var) const {
  DBUG_TRACE;
  DBUG_ASSERT((field_metadata >> 16) == 0);
  uint from_bit_len = 8 * (field_metadata >> 8) + (field_metadata & 0xff);
  uint to_bit_len = max_display_length();
  DBUG_PRINT("debug",
             ("from_bit_len: %u, to_bit_len: %u", from_bit_len, to_bit_len));
  /*
    If the bit length exact flag is clear, we are dealing with an old
    master, so we allow some less strict behaviour if replicating by
    moving both bit lengths to an even multiple of 8.

    We do this by computing the number of bytes to store the field
    instead, and then compare the result.
   */
  if (!(mflags & Table_map_log_event::TM_BIT_LEN_EXACT_F)) {
    from_bit_len = (from_bit_len + 7) / 8;
    to_bit_len = (to_bit_len + 7) / 8;
  }

  *order_var = compare(from_bit_len, to_bit_len);
  return true;
}

void Field_bit::sql_type(String &res) const {
  const CHARSET_INFO *cs = res.charset();
  size_t length = cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                     "bit(%d)", (int)field_length);
  res.length(length);
}

uchar *Field_bit::pack(uchar *to, const uchar *from, uint max_length,
                       bool low_byte_first MY_ATTRIBUTE((unused))) const {
  if (max_length == 0) {
    return to + 1;
  }
  uint length;
  if (bit_len > 0) {
    /*
      We have the following:

      ptr        Points into a field in record R1
      from       Points to a field in a record R2
      bit_ptr    Points to the byte (in the null bytes) that holds the
                 odd bits of R1
      from_bitp  Points to the byte that holds the odd bits of R2

      We have the following:

          ptr - bit_ptr = from - from_bitp

      We want to isolate 'from_bitp', so this gives:

          ptr - bit_ptr - from = - from_bitp
          - ptr + bit_ptr + from = from_bitp
          bit_ptr + from - ptr = from_bitp
     */
    uchar bits = get_rec_bits(bit_ptr + (from - ptr), bit_ofs, bit_len);
    *to++ = bits;
  }
  length = min(bytes_in_rec, max_length - (bit_len > 0));
  memcpy(to, from, length);
  return to + length;
}

/**
   Unpack a bit field from row data.

   This method is used to unpack a bit field from a master whose size
   of the field is less than that of the slave.

   @param   to         Destination of the data
   @param   from       Source of the data
   @param   param_data Bit length (upper) and length (lower) values

   @return  New pointer into memory based on from + length of the data
*/
const uchar *Field_bit::unpack(uchar *to, const uchar *from, uint param_data,
                               bool MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("to: %p, from: %p, param_data: 0x%x", to, from, param_data));
  DBUG_PRINT("debug", ("bit_ptr: %p, bit_len: %u, bit_ofs: %u", bit_ptr,
                       bit_len, bit_ofs));
  uint const from_len = (param_data >> 8U) & 0x00ff;
  uint const from_bit_len = param_data & 0x00ff;
  DBUG_PRINT("debug",
             ("from_len: %u, from_bit_len: %u", from_len, from_bit_len));
  /*
    If the parameter data is zero (i.e., undefined), or if the master
    and slave have the same sizes, then use the old unpack() method.
  */
  if (param_data == 0 ||
      ((from_bit_len == bit_len) && (from_len == bytes_in_rec))) {
    if (bit_len > 0) {
      /*
        set_rec_bits is a macro, don't put the post-increment in the
        argument since that might cause strange side-effects.

        For the choice of the second argument, see the explanation for
        Field_bit::pack().
      */
      set_rec_bits(*from, bit_ptr + (to - ptr), bit_ofs, bit_len);
      from++;
    }
    memcpy(to, from, bytes_in_rec);
    return from + bytes_in_rec;
  }

  /*
    We are converting a smaller bit field to a larger one here.
    To do that, we first need to construct a raw value for the original
    bit value stored in the from buffer. Then that needs to be converted
    to the larger field then sent to store() for writing to the field.
    Lastly the odd bits need to be masked out if the bytes_in_rec > 0.
    Otherwise stray bits can cause spurious values.
  */
  uint new_len = (field_length + 7) / 8;
  char *value = (char *)my_alloca(new_len);
  memset(value, 0, new_len);
  uint len = from_len + ((from_bit_len > 0) ? 1 : 0);
  memcpy(value + (new_len - len), from, len);
  /*
    Mask out the unused bits in the partial byte.
    TODO: Add code to the master to always mask these bits and remove
          the following.
  */
  if ((from_bit_len > 0) && (from_len > 0))
    value[new_len - len] = value[new_len - len] & ((1U << from_bit_len) - 1);
  bitmap_set_bit(table->write_set, field_index);
  store(value, new_len, system_charset_info);
  return from + len;
}

void Field_bit::set_default() {
  if (bit_len > 0) {
    ptrdiff_t offset = table->default_values_offset();
    uchar bits = get_rec_bits(bit_ptr + offset, bit_ofs, bit_len);
    set_rec_bits(bits, bit_ptr, bit_ofs, bit_len);
  }
  Field::set_default();
}

/*
  Bit field support for non-MyISAM tables.
*/

Field_bit_as_char::Field_bit_as_char(uchar *ptr_arg, uint32 len_arg,
                                     uchar *null_ptr_arg, uchar null_bit_arg,
                                     uchar auto_flags_arg,
                                     const char *field_name_arg)
    : Field_bit(ptr_arg, len_arg, null_ptr_arg, null_bit_arg, nullptr, 0,
                auto_flags_arg, field_name_arg) {
  flags |= UNSIGNED_FLAG;
  bit_len = 0;
  bytes_in_rec = (len_arg + 7) / 8;
}

type_conversion_status Field_bit_as_char::store(const char *from, size_t length,
                                                const CHARSET_INFO *) {
  ASSERT_COLUMN_MARKED_FOR_WRITE;
  int delta;
  uchar bits = (uchar)(field_length & 7);

  for (; length && !*from; from++, length--)
    ;  // skip left 0's
  delta = bytes_in_rec - static_cast<int>(length);

  if (delta < 0 ||
      (delta == 0 && bits && (uint)(uchar)*from >= (uint)(1 << bits))) {
    memset(ptr, 0xff, bytes_in_rec);
    if (bits) *ptr &= ((1 << bits) - 1); /* set first uchar */
    if (table->in_use->is_strict_sql_mode())
      set_warning(Sql_condition::SL_WARNING, ER_DATA_TOO_LONG, 1);
    else
      set_warning(Sql_condition::SL_WARNING, ER_WARN_DATA_OUT_OF_RANGE, 1);
    return TYPE_WARN_OUT_OF_RANGE;
  }
  memset(ptr, 0, delta);
  memcpy(ptr + delta, from, length);
  return TYPE_OK;
}

void Field_bit_as_char::sql_type(String &res) const {
  const CHARSET_INFO *cs = res.charset();
  size_t length = cs->cset->snprintf(cs, res.ptr(), res.alloced_length(),
                                     "bit(%d)", (int)field_length);
  res.length(length);
}

/*****************************************************************************
  Handling of field and Create_field
*****************************************************************************/

/**
  Calculate key length for field from its type, length and other attributes.

  @note for string fields "length" parameter is assumed to take into account
        character set.

  TODO: Get rid of this function as its code is redundant with
        Field::key_length() code. However creation of Field object using
        make_field() just to call Field::key_length() is probably overkill.
*/

uint32 calc_key_length(enum_field_types sql_type, uint32 length,
                       uint32 decimals, bool is_unsigned, uint32 elements) {
  uint precision;
  switch (sql_type) {
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
      return 0;
    case MYSQL_TYPE_VARCHAR:
      return length;
    case MYSQL_TYPE_ENUM:
      return get_enum_pack_length(elements);
    case MYSQL_TYPE_SET:
      return get_set_pack_length(elements);
    case MYSQL_TYPE_BIT:
      return length / 8 + (length & 7 ? 1 : 0);
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      precision = std::min<uint>(
          my_decimal_length_to_precision(length, decimals, is_unsigned),
          DECIMAL_MAX_PRECISION);
      return my_decimal_get_binary_size(precision, decimals);
    default:
      return calc_pack_length(sql_type, length);
  }
}

enum_field_types get_blob_type_from_length(size_t length) {
  enum_field_types type;
  if (length < 256)
    type = MYSQL_TYPE_TINY_BLOB;
  else if (length < 65536)
    type = MYSQL_TYPE_BLOB;
  else if (length < 256L * 256L * 256L)
    type = MYSQL_TYPE_MEDIUM_BLOB;
  else
    type = MYSQL_TYPE_LONG_BLOB;
  return type;
}

size_t calc_pack_length(enum_field_types type, size_t length) {
  switch (type) {
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_DECIMAL:
      return (length);
    case MYSQL_TYPE_VARCHAR:
      return (length + (length < 256 ? 1 : 2));
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_TINY:
      return 1;
    case MYSQL_TYPE_SHORT:
      return 2;
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_NEWDATE:
      return 3;
    case MYSQL_TYPE_TIME:
      return 3;
    case MYSQL_TYPE_TIME2:
      return length > MAX_TIME_WIDTH
                 ? my_time_binary_length(length - MAX_TIME_WIDTH - 1)
                 : 3;
    case MYSQL_TYPE_TIMESTAMP:
      return 4;
    case MYSQL_TYPE_TIMESTAMP2:
      return length > MAX_DATETIME_WIDTH
                 ? my_timestamp_binary_length(length - MAX_DATETIME_WIDTH - 1)
                 : 4;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_LONG:
      return 4;
    case MYSQL_TYPE_FLOAT:
      return sizeof(float);
    case MYSQL_TYPE_DOUBLE:
      return sizeof(double);
    case MYSQL_TYPE_DATETIME:
      return 8;
    case MYSQL_TYPE_DATETIME2:
      return length > MAX_DATETIME_WIDTH
                 ? my_datetime_binary_length(length - MAX_DATETIME_WIDTH - 1)
                 : 5;
    case MYSQL_TYPE_LONGLONG:
      return 8; /* Don't crash if no longlong */
    case MYSQL_TYPE_NULL:
      return 0;
    case MYSQL_TYPE_TINY_BLOB:
      return 1 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_BLOB:
      return 2 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_MEDIUM_BLOB:
      return 3 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_LONG_BLOB:
      return 4 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_GEOMETRY:
      return 4 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_JSON:
      return 4 + portable_sizeof_char_ptr;
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_NEWDECIMAL:
      DBUG_ASSERT(false);
      return 0;  // This shouldn't happen
    case MYSQL_TYPE_BIT:
      return length / 8;
    default:
      return 0;
  }
}

size_t calc_pack_length(dd::enum_column_types type, size_t char_length,
                        size_t elements_count, bool treat_bit_as_char,
                        uint numeric_scale, bool is_unsigned) {
  size_t pack_length = 0;

  switch (type) {
    case dd::enum_column_types::TINY_BLOB:
    case dd::enum_column_types::MEDIUM_BLOB:
    case dd::enum_column_types::LONG_BLOB:
    case dd::enum_column_types::BLOB:
    case dd::enum_column_types::GEOMETRY:
    case dd::enum_column_types::VAR_STRING:
    case dd::enum_column_types::STRING:
    case dd::enum_column_types::VARCHAR:
      // The length is already calculated in number of bytes, no need
      // to multiply by number of bytes per symbol.
      pack_length = calc_pack_length(dd_get_old_field_type(type), char_length);
      break;
    case dd::enum_column_types::ENUM:
      pack_length = get_enum_pack_length(elements_count);
      break;
    case dd::enum_column_types::SET:
      pack_length = get_set_pack_length(elements_count);
      break;
    case dd::enum_column_types::BIT: {
      if (treat_bit_as_char)
        pack_length = ((char_length + 7) & ~7) / 8;
      else
        pack_length = char_length / 8;
    } break;
    case dd::enum_column_types::NEWDECIMAL: {
      uint decimals = numeric_scale;
      uint precision = std::min(
          my_decimal_length_to_precision(char_length, decimals, is_unsigned),
          uint(DECIMAL_MAX_PRECISION));
      DBUG_ASSERT((precision <= DECIMAL_MAX_PRECISION) &&
                  (decimals <= DECIMAL_MAX_SCALE));
      pack_length = my_decimal_get_binary_size(precision, decimals);
    } break;
    default:
      pack_length = calc_pack_length(dd_get_old_field_type(type), char_length);
      break;
  }
  return pack_length;
}

Field *make_field(MEM_ROOT *mem_root, TABLE_SHARE *share, uchar *ptr,
                  size_t field_length, uchar *null_pos, uchar null_bit,
                  enum_field_types field_type,
                  const CHARSET_INFO *field_charset,
                  Field::geometry_type geom_type, uchar auto_flags,
                  TYPELIB *interval, const char *field_name, bool is_nullable,
                  bool is_zerofill, bool is_unsigned, uint decimals,
                  bool treat_bit_as_char, uint pack_length_override,
                  Nullable<gis::srid_t> srid, bool is_array) {
  uchar *bit_ptr = nullptr;
  uchar bit_offset = 0;
  DBUG_ASSERT(mem_root);

  if (field_type == MYSQL_TYPE_BIT && !treat_bit_as_char) {
    bit_ptr = null_pos;
    bit_offset = null_bit;
    if (is_nullable)  // if null field
    {
      bit_ptr += (null_bit == 7);  // shift bit_ptr and bit_offset
      bit_offset = (bit_offset + 1) & 7;
    }
  }

  if (!is_nullable) {
    null_pos = nullptr;
    null_bit = 0;
  } else {
    null_bit = ((uchar)1) << null_bit;
  }

  if (is_temporal_real_type(field_type)) field_charset = &my_charset_numeric;

  DBUG_PRINT("debug", ("field_type: %d, field_length: %zu, "
                       "interval: %p, is_array: %s",
                       field_type, field_length, interval,
                       (is_array ? "true" : "false")));

  if (is_array) {
    // See Item_func_array_cast::resolve_type() for supported types
    switch (field_type) {
      case MYSQL_TYPE_VARCHAR:
      case MYSQL_TYPE_NEWDECIMAL:
      case MYSQL_TYPE_LONGLONG:
      case MYSQL_TYPE_NEWDATE:
        break;
      case MYSQL_TYPE_TIME2:
        decimals = (field_length > MAX_TIME_WIDTH)
                       ? field_length - 1 - MAX_TIME_WIDTH
                       : 0;
        break;
      case MYSQL_TYPE_DATETIME2:
        decimals = (field_length > MAX_DATETIME_WIDTH)
                       ? field_length - 1 - MAX_DATETIME_WIDTH
                       : 0;
        break;
      default:
        DBUG_ASSERT(0);  // Shouldn't happen
        return nullptr;
    }
    /*
      Field_json constructor expects number of bytes used to represent length
      of the underlying blob as parameter and not the real field pack_length.
      JSON is used as array storage.
    */
    uint pack_length = calc_pack_length(MYSQL_TYPE_JSON, field_length) -
                       portable_sizeof_char_ptr;

    return new (mem_root) Field_typed_array(
        field_type, is_unsigned, field_length, decimals, ptr, null_pos,
        null_bit, auto_flags, field_name, share, pack_length, field_charset);
  }
  /*
    FRMs from 3.23/4.0 can have strings with field_type == MYSQL_TYPE_DECIMAL.
    We should not be getting them after upgrade to new data-dictionary.
  */

  switch (field_type) {
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
      return new (mem_root) Field_string(ptr, field_length, null_pos, null_bit,
                                         auto_flags, field_name, field_charset);
    case MYSQL_TYPE_VARCHAR:
      return new (mem_root) Field_varstring(
          ptr, field_length, HA_VARCHAR_PACKLENGTH(field_length), null_pos,
          null_bit, auto_flags, field_name, share, field_charset);
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_LONG_BLOB: {
      /*
        Field_blob constructor expects number of bytes used to represent length
        of the blob as parameter and not the real field pack_length.
      */
      uint pack_length =
          calc_pack_length(field_type, field_length) - portable_sizeof_char_ptr;

      return new (mem_root)
          Field_blob(ptr, null_pos, null_bit, auto_flags, field_name, share,
                     pack_length, field_charset);
    }
    case MYSQL_TYPE_GEOMETRY: {
      /*
        Field_geom constructor expects number of bytes used to represent length
        of the underlying blob as parameter and not the real field pack_length.
      */
      uint pack_length =
          calc_pack_length(field_type, field_length) - portable_sizeof_char_ptr;

      return new (mem_root)
          Field_geom(ptr, null_pos, null_bit, auto_flags, field_name, share,
                     pack_length, geom_type, srid);
    }
    case MYSQL_TYPE_JSON: {
      /*
        Field_json constructor expects number of bytes used to represent length
        of the underlying blob as parameter and not the real field pack_length.
      */
      uint pack_length =
          calc_pack_length(field_type, field_length) - portable_sizeof_char_ptr;

      return new (mem_root) Field_json(ptr, null_pos, null_bit, auto_flags,
                                       field_name, share, pack_length);
    }
    case MYSQL_TYPE_ENUM:
      DBUG_ASSERT(interval);
      return new (mem_root) Field_enum(
          ptr, field_length, null_pos, null_bit, auto_flags, field_name,
          (pack_length_override ? pack_length_override
                                : get_enum_pack_length(interval->count)),
          interval, field_charset);
    case MYSQL_TYPE_SET:
      DBUG_ASSERT(interval);
      return new (mem_root) Field_set(
          ptr, field_length, null_pos, null_bit, auto_flags, field_name,
          (pack_length_override ? pack_length_override
                                : get_set_pack_length(interval->count)),
          interval, field_charset);
    case MYSQL_TYPE_DECIMAL:
      return new (mem_root)
          Field_decimal(ptr, field_length, null_pos, null_bit, auto_flags,
                        field_name, decimals, is_zerofill, is_unsigned);
    case MYSQL_TYPE_NEWDECIMAL:
      return new (mem_root)
          Field_new_decimal(ptr, field_length, null_pos, null_bit, auto_flags,
                            field_name, decimals, is_zerofill, is_unsigned);
    case MYSQL_TYPE_FLOAT:
      return new (mem_root)
          Field_float(ptr, field_length, null_pos, null_bit, auto_flags,
                      field_name, decimals, is_zerofill, is_unsigned);
    case MYSQL_TYPE_DOUBLE:
      return new (mem_root)
          Field_double(ptr, field_length, null_pos, null_bit, auto_flags,
                       field_name, decimals, is_zerofill, is_unsigned);
    case MYSQL_TYPE_TINY:
      return new (mem_root)
          Field_tiny(ptr, field_length, null_pos, null_bit, auto_flags,
                     field_name, is_zerofill, is_unsigned);
    case MYSQL_TYPE_SHORT:
      return new (mem_root)
          Field_short(ptr, field_length, null_pos, null_bit, auto_flags,
                      field_name, is_zerofill, is_unsigned);
    case MYSQL_TYPE_INT24:
      return new (mem_root)
          Field_medium(ptr, field_length, null_pos, null_bit, auto_flags,
                       field_name, is_zerofill, is_unsigned);
    case MYSQL_TYPE_LONG:
      return new (mem_root)
          Field_long(ptr, field_length, null_pos, null_bit, auto_flags,
                     field_name, is_zerofill, is_unsigned);
    case MYSQL_TYPE_LONGLONG:
      return new (mem_root)
          Field_longlong(ptr, field_length, null_pos, null_bit, auto_flags,
                         field_name, is_zerofill, is_unsigned);
    case MYSQL_TYPE_TIMESTAMP:
      return new (mem_root) Field_timestamp(ptr, field_length, null_pos,
                                            null_bit, auto_flags, field_name);
    case MYSQL_TYPE_TIMESTAMP2:
      return new (mem_root)
          Field_timestampf(ptr, null_pos, null_bit, auto_flags, field_name,
                           field_length > MAX_DATETIME_WIDTH
                               ? field_length - 1 - MAX_DATETIME_WIDTH
                               : 0);
    case MYSQL_TYPE_YEAR:
      DBUG_ASSERT(field_length == 4);  // Field_year is only for length 4.
      return new (mem_root)
          Field_year(ptr, null_pos, null_bit, auto_flags, field_name);
    case MYSQL_TYPE_NEWDATE:
      return new (mem_root)
          Field_newdate(ptr, null_pos, null_bit, auto_flags, field_name);

    case MYSQL_TYPE_TIME:
      return new (mem_root)
          Field_time(ptr, null_pos, null_bit, auto_flags, field_name);
    case MYSQL_TYPE_TIME2:
      return new (mem_root) Field_timef(
          ptr, null_pos, null_bit, auto_flags, field_name,
          (field_length > MAX_TIME_WIDTH) ? field_length - 1 - MAX_TIME_WIDTH
                                          : 0);
    case MYSQL_TYPE_DATETIME:
      return new (mem_root)
          Field_datetime(ptr, null_pos, null_bit, auto_flags, field_name);
    case MYSQL_TYPE_DATETIME2:
      return new (mem_root)
          Field_datetimef(ptr, null_pos, null_bit, auto_flags, field_name,
                          (field_length > MAX_DATETIME_WIDTH)
                              ? field_length - 1 - MAX_DATETIME_WIDTH
                              : 0);
    case MYSQL_TYPE_NULL:
      return new (mem_root)
          Field_null(ptr, field_length, auto_flags, field_name, field_charset);
    case MYSQL_TYPE_BIT:
      return treat_bit_as_char
                 ? new (mem_root)
                       Field_bit_as_char(ptr, field_length, null_pos, null_bit,
                                         auto_flags, field_name)
                 : new (mem_root)
                       Field_bit(ptr, field_length, null_pos, null_bit, bit_ptr,
                                 bit_offset, auto_flags, field_name);

    default:  // Impossible (Wrong version)
      break;
  }
  return nullptr;
}

Field *make_field(const Create_field &create_field, TABLE_SHARE *share,
                  const char *field_name, size_t field_length, uchar *ptr,
                  uchar *null_pos, size_t null_bit) {
  return make_field(*THR_MALLOC, share, ptr, field_length, null_pos, null_bit,
                    create_field.sql_type, create_field.charset,
                    create_field.geom_type, create_field.auto_flags,
                    create_field.interval, field_name, create_field.is_nullable,
                    create_field.is_zerofill, create_field.is_unsigned,
                    create_field.decimals, create_field.treat_bit_as_char,
                    create_field.pack_length_override, create_field.m_srid,
                    create_field.is_array);
}

Field *make_field(const Create_field &create_field, TABLE_SHARE *share,
                  uchar *ptr, uchar *null_pos, size_t null_bit) {
  return make_field(create_field, share, create_field.field_name,
                    create_field.max_display_width_in_bytes(), ptr, null_pos,
                    null_bit);
}

Field *make_field(const Create_field &create_field, TABLE_SHARE *share) {
  return make_field(create_field, share, create_field.field_name,
                    create_field.max_display_width_in_bytes(), nullptr, nullptr,
                    0);
}

/**
  maximum possible character length for blob.

  This method is used in Item_field::set_field to calculate
  max_length for Item.

  For example:
    CREATE TABLE t2 SELECT CONCAT(tinyblob_utf8_column) FROM t1;
  must create a "VARCHAR(255) CHARACTER SET utf8" column.

  @return
    length
*/

uint32 Field_blob::char_length() const {
  switch (packlength) {
    case 1:
      return 255;
    case 2:
      return 65535;
    case 3:
      return 16777215;
    case 4:
      return (uint32)4294967295U;
    default:
      DBUG_ASSERT(0);  // we should never go here
      return 0;
  }
}

/**
  This function creates a separate copy of blob value.

  @param [in] mem_root
    mem_root that is used to allocate memory for 'copy_of_value'.

  @return - Can fail if we are out of memory.
    @retval false   Success
    @retval true    Failure
*/

bool Field_blob::copy_blob_value(MEM_ROOT *mem_root) {
  DBUG_TRACE;

  // Testing memory allocation failure
  DBUG_EXECUTE_IF("simulate_blob_memory_allocation_fail",
                  DBUG_SET("+d,simulate_out_of_memory"););

  // Allocate new memory location
  size_t ulen = get_length(ptr);
  if (ulen == 0) {
    value.set("", 0, value.charset());
  } else {
    char *blob_value =
        static_cast<char *>(memdup_root(mem_root, get_ptr(), ulen));
    if (blob_value == nullptr) return true;

    // Set 'value' with the duplicated data
    value.set(blob_value, ulen, value.charset());
  }

  // Set ptr of Field for duplicated data
  store_ptr_and_length(value.ptr(), ulen);

  return false;
}

/**
  maximum possible display length for blob.

  @return
    length
*/

uint32 Field_blob::max_display_length() const {
  switch (packlength) {
    case 1:
      return 255 * field_charset->mbmaxlen;
    case 2:
      return 65535 * field_charset->mbmaxlen;
    case 3:
      return 16777215 * field_charset->mbmaxlen;
    case 4:
      return (uint32)4294967295U;
    default:
      DBUG_ASSERT(0);  // we should never go here
      return 0;
  }
}

/*****************************************************************************
 Warning handling
*****************************************************************************/

/**
  Produce warning or note about data saved into field.

  @param level            - level of message (Note/Warning/Error)
  @param code             - error code of message to be produced
  @param truncate_increment  - whether we should increase truncated fields
  count
  @param view_db_name     - if set this is the database name for view
                            that causes the warning
  @param view_name        - if set this is the name of view that causes
                            the warning

  @note
    This function won't produce warning and increase cut fields counter
    if check_for_truncated_fields == CHECK_FIELD_IGNORE for current thread.

    if check_for_truncated_fields == CHECK_FIELD_IGNORE then we ignore notes.
    This allows us to avoid notes in optimisation, like convert_constant_item().

    In case of execution statements INSERT/INSERT SELECT/REPLACE/REPLACE SELECT
    the method emits only one warning message for the following
    types of warning: ER_BAD_NULL_ERROR, ER_WARN_NULL_TO_NOTNULL,
    ER_NO_DEFAULT_FOR_FIELD.
  @retval
    1 if check_for_truncated_fields == CHECK_FIELD_IGNORE and error level
    is not NOTE
  @retval
    0 otherwise
*/

bool Field::set_warning(Sql_condition::enum_severity_level level, uint code,
                        int truncate_increment, const char *view_db_name,
                        const char *view_name) {
  /*
    If this field was created only for type conversion purposes it
    will have table == NULL.
  */

  THD *thd = table ? table->in_use : current_thd;

  if (!thd->check_for_truncated_fields)
    return level >= Sql_condition::SL_WARNING;

  thd->num_truncated_fields += truncate_increment;

  if (thd->lex->sql_command != SQLCOM_INSERT &&
      thd->lex->sql_command != SQLCOM_INSERT_SELECT &&
      thd->lex->sql_command != SQLCOM_REPLACE &&
      thd->lex->sql_command != SQLCOM_REPLACE_SELECT) {
    // We aggregate warnings from only INSERT and REPLACE statements.

    push_warning_printf(thd, level, code, ER_THD_NONCONST(thd, code),
                        field_name,
                        thd->get_stmt_da()->current_row_for_condition());

    return false;
  }

  unsigned int current_warning_mask = 0;

  if (code == ER_BAD_NULL_ERROR)
    current_warning_mask = BAD_NULL_ERROR_PUSHED;
  else if (code == ER_NO_DEFAULT_FOR_FIELD)
    current_warning_mask = NO_DEFAULT_FOR_FIELD_PUSHED;

  if (current_warning_mask) {
    if (!(m_warnings_pushed & current_warning_mask)) {
      push_warning_printf(thd, level, code, ER_THD_NONCONST(thd, code),
                          field_name,
                          thd->get_stmt_da()->current_row_for_condition());
      m_warnings_pushed |= current_warning_mask;
    }
  } else if (code == ER_NO_DEFAULT_FOR_VIEW_FIELD) {
    if (!(m_warnings_pushed & NO_DEFAULT_FOR_VIEW_FIELD_PUSHED)) {
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_NO_DEFAULT_FOR_VIEW_FIELD,
          ER_THD(thd, ER_NO_DEFAULT_FOR_VIEW_FIELD), view_db_name, view_name);
      m_warnings_pushed |= NO_DEFAULT_FOR_VIEW_FIELD_PUSHED;
    }
  } else {
    push_warning_printf(thd, level, code, ER_THD_NONCONST(thd, code),
                        field_name,
                        thd->get_stmt_da()->current_row_for_condition());
  }

  return false;
}

/**
  Produce warning or note about double datetime data saved into field.

  @param level            level of message (Note/Warning/Error)
  @param code             error code of message to be produced
  @param val              error parameter (the value)
  @param ts_type          type of datetime value (datetime/date/time)
  @param truncate_increment  whether we should increase truncated fields count

  @retval false  Function reported warning
  @retval true   Function reported error

  @note
    This function will always produce some warning but won't increase truncated
    fields counter if check_for_truncated_fields == FIELD_CHECK_IGNORE
    for current thread.
*/
bool Field_temporal::set_datetime_warning(
    Sql_condition::enum_severity_level level, uint code,
    const ErrConvString &val, enum_mysql_timestamp_type ts_type,
    int truncate_increment) {
  THD *thd = table ? table->in_use : current_thd;
  if ((!thd->lex->is_ignore() &&
       ((thd->variables.sql_mode & MODE_STRICT_ALL_TABLES) ||
        (thd->variables.sql_mode & MODE_STRICT_TRANS_TABLES &&
         !thd->get_transaction()->cannot_safely_rollback(
             Transaction_ctx::STMT)))) ||
      set_warning(level, code, truncate_increment))
    return make_truncated_value_warning(thd, level, val, ts_type, field_name);

  return false;
}

bool Field::is_part_of_actual_key(THD *thd, uint cur_index,
                                  KEY *cur_index_info) const {
  return thd->optimizer_switch_flag(OPTIMIZER_SWITCH_USE_INDEX_EXTENSIONS) &&
                 !(cur_index_info->flags & HA_NOSAME)
             ? part_of_key.is_set(cur_index)
             : part_of_key_not_extended.is_set(cur_index);
}

Field_typed_array::Field_typed_array(const Field_typed_array &other)
    : Field_json(other),
      m_elt_type(other.m_elt_type),
      m_elt_decimals(other.m_elt_decimals),
      m_elt_charset(other.m_elt_charset) {}

Field_typed_array::Field_typed_array(
    enum_field_types elt_type, bool elt_is_unsigned, size_t elt_length,
    uint elt_decimals, uchar *ptr_arg, uchar *null_ptr_arg, uint null_bit_arg,
    uchar auto_flags_arg, const char *field_name_arg, TABLE_SHARE *share,
    uint blob_pack_length, const CHARSET_INFO *cs)
    : Field_json(ptr_arg, null_ptr_arg, null_bit_arg, auto_flags_arg,
                 field_name_arg, share, blob_pack_length),
      m_elt_type(elt_type),
      m_elt_decimals(elt_decimals),
      m_elt_charset(cs) {
  if (elt_is_unsigned) {
    unsigned_flag = true;
    flags |= UNSIGNED_FLAG;
  }
  if (Field_typed_array::binary()) flags |= BINARY_FLAG;
  field_length = elt_length;
  /*
    Arrays of BLOB aren't supported and can't be created, so mask the BLOB
    flag of JSON
  */
  flags &= ~BLOB_FLAG;
  DBUG_ASSERT(elt_type != MYSQL_TYPE_STRING &&
              elt_type != MYSQL_TYPE_VAR_STRING);
}

uint32 Field_typed_array::key_length() const {
  return calc_key_length(m_elt_type, field_length, m_elt_decimals,
                         unsigned_flag,
                         // Number of intervals isn't applicable here
                         0);
}

Field_typed_array *Field_typed_array::clone(MEM_ROOT *mem_root) const {
  DBUG_ASSERT(is_array());
  return new (mem_root) Field_typed_array(*this);
}

Item_result Field_typed_array::result_type() const {
  return field_types_result_type[field_type2index(m_elt_type)];
}

type_conversion_status Field_typed_array::store(const char *to, size_t length,
                                                const CHARSET_INFO *charset) {
  return m_conv_item->field->store(to, length, charset);
}

type_conversion_status Field_typed_array::store(double nr) {
  return m_conv_item->field->store(nr);
}

type_conversion_status Field_typed_array::store(longlong nr,
                                                bool unsigned_val) {
  return m_conv_item->field->store(nr, unsigned_val);
}

type_conversion_status Field_typed_array::store_array(const Json_wrapper *data,
                                                      Json_array *array) {
  array->clear();

  set_null();

  try {
    // How to store values
    switch (data->type()) {
      case enum_json_type::J_NULL: {
        /*
          Unlike SQL NULL, JSON null is a value, but a special one and it
          can't be coerced to any data type. The latter means it can't be
          indexed by relational SE. Due to that an error is thrown.
        */
        my_error(ER_INVALID_JSON_VALUE_FOR_FUNC_INDEX, MYF(0),
                 get_index_name());
        return TYPE_ERR_BAD_VALUE;
      }
      case enum_json_type::J_DECIMAL:
      case enum_json_type::J_DOUBLE:
      case enum_json_type::J_STRING:
      case enum_json_type::J_DATE:
      case enum_json_type::J_TIME:
      case enum_json_type::J_DATETIME:
      case enum_json_type::J_TIMESTAMP:
      case enum_json_type::J_INT:
      case enum_json_type::J_UINT: {
        // Handle scalars
        Json_wrapper coerced;
        if (coerce_json_value(data, false, &coerced))
          return TYPE_ERR_BAD_VALUE; /* purecov: inspected */
        coerced.set_alias();
        if (array->append_alias(coerced.to_dom(table->in_use)))
          return TYPE_ERR_OOM;
        Json_wrapper wr(array, true);
        /*
          No need to check multi-valued key limits, as single value is always
          allowed if engine supports multi-valued index, and single value
          can't outgrow index length limit.
        */
        set_notnull();
        return store_json(&wr);
      }
      case enum_json_type::J_ARRAY: {
        // Handle array
        Json_wrapper coerced;
        uint max_num_keys = 0;
        size_t keys_length = 0, max_keys_length = 0;

        // Empty array stored as non-NULL empty array
        if (data->length() == 0) {
          Json_wrapper wr(array, true);
          set_notnull();
          return store_json(&wr);
        }
        table->file->ha_mv_key_capacity(&max_num_keys, &max_keys_length);
        DBUG_ASSERT(max_num_keys && max_keys_length);
        for (size_t i = 0; i < data->length(); i++) {
          Json_wrapper elt = (*data)[i];
          if (elt.type() == enum_json_type::J_NULL) {
            /*
              Unlike SQL NULL, JSON null is a value, but a special one and it
              can't be coerced to any data type. The latter means it can't be
              indexed by relational SE. Due to that an error is thrown.
            */
            my_error(ER_INVALID_JSON_VALUE_FOR_FUNC_INDEX, MYF(0),
                     get_index_name());
            return TYPE_ERR_BAD_VALUE;
          }

          if (coerce_json_value(&elt, false, &coerced))
            return TYPE_ERR_BAD_VALUE;
          coerced.set_alias();
          if (array->append_alias(coerced.to_dom(table->in_use)))
            return TYPE_ERR_OOM;
          if (type() == MYSQL_TYPE_VARCHAR)
            keys_length += coerced.get_data_length();
          else
            keys_length += m_conv_item->field->pack_length();
        }
        /*
          Non-strict mode issue:
          While consisting of unique values, bad input can cause duplicates
          after coercion. Remove them, as SE doesn't expect dups in the array.
          This is why we need to sort & remove duplicates only after
          processing all keys.
        */
        if (array->size() > 1)
          array->remove_duplicates(type() == MYSQL_TYPE_VARCHAR ? m_elt_charset
                                                                : nullptr);
        if (array->size() > max_num_keys) {
          my_error(ER_EXCEEDED_MV_KEYS_NUM, MYF(0), get_index_name(),
                   array->size() - max_num_keys);
          return TYPE_ERR_BAD_VALUE;
        }
        if (keys_length > max_keys_length) {
          // Array fields have only one index defined over them
          my_error(ER_EXCEEDED_MV_KEYS_SPACE, MYF(0), get_index_name(),
                   (keys_length - max_keys_length));
          return TYPE_ERR_BAD_VALUE;
        }
        Json_wrapper wr(array, true);
        set_notnull();
        return store_json(&wr);
      }
      case enum_json_type::J_BOOLEAN: {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing JSON BOOLEAN type to array");
        return TYPE_ERR_BAD_VALUE;
      }

      case enum_json_type::J_OBJECT: {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "CAST-ing JSON OBJECT type to array");
        return TYPE_ERR_BAD_VALUE;
      }
      case enum_json_type::J_ERROR:
        my_error(ER_INVALID_JSON_VALUE_FOR_FUNC_INDEX, MYF(0),
                 get_index_name());
        return TYPE_ERR_BAD_VALUE;
      /* purecov: begin inspected */
      default:
        // Shouldn't happen
        DBUG_ASSERT(0);
        return TYPE_ERR_BAD_VALUE;
    }
  } catch (...) {
    handle_std_exception("typed array field");
  }
  return TYPE_ERR_BAD_VALUE;
  /* purecov: end */
}

size_t Field_typed_array::get_key_image(uchar *buff, size_t length,
                                        imagetype type) const {
  return m_conv_item->field->get_key_image(buff, length, type);
}

Field *Field_typed_array::new_key_field(MEM_ROOT *root, TABLE *new_table,
                                        uchar *new_ptr, uchar *, uint) const {
  Field *res = m_conv_item->field->new_key_field(root, new_table, new_ptr);
  if (res != nullptr) {
    // Keep the field hidden to allow error handler to catch functional
    // index's errors
    res->set_hidden(dd::Column::enum_hidden_type::HT_HIDDEN_SQL);
    res->part_of_key = part_of_key;
  }
  return res;
}

void Field_typed_array::init(TABLE *table_arg) {
  Field::init(table_arg);

  switch (type()) {
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_NEWDECIMAL:
      break;
    default:
      // Shouldn't happen
      DBUG_ASSERT(0); /* purecov: inspected */
      return;
  }

  // Create field for data conversion
  Field *conv_field = ::make_field(
      // Allocate conversion field in table's mem_root
      &table_arg->mem_root,
      nullptr,       // TABLE_SHARE, not needed
      nullptr,       // data buffer, isn't allocated yet
      field_length,  // field_length
      nullptr, 0,    // null_pos, nul_bit
      real_type(),   // field_type
      m_elt_charset,
      Field::GEOM_GEOMETRY,  // geom type
      Field::NONE,           // auto_flags
      nullptr,               // intervals aren't supported in array
      field_name, is_nullable(),
      false,  // zerofill is meaningless with JSON
      unsigned_flag, m_elt_decimals,
      false,  // treat_bit_as_char
      0,      // pack_length_override
      {},     // srid
      false   // is_array
  );
  if (conv_field == nullptr) return;
  uchar *buf =
      table_arg->mem_root.ArrayAlloc<uchar>(conv_field->pack_length() + 1);
  if (buf == nullptr) return;
  if (type() == MYSQL_TYPE_NEWDECIMAL)
    (down_cast<Field_new_decimal *>(conv_field))->set_keep_precision(true);
  conv_field->move_field(buf + 1, buf, 0);
  // Allow conv_field to use table->in_use
  conv_field->table = table;
  conv_field->field_index = field_index;
  conv_field->table_name = table_name;

  // Swap arena so that the Item_field is allocated on TABLE::mem_root
  // and so it does not end up in THD's item list which will have a different
  // lifetime than TABLE::mem_root
  Query_arena tmp_arena(&table_arg->mem_root,
                        Query_arena::STMT_REGULAR_EXECUTION);
  Query_arena backup_arena;
  current_thd->swap_query_arena(tmp_arena, &backup_arena);
  m_conv_item = new Item_field(conv_field);
  current_thd->swap_query_arena(backup_arena, &tmp_arena);
}

const char *Field_typed_array::get_index_name() const {
  uint key = part_of_key.get_first_set();
  DBUG_ASSERT(key != MY_BIT_NONE);
  return table->s->key_info[key].name;
}

size_t Field_typed_array::make_sort_key(Json_wrapper *wr, uchar *to,
                                        size_t length) const {
#ifndef DBUG_OFF
  switch (wr->type()) {
    case enum_json_type::J_ERROR:
    case enum_json_type::J_OBJECT:
    case enum_json_type::J_ARRAY:
      // Only scalars are supported
      DBUG_ASSERT(false);
      break;
    default:
      break;
  }
#endif
  THD *thd = table->in_use;
  // Force error on bad data
#ifndef DBUG_OFF
  bool res =
#endif
      save_json_to_field(thd, m_conv_item->field, wr, true);
  // Data should be already properly converted so no error is expected here
  DBUG_ASSERT(!res && !thd->is_error());

  return m_conv_item->field->make_sort_key(to, length);
}

int Field_typed_array::do_save_field_metadata(uchar *metadata_ptr) const {
  *metadata_ptr = static_cast<uchar>(m_elt_type);
  switch (m_elt_type) {
    case MYSQL_TYPE_VARCHAR: {
      DBUG_ASSERT(field_length < 65536);
      char *param_ptr = (char *)(metadata_ptr + 1);
      int3store(param_ptr, field_length);
      return 4;
    }
    case MYSQL_TYPE_NEWDECIMAL: {
      DBUG_ASSERT(field_length < 128);
      uint8 precision = my_decimal_length_to_precision(field_length, decimals(),
                                                       unsigned_flag);
      *(metadata_ptr + 1) = precision;
      *(metadata_ptr + 2) = decimals();
      return 3;
    }
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_NEWDATE:
      return 1;
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_DATETIME2:
      *(metadata_ptr + 1) = decimals();
      return 2;
    default:
      break;
  }
  DBUG_ASSERT(0);  // Shouldn't happen
  return 0;
}

void Field_typed_array::sql_type(String &str) const {
  const Field *const conv_field = m_conv_item->field;
  // There is no need to append the character set and collation to the type,
  // since utf8mb4_0900_bin is the only collation supported for arrays.
  DBUG_ASSERT(!conv_field->has_charset() ||
              conv_field->charset() == &my_charset_utf8mb4_0900_bin);
  conv_field->sql_type(str);
  str.append(STRING_WITH_LEN(" array"));
}

void Field_typed_array::make_send_field(Send_field *field) const {
  Field_json::make_send_field(field);
  // When sending the array to the client (only possible using the debug flag
  // show_hidden_columns), it should be sent as a JSON array. Set the type to
  // JSON instead of the array element type.
  field->type = MYSQL_TYPE_JSON;
}

Key_map Field::get_covering_prefix_keys() const {
  if (table == nullptr) {
    // This function might be called when creating functional indexes. In those
    // cases, we do not have a table object available. Assert that the function
    // is indeed called in a functional index context, and then return an empty
    // key map.
    DBUG_ASSERT(down_cast<const Create_field_wrapper *>(this) != nullptr);
    return Key_map();
  }
  Key_map covering_prefix_keys = part_of_prefixkey;
  covering_prefix_keys.intersect(table->covering_keys);
  return covering_prefix_keys;
}

void Field::set_default() {
  if (has_insert_default_datetime_value_expression() ||
      has_insert_default_general_value_expression())
    evaluate_insert_default_function();
  else
    copy_data(table->default_values_offset());
}

uint Field::null_offset() const { return null_offset(table->record[0]); }

void Field::init(TABLE *table_arg) {
  orig_table = table = table_arg;
  table_name = &table_arg->alias;
}

// Byteswaps and/or truncates int16 values; used for both pack() and unpack().
static inline void handle_int16(uchar *to, const uchar *from, uint max_length,
                                bool low_byte_first_from,
                                bool low_byte_first_to) {
  int16 val;
  uchar buf[sizeof(val)];
  if (low_byte_first_from)
    val = sint2korr(from);
  else
    val = shortget(from);

  if (low_byte_first_to)
    int2store(buf, val);
  else
    shortstore(buf, val);
  if (max_length >= sizeof(buf)) {
    // Common case.
    memcpy(to, buf, sizeof(buf));
  } else {
    memcpy(to, buf, max_length);
  }
}

// Byteswaps and/or truncates int24 values; used for both pack() and unpack().
static inline void handle_int24(uchar *to, const uchar *from, uint max_length,
                                bool low_byte_first_from MY_ATTRIBUTE((unused)),
                                bool low_byte_first_to MY_ATTRIBUTE((unused))) {
  int32 val;
  uchar buf[3];
#ifdef WORDS_BIGENDIAN
  if (low_byte_first_from)
    val = sint3korr(from);
  else
#endif
    val = (from[0] << 16) + (from[1] << 8) + from[2];

#ifdef WORDS_BIGENDIAN
  if (low_byte_first_to)
    int3store(buf, val);
  else
#endif
  {
    buf[0] = 0xFF & (val >> 16);
    buf[1] = 0xFF & (val >> 8);
    buf[2] = 0xFF & val;
  }
  if (max_length >= sizeof(buf)) {
    // Common case.
    memcpy(to, buf, sizeof(buf));
  } else {
    memcpy(to, buf, max_length);
  }
}

// Byteswaps and/or truncates int32 values; used for both pack() and unpack().
static inline void handle_int32(uchar *to, const uchar *from, uint max_length,
                                bool low_byte_first_from,
                                bool low_byte_first_to) {
  int32 val;
  uchar buf[sizeof(val)];
  if (low_byte_first_from)
    val = sint4korr(from);
  else
    val = longget(from);

  if (low_byte_first_to)
    int4store(buf, val);
  else
    longstore(buf, val);
  if (max_length >= sizeof(buf)) {
    // Common case.
    memcpy(to, buf, sizeof(buf));
  } else {
    memcpy(to, buf, max_length);
  }
}

// Byteswaps and/or truncates int64 values; used for both pack() and unpack().
static inline void handle_int64(uchar *to, const uchar *from, uint max_length,
                                bool low_byte_first_from MY_ATTRIBUTE((unused)),
                                bool low_byte_first_to MY_ATTRIBUTE((unused))) {
  int64 val;
  uchar buf[sizeof(val)];
#ifdef WORDS_BIGENDIAN
  if (low_byte_first_from)
    val = sint8korr(from);
  else
#endif
    memcpy(&val, from, sizeof(val));

#ifdef WORDS_BIGENDIAN
  if (low_byte_first_to)
    int8store(buf, val);
  else
#endif
    longlongstore(buf, val);
  if (max_length >= sizeof(buf)) {
    // Common case.
    memcpy(to, buf, sizeof(buf));
  } else {
    memcpy(to, buf, max_length);
  }
}

uchar *Field::pack_int16(uchar *to, const uchar *from, uint max_length,
                         bool low_byte_first_to) const {
  handle_int16(to, from, max_length, table->s->db_low_byte_first,
               low_byte_first_to);
  return to + sizeof(int16);
}

const uchar *Field::unpack_int16(uchar *to, const uchar *from,
                                 bool low_byte_first_from) const {
  handle_int16(to, from, UINT_MAX, low_byte_first_from,
               table->s->db_low_byte_first);
  return from + sizeof(int16);
}

uchar *Field::pack_int24(uchar *to, const uchar *from, uint max_length,
                         bool low_byte_first_to) const {
  handle_int24(to, from, max_length, table->s->db_low_byte_first,
               low_byte_first_to);
  return to + 3;
}

const uchar *Field::unpack_int24(uchar *to, const uchar *from,
                                 bool low_byte_first_from) const {
  handle_int24(to, from, UINT_MAX, low_byte_first_from,
               table->s->db_low_byte_first);
  return from + 3;
}

uchar *Field::pack_int32(uchar *to, const uchar *from, uint max_length,
                         bool low_byte_first_to) const {
  handle_int32(to, from, max_length, table->s->db_low_byte_first,
               low_byte_first_to);
  return to + sizeof(int32);
}

const uchar *Field::unpack_int32(uchar *to, const uchar *from,
                                 bool low_byte_first_from) const {
  handle_int32(to, from, UINT_MAX, low_byte_first_from,
               table->s->db_low_byte_first);
  return from + sizeof(int32);
}

uchar *Field::pack_int64(uchar *to, const uchar *from, uint max_length,
                         bool low_byte_first_to) const {
  handle_int64(to, from, max_length, table->s->db_low_byte_first,
               low_byte_first_to);
  return to + sizeof(int64);
}

const uchar *Field::unpack_int64(uchar *to, const uchar *from,
                                 bool low_byte_first_from) const {
  handle_int64(to, from, UINT_MAX, low_byte_first_from,
               table->s->db_low_byte_first);
  return from + sizeof(int64);
}

bool Field_longstr::is_updatable() const {
  DBUG_ASSERT(table && table->write_set);
  return bitmap_is_set(table->write_set, field_index);
}

Field_varstring::Field_varstring(uchar *ptr_arg, uint32 len_arg,
                                 uint length_bytes_arg, uchar *null_ptr_arg,
                                 uchar null_bit_arg, uchar auto_flags_arg,
                                 const char *field_name_arg, TABLE_SHARE *share,
                                 const CHARSET_INFO *cs)
    : Field_longstr(ptr_arg, len_arg, null_ptr_arg, null_bit_arg,
                    auto_flags_arg, field_name_arg, cs),
      length_bytes(length_bytes_arg) {
  if (share != nullptr) {
    share->varchar_fields++;
  }
}

Field_varstring::Field_varstring(uint32 len_arg, bool is_nullable_arg,
                                 const char *field_name_arg, TABLE_SHARE *share,
                                 const CHARSET_INFO *cs)
    : Field_longstr(nullptr, len_arg,
                    is_nullable_arg ? &dummy_null_buffer : nullptr, 0, NONE,
                    field_name_arg, cs),
      length_bytes(len_arg < 256 ? 1 : 2) {
  if (share != nullptr) {
    share->varchar_fields++;
  }
}

void Field_blob::store_length(uchar *i_ptr, uint i_packlength,
                              uint32 i_number) {
  store_blob_length(i_ptr, i_packlength, i_number, table->s->db_low_byte_first);
}

uint32 Field_blob::get_length(ptrdiff_t row_offset) const {
  return get_length(ptr + row_offset, this->packlength,
                    table->s->db_low_byte_first);
}

uint32 Field_blob::get_length(const uchar *ptr_arg) const {
  return get_length(ptr_arg, this->packlength, table->s->db_low_byte_first);
}

bool Field_blob::backup_blob_field() {
  value.swap(m_blob_backup);
#ifndef DBUG_OFF
  m_uses_backup = true;
#endif
  return false;
}

void Field_blob::restore_blob_backup() {
  DBUG_ASSERT(m_uses_backup);
  value.swap(m_blob_backup);
#ifndef DBUG_OFF
  m_uses_backup = false;
#endif
}

Create_field_wrapper::Create_field_wrapper(const Create_field *fld)
    : Field(nullptr, fld->max_display_width_in_codepoints(), nullptr, 0,
            fld->auto_flags, fld->field_name),
      m_field(fld) {
  if (fld->is_unsigned) {
    flags |= UNSIGNED_FLAG;
  }
}

Item_result Create_field_wrapper::result_type() const {
  return field_types_result_type[field_type2index(m_field->sql_type)];
}

Item_result Create_field_wrapper::numeric_context_result_type() const {
  return ::numeric_context_result_type(type(), result_type(),
                                       m_field->decimals);
}

enum_field_types Create_field_wrapper::type() const {
  return m_field->sql_type;
}

const CHARSET_INFO *Create_field_wrapper::charset() const {
  return m_field->charset;
}

uint32 Create_field_wrapper::pack_length() const {
  return m_field->pack_length();
}

uint32 Create_field_wrapper::max_display_length() const {
  return m_field->max_display_width_in_codepoints();
}

/**
  Generate a Create_field from an Item.

  This function generates a Create_field from an Item by first creating a
  temporary table Field from the Item, and then creating the Create_field from
  this Field (there is currently no way to go directly from Item to
  Create_field). It is used several places:
  - In CREATE TABLE AS SELECT for creating the target table definition.
  - In functional indexes for creating the hidden generated column from the
    indexed expression.

  @param thd       Thread handler
  @param item      The item to generate a Create_field from
  @param tmp_table A table object which is used to generate a temporary table
                   field, as described above. This doesn't need to be an
                   existing table.
  @return          A Create_field generated from the input item, or nullptr
                   in case of errors.
*/
Create_field *generate_create_field(THD *thd, Item *item, TABLE *tmp_table) {
  Field *tmp_table_field;
  if (item->type() == Item::FUNC_ITEM) {
    /*
      If the function returns an array, use the method provided by the function
      to create the tmp table field, as the generic
      tmp_table_field_from_field_type() can't handle typed arrays.
    */
    if (item->result_type() != STRING_RESULT || item->returns_array())
      tmp_table_field = item->tmp_table_field(tmp_table);
    else
      tmp_table_field = item->tmp_table_field_from_field_type(tmp_table, false);
  } else {
    Field *from_field, *default_field;
    tmp_table_field = create_tmp_field(thd, tmp_table, item, item->type(),
                                       nullptr, &from_field, &default_field,
                                       false, false, false, false, false);
  }

  if (!tmp_table_field) {
    return nullptr; /* purecov: inspected */
  }

  Field *table_field;

  switch (item->type()) {
    /*
      We have to take into account both the real table's fields and
      pseudo-fields used in trigger's body. These fields are used
      to copy defaults values later inside constructor of
      the class Create_field.
     */
    case Item::FIELD_ITEM:
    case Item::TRIGGER_FIELD_ITEM:
      table_field = ((Item_field *)item)->field;
      break;
    default: {
      /*
        If the expression is of temporal type having date and non-nullable,
        a zero date is generated. If in strict mode, then zero date is
        invalid. For such cases no default is generated.
       */
      table_field = nullptr;
      if (is_temporal_type_with_date(tmp_table_field->type()) &&
          thd->is_strict_sql_mode() && !item->maybe_null)
        tmp_table_field->flags |= NO_DEFAULT_VALUE_FLAG;
    }
  }

  DBUG_ASSERT(tmp_table_field->gcol_info == nullptr &&
              tmp_table_field->stored_in_db);
  Create_field *cr_field =
      new (thd->mem_root) Create_field(tmp_table_field, table_field);

  if (!cr_field) {
    return nullptr; /* purecov: inspected */
  }

  // Mark if collation was specified explicitly by user for the column.
  if (item->type() == Item::FIELD_ITEM) {
    const TABLE *table = table_field->orig_table;
    DBUG_ASSERT(table);
    const dd::Table *table_obj =
        table->s->tmp_table ? table->s->tmp_table_def : nullptr;

    if (!table_obj && table->s->table_category != TABLE_UNKNOWN_CATEGORY) {
      dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

      if (thd->dd_client()->acquire(table->s->db.str, table->s->table_name.str,
                                    &table_obj)) {
        return nullptr; /* purecov: inspected */
      }
    }

    cr_field->is_explicit_collation = false;
    if (table_obj) {
      const dd::Column *c = table_obj->get_column(table_field->field_name);
      if (c) cr_field->is_explicit_collation = c->is_explicit_collation();
    }
  }

  if (item->maybe_null) cr_field->flags &= ~NOT_NULL_FLAG;

  return cr_field;
}

const char *get_field_name_or_expression(THD *thd, const Field *field) {
  if (field->is_field_for_functional_index()) {
    String expression_buffer;
    field->gcol_info->print_expr(thd, &expression_buffer);
    return thd->strmake(expression_buffer.ptr(), expression_buffer.length());
  }

  return field->field_name;
}
