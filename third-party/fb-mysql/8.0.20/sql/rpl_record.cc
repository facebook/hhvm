/* Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_record.h"

#include <stddef.h>
#include <algorithm>

#include "field_types.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_bitmap.h"  // MY_BITMAP
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"
#include "sql/derror.h"  // ER_THD
#include "sql/field.h"   // Field
#include "sql/log_event.h"
#include "sql/rpl_rli.h"      // Relay_log_info
#include "sql/rpl_utility.h"  // table_def
#include "sql/sql_base.h"     // open_and_lock_tables
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_show.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE
#include "sql_string.h"
#include "template_utils.h"  // down_cast

class Json_diff_vector;

using std::max;
using std::min;

template <typename T, typename UT>
void Bit_stream_base<T, UT>::dbug_print(
    const char *str MY_ATTRIBUTE((unused))) const {
#ifndef DBUG_OFF
  StringBuffer<STRING_BUFFER_USUAL_SIZE> s;
  s.reserve(static_cast<size_t>(m_current_bit + 1));
  for (uint i = 0; i < m_current_bit; i++)
    s.append((m_ptr[i / 8] & (1 << (i % 8))) != 0 ? '1' : '0');
  s.append("\0", 1);
  DBUG_PRINT("info", ("%s: %u bits: %s", str, m_current_bit, s.ptr()));
#endif
}

/**
  Write a single field (column) of a row in a binary log row event to the
  output.

  @param[in,out] pack_ptr Pointer to buffer where the field will be
  written.  It is the caller's responsibility to have allocated enough
  memory for this buffer.  The pointer will be updated to point to the
  next byte after the last byte that was written.

  @param field The field to write.

  @param rec_offset Offset to the record that will be written. This is
  defined by the Field interface: it should be the offset from
  table->record[0], of the record passed to
  ha_[write|update|delete]_row.  In other words, 0 if this is a
  before-image and the size of the before-image record if this is an
  after-image.

  @param row_image_type The type of image: before-image or after-image,
  for a Write/Update/Delete event.

  @param value_options The value of @@session.binlog_row_value_options

  @param[out] is_partial_format Will be set to true if this field was
  written in partial format, otherwise will not be modified.
*/
static void pack_field(uchar **pack_ptr, Field *field, size_t rec_offset,
                       enum_row_image_type row_image_type,
                       ulonglong value_options, bool *is_partial_format) {
  DBUG_TRACE;

  DBUG_PRINT("info", ("value_options=%llu (type==JSON)=%d row_image_type=%d",
                      value_options, field->type() == MYSQL_TYPE_JSON,
                      static_cast<int>(row_image_type)));

  if (row_image_type == enum_row_image_type::UPDATE_AI) {
    /*
      Try to use diff format. But pack_diff may decide to not use the
      full format, in the following cases:

      - The data type does not support diff format.

      - Partial format was not enabled in value_options.

      - The optimizer does not provide diff information.  For JSON,
        this means that optimizer does not provide a Json_diff_vector,
        because the column was updated using anything else than the
        supported JSON functions.

      - pack_diff calculates that the diff would not be smaller than
        the full format.

      In those cases, pack_diff does not write anything, and returns
      true.  So we fall through to call field->pack instead.

      We also set *is_partial_format to true if needed.
    */
    if (field->pack_diff(pack_ptr, value_options) == false) {
      DBUG_PRINT("info", ("stored in partial format"));
      *is_partial_format = true;
      return;
    }
    DBUG_PRINT("info", ("stored in full format"));
  }
  *pack_ptr = field->pack_with_metadata_bytes(
      *pack_ptr, field->ptr + rec_offset, field->max_data_length());
}

/**
  Read a single field (column) of a row from a binary log row event.

  @param[in,out] pack_ptr Pointer to buffer where the field is stored.
  The pointer will be updated to point to the next byte after the last
  byte that was read.

  @param field The field to read.

  @param metadata The so-called 'metadata' for the field. The meaning
  of this may differ depending on the SQL type.  But typically, it is
  the length of the field that holds the length of the value: e.g. it
  is 1 for TINYBLOB, 2 for BLOB, 3 for MEDIUMBLOB, and 4 for LARGEBLOB.

  @param row_image_type The type of image: before-image or after-image,
  for a Write/Update/Delete event.

  @param is_partial_column true if this column is in partial format,
  false otherwise.  (This should be determined by the caller from the
  event_type (PARTIAL_UPDATE_ROWS_EVENT), row_image_type (UPDATE_AI),
  value_options (PARTIAL_JSON), and partial_bits (1 for this column)).

  @retval false Success.

  @retval true Error.  Error can happen when reading in partial format
  and it fails to apply the diff.  The error has already been reported
  through my_error.
*/
static bool unpack_field(const uchar **pack_ptr, Field *field, uint metadata,
                         enum_row_image_type row_image_type,
                         bool is_partial_column) {
  DBUG_TRACE;
  /*
    For a virtual generated column based on the blob type, we have to keep both
    the old and new value for the blob-based field since this might be needed by
    the storage engine during updates.

    The reason why this needs special handling is that the virtual
    generated blobs are neither stored in the record buffers nor
    stored by the storage engine. This special handling for blob-based fields is
    normally taken care of in update_generated_write_fields() but this
    function is not called when applying updated records in
    replication.
  */
  if (field->handle_old_value())
    (down_cast<Field_blob *>(field))->keep_old_value();

  if (is_partial_column) {
    if (down_cast<Field_json *>(field)->unpack_diff(pack_ptr)) return true;
  } else {
    /*
      When PARTIAL_JSON_UPDATES is enabled in the row in the event,
      unpack_row marks all JSON columns included in the after-image as
      eligible for partial updates for the duration of the statement
      (by calling table->mark_column_for_partial_update for the column
      and then table->setup_partial_update for the table).  This means
      that:

      - optimizer may collect binary diffs to send to the engine

      - in case all conditions listed in the no-argument
        setup_partial_update() function are met, optimizer may collect
        logical diffs to send to the binlog.

      Now that we do a full update, no diffs will be collected.  If we
      did not have the code below, the engine would get a list of
      empty binary diffs and the binlog would get a list of empty
      logical diffs, each corresponding to a no-op.  The calls to
      disable_*_diffs_for_current_row tell the optimizer that the
      empty diff lists should be ignored and the full value should be
      used.
    */
    DBUG_PRINT("info", ("row_image_type=%d (field->type==JSON)=%d",
                        static_cast<int>(row_image_type),
                        static_cast<int>(field->type() == MYSQL_TYPE_JSON)));
    if (row_image_type == enum_row_image_type::UPDATE_AI &&
        field->type() == MYSQL_TYPE_JSON) {
      TABLE *table = field->table;
      if (table->is_binary_diff_enabled(field))
        table->disable_binary_diffs_for_current_row(field);
      if (table->is_logical_diff_enabled(field))
        table->disable_logical_diffs_for_current_row(field);
    }

    *pack_ptr = field->unpack(field->ptr, *pack_ptr, metadata, true);
  }

  return false;
}

/**
  Pack a record of data for a table into a format suitable for
  the binary log.

  The format for a row where N columns are included in the image is
  the following:

      +-----------+----------+----------+     +----------+
      | null_bits | column_1 | column_2 | ... | column_N |
      +-----------+----------+----------+     +----------+

  Where

  - null_bits is a bitmap using ceil(N/8) bytes.  There is one bit for
    every column included in the image, *regardless of whether it can
    be null or not*.  The number of null bits is equal to the number
    of bits set in the @c columns_in_image bitmap.

  - column_i: Each of the N columns is stored in a format that depends
    on the type of the column.

  @param table Table describing the format of the record

  @param columns_in_image Bitmap with a set bit for each column that
  should be stored in the row.

  @param row_data Pointer to memory where row will be written

  @param record Pointer to record retrieved from the engine.

  @param row_image_type The type of image: before-image or
  after-image, for a Write/Update/Delete event.

  @param value_options The value of @@session.binlog_row_value_options

  @return The number of bytes written at @c row_data.
*/
size_t pack_row(TABLE *table, MY_BITMAP const *columns_in_image,
                uchar *row_data, const uchar *record,
                enum_row_image_type row_image_type, ulonglong value_options) {
  DBUG_TRACE;
  Replicated_columns_view fields{table, Replicated_columns_view::OUTBOUND};

  // Since we don't want any hidden generated columns to be included in the
  // binlog, we must clear any bits for these columns in the bitmap. We will
  // use TABLE::pack_row_tmp_set for this purpose, so first we ensure that it
  // isn't in use somewhere else.
  DBUG_ASSERT(bitmap_is_clear_all(&table->pack_row_tmp_set));

  // Copy all the bits from the "columns_in_image", and clear all the bits for
  // hidden generated columns.
  bitmap_copy(&table->pack_row_tmp_set, columns_in_image);
  bitmap_intersect(&table->pack_row_tmp_set,
                   &fields.get_included_fields_bitmap());

  // Number of columns in image (counting only those that will be written)
  uint image_column_count = bitmap_bits_set(&table->pack_row_tmp_set);

  ptrdiff_t const rec_offset = record - table->record[0];

  // This is a moving cursor that points to the byte where the next
  // field will be written.
  uchar *pack_ptr = row_data;

  /*
    We write partial_bits, null_bits, and row values using one pass
    over all the fields.
  */

  // Partial bits.
  Bit_writer partial_bits;
  uint json_column_count = 0;
  bool has_any_json_diff = false;
  if ((value_options & PARTIAL_JSON_UPDATES) != 0 &&
      row_image_type == enum_row_image_type::UPDATE_AI) {
    for (auto field : fields) {
      if (field->type() == MYSQL_TYPE_JSON) {
        // Include every JSON column in the count.
        json_column_count++;

        // Check if has_any_json_diff needs to be set.  This is only
        // needed for columns in the after-image, and of course only
        // when has_any_json_diff has not yet been set.
        if (!has_any_json_diff &&
            bitmap_is_set(&table->pack_row_tmp_set, field->field_index)) {
          const Field_json *field_json = down_cast<const Field_json *>(field);
          const Json_diff_vector *diff_vector;
          field_json->get_diff_vector_and_length(value_options, &diff_vector);
          if (diff_vector != nullptr) has_any_json_diff = true;
        }
      }
    }
    pack_ptr =
        net_store_length(pack_ptr, has_any_json_diff ? value_options : 0);
    partial_bits.set_ptr(pack_ptr);
    if (has_any_json_diff) pack_ptr += (json_column_count + 7) / 8;
  }

  // Dump info
  DBUG_PRINT("info", ("table='%.*s' "
                      "table_column_count=%d image_column_count=%d "
                      "has_any_json_diff=%d "
                      "json_column_count=%d "
                      "row_image_type=%d value_options=%llx",
                      (int)table->s->table_name.length,
                      table->s->table_name.str, table->pack_row_tmp_set.n_bits,
                      image_column_count, (int)has_any_json_diff,
                      json_column_count, (int)row_image_type, value_options));
  DBUG_DUMP("rbr", (uchar *)table->pack_row_tmp_set.bitmap,
            table->pack_row_tmp_set.last_word_ptr -
                table->pack_row_tmp_set.bitmap + 1);

  // Null bits.
  Bit_writer null_bits(pack_ptr);
  pack_ptr += (image_column_count + 7) / 8;

  for (auto field : fields) {
    bool is_partial_json = false;
    if (bitmap_is_set(&table->pack_row_tmp_set, field->field_index)) {
      if (field->is_null(rec_offset)) {
        null_bits.set(true);
        DBUG_PRINT("info", ("field %s: NULL", field->field_name));
      } else {
        null_bits.set(false);

        // Store the field when it is not NULL.
#ifndef DBUG_OFF
        const uchar *old_pack_ptr = pack_ptr;
#endif
        pack_field(&pack_ptr, field, rec_offset, row_image_type, value_options,
                   &is_partial_json);
        DBUG_PRINT("info", ("field: %s; real_type: %d, pack_ptr before: %p; "
                            "pack_ptr after: %p; byte length: %d",
                            field->field_name, field->real_type(), old_pack_ptr,
                            pack_ptr, (int)(pack_ptr - old_pack_ptr)));
        DBUG_DUMP("rbr", old_pack_ptr, pack_ptr - old_pack_ptr);
      }
    }
#ifndef DBUG_OFF
    else {
      DBUG_PRINT("info", ("field %s: skipped", field->field_name));
    }
#endif

    if (has_any_json_diff && field->type() == MYSQL_TYPE_JSON) {
      partial_bits.set(is_partial_json);
      DBUG_PRINT("info",
                 ("JSON column partialness: %d", is_partial_json ? 1 : 0));
    }
  }

#ifndef DBUG_OFF
  DBUG_PRINT("info", ("partial_bits.tell()=%u, null_bits.tell()=%u",
                      partial_bits.tell(), null_bits.tell()));
  if (has_any_json_diff)
    DBUG_ASSERT(partial_bits.tell() == json_column_count);
  else
    DBUG_ASSERT(partial_bits.tell() == 0);
  DBUG_ASSERT(null_bits.tell() == image_column_count);
  null_bits.dbug_print("null_bits");
  partial_bits.dbug_print("partial_bits");
  DBUG_DUMP("rbr", row_data, pack_ptr - row_data);
#endif

  // Reset the pack_row_tmp_set so it can be used elsewhere.
  bitmap_clear_all(&table->pack_row_tmp_set);
  return static_cast<size_t>(pack_ptr - row_data);
}

static void insert_row_fields(std::string *row_query, TABLE *table) {
  static constexpr size_t max_field_len = 100;
  row_query->append("(");
  String buf;
#ifndef DBUG_OFF
  uint index = 0;
#endif
  for (Field **ptr = table->field; *ptr; ptr++) {
#ifndef DBUG_OFF
    // Set the read bit to avoid assertions in val_str().
    bool flip = false;
    if (table->read_set && !bitmap_is_set(table->read_set, index)) {
      flip = true;
      bitmap_flip_bit(table->read_set, index);
    }
#endif
    String *s = (*ptr)->val_str(&buf, &buf);
    if (!s)
      row_query->append("NULL");
    else
      row_query->append(s->ptr(), min(max_field_len, s->length()));
    row_query->append(",");
#ifndef DBUG_OFF
    if (flip) bitmap_flip_bit(table->read_set, index);
    ++index;
#endif
  }
  if (row_query->back() == ',') row_query->back() = ')';
}

/**
  Read the value_options from a Partial_update_rows_log_event, and if
  value_options has any bit set, also read partial_bits.

  @param[in] pack_ptr Read position before the value_options.

  @param[in] length Number of bytes between pack_ptr and the end of
  the event.

  @param[in] tabledef Table definition according to previous
  Table_map_log_event.

  @param[out] partial_bits If the event has partial_bits, initialize
  the read position of this Bit_reader to the position of the
  partial_bits.

  @param[out] event_value_options The value of the value_options field
  found in the event.

  @return The read position after value_options and partial_bits (if
  partial_bits is present).
*/
static const uchar *start_partial_bit_reader(const uchar *pack_ptr,
                                             size_t length,
                                             const table_def *tabledef,
                                             Bit_reader *partial_bits,
                                             ulonglong *event_value_options) {
  if (net_field_length_checked<ulonglong>(&pack_ptr, &length,
                                          event_value_options) ||
      *event_value_options > 1) {
    my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
    return nullptr;
  }
  DBUG_PRINT("info", ("event_value_options=%llx", *event_value_options));
  if ((*event_value_options & PARTIAL_JSON_UPDATES) != 0) {
    int json_column_count = tabledef->json_column_count();
    partial_bits->set_ptr(pack_ptr);
    DBUG_PRINT("info", ("there are %d JSON columns in the partial_bits",
                        json_column_count));
    return pack_ptr + (json_column_count + 7) / 8;
  }
  return pack_ptr;
}

/**
   Unpack a row into @c table->record[0].
   This should be only used when table_map_log_event contains column names.
   This function iterates over the columns of master and finds corresponding
   field in slave's table using the column names stored in table_def. Note, a
   hash for field names is created if number of fields in the table is above
   MAX_FIELDS_BEFORE_HASH. We will use this hash for lookup when available (see
   find_field_in_table_sef).
 */
bool unpack_row_with_column_info(const Relay_log_info *rli, TABLE *table,
                                 uchar const *const row_data,
                                 MY_BITMAP const *column_image,
                                 uchar const **const row_image_end_p,
                                 uchar const *const event_end, bool only_seek,
                                 table_def *tabledef, TABLE *conv_table,
                                 std::string *row_query) {
  DBUG_ENTER("unpack_row_with_column_info");

  uint image_column_count = bitmap_bits_set(column_image);
  const uchar *pack_ptr = row_data;
  Bit_reader null_bits(row_data);
  pack_ptr += (image_column_count + 7) / 8;

  for (uint i = 0; i < tabledef->size(); ++i) {
    if (!bitmap_is_set(column_image, i))
      // Field not actually present in the row_data
      continue;
    const char *col_name = tabledef->get_column_name(i);
    DBUG_ASSERT(col_name);
    Field *actual_field = find_field_in_table_sef(table, col_name);
    if (actual_field) {
      // use conversion table if present.
      Field *conv_field =
          conv_table ? find_field_in_table_sef(conv_table, col_name) : NULL;
      Field *const field = conv_field ? conv_field : actual_field;
      if (null_bits.get()) {
        // Handle null column case.
        if (field->is_nullable()) {
          field->reset();
          field->set_null();
        } else {
          field->set_default();
          push_warning_printf(
              current_thd, Sql_condition::SL_WARNING, ER_BAD_NULL_ERROR,
              ER_THD(current_thd, ER_BAD_NULL_ERROR), field->field_name);
        }
      } else {
        field->set_notnull();
        uint16 const metadata = tabledef->field_metadata(i);
        uint32 len =
            tabledef->calc_field_size(i, const_cast<uchar *>(pack_ptr));
        if (pack_ptr + len > event_end) {
          pack_ptr += len;
          my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
          DBUG_RETURN(true);
        } else if (only_seek) {
          pack_ptr += len;
        } else {
          pack_ptr = field->unpack(field->ptr, pack_ptr, metadata, true);
        }
      }
      if (conv_field) {
        Copy_field copy;
        actual_field->table->in_use->check_for_truncated_fields =
            CHECK_FIELD_WARN;
        copy.set(actual_field, conv_field, true);
        const auto conv_status = copy.invoke_do_copy();
        const bool check_conv =
            (slave_type_conversions_options &
             (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_NON_TRUNCATION)) ||
            !rli->get_rbr_column_type_mismatch_whitelist()->empty();
        if (check_conv && (conv_status ||
                           actual_field->table->in_use->num_truncated_fields)) {
          const char *db_name = table->s->db.str;
          const char *tbl_name = table->s->table_name.str;
          char source_buf[MAX_FIELD_WIDTH];
          char target_buf[MAX_FIELD_WIDTH];

          String source_type(source_buf, MAX_FIELD_WIDTH, &my_charset_latin1);
          String target_type(target_buf, MAX_FIELD_WIDTH, &my_charset_latin1);
          show_sql_type(tabledef->type(i), false, tabledef->field_metadata(i),
                        &source_type, conv_field->charset());
          actual_field->sql_type(target_type);
          my_error(ER_SLAVE_CONVERSION_FAILED, MYF(0), i, db_name, tbl_name,
                   source_type.c_ptr_safe(), target_type.c_ptr_safe());
          DBUG_RETURN(true);
        }
      }
    } else {
      // This column is removed on slave, so skip this field.
      if (!null_bits.get()) {
        uint32 len =
            tabledef->calc_field_size(i, const_cast<uchar *>(pack_ptr));
        if (pack_ptr + len > event_end) {
          pack_ptr += len;
          my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
          DBUG_RETURN(true);
        }
        pack_ptr += len;
      }
    }
  }
  if (row_query) insert_row_fields(row_query, table);
  *row_image_end_p = pack_ptr;
  DBUG_RETURN(false);
}

/**
  Unpack a row image (either before-image or after-image) into @c
  table->record[0].

  The row is assumed to only consist of the fields for which the
  corresponding bit in bitset @c column_image is set; the other parts
  of the record are left alone.

  If the slave table has more columns than the master table, then the
  extra columns are not touched by this function.  If the master table
  has more columns than the slave table, then the position is moved to
  after the extra columns, but the values are not used.

  - The layout of a row is:

    For WRITE_ROWS_EVENT:
        +--------------+
        | after-image |
        +--------------+

    For DELETE_ROWS_EVENT:
        +--------------+
        | before-image |
        +--------------+

    For UPDATE_ROWS_EVENT:
        +--------------+-------------+
        | before-image | after-image |
        +--------------+-------------+

    For PARTIAL_UPDATE_ROWS_EVENT:
        +--------------+--------------+-------------+
        | before-image | shared-image | after-image |
        +--------------+--------------+-------------+

  - Each of before-image and after-image has the following format:
        +--------+-------+-------+     +-------+
        | length | col_1 | col_2 | ... | col_N |
        +--------+-------+-------+     +-------+
    length is a 4-byte integer in little-endian format, equal to the
    total length in bytes of col_1, col_2, ..., col_N.

  - The shared-image has one of the following formats:
        +-----------------+
        | value_options=0 |
        +-----------------+
    or
        +-----------------+--------------+
        | value_options=1 | partial_bits |
        +-----------------+--------------+
    where:

    - value_options is a bitmap, stored as an integer, in the format
      of net_field_length. Currently only one bit is allowed:
      1=PARTIAL_JSON_UPDATES (so therefore the integer is always 0 or
      1, so in reality value_options is only one byte).  When
      PARTIAL_JSON_UPDATES=0, there is nothing else in the
      shared-image.  When PARTIAL_JSON_UPDATES=1, there is a
      partial_bits field.

    - partial_bits has one bit for each *JSON* column in the table
      (regardless of whether it is included in the before-image and/or
      after-image).  The bit is 0 if the JSON update is stored as a
      full document in the after-image, and 1 if the JSON update in
      partial form in the after-image.

    - Both when reading the before-image and when reading the
      after-image it is necessary to know the partialness of JSON
      columns: when reading the before-image, before looking up the
      row in the table, we need to set the column in the table's
      'read_set' (even if the column was not in the before-image), in
      order to guarantee that the storage engine reads that column, so
      that there is any base document that the diff can be applied
      on. When reading the after-image, we need to know which columns
      are partial so that we can correctly parse the data for that
      column.

      Therefore, when this function parses the before-image of a
      PARTIAL_UPDATE_ROWS_LOG_EVENT, it reads both the before-image
      and the shared-image, but leaves the read position after the
      before-image.  So when it parses the after-image of a
      PARTIAL_UPDATE_ROWS_LOG_EVENT, the read position is at the
      beginning of the shared-image, so it parses both the
      shared-image and the after-image.

  @param[in] rli Applier execution context

  @param[in,out] table Table to unpack into

  @param[in] master_column_count Number of columns that the master had
  in its table

  @param[in] row_data Beginning of the row image

  @param[in] column_image Pointer to a bit vector where the N'th bit
  is 0 for columns that are not included in the event, and 1 for
  columns that are included in the event.

  @param[out] row_image_end_p If this function returns successfully, it
  sets row_image_end to point to the next byte after the row image
  that it has read.

  @param[in] event_end Pointer to the end of the event.

  @param[in] row_image_type The type of row image that we are going to
  read: WRITE_AI, UPDATE_BI, UPDATE_AI, or DELETE_BI.

  @param[in] event_has_value_options true for PARTIAL_UPDATE_ROWS_EVENT,
  false for UPDATE_ROWS_EVENT.

  @param only_seek If true, this is a seek operation rather than a
  read operation.  It will only compute the row_image_end_p pointer,
  and not read anything into the table and not apply any JSON diffs.
  (This is used in slave_rows_search_algorithms=HASH_SCAN, which (1)
  unpacks and hashes the before-image for all rows in the event, (2)
  scans the table, and for each matching row it (3) unpacks the
  after-image and applies on the table. In step (1) it needs to unpack
  the after-image too, in order to move the read position forwards,
  and then it should use only_seek=true.  This is an optimization, but
  more importantly, when the after-image contains partial JSON, the
  partial JSON cannot be applied in step (1) since there is no JSON
  document to apply it on.)

  @returns false on success, true on error.
 */
bool unpack_row(Relay_log_info const *rli, TABLE *table,
                uint const master_column_count, uchar const *const row_data,
                MY_BITMAP const *column_image,
                uchar const **const row_image_end_p,
                uchar const *const event_end,
                enum_row_image_type row_image_type,
                bool event_has_value_options, bool only_seek,
                std::string *row_query) {
  DBUG_TRACE;
  DBUG_ASSERT(rli != nullptr);
  DBUG_ASSERT(table != nullptr);
  DBUG_ASSERT(row_data != nullptr);
  DBUG_ASSERT(column_image != nullptr);
  // This is guaranteed by the way column_image is initialized in the
  // Rows_log_event constructor.
  DBUG_ASSERT(column_image->n_bits == master_column_count);
  DBUG_ASSERT(row_image_end_p != nullptr);
  DBUG_ASSERT(event_end >= row_data);
  if (event_has_value_options)
    DBUG_ASSERT(row_image_type == enum_row_image_type::UPDATE_BI ||
                row_image_type == enum_row_image_type::UPDATE_AI);

  // Get table_def object and table used for type conversion
  Replicated_columns_view fields{table, Replicated_columns_view::INBOUND,
                                 rli->info_thd};
  table_def *tabledef = nullptr;
  TABLE *conv_table = nullptr;
  rli->get_table_data(table, &tabledef, &conv_table);
  DBUG_ASSERT(tabledef != nullptr);

  if (tabledef->use_column_names(table)) {
    DBUG_ASSERT(!event_has_value_options);
    if (event_has_value_options) {
      my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
      return true;
    }
    return unpack_row_with_column_info(rli, table, row_data, column_image,
                                       row_image_end_p, event_end, only_seek,
                                       tabledef, conv_table, row_query);
  }

  uint image_column_count = bitmap_bits_set(column_image);

  DBUG_PRINT("info",
             ("table=%.*s "
              "master_column_count=%u image_column_count=%u "
              "tabledef=%p, conv_table=%p "
              "row_image_type=%d event_has_value_options=%d",
              (int)table->s->table_name.length, table->s->table_name.str,
              master_column_count, image_column_count, tabledef, conv_table,
              (int)row_image_type, event_has_value_options));

  // check for mismatch between column counts in table_map_event and row_event
  if (tabledef->size() != master_column_count) {
    my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
    return true;
  }

  const uchar *pack_ptr = row_data;

  /*
    For UPDATE AI, partial bits are here.  For UPDATE BI, we
    sneak-peek into partial bits after reaching the end of the row.
  */
  Bit_reader partial_bits;
  ulonglong event_value_options = 0;
  if (event_has_value_options &&
      row_image_type == enum_row_image_type::UPDATE_AI) {
    pack_ptr =
        start_partial_bit_reader(pack_ptr, event_end - pack_ptr, tabledef,
                                 &partial_bits, &event_value_options);
    /*
      We *can* compute partial updates if event_value_options has
      PARTIAL_JSON, unless only_seek==true.
    */
    DBUG_PRINT("info", ("event_value_options=%llu only_seek=%d",
                        event_value_options, (int)only_seek));
    if ((event_value_options & PARTIAL_JSON_UPDATES) != 0 && !only_seek) {
      if (table->has_columns_marked_for_partial_update())
        /*
          partial_update_info has been initialized already (so this is
          not the first row of the statement having the PARTIAL_JSON
          bit set).  Clear the diff vector between rows.
        */
        table->clear_partial_update_diffs();
      else {
        /*
          partial_update_info has not been initialized (so this is the
          first row in the statement having the PARTIAL_JSON bit set).
          Initialize partial_update_info to allow the optimizer to
          collect partial diffs when applying any diff.  Each diff
          vector will be cleared between rows
          (clear_partial_update_diffs above).  The whole
          partial_update_info structure will be cleaned up at the end
          of the statement, when close_thread_tables calls
          cleanup_partial_update.
        */
#ifndef DBUG_OFF
        int marked_columns = 0;
#endif
        for (auto it = fields.begin();
             it != fields.end() && it.filtered_pos() != master_column_count;
             ++it) {
          if (tabledef->type(it.filtered_pos()) == MYSQL_TYPE_JSON &&
              bitmap_is_set(column_image, it.filtered_pos())) {
#ifndef DBUG_OFF
            marked_columns++;
#endif
            if (table->mark_column_for_partial_update(*it))
              // my_error was already called
              return true; /* purecov: inspected */
          }
        }
#ifndef DBUG_OFF
        DBUG_EXECUTE_IF("rpl_row_jsondiff_binarydiff", {
          if (marked_columns == 1) {
            const char act[] =
                "now SIGNAL signal.rpl_row_jsondiff_binarydiff_marked_columns";
            DBUG_ASSERT(opt_debug_sync_timeout > 0);
            DBUG_ASSERT(
                !debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
          }
        };);
#endif
        table->setup_partial_update();
      }
    }
  }

  // NULL bits
  Bit_reader null_bits(pack_ptr);
  pack_ptr += (image_column_count + 7) / 8;

  // Iterate over columns that exist both in master and slave
  for (auto it = fields.begin();
       it != fields.end() && it.filtered_pos() != master_column_count; ++it) {
    Field *field_ptr = *it;
    size_t col_i = it.filtered_pos();

    /*
      If there is a conversion table, we pick up the field pointer to
      the conversion table.  If the conversion table or the field
      pointer is NULL, no conversions are necessary.
     */
    Field *conv_field =
        conv_table ? conv_table->field[it.absolute_pos()] : nullptr;
    Field *const f = conv_field ? conv_field : field_ptr;
    DBUG_PRINT("debug", ("Conversion %srequired for field '%s' (#%lu)",
                         conv_field ? "" : "not ", field_ptr->field_name,
                         static_cast<long unsigned int>(it.absolute_pos())));
    DBUG_ASSERT(f != nullptr);

    DBUG_PRINT("debug",
               ("field name: %s; field position: %p", f->field_name, pack_ptr));

    bool is_partial_json = false;
    if ((event_value_options & PARTIAL_JSON_UPDATES) != 0 &&
        tabledef->type(col_i) == MYSQL_TYPE_JSON) {
      is_partial_json = partial_bits.get();
      DBUG_PRINT("info", ("Read %d from partial_bits", is_partial_json));
    }

    /*
      No need to bother about columns that does not exist: they have
      gotten default values when being emptied above.
     */
    if (bitmap_is_set(column_image, col_i)) {
      /* Field...::unpack() cannot return 0 */
      DBUG_ASSERT(pack_ptr != nullptr);

      if (null_bits.get()) {
        if (f->is_nullable()) {
          DBUG_PRINT("debug", ("Was NULL"));
          /**
            Calling reset just in case one is unpacking on top a
            record with data.

            This could probably go into set_null() but doing so,
            (i) triggers assertion in other parts of the code at
            the moment; (ii) it would make us reset the field,
            always when setting null, which right now doesn't seem
            needed anywhere else except here.

            TODO: maybe in the future we should consider moving
                  the reset to make it part of set_null. But then
                  the assertions triggered need to be
                  addressed/revisited.
           */
          f->reset();
          f->set_null();
        } else {
          f->set_default();
          push_warning_printf(
              current_thd, Sql_condition::SL_WARNING, ER_BAD_NULL_ERROR,
              ER_THD(current_thd, ER_BAD_NULL_ERROR), f->field_name);
        }
      } else {
        f->set_notnull();

        /*
          We only unpack the field if it was non-null.
          Use the master's size information if available else call
          normal unpack operation.
        */
        uint const metadata = tabledef->field_metadata(col_i);
#ifndef DBUG_OFF
        uchar const *const old_pack_ptr = pack_ptr;
#endif
        /// @todo calc_field_size may read out of bounds /Sven
        uint32 len = tabledef->calc_field_size(col_i, pack_ptr);
        uint32 event_len = event_end - pack_ptr;
        DBUG_PRINT("info", ("calc_field_size ret=%d event_len=%d", (int)len,
                            (int)event_len));
        if (len > event_len) {
          my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
          return true;
        }
        if (only_seek)
          pack_ptr += len;
        else if (unpack_field(&pack_ptr, f, metadata, row_image_type,
                              is_partial_json))
          return true;
        DBUG_PRINT("debug", ("Unpacked; metadata: 0x%x;"
                             " pack_ptr: %p; pack_ptr': %p; bytes: %d",
                             metadata, old_pack_ptr, pack_ptr,
                             (int)(pack_ptr - old_pack_ptr)));

        /*
          The raw size of the field, as calculated in calc_field_size,
          should match the one reported by Field_*::unpack unless it is
          a old decimal data type which is unsupported datatype in
          RBR mode.
         */
        DBUG_ASSERT(tabledef->type(col_i) == MYSQL_TYPE_DECIMAL ||
                    tabledef->calc_field_size(col_i, old_pack_ptr) ==
                        (uint32)(pack_ptr - old_pack_ptr));
      }

      /*
        If conv_field is set, then we are doing a conversion. In this
        case, we have unpacked the master data to the conversion
        table, so we need to copy the value stored in the conversion
        table into the final table and do the conversion at the same time.
      */
      if (conv_field) {
        Copy_field copy;
#ifndef DBUG_OFF
        char source_buf[MAX_FIELD_WIDTH];
        char value_buf[MAX_FIELD_WIDTH];
        String source_type(source_buf, sizeof(source_buf), system_charset_info);
        String value_string(value_buf, sizeof(value_buf), system_charset_info);
        conv_field->sql_type(source_type);
        conv_field->val_str(&value_string);
        DBUG_PRINT("debug", ("Copying field '%s' of type '%s' with value '%s'",
                             field_ptr->field_name, source_type.c_ptr_safe(),
                             value_string.c_ptr_safe()));
#endif
        field_ptr->table->in_use->check_for_truncated_fields = CHECK_FIELD_WARN;
        copy.set(field_ptr, f, true);
        const auto conv_status = copy.invoke_do_copy();
        const bool check_conv =
            (slave_type_conversions_options &
             (1ULL << SLAVE_TYPE_CONVERSIONS_ALL_NON_TRUNCATION)) ||
            !rli->get_rbr_column_type_mismatch_whitelist()->empty();
        if (check_conv &&
            (conv_status || field_ptr->table->in_use->num_truncated_fields)) {
          const char *db_name = table->s->db.str;
          const char *tbl_name = table->s->table_name.str;
          char source_buf2[MAX_FIELD_WIDTH];
          char target_buf2[MAX_FIELD_WIDTH];

          String source_type2(source_buf2, MAX_FIELD_WIDTH, &my_charset_latin1);
          String target_type2(target_buf2, MAX_FIELD_WIDTH, &my_charset_latin1);
          show_sql_type(tabledef->type(col_i), false,
                        tabledef->field_metadata(col_i), &source_type2,
                        f->charset());
          field_ptr->sql_type(target_type2);
          my_error(ER_SLAVE_CONVERSION_FAILED, MYF(0), col_i, db_name, tbl_name,
                   source_type2.c_ptr_safe(), target_type2.c_ptr_safe());
          return true;
        }
#ifndef DBUG_OFF
        char target_buf[MAX_FIELD_WIDTH];
        String target_type(target_buf, sizeof(target_buf), system_charset_info);
        field_ptr->sql_type(target_type);
        field_ptr->val_str(&value_string);
        DBUG_PRINT("debug", ("Value of field '%s' of type '%s' is now '%s'",
                             field_ptr->field_name, target_type.c_ptr_safe(),
                             value_string.c_ptr_safe()));
#endif
      }
    }
#ifndef DBUG_OFF
    else {
      DBUG_PRINT("debug", ("Non-existent: skipped"));
    }
#endif
  }

  // move past master's extra fields
  for (size_t col_i = fields.filtered_size(); col_i < master_column_count;
       ++col_i) {
    if ((event_value_options & PARTIAL_JSON_UPDATES) != 0 &&
        tabledef->type(col_i) == MYSQL_TYPE_JSON)
      partial_bits.get();
    if (bitmap_is_set(column_image, col_i)) {
      if (!null_bits.get()) {
        uint32 len = tabledef->calc_field_size(col_i, pack_ptr);
        uint32 event_len = event_end - pack_ptr;
        DBUG_PRINT("info", ("Skipping field"));
        DBUG_DUMP("info", pack_ptr, len);
        if (len > event_len) {
          my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
          return true;
        }
        pack_ptr += len;
      }
    }
  }

  if (row_query) insert_row_fields(row_query, table);

  // We have read all the null bits.
  DBUG_ASSERT(null_bits.tell() == image_column_count);

  DBUG_DUMP("info", row_data, pack_ptr - row_data);

  *row_image_end_p = pack_ptr;

  // Read partial_bits, if this is UPDATE_BI of a PARTIAL_UPDATE_ROWS_LOG_EVENT
  if (event_has_value_options &&
      row_image_type == enum_row_image_type::UPDATE_BI) {
    DBUG_PRINT("info", ("reading partial_bits"));
    pack_ptr =
        start_partial_bit_reader(pack_ptr, event_end - pack_ptr, tabledef,
                                 &partial_bits, &event_value_options);
    if ((event_value_options & PARTIAL_JSON_UPDATES) != 0) {
      for (auto it = fields.begin();
           it != fields.end() && it.filtered_pos() != master_column_count;
           ++it) {
        if (tabledef->type(it.filtered_pos()) == MYSQL_TYPE_JSON) {
          if (partial_bits.get()) {
            DBUG_PRINT("info",
                       ("forcing column %s in the read_set", it->field_name));
            bitmap_set_bit(table->read_set, it.absolute_pos());
          }
#ifndef NO_DBUG
          else
            DBUG_PRINT("info", ("not forcing column %s in the read_set",
                                it->field_name));
#endif
        }
      }
    }
  }

  return false;
}

/**
  Fills @c table->record[0] with default values.

  First @c restore_record() is called to restore the default values for
  the record concerning the given table. Then, if @c check is true,
  a check is performed to see if fields have the default value or can
  be NULL. Otherwise an error is reported.

  @param table  Table whose record[0] buffer is prepared.
  @param cols   bitmap with a set bit for each column that should be stored
                in a row.
  @param check  Specifies if lack of default error needs checking.

  @returns 0 on success or a handler level error code
 */
int prepare_record(TABLE *const table, const MY_BITMAP *cols,
                   const bool check) {
  DBUG_TRACE;

  restore_record(table, s->default_values);

  if (!check) return 0;

  /*
    For fields the extra fields on the slave, we check if they have a default.
    The check follows the same rules as the INSERT query without specifying an
    explicit value for a field not having the explicit default
    (@c check_that_all_fields_are_given_values()).
  */

  DBUG_PRINT_BITSET("debug", "cols: %s", cols);
  /**
    Save a reference to the original write set bitmaps.
    We will need this to restore the bitmaps at the end.
  */
  MY_BITMAP *old_write_set = table->write_set;
  /**
    Just to be sure that tmp_set is currently not in use as
    the write_set already.
  */
  DBUG_ASSERT(table->write_set != &table->tmp_set);
  /* set the temporary write_set */
  table->column_bitmaps_set_no_signal(table->read_set, &table->tmp_set);
  /**
    Set table->write_set bits for all the columns as they
    will be checked in set_default() function.
  */
  bitmap_set_all(table->write_set);

  for (Field **field_ptr = table->field; *field_ptr; ++field_ptr) {
    uint field_index = (uint)(field_ptr - table->field);
    if (field_index >= cols->n_bits || !bitmap_is_set(cols, field_index)) {
      Field *const f = *field_ptr;
      if ((f->flags & NO_DEFAULT_VALUE_FLAG) &&
          (f->real_type() != MYSQL_TYPE_ENUM)) {
        f->set_default();
        push_warning_printf(
            current_thd, Sql_condition::SL_WARNING, ER_NO_DEFAULT_FOR_FIELD,
            ER_THD(current_thd, ER_NO_DEFAULT_FOR_FIELD), f->field_name);
      } else if (f->has_insert_default_datetime_value_expression() ||
                 f->has_insert_default_general_value_expression()) {
        f->set_default();
      }
    }
  }

  /* set the write_set back to original*/
  table->column_bitmaps_set_no_signal(table->read_set, old_write_set);

  return 0;
}
