/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/dd_table.h"

#include <string.h>
#include <algorithm>
#include <memory>  // unique_ptr

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/collection.h"               // dd::Collection
#include "sql/dd/dd.h"                       // dd::get_dictionary
#include "sql/dd/dictionary.h"               // dd::Dictionary
// TODO: Avoid exposing dd/impl headers in public files.
#include "sql/dd/impl/dictionary_impl.h"       // default_catalog_name
#include "sql/dd/impl/system_registry.h"       // dd::System_tables
#include "sql/dd/impl/tables/dd_properties.h"  // dd::tables:.DD_properties
#include "sql/dd/impl/utils.h"                 // dd::escape
#include "sql/dd/performance_schema/init.h"    // performance_schema::
                                               //   set_PS_version_for_table
#include "sql/create_field.h"
#include "sql/dd/dd_version.h"  // DD_VERSION
#include "sql/dd/properties.h"  // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/tablespace_id_owner_visitor.h"  // visit_tablespace_id_owners
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/check_constraint.h"     // dd::Check_constraint
#include "sql/dd/types/column.h"               // dd::Column
#include "sql/dd/types/column_type_element.h"  // dd::Column_type_element
#include "sql/dd/types/foreign_key.h"          // dd::Foreign_key
#include "sql/dd/types/foreign_key_element.h"  // dd::Foreign_key_element
#include "sql/dd/types/index.h"                // dd::Index
#include "sql/dd/types/index_element.h"        // dd::Index_element
#include "sql/dd/types/object_table.h"         // dd::Object_table
#include "sql/dd/types/partition.h"            // dd::Partition
#include "sql/dd/types/partition_index.h"      // dd::Partition_index
#include "sql/dd/types/partition_value.h"      // dd::Partition_value
#include "sql/dd/types/schema.h"               // dd::Schema
#include "sql/dd/types/table.h"                // dd::Table
#include "sql/dd/types/tablespace.h"           // dd::Tablespace
#include "sql/dd_table_share.h"                // is_suitable_for_primary_key
#include "sql/debug_sync.h"                    // DEBUG_SYNC
#include "sql/default_values.h"                // max_pack_length
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/handler.h"  // FK_NAME_DEFAULT_SUFFIX
#include "sql/item.h"
#include "sql/key.h"
#include "sql/key_spec.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mem_root_array.h"
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // lower_case_table_names
#include "sql/partition_element.h"
#include "sql/partition_info.h"        // partition_info
#include "sql/psi_memory_key.h"        // key_memory_frm
#include "sql/sql_check_constraint.h"  // Sql_check_constraint_spec_list
#include "sql/sql_class.h"             // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"
#include "sql/sql_partition.h"  // expr_to_string
#include "sql/sql_plugin_ref.h"
#include "sql/sql_show.h"
#include "sql/sql_table.h"  // primary_key_name
#include "sql/srs_fetcher.h"
#include "sql/strfunc.h"  // lex_cstring_handle
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql_string.h"
#include "typelib.h"

namespace dd {

/**
  Convert to and from new enum types in DD framework to current MySQL
  server enum types. We have plans to retain both old and new enum
  values in DD tables so as to handle client compatibility and
  information schema requirements.
*/

dd::enum_column_types get_new_field_type(enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_DECIMAL:
      return dd::enum_column_types::DECIMAL;

    case MYSQL_TYPE_TINY:
      return dd::enum_column_types::TINY;

    case MYSQL_TYPE_SHORT:
      return dd::enum_column_types::SHORT;

    case MYSQL_TYPE_LONG:
      return dd::enum_column_types::LONG;

    case MYSQL_TYPE_FLOAT:
      return dd::enum_column_types::FLOAT;

    case MYSQL_TYPE_DOUBLE:
      return dd::enum_column_types::DOUBLE;

    case MYSQL_TYPE_NULL:
      return dd::enum_column_types::TYPE_NULL;

    case MYSQL_TYPE_TIMESTAMP:
      return dd::enum_column_types::TIMESTAMP;

    case MYSQL_TYPE_LONGLONG:
      return dd::enum_column_types::LONGLONG;

    case MYSQL_TYPE_INT24:
      return dd::enum_column_types::INT24;

    case MYSQL_TYPE_DATE:
      return dd::enum_column_types::DATE;

    case MYSQL_TYPE_TIME:
      return dd::enum_column_types::TIME;

    case MYSQL_TYPE_DATETIME:
      return dd::enum_column_types::DATETIME;

    case MYSQL_TYPE_YEAR:
      return dd::enum_column_types::YEAR;

    case MYSQL_TYPE_NEWDATE:
      return dd::enum_column_types::NEWDATE;

    case MYSQL_TYPE_VARCHAR:
      return dd::enum_column_types::VARCHAR;

    case MYSQL_TYPE_BIT:
      return dd::enum_column_types::BIT;

    case MYSQL_TYPE_TIMESTAMP2:
      return dd::enum_column_types::TIMESTAMP2;

    case MYSQL_TYPE_DATETIME2:
      return dd::enum_column_types::DATETIME2;

    case MYSQL_TYPE_TIME2:
      return dd::enum_column_types::TIME2;

    case MYSQL_TYPE_NEWDECIMAL:
      return dd::enum_column_types::NEWDECIMAL;

    case MYSQL_TYPE_ENUM:
      return dd::enum_column_types::ENUM;

    case MYSQL_TYPE_SET:
      return dd::enum_column_types::SET;

    case MYSQL_TYPE_TINY_BLOB:
      return dd::enum_column_types::TINY_BLOB;

    case MYSQL_TYPE_MEDIUM_BLOB:
      return dd::enum_column_types::MEDIUM_BLOB;

    case MYSQL_TYPE_LONG_BLOB:
      return dd::enum_column_types::LONG_BLOB;

    case MYSQL_TYPE_BLOB:
      return dd::enum_column_types::BLOB;

    case MYSQL_TYPE_VAR_STRING:
      return dd::enum_column_types::VAR_STRING;

    case MYSQL_TYPE_STRING:
      return dd::enum_column_types::STRING;

    case MYSQL_TYPE_GEOMETRY:
      return dd::enum_column_types::GEOMETRY;

    case MYSQL_TYPE_JSON:
      return dd::enum_column_types::JSON;

    default:
      break;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "field type.");
  DBUG_ASSERT(false);

  return dd::enum_column_types::LONG;
  /* purecov: end */
}

/**
  Function returns string representing column type by Create_field.
  This is required for the IS implementation which uses views on DD
*/

dd::String_type get_sql_type_by_create_field(TABLE *table,
                                             const Create_field &field) {
  DBUG_TRACE;

  unique_ptr_destroy_only<Field> fld(make_field(field, table->s));
  fld->init(table);

  // Read column display type.
  char tmp[MAX_FIELD_WIDTH];
  String type(tmp, sizeof(tmp), system_charset_info);
  fld->sql_type(type);

  dd::String_type col_display_str(type.ptr(), type.length());

  return col_display_str;
}

/**
  Helper method to get default value of column in the string
  format.  The default value prepared from this methods is stored
  in the columns.default_value_utf8. This information is mostly
  used by the I_S queries only.
  For others, default value can be obtained from the columns.default_values.

  @param[in]      buf        Default value buffer.
  @param[in]      table      Table object.
  @param[in]      field      Field information.
  @param[in]      col_obj    DD column object for the field.
  @param[out]     def_value  Default value is stored in the string format if
                             non-NULL default value is specified for the column.
                             Empty string is stored if no default value is
                             specified for the column.
                             def_value is *not* set if default value for the
                             column is nullptr.
*/

static void prepare_default_value_string(uchar *buf, TABLE *table,
                                         const Create_field &field,
                                         dd::Column *col_obj,
                                         String *def_value) {
  // Create a fake field with the default value buffer 'buf'.
  unique_ptr_destroy_only<Field> f(
      make_field(field, table->s, buf + 1, buf, 0));
  f->init(table);

  if (col_obj->has_no_default()) f->flags |= NO_DEFAULT_VALUE_FLAG;

  const bool has_default =
      (f->type() != FIELD_TYPE_BLOB && !(f->flags & NO_DEFAULT_VALUE_FLAG) &&
       !(f->auto_flags & Field::NEXT_NUMBER));

  if (f->gcol_info || !has_default) return;

  // If we have DEFAULT (expression)
  if (field.m_default_val_expr) {
    // Convert from Basic_string to String
    String default_val_expr(col_obj->default_option().c_str(),
                            col_obj->default_option().length(),
                            &my_charset_bin);
    // convert the expression stored in default_option to UTF8
    convert_and_print(&default_val_expr, def_value, system_charset_info);
    return;
  }

  // If we have DEFAULT NOW()
  if (f->has_insert_default_datetime_value_expression()) {
    def_value->copy(STRING_WITH_LEN("CURRENT_TIMESTAMP"), system_charset_info);
    if (f->decimals() > 0) def_value->append_parenthesized(f->decimals());

    return;
  }

  // If NOT NULL
  if (!f->is_null()) {
    char tmp[MAX_FIELD_WIDTH];
    String type(tmp, sizeof(tmp), f->charset());
    const bool is_binary_type =
        ((f->type() == MYSQL_TYPE_VARCHAR || f->type() == MYSQL_TYPE_STRING) &&
         (f->flags & BINARY_FLAG) && f->charset() == &my_charset_bin);

    if (f->type() == MYSQL_TYPE_BIT) {
      longlong dec = f->val_int();
      char *ptr = longlong2str(dec, tmp + 2, 2);
      uint32 length = (uint32)(ptr - tmp);
      tmp[0] = 'b';
      tmp[1] = '\'';
      tmp[length] = '\'';
      type.length(length + 1);
    } else if (is_binary_type) {
      String type2;
      char *ptr = type.c_ptr_safe();

      // Get the default value.
      f->val_str(&type2);

      if (type2.length() > 0) {
        /*
          The default value for BINARY and VARBINARY type is converted to the
          hex string if hex format is used for default value at the parsing
          stage. Converting hex string to system_charset_info charset while
          storing value in DD table might fail because of unsupported byte
          value in hex string. Hence converting default value to printable
          HEX encoded string before store.

          The original format as supplied by user is lost after parsing stage.
          So regardless of the type specified by the user, default for
          varbinary/binary is stored in the printable HEX encoded format.
          I_S queries and SHOW COLUMNS always list such default value in HEX
          format instead of user specified one.
        */
        *ptr++ = '0';
        *ptr++ = 'x';
        size_t len = bin_to_hex_str(ptr, type.length() - 2, type2.c_ptr_safe(),
                                    strlen(type2.c_ptr_safe()));
        type.length(len + 2);
      } else
        // For BINARY(0) and VARBINARY type with empty string as default value.
        f->val_str(&type);
    } else
      f->val_str(&type);

    if (type.length()) {
      uint dummy_errors;
      def_value->copy(type.ptr(), type.length(), f->charset(),
                      system_charset_info, &dummy_errors);
    } else
      def_value->copy(STRING_WITH_LEN(""), system_charset_info);
  }
}

/**
  Helper method to get numeric scale for types using
  Create_field type object.
*/
bool get_field_numeric_scale(const Create_field *field, uint *scale) {
  DBUG_ASSERT(*scale == 0);

  switch (field->sql_type) {
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      /* For these types we show NULL in I_S if scale was not given. */
      if (field->decimals != DECIMAL_NOT_SPECIFIED) {
        *scale = field->decimals;
        return false;
      }
      break;
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_DECIMAL:
      *scale = field->decimals;
      return false;
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONGLONG:
      DBUG_ASSERT(field->decimals == 0);
    default:
      return true;
  }

  return true;
}

/**
  Helper method to get numeric precision for types using
  Create_field type object.
*/
bool get_field_numeric_precision(const Create_field *field,
                                 uint *numeric_precision) {
  switch (field->sql_type) {
      // these value is taken from Field_XXX::max_display_length() -1
    case MYSQL_TYPE_TINY:
      *numeric_precision = 3;
      return false;
    case MYSQL_TYPE_SHORT:
      *numeric_precision = 5;
      return false;
    case MYSQL_TYPE_INT24:
      *numeric_precision = 7;
      return false;
    case MYSQL_TYPE_LONG:
      *numeric_precision = 10;
      return false;
    case MYSQL_TYPE_LONGLONG:
      if (field->is_unsigned)
        *numeric_precision = 20;
      else
        *numeric_precision = 19;

      return false;
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      *numeric_precision = field->max_display_width_in_codepoints();
      return false;
    case MYSQL_TYPE_DECIMAL: {
      uint tmp = field->max_display_width_in_codepoints();
      if (!field->is_unsigned) tmp--;
      if (field->decimals) tmp--;
      *numeric_precision = tmp;
      return false;
    }
    case MYSQL_TYPE_NEWDECIMAL:
      *numeric_precision = my_decimal_length_to_precision(
          field->max_display_width_in_codepoints(), field->decimals,
          field->is_unsigned);
      return false;
    default:
      return true;
  }

  return true;
}

/**
  Helper method to get datetime precision for types using
  Create_field type object.
*/
bool get_field_datetime_precision(const Create_field *field,
                                  uint *datetime_precision) {
  switch (field->sql_type) {
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
      *datetime_precision =
          field->max_display_width_in_codepoints() > MAX_DATETIME_WIDTH
              ? (field->max_display_width_in_codepoints() - 1 -
                 MAX_DATETIME_WIDTH)
              : 0;
      return false;
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
      *datetime_precision =
          field->max_display_width_in_codepoints() > MAX_TIME_WIDTH
              ? (field->max_display_width_in_codepoints() - 1 - MAX_TIME_WIDTH)
              : 0;
      return false;
    default:
      return true;
  }

  return true;
}

static dd::String_type now_with_opt_decimals(uint decimals) {
  char buff[17 + 1 + 1 + 1 + 1];
  String val(buff, sizeof(buff), &my_charset_bin);
  val.length(0);
  val.append("CURRENT_TIMESTAMP");
  if (decimals > 0) val.append_parenthesized(decimals);
  return dd::String_type(val.ptr(), val.length());
}

/**
  Add column objects to dd::Abstract_table according to list of Create_field
  objects.
*/

bool fill_dd_columns_from_create_fields(THD *thd, dd::Abstract_table *tab_obj,
                                        const List<Create_field> &create_fields,
                                        handler *file) {
  // Helper class which takes care of restoration of
  // THD::check_for_truncated_fields after it was temporarily changed to
  // CHECK_FIELD_WARN in order to prepare default values and freeing buffer
  // which is allocated for the same purpose.
  class Context_handler {
   private:
    THD *m_thd;
    uchar *m_buf;
    enum_check_fields m_check_for_truncated_fields;

   public:
    Context_handler(THD *thd, uchar *buf)
        : m_thd(thd),
          m_buf(buf),
          m_check_for_truncated_fields(m_thd->check_for_truncated_fields) {
      // Set to warn about wrong default values.
      m_thd->check_for_truncated_fields = CHECK_FIELD_WARN;
    }
    ~Context_handler() {
      // Delete buffer and restore context.
      my_free(m_buf);
      m_thd->check_for_truncated_fields = m_check_for_truncated_fields;
    }
  };

  // Allocate buffer large enough to hold the largest field. Add one byte
  // of potential null bit and leftover bits.
  size_t bufsize = 1 + max_pack_length(create_fields);

  // When accessing leftover bits in the preamble while preparing default
  // values, the get_rec_buf() function applied will assume the buffer
  // size to be at least two bytes.
  bufsize = std::max<size_t>(2, bufsize);
  uchar *buf = reinterpret_cast<uchar *>(
      my_malloc(key_memory_DD_default_values, bufsize, MYF(MY_WME)));

  if (!buf) return true; /* purecov: inspected */

  // Use RAII to save old context and restore at function return.
  Context_handler save_and_restore_thd_context(thd, buf);

  // We need a fake table and share to generate the default values.
  // We prepare these once, and reuse them for all fields.
  TABLE table;
  TABLE_SHARE share;
  table.s = &share;
  table.in_use = thd;
  table.s->db_low_byte_first = file->low_byte_first();

  //
  // Iterate through all the table columns
  //
  for (const Create_field &field : create_fields) {
    //
    // Add new DD column
    //

    dd::Column *col_obj = tab_obj->add_column();

    col_obj->set_name(field.field_name);

    col_obj->set_type(dd::get_new_field_type(field.sql_type));

    col_obj->set_char_length(field.max_display_width_in_bytes());

    // Set result numeric scale.
    uint value = 0;
    if (get_field_numeric_scale(&field, &value) == false)
      col_obj->set_numeric_scale(value);

    // Set result numeric precision.
    if (get_field_numeric_precision(&field, &value) == false)
      col_obj->set_numeric_precision(value);

    // Set result datetime precision.
    if (get_field_datetime_precision(&field, &value) == false)
      col_obj->set_datetime_precision(value);

    col_obj->set_nullable(field.is_nullable);

    col_obj->set_unsigned(field.is_unsigned);

    col_obj->set_zerofill(field.is_zerofill);

    col_obj->set_srs_id(field.m_srid);

    // Check that the hidden type isn't the type that is used internally by
    // storage engines.
    DBUG_ASSERT(field.hidden != dd::Column::enum_hidden_type::HT_HIDDEN_SE);
    col_obj->set_hidden(field.hidden);

    /*
      AUTO_INCREMENT, DEFAULT/ON UPDATE CURRENT_TIMESTAMP properties are
      stored in Create_field::auto_flags.
    */
    if (field.auto_flags & Field::DEFAULT_NOW)
      col_obj->set_default_option(now_with_opt_decimals(field.decimals));

    if (field.auto_flags & Field::ON_UPDATE_NOW)
      col_obj->set_update_option(now_with_opt_decimals(field.decimals));

    col_obj->set_auto_increment((field.auto_flags & Field::NEXT_NUMBER) != 0);

    // Handle generated default
    if (field.m_default_val_expr) {
      char buffer[128];
      String default_val_expr(buffer, sizeof(buffer), &my_charset_bin);
      // Convert the expression from Item* to text
      field.m_default_val_expr->print_expr(thd, &default_val_expr);
      col_obj->set_default_option(
          dd::String_type(default_val_expr.ptr(), default_val_expr.length()));
    }

    // Handle generated columns
    if (field.gcol_info) {
      /*
        It is important to normalize the expression's text into the DD, to
        make it independent from sql_mode. For example, 'a||b' means 'a OR b'
        or 'CONCAT(a,b)', depending on if PIPES_AS_CONCAT is on. Using
        Item::print(), we get self-sufficient text containing 'OR' or
        'CONCAT'. If sql_mode later changes, it will not affect the column.
       */
      char buffer[128];
      String gc_expr(buffer, sizeof(buffer), &my_charset_bin);
      col_obj->set_virtual(!field.stored_in_db);
      field.gcol_info->print_expr(thd, &gc_expr);
      col_obj->set_generation_expression(
          dd::String_type(gc_expr.ptr(), gc_expr.length()));

      // Prepare UTF expression for IS.
      String gc_expr_for_IS;
      convert_and_print(&gc_expr, &gc_expr_for_IS, system_charset_info);

      col_obj->set_generation_expression_utf8(
          dd::String_type(gc_expr_for_IS.ptr(), gc_expr_for_IS.length()));
    }

    if (field.comment.str && field.comment.length)
      col_obj->set_comment(
          dd::String_type(field.comment.str, field.comment.length));

    // Collation ID
    col_obj->set_collation_id(field.charset->number);

    // Was collation supplied explicitly ?
    col_obj->set_is_explicit_collation(field.is_explicit_collation);

    /*
      Store numeric scale for types relying on this info (old and new decimal
      and floating point types). Also store 0 for integer types to simplify I_S
      implementation.
    */
    switch (field.sql_type) {
      case MYSQL_TYPE_FLOAT:
      case MYSQL_TYPE_DOUBLE:
        /* For these types we show NULL in I_S if scale was not given. */
        if (field.decimals != DECIMAL_NOT_SPECIFIED)
          col_obj->set_numeric_scale(field.decimals);
        else {
          DBUG_ASSERT(col_obj->is_numeric_scale_null());
        }
        break;
      case MYSQL_TYPE_NEWDECIMAL:
      case MYSQL_TYPE_DECIMAL:
        col_obj->set_numeric_scale(field.decimals);
        break;
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONGLONG:
        DBUG_ASSERT(field.decimals == 0);
        col_obj->set_numeric_scale(0);
        break;
      default:
        DBUG_ASSERT(col_obj->is_numeric_scale_null());
        break;
    }

    //
    // Set options
    //

    dd::Properties *col_options = &col_obj->options();

    /*
      Store flag indicating whether BIT type storage optimized or not.
      We need to store this flag in DD to correctly handle the case
      when SE starts supporting optimized BIT storage but still needs
      to handle correctly columns which were created before this change.
    */
    if (field.sql_type == MYSQL_TYPE_BIT)
      col_options->set("treat_bit_as_char", field.treat_bit_as_char);

    // Store geometry sub type
    if (field.sql_type == MYSQL_TYPE_GEOMETRY) {
      col_options->set("geom_type", field.geom_type);
    }

    // Field storage media and column format options
    if (field.field_storage_type() != HA_SM_DEFAULT)
      col_options->set("storage",
                       static_cast<uint32>(field.field_storage_type()));

    if (field.column_format() != COLUMN_FORMAT_TYPE_DEFAULT)
      col_options->set("column_format",
                       static_cast<uint32>(field.column_format()));

    // NOT SECONDARY column option.
    if (field.flags & NOT_SECONDARY_FLAG)
      col_options->set("not_secondary", true);

    if (field.is_array) {
      col_options->set("is_array", true);
    }

    //
    // Write intervals
    //
    uint i = 0;
    if (field.interval) {
      uchar buff[MAX_FIELD_WIDTH];
      String tmp((char *)buff, sizeof(buff), &my_charset_bin);
      tmp.length(0);

      for (const char **pos = field.interval->type_names; *pos; pos++) {
        //
        // Create enum/set object
        //
        DBUG_ASSERT(col_obj->type() == dd::enum_column_types::SET ||
                    col_obj->type() == dd::enum_column_types::ENUM);

        dd::Column_type_element *elem_obj = col_obj->add_element();

        //  Copy type_lengths[i] bytes including '\0'
        //  This helps store typelib names that are of different charsets.
        dd::String_type interval_name(*pos, field.interval->type_lengths[i]);
        elem_obj->set_name(interval_name);

        i++;
      }
    }

    // Store column display type in dd::Column
    col_obj->set_column_type_utf8(get_sql_type_by_create_field(&table, field));

    // Store element count in dd::Column
    col_options->set("interval_count", i);

    // Store geometry sub type
    if (field.sql_type == MYSQL_TYPE_GEOMETRY) {
      col_options->set("geom_type", field.geom_type);
    }

    // Reset the buffer and assign the column's default value.
    memset(buf, 0, bufsize);
    if (prepare_default_value(thd, buf, &table, field, col_obj)) return true;

    /**
      Storing default value specified for column in
      columns.default_value_utf8.  The values are stored in
      string form here. This information is mostly used by the
      I_S queries. For others, default value can be obtained from
      the columns.default_values.

      So now column.default_value_utf8 is not just used for
      storing "CURRENT_TIMESTAMP" for timestamp columns but also
      used to hold the default value of column of all types.

      To get the default value in string form, buffer "buf"
      prepared in prepare_default_value() is used.
    */
    String def_val;
    prepare_default_value_string(buf, &table, field, col_obj, &def_val);
    if (def_val.ptr() != nullptr)
      col_obj->set_default_value_utf8(
          dd::String_type(def_val.ptr(), def_val.length()));
  }

  return false;
}

static dd::Index::enum_index_algorithm dd_get_new_index_algorithm_type(
    enum ha_key_alg type) {
  switch (type) {
    case HA_KEY_ALG_SE_SPECIFIC:
      return dd::Index::IA_SE_SPECIFIC;

    case HA_KEY_ALG_BTREE:
      return dd::Index::IA_BTREE;

    case HA_KEY_ALG_RTREE:
      return dd::Index::IA_RTREE;

    case HA_KEY_ALG_HASH:
      return dd::Index::IA_HASH;

    case HA_KEY_ALG_FULLTEXT:
      return dd::Index::IA_FULLTEXT;
  }

  /* purecov: begin deadcode */
  LogErr(ERROR_LEVEL, ER_DD_FAILSAFE, "index algorithm.");
  DBUG_ASSERT(false);

  return dd::Index::IA_SE_SPECIFIC;
  /* purecov: end */
}

static dd::Index::enum_index_type dd_get_new_index_type(const KEY *key) {
  if (key->flags & HA_FULLTEXT) return dd::Index::IT_FULLTEXT;

  if (key->flags & HA_SPATIAL) return dd::Index::IT_SPATIAL;

  if (key->flags & HA_NOSAME) {
    /*
      mysql_prepare_create_table() marks PRIMARY KEY by assigning
      KEY::name special value. We rely on this here and in several
      other places in server (e.g. in sort_keys()).
    */
    if (key->name == primary_key_name)
      return dd::Index::IT_PRIMARY;
    else
      return dd::Index::IT_UNIQUE;
  }

  return dd::Index::IT_MULTIPLE;
}

/**
  Add dd::Index_element objects to dd::Index/Table according to
  KEY_PART_INFO array for the index.
*/

static void fill_dd_index_elements_from_key_parts(
    const dd::Table *tab_obj, dd::Index *idx_obj, uint key_part_count,
    const KEY_PART_INFO *key_parts, handler *file, bool is_primary_key) {
  //
  // Iterate through all the index element
  //

  const KEY_PART_INFO *key_part = key_parts;
  const KEY_PART_INFO *key_part_end = key_parts + key_part_count;

  for (uint key_part_no = 0; key_part != key_part_end;
       ++key_part, ++key_part_no) {
    //
    // Get reference to column object
    //

    const dd::Column *key_col_obj = nullptr;

    {
      int i = 0;
      for (const dd::Column *c : tab_obj->columns()) {
        // Skip hidden columns
        if (c->is_se_hidden()) continue;

        if (i == key_part->fieldnr) {
          key_col_obj = c;
          break;
        }
        i++;
      }
    }
    DBUG_ASSERT(key_col_obj);

    //
    // Create new index element object
    //

    if (key_col_obj->column_key() == dd::Column::CK_NONE) {
      // We might have a unique key that would be promoted as PRIMARY
      dd::Index::enum_index_type idx_type = idx_obj->type();
      if (is_primary_key) idx_type = dd::Index::IT_PRIMARY;

      switch (idx_type) {
        case dd::Index::IT_PRIMARY:
          const_cast<dd::Column *>(key_col_obj)
              ->set_column_key(dd::Column::CK_PRIMARY);
          break;
        case dd::Index::IT_UNIQUE:
          if (key_part == key_parts) {
            if (key_part_count == 1)
              const_cast<dd::Column *>(key_col_obj)
                  ->set_column_key(dd::Column::CK_UNIQUE);
            else
              const_cast<dd::Column *>(key_col_obj)
                  ->set_column_key(dd::Column::CK_MULTIPLE);
          }
          break;
        case dd::Index::IT_MULTIPLE:
        case dd::Index::IT_FULLTEXT:
        case dd::Index::IT_SPATIAL:
          if (key_part == key_parts)
            const_cast<dd::Column *>(key_col_obj)
                ->set_column_key(dd::Column::CK_MULTIPLE);
          break;
        default:
          DBUG_ASSERT(!"Invalid index type");
          break;
      }
    }

    dd::Index_element *idx_elem =
        idx_obj->add_element(const_cast<dd::Column *>(key_col_obj));

    idx_elem->set_length(key_part->length);
    idx_elem->set_order(key_part->key_part_flag & HA_REVERSE_SORT
                            ? Index_element::ORDER_DESC
                            : Index_element::ORDER_ASC);

    //
    // Set index order
    //

    if (file->index_flags(idx_obj->ordinal_position() - 1, key_part_no, false) &
        HA_READ_ORDER)
      idx_elem->set_order(key_part->key_part_flag & HA_REVERSE_SORT
                              ? dd::Index_element::ORDER_DESC
                              : dd::Index_element::ORDER_ASC);
    else
      idx_elem->set_order(dd::Index_element::ORDER_UNDEF);
  }
}

//  Check if a given key is candidate to be promoted to primary key.
static bool is_candidate_primary_key(THD *thd, const KEY *key,
                                     const List<Create_field> &create_fields) {
  KEY_PART_INFO *key_part;
  KEY_PART_INFO *key_part_end = key->key_part + key->user_defined_key_parts;

  if (!(key->flags & HA_NOSAME) || (key->flags & HA_NULL_PART_KEY))
    return false;

  if (key->flags & HA_VIRTUAL_GEN_KEY) return false;

  // Use temporary objects to get Field*
  TABLE_SHARE share;
  TABLE table;
  table.s = &share;
  table.in_use = thd;

  for (key_part = key->key_part; key_part < key_part_end; key_part++) {
    /* Create the Create_field object for this key_part */

    Create_field *cfield;
    List_iterator<Create_field> it(
        const_cast<List<Create_field> &>(create_fields));
    int i = 0;
    while ((cfield = it++)) {
      if (i == key_part->fieldnr) break;
      i++;
    }
    if (cfield->is_array) return false;
    /* Prepare Field* object from Create_field */

    unique_ptr_destroy_only<Field> table_field(make_field(*cfield, table.s));
    table_field->init(&table);

    if (is_suitable_for_primary_key(key_part, table_field.get()) == false)
      return false;
  }

  return true;
}

/** Add index objects to dd::Table according to array of KEY structures. */
static void fill_dd_indexes_from_keyinfo(
    THD *thd, dd::Table *tab_obj, uint key_count, const KEY *keyinfo,
    const List<Create_field> &create_fields, handler *file) {
  /**
    Currently the index order type is not persisted in new DD or in .FRM. In
    I_S with new DD index order is calculated from the index type. That is,
    the index order is always calculated as ascending except for FULLTEXT and
    HASH index.
    Type of index ordering(ASC/DESC/UNDEF) is property of handler and index
    type. With the proper handler and the index type, index order type can be
    easily retrieved.
    So here using keyinfo with table share of handler to get the index order
    type. If table share does not exist for handler then dummy_table_share
    is created. Index order type value is stored in the
    index_column_usage.index_order.

    Note:
      The keyinfo prepared here is some what different from one prepared at
      table opening time.
      For example: actual_flags, unused_key_parts, usable_key_parts,
                   rec_per_key, rec_per_key_float ...
                   member of keyinfo might be different in one prepared at
                   table opening time.

      But index_flags() implementations mostly uses algorithm and flags members
      of keyinfo to get the flag value. Apparently these members are not
      different from the one prepared at table opening time. So approach to get
      index order type from keyinfo works fine.

    Alternative approach:
      Introduce a new handler API to get index order type using the index type.
      Usage of dummy_table_share and backup variables to reset handler's table
      share can be avoided with this approach.

    TODO:Refine approach during the complete WL6599 review by dlenev.
  */
  TABLE_SHARE dummy_table_share;
  uint *pk_key_nr = nullptr;
  uint pk_key_nr_bkp = 0;
  KEY *key_info_bkp = nullptr;

  TABLE_SHARE *table_share = const_cast<TABLE_SHARE *>(file->get_table_share());
  if (table_share == nullptr) {
    dummy_table_share.key_info = const_cast<KEY *>(keyinfo);
    /*
      Primary key number in table share is set while iterating through all
      the indexes.
    */
    pk_key_nr = &dummy_table_share.primary_key;
    file->change_table_ptr(nullptr, &dummy_table_share);
  } else {
    /*
      keyinfo and primary key number from it is used with the table_share here
      to get the index order type. So before assigning keyinfo and primary key
      number to table_share, backup current key info and primary key number.
    */
    key_info_bkp = table_share->key_info;
    pk_key_nr_bkp = table_share->primary_key;
    /*
      Primary key number in table share is set while iterating through all
      the indexes.
    */
    pk_key_nr = &table_share->primary_key;
    table_share->key_info = const_cast<KEY *>(keyinfo);
  }

  //
  // Iterate through all the indexes
  //

  const KEY *key = keyinfo;
  const KEY *end = keyinfo + key_count;

  const KEY *primary_key_info = nullptr;
  for (int key_nr = 1; key != end; ++key, ++key_nr) {
    //
    // Add new DD index
    //

    dd::Index *idx_obj = tab_obj->add_index();

    idx_obj->set_name(key->name);

    idx_obj->set_algorithm(dd_get_new_index_algorithm_type(key->algorithm));
    idx_obj->set_algorithm_explicit(key->is_algorithm_explicit);
    idx_obj->set_visible(key->is_visible);

    if (dd_get_new_index_type(key) == dd::Index::IT_PRIMARY) {
      *pk_key_nr = key_nr - 1;
      primary_key_info = key;
    }

    idx_obj->set_type(dd_get_new_index_type(key));

    idx_obj->set_generated(key->flags & HA_GENERATED_KEY);

    if (key->comment.str)
      idx_obj->set_comment(
          dd::String_type(key->comment.str, key->comment.length));

    idx_obj->set_engine(tab_obj->engine());
    idx_obj->set_visible(key->is_visible);

    //
    // Set options
    //

    dd::Properties *idx_options = &idx_obj->options();

    /*
      Most of flags in KEY::flags bitmap can be easily calculated from other
      attributes of Index, Index_element or Column objects, so we avoid
      storing this redundant information in DD.

      HA_PACK_KEY and HA_BINARY_PACK_KEY are special in this respect. Even
      though we calculate them on the basis of key part attributes, they,
      unlike other flags, do not reflect immanent property of key or its
      parts, but rather reflect our decision to apply certain optimization
      in the specific case. So it is better to store these flags explicitly
      in DD in order to avoid problems with binary compatibility if we decide
      to change conditions in which optimization is applied in future releases.
    */
    idx_options->set("flags",
                     (key->flags & (HA_PACK_KEY | HA_BINARY_PACK_KEY)));

    if (key->block_size) idx_options->set("block_size", key->block_size);

    if (key->parser_name.str)
      idx_options->set("parser_name", key->parser_name.str);

    /*
      If we have no primary key, then we pick the first candidate primary
      key and promote it. When we promote, the field's of key_part needs to
      be marked as PRIMARY. So we find the candidate key and convey to
      fill_dd_index_elements_from_key_parts() about the same.
    */
    if (primary_key_info == nullptr &&
        is_candidate_primary_key(thd, key, create_fields)) {
      primary_key_info = key;
    }

    // Add Index elements
    fill_dd_index_elements_from_key_parts(
        tab_obj, idx_obj, key->user_defined_key_parts, key->key_part, file,
        key == primary_key_info);
  }

  if (table_share == nullptr)
    file->change_table_ptr(nullptr, nullptr);
  else {
    table_share->key_info = key_info_bkp;
    table_share->primary_key = pk_key_nr_bkp;
  }
}

/**
  Translate from the old fk_option enum to the new
  dd::Foreign_key::enum_rule enum.

  @param opt  old fk_option enum.
  @return     new dd::Foreign_key::enum_rule
*/

static dd::Foreign_key::enum_rule get_fk_rule(fk_option opt) {
  switch (opt) {
    case FK_OPTION_RESTRICT:
      return dd::Foreign_key::RULE_RESTRICT;
    case FK_OPTION_CASCADE:
      return dd::Foreign_key::RULE_CASCADE;
    case FK_OPTION_SET_NULL:
      return dd::Foreign_key::RULE_SET_NULL;
    case FK_OPTION_DEFAULT:
      return dd::Foreign_key::RULE_SET_DEFAULT;
    case FK_OPTION_NO_ACTION:
    case FK_OPTION_UNDEF:
    default:
      return dd::Foreign_key::RULE_NO_ACTION;
  }
}

/**
  Add foreign keys to dd::Table according to Foreign_key_spec structs.

  @param tab_obj      table to add foreign keys to
  @param key_count    number of foreign keys
  @param keyinfo      array containing foreign key info

  @retval true if error (error reported), false otherwise.
*/

static bool fill_dd_foreign_keys_from_create_fields(
    dd::Table *tab_obj, uint key_count, const FOREIGN_KEY *keyinfo) {
  DBUG_TRACE;
  for (const FOREIGN_KEY *key = keyinfo; key != keyinfo + key_count; ++key) {
    dd::Foreign_key *fk_obj = tab_obj->add_foreign_key();

    fk_obj->set_name(key->name);

    // Note: Setting "" is interpreted as NULL.
    fk_obj->set_unique_constraint_name(
        key->unique_index_name ? key->unique_index_name : "");

    switch (key->match_opt) {
      case FK_MATCH_FULL:
        fk_obj->set_match_option(dd::Foreign_key::OPTION_FULL);
        break;
      case FK_MATCH_PARTIAL:
        fk_obj->set_match_option(dd::Foreign_key::OPTION_PARTIAL);
        break;
      case FK_MATCH_SIMPLE:
      case FK_MATCH_UNDEF:
      default:
        fk_obj->set_match_option(dd::Foreign_key::OPTION_NONE);
        break;
    }

    fk_obj->set_update_rule(get_fk_rule(key->update_opt));

    fk_obj->set_delete_rule(get_fk_rule(key->delete_opt));

    fk_obj->set_referenced_table_catalog_name(
        Dictionary_impl::instance()->default_catalog_name());

    fk_obj->set_referenced_table_schema_name(
        dd::String_type(key->ref_db.str, key->ref_db.length));

    fk_obj->set_referenced_table_name(
        dd::String_type(key->ref_table.str, key->ref_table.length));

    for (uint i = 0; i < key->key_parts; i++) {
      dd::Foreign_key_element *fk_col_obj = fk_obj->add_element();

      const dd::Column *column = tab_obj->get_column(
          dd::String_type(key->key_part[i].str, key->key_part[i].length));

      DBUG_ASSERT(column);
      fk_col_obj->set_column(column);

      fk_col_obj->referenced_column_name(
          dd::String_type(key->fk_key_part[i].str, key->fk_key_part[i].length));
    }
  }

  return false;
}

/**
  Set dd::Tablespace object id for dd::Table and dd::Partition
  object during CREATE TABLE.

  @param thd                 - Thread handle.
  @param obj                 - dd::Table or dd::Partition.
  @param hton                - handlerton of table or a partition.
  @param tablespace_name     - Tablespace name to be associated
                               with Table or partition.
  @param is_temporary_table  - Is this temporary table ?

  @return true  - On failure.
  @return false - On success.
*/

template <typename T>
static bool fill_dd_tablespace_id_or_name(THD *thd, T *obj, handlerton *hton,
                                          const char *tablespace_name,
                                          bool is_temporary_table) {
  DBUG_TRACE;

  if (!(tablespace_name && strlen(tablespace_name))) return false;

  /*
    Tablespace metadata can be stored in new DD for following cases.

    1) For engines NDB and InnoDB

    2) A temporary table cannot be assigned with non-temporary tablespace.
       And meta data of temporary tablespace is not captured by new DD.
       Hence it is not necessary to look up tablespaces for temporary
       tables. We store the tablespace name in 'tablespace' table
       option.

    3) Note that we store tablespace name for non-tablespace-capable SEs
       for compatibility reasons. This is store in the options field. We
       also store the innodb_file_per_table tablespace name here since it
       is not a name of a real tablespace.
  */
  const char *innodb_prefix = "innodb_file_per_table";
  dd::Properties *options = &obj->options();

  if (hton->alter_tablespace && !is_temporary_table &&
      strncmp(tablespace_name, innodb_prefix, strlen(innodb_prefix)) != 0) {
    /*
      Make sure we have at least an IX lock on the tablespace name,
      unless this is a temporary table. For temporary tables, the
      tablespace name is not IX locked. When setting tablespace id
      for dd::Partition, we acquire IX lock here.
    */
    DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::TABLESPACE, "", tablespace_name, MDL_INTENTION_EXCLUSIVE));

    // Acquire tablespace.
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Tablespace *ts_obj = nullptr;
    if (thd->dd_client()->acquire(tablespace_name, &ts_obj)) {
      // acquire() always fails with a error being reported.
      return true;
    }

    if (!ts_obj) {
      my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), tablespace_name);
      return true;
    }

    // We found valid tablespace so store the ID with dd::Table now.
    obj->set_tablespace_id(ts_obj->id());
  } else {
    /*
      Persist the tablespace name for non-ndb/non-innodb engines
      This is the current behavior to retain them. The SHOW CREATE
      is suppose to show the options that are provided in CREATE
      TABLE, even though the tablespaces are not supported by
      the engine.
    */
    options->set("tablespace", tablespace_name);
  }

  /*
    We are here only when user explicitly specifies the tablespace clause
    in CREATE TABLE statement. Store a boolean flag in dd::Table::options
    properties.
    This is required in order for SHOW CREATE and CREATE LIKE to ignore
    implicitly assumed tablespace, e.g., 'innodb_system'
  */
  options->set("explicit_tablespace", true);

  return false;
}

/**
  Get a string of fields to be stores as partition_expression.

  Must be in sync with set_field_list()!

  @param[in,out] str       String to add the field name list to.
  @param[in]     name_list Field name list.

  @return false on success, else true.
*/

static bool get_field_list_str(dd::String_type &str, List<char> *name_list) {
  List_iterator<char> it(*name_list);
  const char *name;
  uint i = 0, elements = name_list->elements;
  while ((name = it++)) {
    dd::escape(&str, name);
    if (++i < elements) str.push_back(FIELD_NAME_SEPARATOR_CHAR);
  }
  DBUG_ASSERT(i == name_list->elements);
  return false;
}

/** Helper function to set partition options. */
static void set_partition_options(partition_element *part_elem,
                                  dd::Properties *part_options) {
  if (part_elem->part_max_rows)
    part_options->set("max_rows", part_elem->part_max_rows);
  if (part_elem->part_min_rows)
    part_options->set("min_rows", part_elem->part_min_rows);
  if (part_elem->data_file_name && part_elem->data_file_name[0])
    part_options->set("data_file_name", part_elem->data_file_name);
  if (part_elem->index_file_name && part_elem->index_file_name[0])
    part_options->set("index_file_name", part_elem->index_file_name);
  if (part_elem->nodegroup_id != UNDEF_NODEGROUP)
    part_options->set("nodegroup_id", part_elem->nodegroup_id);
}

/*
  Helper function to add partition column values.

  @param      part_info          Parition info.
  @param      list_value         List of partition element value.
  @param      list_index         Element index.
  @param      part_obj           DD partition object.
  @param      create_info        Create info.
  @param      create_fields      List of fields being created.
  @param[out] part_desc_str Partiton description string.
*/
static bool add_part_col_vals(partition_info *part_info,
                              part_elem_value *list_value, uint list_index,
                              dd::Partition *part_obj,
                              const HA_CREATE_INFO *create_info,
                              const List<Create_field> &create_fields,
                              String *part_desc_str) {
  uint i;
  List_iterator<char> it(part_info->part_field_list);
  uint num_elements = part_info->part_field_list.elements;

  for (i = 0; i < num_elements; i++) {
    dd::Partition_value *val_obj = part_obj->add_value();
    part_column_list_val *col_val = &list_value->col_val_array[i];
    char *field_name = it++;
    val_obj->set_column_num(i);
    val_obj->set_list_num(list_index);
    if (col_val->max_value) {
      val_obj->set_max_value(true);
      part_desc_str->append(partition_keywords[PKW_MAXVALUE].str);
    } else if (col_val->null_value) {
      val_obj->set_value_null(true);
      part_desc_str->append("NULL");
    } else {
      //  Store in value in utf8 string format.
      String val_str;
      DBUG_ASSERT(!col_val->item_expression->null_value);
      if (expr_to_string(&val_str, col_val->item_expression, nullptr,
                         field_name, create_info,
                         const_cast<List<Create_field> *>(&create_fields))) {
        return true;
      }
      dd::String_type std_str(val_str.ptr(), val_str.length());
      val_obj->set_value_utf8(std_str);
      part_desc_str->append(std_str.c_str());
    }
    if (i != num_elements - 1) part_desc_str->append(",");
  }
  return false;
}

static void collect_partition_expr(const THD *thd, List<char> &field_list,
                                   String *str) {
  List_iterator<char> part_it(field_list);
  ulong no_fields = field_list.elements;
  const char *field_str;
  str->length(0);
  while ((field_str = part_it++)) {
    append_identifier(thd, str, field_str, strlen(field_str));
    if (--no_fields != 0) str->append(",");
  }
  return;
}

/**
  Fill in partitioning meta data form create_info
  to the table object.

  @param[in]     thd            Thread handle.
  @param[in,out] tab_obj        Table object where to store the info.
  @param[in]     create_info    Create info.
  @param[in]     create_fields  List of fiels in the new table.
  @param[in]     part_info      Partition info object.

  @return false on success, else true.
*/

static bool fill_dd_partition_from_create_info(
    THD *thd, dd::Table *tab_obj, const HA_CREATE_INFO *create_info,
    const List<Create_field> &create_fields, partition_info *part_info) {
  // TODO-PARTITION: move into partitioning service, WL#4827
  // TODO-PARTITION: Change partition_info, partition_element,
  // part_column_list_val
  //       and p_elem_val to be more similar with
  //       the DD counterparts to ease conversions!
  if (part_info) {
    switch (part_info->part_type) {
      case partition_type::RANGE:
        if (part_info->column_list)
          tab_obj->set_partition_type(dd::Table::PT_RANGE_COLUMNS);
        else
          tab_obj->set_partition_type(dd::Table::PT_RANGE);
        break;
      case partition_type::LIST:
        if (part_info->column_list)
          tab_obj->set_partition_type(dd::Table::PT_LIST_COLUMNS);
        else
          tab_obj->set_partition_type(dd::Table::PT_LIST);
        break;
      case partition_type::HASH:
        if (part_info->list_of_part_fields) {
          /* KEY partitioning */
          if (part_info->linear_hash_ind) {
            if (part_info->key_algorithm ==
                enum_key_algorithm::KEY_ALGORITHM_51)
              tab_obj->set_partition_type(dd::Table::PT_LINEAR_KEY_51);
            else
              tab_obj->set_partition_type(dd::Table::PT_LINEAR_KEY_55);
          } else {
            if (part_info->key_algorithm ==
                enum_key_algorithm::KEY_ALGORITHM_51)
              tab_obj->set_partition_type(dd::Table::PT_KEY_51);
            else
              tab_obj->set_partition_type(dd::Table::PT_KEY_55);
          }
        } else {
          if (part_info->linear_hash_ind)
            tab_obj->set_partition_type(dd::Table::PT_LINEAR_HASH);
          else
            tab_obj->set_partition_type(dd::Table::PT_HASH);
        }
        break;
      default:
        DBUG_ASSERT(0); /* purecov: deadcode */
    }

    if (part_info->is_auto_partitioned) {
      if (tab_obj->partition_type() == dd::Table::PT_KEY_55) {
        tab_obj->set_partition_type(dd::Table::PT_AUTO);
      } else if (tab_obj->partition_type() == dd::Table::PT_LINEAR_KEY_55) {
        tab_obj->set_partition_type(dd::Table::PT_AUTO_LINEAR);
      } else {
        /*
          Currently only [LINEAR] KEY partitioning is used for auto
          partitioning.
        */
        DBUG_ASSERT(0); /* purecov: deadcode */
      }
    }

    /* Set partition_expression */
    dd::String_type expr;
    dd::String_type expr_utf8;
    char expr_buff[256];
    String tmp(expr_buff, sizeof(expr_buff), system_charset_info);
    // Default on-stack buffer which allows to avoid malloc() in most cases.
    tmp.length(0);
    if (part_info->list_of_part_fields) {
      if (get_field_list_str(expr, &part_info->part_field_list)) return true;
      collect_partition_expr(thd, part_info->part_field_list, &tmp);
      expr_utf8.assign(tmp.ptr(), tmp.length());
    } else {
      /* column_list also has list_of_part_fields set! */
      DBUG_ASSERT(!part_info->column_list);

      // Turn off ANSI_QUOTES and other SQL modes which affect printing of
      // expressions.
      Sql_mode_parse_guard parse_guard(thd);

      // No point in including schema and table name for identifiers
      // since any columns must be in this table.
      part_info->part_expr->print(
          thd, &tmp,
          enum_query_type(QT_TO_SYSTEM_CHARSET | QT_NO_DB | QT_NO_TABLE));

      if (tmp.numchars() > PARTITION_EXPR_CHAR_LEN) {
        my_error(ER_PART_EXPR_TOO_LONG, MYF(0));
        return true;
      }

      expr.assign(tmp.ptr(), tmp.length());
      expr_utf8 = expr;
    }
    tab_obj->set_partition_expression(expr);
    tab_obj->set_partition_expression_utf8(expr_utf8);

    if (part_info->use_default_partitions) {
      if (!part_info->use_default_num_partitions)
        tab_obj->set_default_partitioning(dd::Table::DP_NUMBER);
      else
        tab_obj->set_default_partitioning(dd::Table::DP_YES);
    } else
      tab_obj->set_default_partitioning(dd::Table::DP_NO);

    /* Set up subpartitioning. */
    if (part_info->is_sub_partitioned()) {
      if (part_info->list_of_subpart_fields) {
        /* KEY partitioning */
        if (part_info->linear_hash_ind) {
          if (part_info->key_algorithm == enum_key_algorithm::KEY_ALGORITHM_51)
            tab_obj->set_subpartition_type(dd::Table::ST_LINEAR_KEY_51);
          else
            tab_obj->set_subpartition_type(dd::Table::ST_LINEAR_KEY_55);
        } else {
          if (part_info->key_algorithm == enum_key_algorithm::KEY_ALGORITHM_51)
            tab_obj->set_subpartition_type(dd::Table::ST_KEY_51);
          else
            tab_obj->set_subpartition_type(dd::Table::ST_KEY_55);
        }
      } else {
        if (part_info->linear_hash_ind)
          tab_obj->set_subpartition_type(dd::Table::ST_LINEAR_HASH);
        else
          tab_obj->set_subpartition_type(dd::Table::ST_HASH);
      }

      /* Set subpartition_expression */
      expr.clear();
      expr_utf8.clear();
      tmp.length(0);
      if (part_info->list_of_subpart_fields) {
        if (get_field_list_str(expr, &part_info->subpart_field_list))
          return true;

        collect_partition_expr(thd, part_info->subpart_field_list, &tmp);
        expr_utf8.assign(tmp.ptr(), tmp.length());
      } else {
        // Turn off ANSI_QUOTES and other SQL modes which affect printing of
        // expressions.
        Sql_mode_parse_guard parse_guard(thd);

        // No point in including schema and table name for identifiers
        // since any columns must be in this table.
        part_info->subpart_expr->print(
            thd, &tmp,
            enum_query_type(QT_TO_SYSTEM_CHARSET | QT_NO_DB | QT_NO_TABLE));

        if (tmp.numchars() > PARTITION_EXPR_CHAR_LEN) {
          my_error(ER_PART_EXPR_TOO_LONG, MYF(0));
          return true;
        }

        expr.assign(tmp.ptr(), tmp.length());
        expr_utf8 = expr;
      }
      tab_obj->set_subpartition_expression(expr);
      tab_obj->set_subpartition_expression_utf8(expr_utf8);

      if (part_info->use_default_subpartitions) {
        if (!part_info->use_default_num_subpartitions)
          tab_obj->set_default_subpartitioning(dd::Table::DP_NUMBER);
        else
          tab_obj->set_default_subpartitioning(dd::Table::DP_YES);
      } else
        tab_obj->set_default_subpartitioning(dd::Table::DP_NO);
    }

    /* Add partitions and subpartitions. */
    {
      List_iterator<partition_element> part_it(part_info->partitions);
      partition_element *part_elem;
      uint part_num = 0;
      CHARSET_INFO *cs = system_charset_info;
      char buff[2048];
      String part_desc_res(buff, sizeof(buff), cs);
      String part_desc_str;

      while ((part_elem = part_it++)) {
        if (part_elem->part_state == PART_TO_BE_DROPPED ||
            part_elem->part_state == PART_REORGED_DROPPED) {
          /* These should not be included in the new table definition. */
          continue;
        }

        dd::Partition *part_obj = tab_obj->add_partition();

        part_obj->set_name(part_elem->partition_name);
        part_obj->set_engine(tab_obj->engine());
        if (part_elem->part_comment)
          part_obj->set_comment(part_elem->part_comment);
        part_obj->set_number(part_num);
        dd::Properties *part_options = &part_obj->options();
        set_partition_options(part_elem, part_options);

        // Set partition tablespace
        if (fill_dd_tablespace_id_or_name<dd::Partition>(
                thd, part_obj, create_info->db_type, part_elem->tablespace_name,
                create_info->options & HA_LEX_CREATE_TMP_TABLE))
          return true;

        /* Fill in partition values if not KEY/HASH. */
        if (part_info->part_type == partition_type::RANGE) {
          if (part_info->column_list) {
            List_iterator<part_elem_value> list_it(part_elem->list_val_list);
            part_desc_str.length(0);
            part_elem_value *list_value = list_it++;
            if (add_part_col_vals(part_info, list_value, 0, part_obj,
                                  create_info, create_fields, &part_desc_str)) {
              return true;
            }

            part_obj->set_description_utf8(
                String_type(part_desc_str.ptr(), part_desc_str.length()));

            DBUG_ASSERT(list_it++ == nullptr);
          } else {
            dd::Partition_value *val_obj = part_obj->add_value();
            if (part_elem->max_value) {
              val_obj->set_max_value(true);
            } else {
              if (part_elem->signed_flag) {
                val_obj->set_value_utf8(
                    dd::Properties::to_str(part_elem->range_value));
              } else {
                val_obj->set_value_utf8(
                    dd::Properties::to_str((ulonglong)part_elem->range_value));
              }
            }

            // Set partition description. Used only by I_S.
            part_desc_str.length(0);
            if (part_elem->range_value != LLONG_MAX) {
              part_desc_res.set(part_elem->range_value, cs);
              part_desc_str.append(part_desc_res);
            } else
              part_desc_str.append(partition_keywords[PKW_MAXVALUE].str);

            part_obj->set_description_utf8(
                String_type(part_desc_str.ptr(), part_desc_str.length()));
          }
        } else if (part_info->part_type == partition_type::LIST) {
          uint list_index = 0;
          List_iterator<part_elem_value> list_val_it(part_elem->list_val_list);
          uint num_items = part_elem->list_val_list.elements;
          part_desc_str.length(0);
          part_desc_res.length(0);
          if (part_elem->has_null_value) {
            DBUG_ASSERT(!part_info->column_list);
            dd::Partition_value *val_obj = part_obj->add_value();
            val_obj->set_value_null(true);
            val_obj->set_list_num(list_index++);
            part_desc_str.append("NULL");
            if (num_items > 0) part_desc_str.append(",");
          }
          part_elem_value *list_value;
          while ((list_value = list_val_it++)) {
            if (part_info->column_list) {
              // Store partition description. Used by I_S only.
              if (part_info->part_field_list.elements > 1U)
                part_desc_str.append("(");

              if (add_part_col_vals(part_info, list_value, list_index, part_obj,
                                    create_info, create_fields,
                                    &part_desc_str)) {
                return true;
              }

              if (part_info->part_field_list.elements > 1U)
                part_desc_str.append(")");
            } else {
              dd::Partition_value *val_obj = part_obj->add_value();
              val_obj->set_list_num(list_index);
              if (list_value->unsigned_flag) {
                val_obj->set_value_utf8(
                    dd::Properties::to_str((ulonglong)list_value->value));
                part_desc_res.set((ulonglong)list_value->value, cs);
              } else {
                val_obj->set_value_utf8(
                    dd::Properties::to_str(list_value->value));
                part_desc_res.set(list_value->value, cs);
              }
              part_desc_str.append(part_desc_res);
            }
            if (--num_items != 0) part_desc_str.append(",");

            list_index++;
          }
          part_obj->set_description_utf8(
              String_type(part_desc_str.ptr(), part_desc_str.length()));
        } else {
          // HASH/KEY partition, nothing to fill in?
          DBUG_ASSERT(part_info->part_type == partition_type::HASH);
        }

        if (!part_info->is_sub_partitioned()) {
          /*
            If table is not subpartitioned then Partition_index object is
            required for each partition, index pair.
            */
          for (dd::Index *idx : *tab_obj->indexes()) part_obj->add_index(idx);
        } else {
          List_iterator<partition_element> sub_it(part_elem->subpartitions);
          partition_element *sub_elem;
          uint sub_part_num = 0;
          while ((sub_elem = sub_it++)) {
            dd::Partition *sub_obj = part_obj->add_subpartition();

            sub_obj->set_engine(tab_obj->engine());
            if (sub_elem->part_comment)
              sub_obj->set_comment(sub_elem->part_comment);
            sub_obj->set_name(sub_elem->partition_name);
            sub_obj->set_number(sub_part_num);
            dd::Properties *sub_options = &sub_obj->options();
            set_partition_options(sub_elem, sub_options);

            // Set partition tablespace
            if (fill_dd_tablespace_id_or_name<dd::Partition>(
                    thd, sub_obj, create_info->db_type,
                    sub_elem->tablespace_name,
                    create_info->options & HA_LEX_CREATE_TMP_TABLE))
              return true;

            /*
              If table is subpartitioned for each subpartition, index pair
              we need to create Partition_index object.
            */
            for (dd::Index *idx : *tab_obj->indexes()) sub_obj->add_index(idx);

            sub_part_num++;
          }
        }

        part_num++;
      }
    }
  } else {
    tab_obj->set_partition_type(dd::Table::PT_NONE);
  }
  return false;
}

/**
  Fill in check constraints metadata to the Table object from the list of
  check constraint specifications.

  @param[in]     thd                     Thread handle.
  @param[in,out] tab_obj                 Table object where to store the info.
  @param[in]     check_cons_spec         Check constraints specification list.

  @return false on success, else true.
*/
bool fill_dd_check_constraints(
    THD *thd, dd::Table *tab_obj,
    const Sql_check_constraint_spec_list *check_cons_spec) {
  if (check_cons_spec == nullptr) return false;

  for (auto &cc_spec : *check_cons_spec) {
    Check_constraint *cc = tab_obj->add_check_constraint();
    if (cc == nullptr) return true;  // OOM

    // Constraint name.
    cc->set_name(cc_spec->name.str);

    if (cc_spec->is_alter_mode) {
      // alter mode.
      cc->set_alter_mode(true);
      // Alias name.
      cc->set_alias_name(cc_spec->alias_name.str);
    }

    // Constraint clause.
    char buffer[256];
    String expr(buffer, sizeof(buffer), &my_charset_bin);
    cc_spec->print_expr(thd, expr);
    cc->set_check_clause(String_type(expr.c_ptr_safe()));

    // Prepare UTF8 expressions for INFORMATION_SCHEMA tables.
    String expr_for_IS;
    convert_and_print(&expr, &expr_for_IS, system_charset_info);
    cc->set_check_clause_utf8(
        String_type(expr_for_IS.ptr(), expr_for_IS.length()));

    // State. (enforced / not enforced)
    cc->set_constraint_state(cc_spec->is_enforced);
  }

  return false;
}

/**
  Convert old row type value to corresponding value in new row format enum
  used by DD framework.
*/

static Table::enum_row_format dd_get_new_row_format(row_type old_format) {
  switch (old_format) {
    case ROW_TYPE_FIXED:
      return Table::RF_FIXED;
    case ROW_TYPE_DYNAMIC:
      return Table::RF_DYNAMIC;
    case ROW_TYPE_COMPRESSED:
      return Table::RF_COMPRESSED;
    case ROW_TYPE_REDUNDANT:
      return Table::RF_REDUNDANT;
    case ROW_TYPE_COMPACT:
      return Table::RF_COMPACT;
    case ROW_TYPE_PAGED:
      return Table::RF_PAGED;
    case ROW_TYPE_NOT_USED:
    case ROW_TYPE_DEFAULT:
    default:
      DBUG_ASSERT(0);
      break;
  }
  return Table::RF_FIXED;
}

/**
  Check if the storage engine supports geographic geometry columns. If not,
  check that the columns defined only has Cartesian coordinate systems
  (projected SRS or SRID 0).

  @param thd Thread handle
  @param table The table definition
  @param handler Handler to the storage engine

  @retval true if the engine does not supports the provided SRS id. In that case
          my_error is called
  @retval false on success
*/
static bool engine_supports_provided_srs_id(THD *thd, const dd::Table &table,
                                            const handler *handler) {
  if (!(handler->ha_table_flags() & HA_SUPPORTS_GEOGRAPHIC_GEOMETRY_COLUMN)) {
    for (const auto col : table.columns()) {
      if (col->srs_id().has_value() && col->srs_id() != 0) {
        Srs_fetcher fetcher(thd);
        const dd::Spatial_reference_system *srs = nullptr;
        dd::cache::Dictionary_client::Auto_releaser m_releaser(
            thd->dd_client());
        if (fetcher.acquire(col->srs_id().value(), &srs)) {
          // An error has already been flagged.
          return true; /* purecov: deadcode */
        }

        // Non-existing spatial reference systems should already been stopped
        DBUG_ASSERT(srs != nullptr);
        if (srs->is_geographic()) {
          my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0),
                   "geographic spatial reference systems");
          return true;
        }
      }
    }
  }

  return false;
}

bool invalid_tablespace_usage(THD *thd, const dd::String_type &schema_name,
                              const dd::String_type &table_name,
                              const HA_CREATE_INFO *create_info) {
  DBUG_ASSERT(create_info);

  // Checking if partitions contain a reserved tablespace.
  bool rsrvd_tablespace = false;

  const System_tables::Types *type =
      System_tables::instance()->find_type(schema_name, table_name);

  /*
    DD tables (type INERT, CORE, SECOND or DDSE) and system
    tables are allowed to be in the DD tablespace. And
    additionally, a system thread can do what it likes.
    Tables in the 'mysql' schema, with temporary names, are
    also allowed to be in the DD tablespace, since mysql_upgrade
    will ned to do ALTER TABLE.
  */
  if (dd::get_dictionary()->is_dd_table_name(schema_name, table_name) ||
      (type != nullptr && *type == System_tables::Types::SYSTEM) ||
      thd->is_dd_system_thread() ||
      (schema_name == dd::String_type(MYSQL_SCHEMA_NAME.str) &&
       strstr(table_name.c_str(), tmp_file_prefix))) {
    /*
      So for these tables, there is no point checking whether they
      actually are in the 'mysql' tablespace, since the important
      fact is that they are allowed to be there, not whether they
      actually are there.
    */
    return false;
  }

  /*
    Other tables are not allowed in the DD tablespace, neither at
    the table level nor at the partition or subpartition level.
  */
  if (create_info->tablespace &&
      strcmp(create_info->tablespace, MYSQL_TABLESPACE_NAME.str) == 0) {
    rsrvd_tablespace = true;
  } else if (thd->lex->part_info) {
    // Traverse through all partitions.
    List_iterator<partition_element> part_it(thd->lex->part_info->partitions);
    partition_element *part_elem;
    while ((part_elem = part_it++) && !rsrvd_tablespace) {
      if (part_elem->tablespace_name &&
          strcmp(part_elem->tablespace_name, MYSQL_TABLESPACE_NAME.str) == 0) {
        rsrvd_tablespace = true;
      }
      // Traverse through all subpartitions.
      List_iterator<partition_element> sub_it(part_elem->subpartitions);
      partition_element *sub_elem;
      while ((sub_elem = sub_it++) && !rsrvd_tablespace) {
        // Check tablespace name from sub-partition elements, if used.
        if (sub_elem->tablespace_name &&
            strcmp(sub_elem->tablespace_name, MYSQL_TABLESPACE_NAME.str) == 0) {
          rsrvd_tablespace = true;
        }
      }
    }
  }

  if (rsrvd_tablespace) {
    my_error(ER_RESERVED_TABLESPACE_NAME, MYF(0), table_name.c_str(),
             MYSQL_TABLESPACE_NAME.str);
    return true;
  }

  return false;
}

/** Fill dd::Table object from mysql_prepare_create_table() output. */
static bool fill_dd_table_from_create_info(
    THD *thd, dd::Table *tab_obj, const dd::String_type &table_name,
    const dd::String_type &schema_name, const HA_CREATE_INFO *create_info,
    const List<Create_field> &create_fields, const KEY *keyinfo, uint keys,
    Alter_info::enum_enable_or_disable keys_onoff,
    const FOREIGN_KEY *fk_keyinfo, uint fk_keys,
    const Sql_check_constraint_spec_list *check_cons_spec, handler *file) {
  // Table name must be set with the correct case depending on l_c_t_n
  tab_obj->set_name(table_case_name(create_info, table_name.c_str()));

  // TODO-POST-MERGE-TO-TRUNK:
  // Initialize new field tab_obj->last_checked_for_upgrade

  // No need set tab_obj->m_mysql_version_id here. It is always
  // initialized to MYSQL_VERSION_ID by the dd::Abstract_table_impl
  // constructor.

  // Storing real storage engine name in tab_obj.
  handlerton *hton = thd->work_part_info
                         ? thd->work_part_info->default_engine_type
                         : create_info->db_type;
  DBUG_ASSERT(hton && ha_storage_engine_is_enabled(hton));
  tab_obj->set_engine(ha_resolve_storage_engine_name(hton));

  // Comments
  if (create_info->comment.str && create_info->comment.length)
    tab_obj->set_comment(
        dd::String_type(create_info->comment.str, create_info->comment.length));

  //
  // Set options
  //
  dd::Properties *table_options = &tab_obj->options();

  if (create_info->max_rows)
    table_options->set("max_rows", create_info->max_rows);

  if (create_info->min_rows)
    table_options->set("min_rows", create_info->min_rows);

  //
  // Options encoded in HA_CREATE_INFO::table_options.
  //

  /* We should not get any unexpected flags which are not handled below. */
  DBUG_ASSERT(
      !(create_info->table_options &
        ~(HA_OPTION_PACK_RECORD | HA_OPTION_PACK_KEYS | HA_OPTION_NO_PACK_KEYS |
          HA_OPTION_CHECKSUM | HA_OPTION_NO_CHECKSUM |
          HA_OPTION_DELAY_KEY_WRITE | HA_OPTION_NO_DELAY_KEY_WRITE |
          HA_OPTION_STATS_PERSISTENT | HA_OPTION_NO_STATS_PERSISTENT)));

  /*
    Even though we calculate HA_OPTION_PACK_RECORD flag from the value of
    ROW_FORMAT option and column types, it doesn't really reflect property
    of table, but rather our decision to apply optimization in some cases.
    So it is better to store this flag explicitly in DD in order to avoid
    problems with binary compatibility if we decide to change rules for
    applying this optimization in future releases.
  */
  table_options->set("pack_record",
                     create_info->table_options & HA_OPTION_PACK_RECORD);

  /*
    PACK_KEYS=# clause. Absence of PACK_KEYS option/PACK_KEYS=DEFAULT is
    represented by absence of "pack_keys" property.
  */
  if (create_info->table_options &
      (HA_OPTION_PACK_KEYS | HA_OPTION_NO_PACK_KEYS)) {
    DBUG_ASSERT((create_info->table_options &
                 (HA_OPTION_PACK_KEYS | HA_OPTION_NO_PACK_KEYS)) !=
                (HA_OPTION_PACK_KEYS | HA_OPTION_NO_PACK_KEYS));

    table_options->set("pack_keys",
                       create_info->table_options & HA_OPTION_PACK_KEYS);
  }

  /*
    CHECKSUM=# clause. CHECKSUM=DEFAULT doesn't have special meaning and
    is equivalent to CHECKSUM=0.
  */
  DBUG_ASSERT(!((create_info->table_options & HA_OPTION_CHECKSUM) &&
                (create_info->table_options & HA_OPTION_NO_CHECKSUM)));
  if (create_info->table_options & (HA_OPTION_CHECKSUM | HA_OPTION_NO_CHECKSUM))
    table_options->set("checksum",
                       create_info->table_options & HA_OPTION_CHECKSUM);

  /* DELAY_KEY_WRITE=# clause. Same situation as for CHECKSUM option. */
  DBUG_ASSERT(!((create_info->table_options & HA_OPTION_DELAY_KEY_WRITE) &&
                (create_info->table_options & HA_OPTION_NO_DELAY_KEY_WRITE)));
  if (create_info->table_options &
      (HA_OPTION_DELAY_KEY_WRITE | HA_OPTION_NO_DELAY_KEY_WRITE))
    table_options->set("delay_key_write",
                       create_info->table_options & HA_OPTION_DELAY_KEY_WRITE);

  /*
    STATS_PERSISTENT=# clause. Absence option in dd::Properties represents
    STATS_PERSIST=DEFAULT value (which means that global server default
    should be used).
  */
  if (create_info->table_options &
      (HA_OPTION_STATS_PERSISTENT | HA_OPTION_NO_STATS_PERSISTENT)) {
    DBUG_ASSERT(
        (create_info->table_options &
         (HA_OPTION_STATS_PERSISTENT | HA_OPTION_NO_STATS_PERSISTENT)) !=
        (HA_OPTION_STATS_PERSISTENT | HA_OPTION_NO_STATS_PERSISTENT));

    table_options->set("stats_persistent", (create_info->table_options &
                                            HA_OPTION_STATS_PERSISTENT));
  }

  //
  // Set other table options.
  //

  table_options->set("avg_row_length", create_info->avg_row_length);

  if (create_info->row_type != ROW_TYPE_DEFAULT)
    table_options->set("row_type", create_info->row_type);

  // ROW_FORMAT which was explicitly specified by user (if any).
  if (create_info->row_type != ROW_TYPE_DEFAULT)
    table_options->set("row_type",
                       dd_get_new_row_format(create_info->row_type));

  // ROW_FORMAT which is really used for the table by SE (perhaps implicitly).
  tab_obj->set_row_format(
      dd_get_new_row_format(file->get_real_row_type(create_info)));

  table_options->set("stats_sample_pages",
                     create_info->stats_sample_pages & 0xffff);

  table_options->set("stats_auto_recalc", create_info->stats_auto_recalc);

  table_options->set("key_block_size", create_info->key_block_size);

  if (create_info->connect_string.str && create_info->connect_string.length) {
    dd::String_type connect_string;
    connect_string.assign(create_info->connect_string.str,
                          create_info->connect_string.length);
    table_options->set("connection_string", connect_string);
  }

  if (create_info->compress.str && create_info->compress.length) {
    dd::String_type compress;
    compress.assign(create_info->compress.str, create_info->compress.length);
    table_options->set("compress", compress);
  }

  /*
    Store the ENCRYPTION clause for SE's that support encryption.
    We always store 'N' if user has not specified the encryption clause.
  */
  if ((hton->flags & HTON_SUPPORTS_TABLE_ENCRYPTION) &&
      !(create_info->options & HA_LEX_CREATE_TMP_TABLE)) {
    dd::String_type encrypt_type = "N";
    if (create_info->encrypt_type.str && create_info->encrypt_type.length) {
      encrypt_type.assign(create_info->encrypt_type.str,
                          create_info->encrypt_type.length);
    }
    table_options->set("encrypt_type", encrypt_type);
  }

  // Storage media
  if (create_info->storage_media > HA_SM_DEFAULT)
    table_options->set("storage", create_info->storage_media);

  // Update option keys_disabled
  table_options->set("keys_disabled",
                     (keys_onoff == Alter_info::DISABLE ? 1 : 0));

  // Collation ID
  DBUG_ASSERT(create_info->default_table_charset);
  tab_obj->set_collation_id(create_info->default_table_charset->number);

  // Secondary engine.
  if (create_info->secondary_engine.str != nullptr)
    table_options->set("secondary_engine",
                       make_string_type(create_info->secondary_engine));

  // TODO-MYSQL_VERSION: We decided not to store MYSQL_VERSION_ID ?
  //
  //       If we are to introduce this version we need to explain when
  //       it can be useful (e.g. informational and for backward
  //       compatibility reasons, to handle rare cases when meaning of
  //       some option values changed like it happened for partitioning
  //       by KEY, to optimize CHECK FOR UPGRADE). Note that in practice
  //       we can't use this version ID as a robust binary format version
  //       number, because our shows that we often must be able to create
  //       tables in old binary format even in newer versions to avoid
  //       expensive table rebuilds by ALTER TABLE.

  // Add field definitions
  if (fill_dd_columns_from_create_fields(thd, tab_obj, create_fields, file))
    return true;

  /*
    Reject the create if the SRID represents a geographic spatial reference
    system in an engine that does not support it. The function will call
    my_error in case of any errors.
  */
  if (engine_supports_provided_srs_id(thd, *tab_obj, file)) return true;

  // Add index definitions
  fill_dd_indexes_from_keyinfo(thd, tab_obj, keys, keyinfo, create_fields,
                               file);

  // Add check constraints.
  if (fill_dd_check_constraints(thd, tab_obj, check_cons_spec)) return true;

  // Only add foreign key definitions for engines that support it.
  if (ha_check_storage_engine_flag(create_info->db_type,
                                   HTON_SUPPORTS_FOREIGN_KEYS)) {
    if (fill_dd_foreign_keys_from_create_fields(tab_obj, fk_keys, fk_keyinfo))
      return true;
  }

  // Add tablespace definition.
  if (fill_dd_tablespace_id_or_name<dd::Table>(
          thd, tab_obj, create_info->db_type, create_info->tablespace,
          create_info->options & HA_LEX_CREATE_TMP_TABLE))
    return true;

  if (invalid_tablespace_usage(thd, schema_name, table_name, create_info))
    return true;

  /*
    Add hidden columns and indexes which are implicitly created by storage
    engine for the table. This needs to be done before handling partitions
    since we want to create proper dd::Index_partition objects for such
    indexes.
  */
  if (file->get_extra_columns_and_keys(create_info, &create_fields, keyinfo,
                                       keys, tab_obj))
    return true;

  // Add partition definitions
  if (fill_dd_partition_from_create_info(thd, tab_obj, create_info,
                                         create_fields, thd->work_part_info))
    return true;

  return false;
}

/**
  Get the SE private data from the dd_properties table.

  @note During restart, when the scaffolding is created, we assign the
  DD tablespace id (==1) even though this is different that the scaffolding
  DD tablespace id. This is because both server code and innodb code has
  hard coded expectations regarding the DD tablespace id.

  @param [in]     thd        Thread context.
  @param [in,out] tab_obj    Table object to which SE private
                             data should be added.

  @returns true if error, false otherwise.
*/
static bool get_se_private_data(THD *thd, dd::Table *tab_obj) {
  using dd::tables::DD_properties;
  std::unique_ptr<dd::Properties> sys_tbl_props;
  bool exists = false;
  String_type tbl_prop_str;
  if (dd::tables::DD_properties::instance().get(thd, "SYSTEM_TABLES",
                                                &sys_tbl_props, &exists) ||
      !exists || sys_tbl_props->get(tab_obj->name(), &tbl_prop_str)) {
    my_error(ER_DD_METADATA_NOT_FOUND, MYF(0), tab_obj->name().c_str());
    return true;
  }

  std::unique_ptr<dd::Properties> tbl_props(
      Properties::parse_properties(tbl_prop_str));
  Object_id se_id = INVALID_OBJECT_ID;
  Object_id space_id = INVALID_OBJECT_ID;
  String_type se_data;

  if (tbl_props->get(DD_properties::dd_key(DD_properties::DD_property::ID),
                     &se_id) ||
      tbl_props->get(
          DD_properties::dd_key(DD_properties::DD_property::SPACE_ID),
          &space_id) ||
      tbl_props->get(DD_properties::dd_key(DD_properties::DD_property::DATA),
                     &se_data)) {
    my_error(ER_DD_METADATA_NOT_FOUND, MYF(0), tab_obj->name().c_str());
    return true;
  }

  tab_obj->set_se_private_id(se_id);
  tab_obj->set_tablespace_id(space_id);
  tab_obj->set_se_private_data(se_data);

  // Assign SE private data for indexes.
  int count = 0;
  for (auto idx : *tab_obj->indexes()) {
    std::stringstream ss;
    ss << DD_properties::dd_key(DD_properties::DD_property::IDX) << count++;
    if (tbl_props->get(ss.str().c_str(), &se_data)) {
      my_error(ER_DD_METADATA_NOT_FOUND, MYF(0), tab_obj->name().c_str());
      return true;
    }
    idx->set_se_private_data(se_data);
    // Assign the same tablespace id for the indexes as for the table.
    idx->set_tablespace_id(space_id);
  }

  // Assign SE private data for columns.
  count = 0;
  for (auto col : *tab_obj->columns()) {
    std::stringstream ss;
    ss << DD_properties::dd_key(DD_properties::DD_property::COL) << count++;
    if (tbl_props->get(ss.str().c_str(), &se_data)) {
      my_error(ER_DD_METADATA_NOT_FOUND, MYF(0), tab_obj->name().c_str());
      return true;
    }
    col->set_se_private_data(se_data);
  }
  return false;
}

static std::unique_ptr<dd::Table> create_dd_system_table(
    THD *thd, const dd::Schema &system_schema,
    const dd::String_type &table_name, HA_CREATE_INFO *create_info,
    const List<Create_field> &create_fields, const KEY *keyinfo, uint keys,
    const FOREIGN_KEY *fk_keyinfo, uint fk_keys,
    const Sql_check_constraint_spec_list *check_cons_spec, handler *file,
    const dd::Object_table &dd_table) {
  // Create dd::Table object.
  std::unique_ptr<dd::Table> tab_obj(system_schema.create_table(thd));

  // Set to be hidden if appropriate.
  tab_obj->set_hidden(dd_table.is_hidden()
                          ? dd::Abstract_table::HT_HIDDEN_SYSTEM
                          : dd::Abstract_table::HT_VISIBLE);

  if (fill_dd_table_from_create_info(
          thd, tab_obj.get(), table_name, system_schema.name(), create_info,
          create_fields, keyinfo, keys, Alter_info::ENABLE, fk_keyinfo, fk_keys,
          check_cons_spec, file))
    return nullptr;

  /*
    During --initialize, and for inert tables, get the SE private data
    from the SE, and store it in the dd_properties table at a later stage.
    Otherwise, get the SE private data from the 'dd_properties' table.
  */
  const System_tables::Types *table_type =
      System_tables::instance()->find_type(system_schema.name(), table_name);
  if (opt_initialize ||
      (table_type != nullptr && *table_type == System_tables::Types::INERT)) {
    if (file->ha_get_se_private_data(
            tab_obj.get(), (table_type != nullptr &&
                            *table_type == System_tables::Types::INERT)))
      return nullptr;
  } else {
    if (get_se_private_data(thd, tab_obj.get())) return nullptr;
  }

  // Register the se private id with the DDSE.
  handlerton *ddse = ha_resolve_by_legacy_type(thd, DB_TYPE_INNODB);
  if (ddse->dict_register_dd_table_id == nullptr) return nullptr;
  ddse->dict_register_dd_table_id(tab_obj->se_private_id());

  return tab_obj;
}

bool is_server_ps_table_name(const dd::String_type &schema_name,
                             const dd::String_type &table_name) {
  return is_perfschema_db(schema_name.c_str(), schema_name.length()) &&
         System_tables::instance()->find_table(schema_name, table_name) !=
             nullptr;
}

std::unique_ptr<dd::Table> create_dd_user_table(
    THD *thd, const dd::Schema &sch_obj, const dd::String_type &table_name,
    HA_CREATE_INFO *create_info, const List<Create_field> &create_fields,
    const KEY *keyinfo, uint keys,
    Alter_info::enum_enable_or_disable keys_onoff,
    const FOREIGN_KEY *fk_keyinfo, uint fk_keys,
    const Sql_check_constraint_spec_list *check_cons_spec, handler *file) {
  // Verify that this is not a dd table.
  DBUG_ASSERT(
      !dd::get_dictionary()->is_dd_table_name(sch_obj.name(), table_name));

  // Create dd::Table object.
  std::unique_ptr<dd::Table> tab_obj(sch_obj.create_table(thd));

  // Mark the hidden flag.
  tab_obj->set_hidden(create_info->m_hidden ? dd::Abstract_table::HT_HIDDEN_DDL
                                            : dd::Abstract_table::HT_VISIBLE);

  if (is_server_ps_table_name(sch_obj.name(), table_name))
    performance_schema::set_PS_version_for_table(&tab_obj->options());

  if (fill_dd_table_from_create_info(thd, tab_obj.get(), table_name,
                                     sch_obj.name(), create_info, create_fields,
                                     keyinfo, keys, keys_onoff, fk_keyinfo,
                                     fk_keys, check_cons_spec, file))
    return nullptr;

  return tab_obj;
}

std::unique_ptr<dd::Table> create_table(
    THD *thd, const dd::Schema &sch_obj, const dd::String_type &table_name,
    HA_CREATE_INFO *create_info, const List<Create_field> &create_fields,
    const KEY *keyinfo, uint keys,
    Alter_info::enum_enable_or_disable keys_onoff,
    const FOREIGN_KEY *fk_keyinfo, uint fk_keys,
    const Sql_check_constraint_spec_list *check_cons_spec, handler *file) {
  dd::Dictionary *dict = dd::get_dictionary();
  const dd::Object_table *dd_table =
      dict->get_dd_table(sch_obj.name(), table_name);

  return dd_table
             ? create_dd_system_table(thd, sch_obj, table_name, create_info,
                                      create_fields, keyinfo, keys, fk_keyinfo,
                                      fk_keys, check_cons_spec, file, *dd_table)
             : create_dd_user_table(thd, sch_obj, table_name, create_info,
                                    create_fields, keyinfo, keys, keys_onoff,
                                    fk_keyinfo, fk_keys, check_cons_spec, file);
}

std::unique_ptr<dd::Table> create_tmp_table(
    THD *thd, const dd::Schema &sch_obj, const dd::String_type &table_name,
    HA_CREATE_INFO *create_info, const List<Create_field> &create_fields,
    const KEY *keyinfo, uint keys,
    Alter_info::enum_enable_or_disable keys_onoff,
    const Sql_check_constraint_spec_list *check_cons_spec, handler *file) {
  // Create dd::Table object.
  std::unique_ptr<dd::Table> tab_obj(sch_obj.create_table(thd));

  tab_obj->set_is_temporary(true);

  if (fill_dd_table_from_create_info(thd, tab_obj.get(), table_name,
                                     sch_obj.name(), create_info, create_fields,
                                     keyinfo, keys, keys_onoff, nullptr, 0,
                                     check_cons_spec, file))
    return nullptr;

  return tab_obj;
}

bool drop_table(THD *thd, const char *schema_name, const char *name,
                const dd::Table &table_def) {
  return thd->dd_client()->drop(&table_def) ||
         thd->dd_client()->remove_table_dynamic_statistics(schema_name, name);
}

bool table_exists(dd::cache::Dictionary_client *client, const char *schema_name,
                  const char *name, bool *exists) {
  DBUG_TRACE;
  DBUG_ASSERT(exists);

  // Tables exist if they can be acquired.
  dd::cache::Dictionary_client::Auto_releaser releaser(client);
  const dd::Abstract_table *tab_obj = nullptr;
  if (client->acquire(schema_name, name, &tab_obj)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }
  *exists = (tab_obj != nullptr);

  return false;
}

bool is_generated_foreign_key_name(const char *table_name,
                                   size_t table_name_length, handlerton *hton,
                                   const dd::Foreign_key &fk) {
  /*
    We assume that the name is generated if it starts with
    <table_name><SE-specific or default foreign key name suffix>
    (e.g. "_ibfk_" for InnoDB or "_fk_" for NDB).
  */
  const LEX_CSTRING &fk_name_suffix =
      hton->fk_name_suffix.str ? hton->fk_name_suffix : FK_NAME_DEFAULT_SUFFIX;

  return ((fk.name().length() > table_name_length + fk_name_suffix.length) &&
          (memcmp(fk.name().c_str(), table_name, table_name_length) == 0) &&
          (memcmp(fk.name().c_str() + table_name_length, fk_name_suffix.str,
                  fk_name_suffix.length) == 0));
}

#ifndef DBUG_OFF
static bool is_foreign_key_name_locked(THD *thd, const char *db,
                                       const char *fk_name) {
  char db_name_buff[NAME_LEN + 1], fk_name_buff[NAME_LEN + 1];
  my_stpcpy(db_name_buff, db);
  if (lower_case_table_names == 2)
    my_casedn_str(system_charset_info, db_name_buff);
  my_stpcpy(fk_name_buff, fk_name);
  my_casedn_str(system_charset_info, fk_name_buff);
  return thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::FOREIGN_KEY, db_name_buff, fk_name_buff, MDL_EXCLUSIVE);
}
#endif

bool rename_foreign_keys(THD *thd MY_ATTRIBUTE((unused)),
                         const char *old_db MY_ATTRIBUTE((unused)),
                         const char *old_table_name, handlerton *hton,
                         const char *new_db MY_ATTRIBUTE((unused)),
                         dd::Table *new_tab) {
  // With LCTN = 2, we are using lower-case tablename for FK name.
  char old_table_name_norm[NAME_LEN + 1];
  strmake(old_table_name_norm, old_table_name, NAME_LEN);
  if (lower_case_table_names == 2)
    my_casedn_str(system_charset_info, old_table_name_norm);
  size_t old_table_name_norm_len = strlen(old_table_name_norm);

#ifndef DBUG_OFF
  bool is_db_changed =
      (my_strcasecmp(table_alias_charset, old_db, new_db) != 0);
#endif

  for (dd::Foreign_key *fk : *new_tab->foreign_keys()) {
    /*
      We assume that original foreign key name is locked.

      This assumption might be too zealous in some cases (e.g.
      if foreign key name is not generated and we are not moving
      table between databases) however it holds.
    */
    DBUG_ASSERT(is_foreign_key_name_locked(thd, old_db, fk->name().c_str()));

    if (is_generated_foreign_key_name(old_table_name_norm,
                                      old_table_name_norm_len, hton, *fk)) {
      char table_name[NAME_LEN + 1];
      my_stpncpy(table_name, new_tab->name().c_str(), sizeof(table_name));
      if (lower_case_table_names == 2)
        my_casedn_str(system_charset_info, table_name);
      dd::String_type new_name(table_name);
      // Copy <fk_name_suffix><number> (e.g. "_ibfk_nnnn") from the old name.
      new_name.append(fk->name().substr(old_table_name_norm_len));
      if (check_string_char_length(to_lex_cstring(new_name.c_str()), "",
                                   NAME_CHAR_LEN, system_charset_info, true)) {
        my_error(ER_TOO_LONG_IDENT, MYF(0), new_name.c_str());
        return true;
      }

      // We should have lock on the new name as well.
      DBUG_ASSERT(is_foreign_key_name_locked(thd, new_db, new_name.c_str()));

      fk->set_name(new_name);
    }
#ifndef DBUG_OFF
    else if (is_db_changed) {
      /*
        If we are moving table between databases we should have lock on
        the foreign key name in new database.
      */
      DBUG_ASSERT(is_foreign_key_name_locked(thd, new_db, fk->name().c_str()));
    }
#endif
  }
  return false;
}

// Only used by NDB
/* purecov: begin deadcode */
bool table_legacy_db_type(THD *thd, const char *schema_name,
                          const char *table_name,
                          enum legacy_db_type *db_type) {
  DBUG_TRACE;

  // TODO-NOW: Getting DD objects without getting MDL lock on them
  //       is likely to cause problems. We need to revisit
  //       this function at some point.
  // Sivert: Can you please elaborate the problem ?
  // Sivert: Not much to add. Without an MDL lock, we can risk that
  //         the object is modified while we're using it. The global
  //         cache guard does not apply to the new cache (wl#8150).
  // If we are talking about 'problems' point to DD cache issue,
  // probably we can solve now, as we have a DD cache guard
  // introduced already to solve one of similar problem with
  // Innodb.
  // Dlenev: Yes. I guess cache guard can help in this case as a temporary
  // workaround.
  // However long-term we need some better solution. Perhaps this function
  // might turn out unnecessary after discussions with Cluster team.

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  // Get hold of the dd::Table object.
  const dd::Table *table = nullptr;
  if (thd->dd_client()->acquire(schema_name, table_name, &table)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  if (table == nullptr) {
    my_error(ER_NO_SUCH_TABLE, MYF(0), schema_name, table_name);
    return true;
  }

  // Get engine by name
  plugin_ref tmp_plugin =
      ha_resolve_by_name_raw(thd, lex_cstring_handle(table->engine()));

  // Return DB_TYPE_UNKNOWN and no error if engine is not loaded.
  *db_type =
      ha_legacy_type(tmp_plugin ? plugin_data<handlerton *>(tmp_plugin) : NULL);

  return false;
}
/* purecov: end */

template <typename T>
bool table_storage_engine(THD *thd, const T *obj, handlerton **hton) {
  DBUG_TRACE;

  DBUG_ASSERT(hton);

  // Get engine by name
  plugin_ref tmp_plugin =
      ha_resolve_by_name_raw(thd, lex_cstring_handle(obj->engine()));
  if (!tmp_plugin) {
    my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), obj->engine().c_str());
    return true;
  }

  *hton = plugin_data<handlerton *>(tmp_plugin);
  DBUG_ASSERT(*hton && ha_storage_engine_is_enabled(*hton));

  return false;
}

template bool table_storage_engine<dd::Table>(THD *, const dd::Table *,
                                              handlerton **);
template bool table_storage_engine<dd::Tablespace>(THD *,
                                                   const dd::Tablespace *,
                                                   handlerton **);

bool recreate_table(THD *thd, const char *schema_name, const char *table_name) {
  // There should be an exclusive metadata lock on the table
  DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, schema_name, table_name, MDL_EXCLUSIVE));

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Table *table_def = nullptr;

  if (thd->dd_client()->acquire_for_modification(schema_name, table_name,
                                                 &table_def))
    return true;

  // Table must exist.
  DBUG_ASSERT(table_def);

  HA_CREATE_INFO create_info;

  // Create a path to the table, but without a extension
  char path[FN_REFLEN + 1];
  build_table_filename(path, sizeof(path) - 1, schema_name, table_name, "", 0);

  // Attempt to reconstruct the table
  return ha_create_table(thd, path, schema_name, table_name, &create_info, true,
                         false, table_def);
}

/**
  @brief Function returns string representing column type by ST_FIELD_INFO.
         This is required for the IS implementation which uses views on DD
         tables

  @param[in]   thd             The thread handle.
  @param[in]   field_type      Column type.
  @param[in]   field_length    Column length.
  @param[in]   decimals        Decimals.
  @param[in]   maybe_null      Column is null.
  @param[in]   is_unsigned     Column is unsigned.
  @param[in]   field_charset   Column charset.

  @return dd::String_type representing column type.
*/

dd::String_type get_sql_type_by_field_info(THD *thd,
                                           enum_field_types field_type,
                                           uint32 field_length, uint32 decimals,
                                           bool maybe_null, bool is_unsigned,
                                           const CHARSET_INFO *field_charset) {
  DBUG_TRACE;

  TABLE_SHARE share;
  TABLE table;
  table.s = &share;
  table.in_use = thd;

  Create_field field;
  // Initializing field using field_type and field_length.
  field.init_for_tmp_table(field_type, field_length, decimals, maybe_null,
                           is_unsigned, 0);
  field.charset = field_charset;

  return get_sql_type_by_create_field(&table, field);
}

bool fix_row_type(THD *thd, dd::Table *table_def, row_type correct_row_type) {
  DBUG_ASSERT(table_def != nullptr);

  table_def->set_row_format(dd_get_new_row_format(correct_row_type));

  return thd->dd_client()->update(table_def);
}

inline void report_error_as_tablespace_missing(Object_id id) {
  my_error(ER_INVALID_DD_OBJECT_ID, MYF(0), id);
}

inline void report_error_as_tablespace_missing(const String_type name) {
  my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), name.c_str());
}

/*
  Find if tablespace is a general tablespace and if it is encrypted.

  @param k    KEY to search the tablespace object in DD.
  @param thd  Thread
  @param *is_encrypted[out]  On success, this represents table encryption type.
  @param *is_general_tablespace[out] On success, this represents if table
                                     is general tablespace.

  @returns true for failure, false for success.
*/
template <typename KEY>
bool is_general_tablespace_and_encrypted(const KEY k, THD *thd,
                                         bool *is_encrypted_tablespace,
                                         bool *is_general_tablespace) {
  *is_encrypted_tablespace = false;
  *is_general_tablespace = false;

  // Acquire the tablespace object.
  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const Tablespace *tsp = nullptr;
  DEBUG_SYNC(thd, "before_acquire_in_read_tablespace_encryption");
  if (thd->dd_client()->acquire(k, &tsp)) {
    return true;
  }

  // Stop if we did not find a tablespace.
  if (!tsp) {
    report_error_as_tablespace_missing(k);
    return true;
  }

  // Acquire the tablespace engine hton.
  handlerton *hton = nullptr;
  Tablespace_type space_type = Tablespace_type::SPACE_TYPE_IMPLICIT;
  // If the engine is not found, my_error() has already been called
  if (dd::table_storage_engine(thd, tsp, &hton)) return true;

  // Find the tablespace type.
  if (hton->get_tablespace_type &&
      hton->get_tablespace_type(*tsp, &space_type)) {
    my_error(ER_TABLESPACE_TYPE_UNKNOWN, MYF(0), tsp->name().c_str());
    return true;
  }

  // Determine if we have a general tablespace and if it is encrypted.
  if (space_type != Tablespace_type::SPACE_TYPE_IMPLICIT &&
      tsp->options().exists("encryption")) {
    String_type e;
    (void)tsp->options().get("encryption", &e);
    DBUG_ASSERT(e.empty() == false);
    *is_general_tablespace = true;
    *is_encrypted_tablespace = is_encrypted(e);
  }

  return false;
}

/**
   Predicate to determine if a table resides in an encrypted
   tablespace.  First checks if the option "encrypt_type" is set on
   the table itself (implicit tablespace), then proceeds to acquire
   and check the "ecryption" option in table's tablespaces.

   @param[in] thd
   @param[in] t table to check
   @param[out] is_general_tablespace Denotes if we found general tablespace.

   @retval {true, *} in case of errors
   @retval {false, true} if at least one tablespace is encrypted
   @retval {false, false} if no tablespace is encrypted
 */
Encrypt_result is_tablespace_encrypted(THD *thd, const Table &t,
                                       bool *is_general_tablespace) {
  std::vector<Object_id> tspids;
  visit_tablespace_id_owners(t, [&](const auto &tsh) {
    tspids.push_back(tsh.tablespace_id());
    return false;
  });

  // There are no tablespaces used.
  if (tspids.size() == 0) {
    *is_general_tablespace = false;
    return {false, false};
  }

  auto valid_end =
      std::partition(tspids.begin(), tspids.end(),
                     [](Object_id id) { return id != INVALID_OBJECT_ID; });
  std::sort(tspids.begin(), valid_end);
  auto unique_end = std::unique(tspids.begin(), valid_end);

  bool error = false;
  bool encrypted =
      std::any_of(tspids.begin(), unique_end, [&](const Object_id id) {
        bool is_encrypted = false;
        if (is_general_tablespace_and_encrypted(id, thd, &is_encrypted,
                                                is_general_tablespace)) {
          error = true;
          return false;
        }
        return is_encrypted;
      });
  return {error, encrypted};
}

bool has_primary_key(const Table &t) {
  auto &ic = t.indexes();
  return std::any_of(ic.begin(), ic.end(), [](const Index *ix) {
    return ix->type() == Index::IT_PRIMARY && !ix->is_hidden();
  });
}

bool is_generated_check_constraint_name(const char *table_name,
                                        size_t table_name_length,
                                        const char *cc_name,
                                        size_t cc_name_length) {
  // We assume that the name is generated if it starts with <table_name>_chk_
  return ((cc_name_length >
           table_name_length + sizeof(dd::CHECK_CONSTRAINT_NAME_SUBSTR) - 1) &&
          (memcmp(cc_name, table_name, table_name_length) == 0) &&
          (memcmp(cc_name + table_name_length, dd::CHECK_CONSTRAINT_NAME_SUBSTR,
                  sizeof(dd::CHECK_CONSTRAINT_NAME_SUBSTR) - 1) == 0));
}

bool rename_check_constraints(const char *old_table_name, dd::Table *new_tab) {
  size_t old_table_name_length = strlen(old_table_name);
  for (auto &cc : *new_tab->check_constraints()) {
    if (is_generated_check_constraint_name(
            old_table_name, old_table_name_length, cc->name().c_str(),
            cc->name().length())) {
      // Generate new name.
      dd::String_type new_name(new_tab->name());
      new_name.append(cc->name().substr(old_table_name_length));
      if (check_string_char_length(to_lex_cstring(new_name.c_str()), "",
                                   NAME_CHAR_LEN, system_charset_info, true)) {
        my_error(ER_TOO_LONG_IDENT, MYF(0), new_name.c_str());
        return true;
      }
      // Set new name.
      cc->set_name(new_name);
    }
  }

  return false;
}

/*
  Helper function which copies all tablespace ids referenced by table to an
  (output) iterator.
*/
template <typename IT>
static void copy_tablespace_names(const HA_CREATE_INFO *ci, partition_info *pi,
                                  IT it) {
  if (ci->tablespace) {
    *it = ci->tablespace;
    ++it;
  }

  if (!pi) return;

  /*
    Traverse through all partitions.

    Note: We may really not find a general tablespace in 8.0.15.
    Probably this piece of code is good to have if InnoDB later supports
    shared tablespaces in partitioned tables.
  */
  List_iterator<partition_element> part_it(pi->partitions);
  partition_element *part_elem;
  while ((part_elem = part_it++)) {
    if (part_elem->tablespace_name) {
      *it = part_elem->tablespace_name;
      ++it;
    }
    // Traverse through all subpartitions.
    List_iterator<partition_element> sub_it(part_elem->subpartitions);
    partition_element *sub_elem;
    while ((sub_elem = sub_it++)) {
      if (sub_elem->tablespace_name) {
        *it = sub_elem->tablespace_name;
        ++it;
      }
    }  // end of sub_parts
  }    // end of parts
}

/**
   Predicate to determine if a table resides in an encrypted
   tablespace and if it a general tablespace.

   @param[in] thd Thread
   @param[in] ci  HA_CREATE_INFO *  Representing table DDL.
   @param[out] is_general_tablespace Marked as true on success if its
                                general tablespace.

   @retval {true, *} in case of errors
   @retval {false, true} if at least one tablespace is encrypted
   @retval {false, false} if no tablespace is encrypted
 */
Encrypt_result is_tablespace_encrypted(THD *thd, const HA_CREATE_INFO *ci,
                                       bool *is_general_tablespace) {
  // If SE does not support encrypted tablespace, stop here.
  if (!(ci->db_type->flags & HTON_SUPPORTS_TABLE_ENCRYPTION)) {
    *is_general_tablespace = false;
    return {false, false};
  }

  // Copy all tablespace names.
  std::vector<String_type> ts_names;
  copy_tablespace_names(ci, thd->work_part_info, std::back_inserter(ts_names));

  Tablespace_type tt;
  if (ci->db_type->get_tablespace_type_by_name(ci->tablespace, &tt)) {
    return {true, false};
  }
  if (ts_names.empty() ||  // If no explicit tablespace names used OR
      /* If user provided implicit tablespace 'innodb_file_per_table' */
      (ts_names.size() == 1 && ci->tablespace &&
       tt == Tablespace_type::SPACE_TYPE_IMPLICIT)) {
    *is_general_tablespace = false;
    return {false, false};
  }

  /*
    Table uses a general tablespace. Now check if any one of them have been
    encrypted.
  */
  bool error = false;
  bool encrypted = std::any_of(
      ts_names.begin(), ts_names.end(), [&](const String_type name) {
        bool is_encrypted = false;
        if (ci->db_type->get_tablespace_type_by_name(name.c_str(), &tt)) {
          error = true;
          return false;
        }
        if (tt != Tablespace_type::SPACE_TYPE_TEMPORARY &&
            tt != Tablespace_type::SPACE_TYPE_IMPLICIT &&
            is_general_tablespace_and_encrypted(name, thd, &is_encrypted,
                                                is_general_tablespace)) {
          error = true;
          return false;
        }
        return is_encrypted;
      });
  return {error, encrypted};
}

bool uses_general_tablespace(const Table &t) {
  /*
    dd::Table::tablespace_id() and dd::Partition::tablespace_id() is set
    only when table is using general partition.
  */
  if (t.tablespace_id() != INVALID_OBJECT_ID) return true;

  for (const dd::Partition *p : t.leaf_partitions()) {
    if (p->tablespace_id() != INVALID_OBJECT_ID) return true;
  }

  return false;
}

}  // namespace dd
