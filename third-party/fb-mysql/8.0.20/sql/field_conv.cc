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

/**
  @file
  Functions to copy data to or from fields.
*/

#include <string.h>
#include <sys/types.h>
#include <algorithm>

#include "field_types.h"
#include "m_ctype.h"
#include "my_byteorder.h"
#include "my_compare.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "nullable.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/item_timefunc.h"  // Item_func_now_local
#include "sql/my_decimal.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_time.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql_string.h"
#include "template_utils.h"  // down_cast

/**
  Check if geometry type sub is a subtype of super.

  @param sub The type to check
  @param super The supertype

  @return True if sub is a subtype of super
 */
inline static bool is_subtype_of(Field::geometry_type sub,
                                 Field::geometry_type super) {
  return (
      super == Field::GEOM_GEOMETRY ||
      (super == Field::GEOM_GEOMETRYCOLLECTION &&
       (sub == Field::GEOM_MULTIPOINT || sub == Field::GEOM_MULTILINESTRING ||
        sub == Field::GEOM_MULTIPOLYGON)));
}

static type_conversion_status do_field_eq(Copy_field *, const Field *from_field,
                                          Field *to_field) {
  memcpy(to_field->ptr, from_field->ptr, from_field->pack_length());
  return TYPE_OK;
}

static void set_to_is_null(Field *to_field, bool is_null) {
  if (to_field->is_nullable() || to_field->is_tmp_nullable()) {
    if (is_null) {
      to_field->set_null();
    } else {
      to_field->set_notnull();
    }
  } else {
    // This is used in the case of window functions, where
    // bring_back_frame_row() may want to set TABLE::null_row to be what it was
    // when the row was buffered.
    if (is_null) {
      to_field->table->set_null_row();
    } else {
      to_field->table->reset_null_row();
    }
  }
}

static type_conversion_status do_field_to_null_str(Copy_field *,
                                                   const Field *from_field,
                                                   Field *to_field) {
  if (from_field->is_null()) {
    memset(to_field->ptr, 0, from_field->pack_length());
    set_to_is_null(to_field, true);
  } else {
    memcpy(to_field->ptr, from_field->ptr, from_field->pack_length());
    set_to_is_null(to_field, false);
  }
  return TYPE_OK;
}

type_conversion_status set_field_to_null(Field *field) {
  if (field->is_nullable() || field->is_tmp_nullable()) {
    field->set_null();
    field->reset();
    return TYPE_OK;
  }

  /**
    The following piece of code is run for the case when a BLOB column
    that has value NULL is queried with GROUP BY NULL and the result
    is inserted into a some table's column declared having primitive type
    (e.g. INT) and NOT NULL.

    For example, the following test case will hit this piece of code:
    CREATE TABLE t1 (a BLOB);
    CREATE TABLE t2 (a INT NOT NULL);

    INSERT t1 VALUES (NULL);
    INSERT INTO t2(a) SELECT a FROM t1 GROUP BY NULL; <<== Hit here

    In general, when set_field_to_null() is called a Field has to be either
    declared as NULL-able or be marked as temporary NULL-able.
    But in case of INSERT SELECT from a BLOB field and when GROUP BY NULL
    is specified the Field object for a destination column doesn't set
    neither NULL-able nor temporary NULL-able (see setup_copy_fields()).
  */
  field->reset();
  switch (field->table->in_use->check_for_truncated_fields) {
    case CHECK_FIELD_WARN:
      field->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
      /* fall through */
    case CHECK_FIELD_IGNORE:
      return TYPE_OK;
    case CHECK_FIELD_ERROR_FOR_NULL:
      my_error(ER_BAD_NULL_ERROR, MYF(0), field->field_name);
      return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
  }
  DBUG_ASSERT(false);  // impossible

  my_error(ER_BAD_NULL_ERROR, MYF(0), field->field_name);
  return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;  // to avoid compiler's warning
}

/**
  Set field to NULL or TIMESTAMP or to next auto_increment number.

  @param field           Field to update
  @param no_conversions  Set to 1 if we should return 1 if field can't
                         take null values.
                         If set to 0 we will do store the 'default value'
                         if the field is a special field. If not we will
                         give an error.

  @retval
    0    Field could take 0 or an automatic conversion was used
  @retval
    -1   Field could not take NULL and no conversion was used.
    If no_conversion was not set, an error message is printed
*/

type_conversion_status set_field_to_null_with_conversions(Field *field,
                                                          bool no_conversions) {
  if (field->is_nullable()) {
    field->set_null();
    field->reset();
    return TYPE_OK;
  }

  if (no_conversions) return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;

  /*
    Check if this is a special type, which will get a special walue
    when set to NULL (TIMESTAMP fields which allow setting to NULL
    are handled by first check).

    From the manual:

    TIMESTAMP columns [...] assigning NULL assigns the current timestamp.

    But if explicit_defaults_for_timestamp, use standard-compliant behaviour:
    no special value.
  */
  if (field->type() == MYSQL_TYPE_TIMESTAMP &&
      !field->table->in_use->variables.explicit_defaults_for_timestamp) {
    Item_func_now_local::store_in(field);
    return TYPE_OK;  // Ok to set time to NULL
  }

  // Note: we ignore any potential failure of reset() here.
  field->reset();

  if (field == field->table->next_number_field) {
    field->table->autoinc_field_has_explicit_non_null_value = false;
    return TYPE_OK;  // field is set in fill_record()
  }

  if (field->is_tmp_nullable()) {
    field->set_null();
    field->reset();
    return TYPE_OK;
  }

  // Conversion of NULL to empty string does not apply to geometry columns.
  if (field->type() == MYSQL_TYPE_GEOMETRY) {
    my_error(ER_BAD_NULL_ERROR_NOT_IGNORED, MYF(0), field->field_name);
    return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
  }

  switch (field->table->in_use->check_for_truncated_fields) {
    case CHECK_FIELD_WARN:
      field->set_warning(Sql_condition::SL_WARNING, ER_BAD_NULL_ERROR, 1);
      /* fall through */
    case CHECK_FIELD_IGNORE:
      if (field->type() == MYSQL_TYPE_BLOB) {
        /*
          BLOB/TEXT fields only store a pointer to their actual contents
          in the record. Make this a valid pointer to an empty string
          instead of nullptr.
        */
        return field->store("", 0, field->charset());
      }
      return TYPE_OK;
    case CHECK_FIELD_ERROR_FOR_NULL:
      my_error(ER_BAD_NULL_ERROR, MYF(0), field->field_name);
      return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
  }
  DBUG_ASSERT(false);  // impossible
  my_error(ER_BAD_NULL_ERROR, MYF(0), field->field_name);
  return TYPE_ERR_NULL_CONSTRAINT_VIOLATION;
}

static type_conversion_status do_skip(Copy_field *, const Field *, Field *) {
  return TYPE_OK;
}

static type_conversion_status do_copy_null(Copy_field *copy,
                                           const Field *from_field,
                                           Field *to_field) {
  type_conversion_status ret = TYPE_OK;
  if (from_field->is_null()) {
    set_to_is_null(to_field, true);
    to_field->reset();
  } else {
    set_to_is_null(to_field, false);
    ret = copy->invoke_do_copy2(from_field, to_field);
  }
  return ret;
}

static type_conversion_status do_copy_not_null(Copy_field *copy,
                                               const Field *from_field,
                                               Field *to_field) {
  type_conversion_status ret = TYPE_OK;
  if (from_field->is_null()) {
    if (to_field->reset() == TYPE_ERR_NULL_CONSTRAINT_VIOLATION)
      my_error(ER_INVALID_USE_OF_NULL, MYF(0));
    else
      to_field->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
  } else
    ret = copy->invoke_do_copy2(from_field, to_field);
  return ret;
}

static type_conversion_status do_copy_maybe_null(Copy_field *copy,
                                                 const Field *from_field,
                                                 Field *to_field) {
  /*
    NOTE: In reverse copying (see bring_back_frame_row() for windowing),
    "to" is "from".
  */
  set_to_is_null(to_field, false);
  return copy->invoke_do_copy2(from_field, to_field);
}

/* timestamp and next_number has special handling in case of NULL values */

static type_conversion_status do_copy_timestamp(Copy_field *copy,
                                                const Field *from_field,
                                                Field *to_field) {
  type_conversion_status ret = TYPE_OK;
  if (from_field->is_null()) {
    /* Same as in set_field_to_null_with_conversions() */
    Item_func_now_local::store_in(to_field);
  } else
    ret = copy->invoke_do_copy2(from_field, to_field);
  return ret;
}

static type_conversion_status do_copy_next_number(Copy_field *copy,
                                                  const Field *from_field,
                                                  Field *to_field) {
  type_conversion_status ret = TYPE_OK;
  if (from_field->is_null()) {
    /* Same as in set_field_to_null_with_conversions() */
    to_field->table->autoinc_field_has_explicit_non_null_value = false;
    to_field->reset();
  } else
    ret = copy->invoke_do_copy2(from_field, to_field);
  return ret;
}

static type_conversion_status do_copy_blob(Copy_field *,
                                           const Field *from_field,
                                           Field *to_field) {
  const Field_blob *from_blob = down_cast<const Field_blob *>(from_field);
  Field_blob *to_blob = down_cast<Field_blob *>(to_field);
  uint32 from_length = from_blob->get_length();
  to_blob->set_ptr(from_length, from_blob->get_ptr());
  if (to_blob->get_length() < from_length) {
    if (to_field->table->in_use->is_strict_sql_mode()) {
      to_field->set_warning(Sql_condition::SL_WARNING, ER_DATA_TOO_LONG, 1);
    } else {
      to_field->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    }
  }
  return TYPE_OK;
}

static type_conversion_status do_conv_blob(Copy_field *copy,
                                           const Field *from_field,
                                           Field *to_field) {
  from_field->val_str(&copy->tmp);
  static_cast<Field_blob *>(to_field)->store(
      copy->tmp.ptr(), copy->tmp.length(), copy->tmp.charset());
  return TYPE_OK;
}

/** Save blob in copy->tmp for GROUP BY. */

static type_conversion_status do_save_blob(Copy_field *copy,
                                           const Field *from_field,
                                           Field *to_field) {
  char buff[MAX_FIELD_WIDTH];
  String res(buff, sizeof(buff), copy->tmp.charset());
  from_field->val_str(&res);
  copy->tmp.copy(res);
  return down_cast<Field_blob *>(to_field)->store(
      copy->tmp.ptr(), copy->tmp.length(), copy->tmp.charset());
}

/**
  Copy the contents of one Field_json into another Field_json.
*/
static type_conversion_status do_save_json(Copy_field *,
                                           const Field *from_field,
                                           Field *to_field) {
  const Field_json *from = down_cast<const Field_json *>(from_field);
  Field_json *to = down_cast<Field_json *>(to_field);
  return to->store(from);
}

static type_conversion_status do_field_string(Copy_field *,
                                              const Field *from_field,
                                              Field *to_field) {
  StringBuffer<MAX_FIELD_WIDTH> res(from_field->charset());
  res.length(0U);

  from_field->val_str(&res);
  return to_field->store(res.ptr(), res.length(), res.charset());
}

static type_conversion_status do_field_enum(Copy_field *copy,
                                            const Field *from_field,
                                            Field *to_field) {
  type_conversion_status ret = TYPE_OK;
  if (from_field->val_int() == 0) {
    down_cast<Field_enum *>(to_field)->store_type(0ULL);
  } else
    ret = do_field_string(copy, from_field, to_field);
  return ret;
}

static type_conversion_status do_field_varbinary_pre50(Copy_field *copy,
                                                       const Field *from_field,
                                                       Field *to_field) {
  char buff[MAX_FIELD_WIDTH];
  copy->tmp.set_quick(buff, sizeof(buff), copy->tmp.charset());
  from_field->val_str(&copy->tmp);

  /* Use the same function as in 4.1 to trim trailing spaces */
  size_t length = my_lengthsp_8bit(&my_charset_bin, copy->tmp.c_ptr_quick(),
                                   from_field->field_length);

  return to_field->store(copy->tmp.c_ptr_quick(), length, copy->tmp.charset());
}

static type_conversion_status do_field_int(Copy_field *,
                                           const Field *from_field,
                                           Field *to_field) {
  longlong value = from_field->val_int();
  return to_field->store(value, from_field->flags & UNSIGNED_FLAG);
}

static type_conversion_status do_field_real(Copy_field *,
                                            const Field *from_field,
                                            Field *to_field) {
  return to_field->store(from_field->val_real());
}

static type_conversion_status do_field_decimal(Copy_field *,
                                               const Field *from_field,
                                               Field *to_field) {
  my_decimal value;
  return to_field->store_decimal(from_field->val_decimal(&value));
}

inline type_conversion_status copy_time_to_time(const Field *from, Field *to) {
  MYSQL_TIME ltime;
  from->get_time(&ltime);
  return to->store_time(&ltime);
}

/**
  Convert between fields using time representation.
*/
static type_conversion_status do_field_time(Copy_field *,
                                            const Field *from_field,
                                            Field *to_field) {
  return copy_time_to_time(from_field, to_field);
}

/**
  string copy for single byte characters set when to string is shorter than
  from string.
*/

static type_conversion_status do_cut_string(Copy_field *,
                                            const Field *from_field,
                                            Field *to_field) {
  const CHARSET_INFO *cs = from_field->charset();
  memcpy(to_field->ptr, from_field->ptr, to_field->pack_length());

  /* Check if we loosed any important characters */
  if (cs->cset->scan(
          cs,
          pointer_cast<const char *>(from_field->ptr + to_field->pack_length()),
          pointer_cast<const char *>(from_field->ptr +
                                     from_field->pack_length()),
          MY_SEQ_SPACES) <
      from_field->pack_length() - to_field->pack_length()) {
    to_field->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    return TYPE_WARN_TRUNCATED;
  }
  return TYPE_OK;
}

/**
  string copy for multi byte characters set when to string is shorter than
  from string.
*/

static type_conversion_status do_cut_string_complex(
    Copy_field *, const Field *from_field,
    Field *to_field) {  // Shorter string field
  int well_formed_error;
  type_conversion_status ret = TYPE_OK;
  const CHARSET_INFO *cs = from_field->charset();
  const uchar *from_end = from_field->ptr + from_field->pack_length();
  size_t copy_length = cs->cset->well_formed_len(
      cs, pointer_cast<const char *>(from_field->ptr),
      pointer_cast<const char *>(from_end),
      to_field->pack_length() / cs->mbmaxlen, &well_formed_error);
  if (to_field->pack_length() < copy_length) {
    copy_length = to_field->pack_length();
  }
  memcpy(to_field->ptr, from_field->ptr, copy_length);

  /* Check if we lost any important characters */
  if (well_formed_error ||
      cs->cset->scan(cs,
                     pointer_cast<const char *>(from_field->ptr) + copy_length,
                     pointer_cast<const char *>(from_end), MY_SEQ_SPACES) <
          (from_field->pack_length() - copy_length)) {
    to_field->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    ret = TYPE_WARN_TRUNCATED;
  }

  if (copy_length < to_field->pack_length()) {
    cs->cset->fill(cs, pointer_cast<char *>(to_field->ptr) + copy_length,
                   to_field->pack_length() - copy_length, ' ');
  }
  return ret;
}

static type_conversion_status do_expand_binary(Copy_field *,
                                               const Field *from_field,
                                               Field *to_field) {
  const CHARSET_INFO *cs = from_field->charset();
  memcpy(to_field->ptr, from_field->ptr, from_field->pack_length());
  cs->cset->fill(
      cs, pointer_cast<char *>(to_field->ptr) + from_field->pack_length(),
      to_field->pack_length() - from_field->pack_length(), '\0');
  return TYPE_OK;
}

static type_conversion_status do_expand_string(Copy_field *,
                                               const Field *from_field,
                                               Field *to_field) {
  const CHARSET_INFO *cs = from_field->charset();
  memcpy(to_field->ptr, from_field->ptr, from_field->pack_length());
  cs->cset->fill(
      cs, pointer_cast<char *>(to_field->ptr) + from_field->pack_length(),
      to_field->pack_length() - from_field->pack_length(), ' ');
  return TYPE_OK;
}

/**
  Find how many bytes should be copied between Field_varstring fields
  so that only the bytes in use in the 'from' field are copied.
  Handles single and multi-byte charsets. Adds warning if not all
  bytes in 'from' will fit into 'to'.

  @param to   Variable length field we're copying to
  @param from Variable length field we're copying from

  @return Number of bytes that should be copied from 'from' to 'to'.
*/
static size_t get_varstring_copy_length(Field_varstring *to,
                                        const Field_varstring *from,
                                        bool *truncated) {
  const CHARSET_INFO *const cs = from->charset();
  const bool is_multibyte_charset = (cs->mbmaxlen != 1);
  const uint to_byte_length = to->row_pack_length();

  if (truncated) *truncated = false;

  size_t bytes_to_copy;
  if (from->length_bytes == 1)
    bytes_to_copy = *from->ptr;
  else
    bytes_to_copy = uint2korr(from->ptr);

  if (from->pack_length() - from->length_bytes <= to_byte_length) {
    /*
      There's room for everything in the destination buffer;
      no need to truncate.
    */
    return bytes_to_copy;
  }

  if (is_multibyte_charset) {
    int well_formed_error;
    const char *from_beg =
        reinterpret_cast<char *>(from->ptr + from->length_bytes);
    const uint to_char_length = (to_byte_length) / cs->mbmaxlen;
    const size_t from_byte_length = bytes_to_copy;
    bytes_to_copy =
        cs->cset->well_formed_len(cs, from_beg, from_beg + from_byte_length,
                                  to_char_length, &well_formed_error);
    if (bytes_to_copy < from_byte_length) {
      if (truncated) *truncated = true;
      if (from->table->in_use->check_for_truncated_fields)
        to->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    }
  } else {
    if (bytes_to_copy > (to_byte_length)) {
      if (truncated) *truncated = true;
      bytes_to_copy = to_byte_length;
      if (from->table->in_use->check_for_truncated_fields)
        to->set_warning(Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED, 1);
    }
  }
  return bytes_to_copy;
}

/**
  A variable length string field consists of:
   (a) 1 or 2 length bytes, depending on the VARCHAR column definition
   (b) as many relevant character bytes, as defined in the length byte(s)
   (c) unused padding up to the full length of the column

  This function only copies (a) and (b)

  Condition for using this function: to and from must use the same
  number of bytes for length, i.e: to->length_bytes==from->length_bytes

  @param to   Variable length field we're copying to
  @param from Variable length field we're copying from
*/
static type_conversion_status copy_field_varstring(
    Field_varstring *const to, const Field_varstring *const from) {
  const uint length_bytes = from->length_bytes;
  DBUG_ASSERT(length_bytes == to->length_bytes);
  DBUG_ASSERT(length_bytes == 1 || length_bytes == 2);

  bool truncated = false;
  const size_t bytes_to_copy = get_varstring_copy_length(to, from, &truncated);
  if (length_bytes == 1)
    *to->ptr = static_cast<uchar>(bytes_to_copy);
  else
    int2store(to->ptr, bytes_to_copy);

  // memcpy should not be used for overlaping memory blocks
  DBUG_ASSERT(to->ptr != from->ptr);
  memcpy(to->ptr + length_bytes, from->ptr + length_bytes, bytes_to_copy);

  return !truncated ? TYPE_OK : TYPE_WARN_TRUNCATED;
}

static type_conversion_status do_varstring(Copy_field *,
                                           const Field *from_field,
                                           Field *to_field) {
  copy_field_varstring(static_cast<Field_varstring *>(to_field),
                       static_cast<const Field_varstring *>(from_field));
  return TYPE_OK;
}

/***************************************************************************
** The different functions that fills in a Copy_field class
***************************************************************************/

type_conversion_status Copy_field::invoke_do_copy(bool reverse) {
  const Field *from = reverse ? m_to_field : m_from_field;
  Field *to = reverse ? m_from_field : m_to_field;

  type_conversion_status ret = (*(m_do_copy))(this, from, to);

  if (from->is_tmp_null() && !to->is_tmp_null()) {
    to->set_tmp_nullable();
    to->set_tmp_null();
  }
  return ret;
}

type_conversion_status Copy_field::invoke_do_copy2(const Field *from,
                                                   Field *to) {
  // from will be m_to_field if invoke_do_copy was called with reverse = true
  return (*(m_do_copy2))(this, from, to);
}

/**
  copy of field to maybe null string.
  If field is null then the all bytes are set to 0.
  if field is not null then the first byte is set to 1 and the rest of the
  string is the field value.
*/

Copy_field::Copy_field(MEM_ROOT *mem_root, Item_field *item) : Copy_field() {
  /*
     Set up the record buffer and change result_field to point at
     the saved value.
  */
  m_from_field = item->field->new_field(mem_root, item->field->table);
  if (m_from_field == nullptr) return;

  if (m_from_field->is_nullable()) {
    // We need to allocate one extra byte for null handling.
    uchar *ptr = mem_root->ArrayAlloc<uchar>(m_from_field->pack_length() + 1);
    m_to_field =
        item->field->new_field(mem_root, item->field->table, ptr + 1, ptr, 1);
    if (m_to_field == nullptr) return;
    m_to_field->set_null();  // Null as default value
    m_do_copy = do_field_to_null_str;
  } else {
    uchar *ptr = mem_root->ArrayAlloc<uchar>(m_from_field->pack_length());
    m_to_field =
        item->field->new_field(mem_root, item->field->table, ptr, nullptr, 1);
    if (m_to_field == nullptr) return;
    m_do_copy = do_field_eq;
  }

  /*
    We have created a new Item_field; its field points into the
    previous table; its result_field points into a memory area
    (REF_SLICE_ORDERED_GROUP_BY) which represents the pseudo-tmp-table
    from where aggregates' values can be read. So does 'field'. A
    Copy_field manages copying from 'field' to the memory area.
  */
  item->field = m_to_field;
  item->set_result_field(m_to_field);
}

void Copy_field::set(Field *to, Field *from, bool save) {
  if (to->type() == MYSQL_TYPE_NULL) {
    m_do_copy = do_skip;
    return;
  }
  m_from_field = from;
  m_to_field = to;

  m_do_copy2 = get_copy_func(save);

  if (m_from_field->is_nullable() || m_from_field->table->is_nullable()) {
    if (m_to_field->is_nullable() || m_to_field->is_tmp_nullable())
      m_do_copy = do_copy_null;
    else if (m_to_field->type() == MYSQL_TYPE_TIMESTAMP)
      m_do_copy = do_copy_timestamp;  // Automatic timestamp
    else if (m_to_field == m_to_field->table->next_number_field)
      m_do_copy = do_copy_next_number;
    else
      m_do_copy = do_copy_not_null;
  } else if (m_to_field->is_nullable()) {
    m_do_copy = do_copy_maybe_null;
  } else {
    m_do_copy = m_do_copy2;
  }
}

/*
  To do:

  If 'save' is set to true and the 'from' is a blob field, m_do_copy is set to
  do_save_blob rather than do_conv_blob.  The only differences between them
  appears to be:

  - do_save_blob allocates and uses an intermediate buffer before calling
    Field_blob::store. Is this in order to trigger the call to
    well_formed_copy_nchars, by changing the pointer copy->tmp.ptr()?
    That call will take place anyway in all known cases.
 */
Copy_field::Copy_func *Copy_field::get_copy_func(bool save) {
  if ((m_to_field->flags & BLOB_FLAG) && save) {
    if (m_to_field->real_type() == MYSQL_TYPE_JSON &&
        m_from_field->real_type() == MYSQL_TYPE_JSON)
      return do_save_json;
    else
      return do_save_blob;
  }

  bool compatible_db_low_byte_first =
      (m_to_field->table->s->db_low_byte_first ==
       m_from_field->table->s->db_low_byte_first);
  if (m_to_field->type() == MYSQL_TYPE_GEOMETRY) {
    if (m_from_field->type() != MYSQL_TYPE_GEOMETRY ||
        m_to_field->is_nullable() != m_from_field->is_nullable() ||
        m_to_field->table->is_nullable() != m_from_field->table->is_nullable())
      return do_conv_blob;

    const Field_geom *to_geom = down_cast<const Field_geom *>(m_to_field);
    const Field_geom *from_geom = down_cast<const Field_geom *>(m_from_field);

    // If changing the SRID property of the field, we must do a full conversion.
    if (to_geom->get_srid() != from_geom->get_srid() &&
        to_geom->get_srid().has_value())
      return do_conv_blob;

    // to is same as or a wider type than from
    if (to_geom->get_geometry_type() == from_geom->get_geometry_type() ||
        is_subtype_of(from_geom->get_geometry_type(),
                      to_geom->get_geometry_type()))
      return do_field_eq;

    return do_conv_blob;
  } else if (m_to_field->flags & BLOB_FLAG) {
    /*
      We need to do conversion if we are copying from BLOB to
      non-BLOB, or if we are copying between BLOBs with different
      character sets, or if we are copying between JSON and non-JSON.
    */
    if (!(m_from_field->flags & BLOB_FLAG) ||
        m_from_field->charset() != m_to_field->charset() ||
        ((m_to_field->type() == MYSQL_TYPE_JSON) !=
         (m_from_field->type() == MYSQL_TYPE_JSON)))
      return do_conv_blob;
    if (m_from_field->pack_length() != m_to_field->pack_length() ||
        !compatible_db_low_byte_first) {
      return do_copy_blob;
    }
  } else {
    if (m_to_field->real_type() == MYSQL_TYPE_BIT ||
        m_from_field->real_type() == MYSQL_TYPE_BIT)
      return do_field_int;
    if (m_to_field->result_type() == DECIMAL_RESULT) return do_field_decimal;
    // Check if identical fields
    if (m_from_field->result_type() == STRING_RESULT) {
      if (is_temporal_type(m_from_field->type())) {
        if (is_temporal_type(m_to_field->type())) {
          return do_field_time;
        } else {
          if (m_to_field->result_type() == INT_RESULT) return do_field_int;
          if (m_to_field->result_type() == REAL_RESULT) return do_field_real;
          /* Note: conversion from any to DECIMAL_RESULT is handled earlier */
        }
      }
      /*
        Detect copy from pre 5.0 varbinary to varbinary as of 5.0 and
        use special copy function that removes trailing spaces and thus
        repairs data.
      */
      if (m_from_field->type() == MYSQL_TYPE_VAR_STRING &&
          !m_from_field->has_charset() &&
          m_to_field->type() == MYSQL_TYPE_VARCHAR &&
          !m_to_field->has_charset())
        return do_field_varbinary_pre50;

      /*
        If we are copying date or datetime's we have to check the dates
        if we don't allow 'all' dates.
      */
      if (m_to_field->real_type() != m_from_field->real_type() ||
          m_to_field->decimals() !=
              m_from_field->decimals() /* e.g. TIME vs TIME(6) */
          || !compatible_db_low_byte_first ||
          (((m_to_field->table->in_use->variables.sql_mode &
             (MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE | MODE_INVALID_DATES)) &&
            m_to_field->type() == MYSQL_TYPE_DATE) ||
           m_to_field->type() == MYSQL_TYPE_DATETIME)) {
        if (m_from_field->real_type() == MYSQL_TYPE_ENUM ||
            m_from_field->real_type() == MYSQL_TYPE_SET)
          if (m_to_field->result_type() != STRING_RESULT)
            return do_field_int;  // Convert SET to number
        return do_field_string;
      }
      if (m_to_field->real_type() == MYSQL_TYPE_ENUM ||
          m_to_field->real_type() == MYSQL_TYPE_SET) {
        if (!m_to_field->eq_def(m_from_field)) {
          if (m_from_field->real_type() == MYSQL_TYPE_ENUM &&
              m_to_field->real_type() == MYSQL_TYPE_ENUM)
            return do_field_enum;
          else
            return do_field_string;
        }
      } else if (m_to_field->charset() != m_from_field->charset())
        return do_field_string;
      else if (m_to_field->real_type() == MYSQL_TYPE_VARCHAR) {
        if (down_cast<Field_varstring *>(m_to_field)->length_bytes !=
            down_cast<Field_varstring *>(m_from_field)->length_bytes)
          return do_field_string;
        else
          return do_varstring;
      } else if (m_to_field->pack_length() < m_from_field->pack_length())
        return (m_from_field->charset()->mbmaxlen == 1 ? do_cut_string
                                                       : do_cut_string_complex);
      else if (m_to_field->pack_length() > m_from_field->pack_length()) {
        if (m_to_field->charset() == &my_charset_bin)
          return do_expand_binary;
        else
          return do_expand_string;
      }

    } else if (m_to_field->real_type() != m_from_field->real_type() ||
               m_to_field->pack_length() != m_from_field->pack_length() ||
               !compatible_db_low_byte_first) {
      if (m_to_field->real_type() == MYSQL_TYPE_DECIMAL ||
          m_to_field->result_type() == STRING_RESULT)
        return do_field_string;
      if (m_to_field->result_type() == INT_RESULT) return do_field_int;
      return do_field_real;
    } else {
      if (!m_to_field->eq_def(m_from_field) || !compatible_db_low_byte_first) {
        if (m_to_field->real_type() == MYSQL_TYPE_DECIMAL)
          return do_field_string;
        if (m_to_field->result_type() == INT_RESULT)
          return do_field_int;
        else
          return do_field_real;
      }
    }
  }
  /* Eq fields */
  DBUG_ASSERT(m_to_field->pack_length() == m_from_field->pack_length());
  return do_field_eq;
}

static inline bool is_blob_type(Field *to) {
  return (to->type() == MYSQL_TYPE_BLOB || to->type() == MYSQL_TYPE_GEOMETRY);
}

/** Simple quick field convert that is called on insert. */

type_conversion_status field_conv(Field *to, const Field *from) {
  const enum_field_types from_type = from->type();
  const enum_field_types to_type = to->type();

  if ((to_type == MYSQL_TYPE_JSON) && (from_type == MYSQL_TYPE_JSON)) {
    Field_json *to_json = down_cast<Field_json *>(to);
    const Field_json *from_json = down_cast<const Field_json *>(from);
    return to_json->store(from_json);
  }

  if (to->real_type() == from->real_type() &&
      !((is_blob_type(to)) && to->table->copy_blobs) &&
      to->charset() == from->charset() && to_type != MYSQL_TYPE_GEOMETRY) {
    if (to->real_type() == MYSQL_TYPE_VARCHAR &&
        from->real_type() == MYSQL_TYPE_VARCHAR) {
      Field_varstring *to_vc = down_cast<Field_varstring *>(to);
      const Field_varstring *from_vc = down_cast<const Field_varstring *>(from);
      if (to_vc->length_bytes == from_vc->length_bytes) {
        copy_field_varstring(to_vc, from_vc);
        return TYPE_OK;
      }
    }
    if (to->pack_length() == from->pack_length() &&
        !(to->flags & UNSIGNED_FLAG && !(from->flags & UNSIGNED_FLAG)) &&
        to->real_type() != MYSQL_TYPE_ENUM &&
        to->real_type() != MYSQL_TYPE_SET &&
        to->real_type() != MYSQL_TYPE_BIT &&
        (!is_temporal_type_with_time(to_type) ||
         to->decimals() == from->decimals()) &&
        (to->real_type() != MYSQL_TYPE_NEWDECIMAL ||
         (to->field_length == from->field_length &&
          (down_cast<Field_num *>(to)->dec ==
           down_cast<const Field_num *>(from)->dec))) &&
        to->table->s->db_low_byte_first == from->table->s->db_low_byte_first &&
        (!(to->table->in_use->variables.sql_mode &
           (MODE_NO_ZERO_IN_DATE | MODE_NO_ZERO_DATE | MODE_INVALID_DATES)) ||
         (to_type != MYSQL_TYPE_DATE && to_type != MYSQL_TYPE_DATETIME &&
          (!to->table->in_use->variables.explicit_defaults_for_timestamp ||
           to_type != MYSQL_TYPE_TIMESTAMP))) &&
        (from->real_type() != MYSQL_TYPE_VARCHAR)) {  // Identical fields
      // to->ptr==from->ptr may happen if one does 'UPDATE ... SET x=x'
      memmove(to->ptr, from->ptr, to->pack_length());
      return TYPE_OK;
    }
  }
  if (to_type == MYSQL_TYPE_BLOB) {  // Be sure the value is stored
    Field_blob *blob = (Field_blob *)to;
    return blob->store(from);
  }
  if (from->real_type() == MYSQL_TYPE_ENUM &&
      to->real_type() == MYSQL_TYPE_ENUM && from->val_int() == 0) {
    ((Field_enum *)(to))->store_type(0);
    return TYPE_OK;
  } else if (is_temporal_type(from_type) && to->result_type() == INT_RESULT) {
    MYSQL_TIME ltime;
    longlong nr;
    if (from_type == MYSQL_TYPE_TIME) {
      from->get_time(&ltime);
      if (current_thd->is_fsp_truncate_mode())
        nr = TIME_to_ulonglong_time(ltime);
      else
        nr = TIME_to_ulonglong_time_round(ltime);
    } else {
      from->get_date(&ltime, TIME_FUZZY_DATE);
      if (current_thd->is_fsp_truncate_mode())
        nr = TIME_to_ulonglong_datetime(ltime);
      else {
        nr = propagate_datetime_overflow(current_thd, [&](int *w) {
          return TIME_to_ulonglong_datetime_round(ltime, w);
        });
      }
    }
    return to->store(ltime.neg ? -nr : nr, false);
  } else if (is_temporal_type(from_type) &&
             (to->result_type() == REAL_RESULT ||
              to->result_type() == DECIMAL_RESULT ||
              to->result_type() == INT_RESULT)) {
    my_decimal tmp;
    /*
      We prefer DECIMAL as the safest precise type:
      double supports only 15 digits, which is not enough for DATETIME(6).
    */
    return to->store_decimal(from->val_decimal(&tmp));
  } else if (is_temporal_type(from_type) && is_temporal_type(to_type)) {
    return copy_time_to_time(from, to);
  } else if (from_type == MYSQL_TYPE_JSON && is_integer_type(to_type)) {
    return to->store(from->val_int(), from->flags & UNSIGNED_FLAG);
  } else if (from_type == MYSQL_TYPE_JSON && to_type == MYSQL_TYPE_NEWDECIMAL) {
    my_decimal buff;
    return to->store_decimal(from->val_decimal(&buff));
  } else if (from_type == MYSQL_TYPE_JSON &&
             (to_type == MYSQL_TYPE_FLOAT || to_type == MYSQL_TYPE_DOUBLE)) {
    return to->store(from->val_real());
  } else if (from_type == MYSQL_TYPE_JSON && is_temporal_type(to_type)) {
    MYSQL_TIME ltime;
    bool res = true;
    switch (to_type) {
      case MYSQL_TYPE_TIME:
        res = from->get_time(&ltime);
        break;
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_NEWDATE:
        res = from->get_date(&ltime, 0);
        break;
      default:
        DBUG_ASSERT(0);
    }
    if (res) return TYPE_ERR_BAD_VALUE;
    return to->store_time(&ltime);
  } else if ((from->result_type() == STRING_RESULT &&
              (to->result_type() == STRING_RESULT ||
               (from->real_type() != MYSQL_TYPE_ENUM &&
                from->real_type() != MYSQL_TYPE_SET))) ||
             to_type == MYSQL_TYPE_DECIMAL) {
    char buff[MAX_FIELD_WIDTH];
    String result(buff, sizeof(buff), from->charset());
    from->val_str(&result);
    /*
      We use c_ptr_quick() here to make it easier if to is a float/double
      as the conversion routines will do a copy of the result doesn't
      end with \0. Can be replaced with .ptr() when we have our own
      string->double conversion.
    */
    return to->store(result.c_ptr_quick(), result.length(), from->charset());
  } else if (from->result_type() == REAL_RESULT)
    return to->store(from->val_real());
  else if (from->result_type() == DECIMAL_RESULT) {
    my_decimal buff;
    return to->store_decimal(from->val_decimal(&buff));
  } else
    return to->store(from->val_int(), from->flags & UNSIGNED_FLAG);
}
