/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/raw/raw_record.h"

#include <stddef.h>

#include "m_ctype.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_time.h"
#include "mysql/udf_registration_types.h"
#include "sql/dd/properties.h"  // dd::Properties
#include "sql/field.h"          // Field
#include "sql/handler.h"
#include "sql/my_decimal.h"
#include "sql/sql_const.h"
#include "sql/table.h"   // TABLE
#include "sql/tztime.h"  // Time_zone_offset
#include "sql_string.h"
#include "template_utils.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

Raw_record::Raw_record(TABLE *table) : m_table(table) {
  bitmap_set_all(m_table->read_set);
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
    Update table record into SE

  @return true - on failure and error is reported.
  @return false - on success.
*/
bool Raw_record::update() {
  DBUG_TRACE;

  int rc = m_table->file->ha_update_row(m_table->record[1], m_table->record[0]);

  /**
    We ignore HA_ERR_RECORD_IS_THE_SAME here for following reason.
    If in case we are updating childrens of some DD object,
    and only one of the children has really changed and other have
    not. Then we get HA_ERR_RECORD_IS_THE_SAME for children (rows)
    which has not really been modified.

    Currently DD framework creates/updates *all* childrens at once
    and we don't have machinism to update only required child.
    May be this is part of task which will implement inplace
    alter in better way, updating only the changed child (or row)
    and ignore others. Then we can remove the below check which
    ignores HA_ERR_RECORD_IS_THE_SAME.
  */

  if (rc && rc != HA_ERR_RECORD_IS_THE_SAME) {
    m_table->file->print_error(rc, MYF(0));
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
    Drop the record from SE

  @return true - on failure and error is reported.
  @return false - on success.
*/
bool Raw_record::drop() {
  DBUG_TRACE;

  int rc = m_table->file->ha_delete_row(m_table->record[1]);

  if (rc) {
    m_table->file->print_error(rc, MYF(0));
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

Field *Raw_record::field(int field_no) const {
  return m_table->field[field_no];
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store_pk_id(int field_no, Object_id id) {
  field(field_no)->set_notnull();

  return (id == INVALID_OBJECT_ID) ? false : store(field_no, id);
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store_ref_id(int field_no, Object_id id) {
  if (id == INVALID_OBJECT_ID) {
    set_null(field_no, true);
    return false;
  }

  set_null(field_no, false);
  type_conversion_status rc = field(field_no)->store(id, true);

  DBUG_ASSERT(rc == TYPE_OK);
  return rc != TYPE_OK;
}

///////////////////////////////////////////////////////////////////////////

void Raw_record::set_null(int field_no, bool is_null) {
  if (is_null)
    field(field_no)->set_null();
  else
    field(field_no)->set_notnull();
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store(int field_no, const String_type &s, bool is_null) {
  set_null(field_no, is_null);

  if (is_null) return false;

  type_conversion_status rc =
      field(field_no)->store(s.c_str(), s.length(), system_charset_info);

  DBUG_ASSERT(rc == TYPE_OK);
  return rc != TYPE_OK;
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store(int field_no, ulonglong ull, bool is_null) {
  set_null(field_no, is_null);

  if (is_null) return false;

  type_conversion_status rc = field(field_no)->store(ull, true);

  DBUG_ASSERT(rc == TYPE_OK);
  return rc != TYPE_OK;
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store(int field_no, longlong ll, bool is_null) {
  set_null(field_no, is_null);

  if (is_null) return false;

  type_conversion_status rc = field(field_no)->store(ll, false);

  DBUG_ASSERT(rc == TYPE_OK);
  return rc != TYPE_OK;
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store(int field_no, const Properties &p) {
  return store(field_no, p.raw_string(), p.empty());
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store_time(int field_no, my_time_t val, bool is_null) {
  set_null(field_no, is_null);

  if (is_null) return false;

  MYSQL_TIME time;

  my_tz_OFFSET0->gmt_sec_to_TIME(&time, val);
  return field(field_no)->store_time(&time);
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store_timestamp(int field_no, const timeval &tv) {
  field(field_no)->store_timestamp(&tv);
  return false;
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::store_json(int field_no, const Json_wrapper &json) {
  Field_json *json_field = down_cast<Field_json *>(field(field_no));
  return json_field->store_json(&json) != TYPE_OK;
}

///////////////////////////////////////////////////////////////////////////

bool Raw_record::is_null(int field_no) const {
  return field(field_no)->is_null();
}

///////////////////////////////////////////////////////////////////////////

longlong Raw_record::read_int(int field_no) const {
  return field(field_no)->val_int();
}

///////////////////////////////////////////////////////////////////////////

ulonglong Raw_record::read_uint(int field_no) const {
  return static_cast<ulonglong>(field(field_no)->val_int());
}

///////////////////////////////////////////////////////////////////////////

String_type Raw_record::read_str(int field_no) const {
  char buff[MAX_FIELD_WIDTH];
  String val(buff, sizeof(buff), &my_charset_bin);

  field(field_no)->val_str(&val);

  return String_type(val.ptr(), val.length());
}

///////////////////////////////////////////////////////////////////////////

Object_id Raw_record::read_ref_id(int field_no) const {
  return field(field_no)->is_null() ? dd::INVALID_OBJECT_ID
                                    : read_int(field_no);
}

///////////////////////////////////////////////////////////////////////////

my_time_t Raw_record::read_time(int field_no) const {
  MYSQL_TIME time;
  bool not_used;

  field(field_no)->get_date(&time, TIME_DATETIME_ONLY);
  return my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);
}

///////////////////////////////////////////////////////////////////////////

timeval Raw_record::read_timestamp(int field_no) const {
  int warnings = 0;
  timeval tv;
  if (field(field_no)->get_timestamp(&tv, &warnings)) {
    DBUG_ASSERT(false);
    return {0, 0};
  }
  return tv;
}

////////////////////////////////////////////////////////////////////////////

bool Raw_record::read_json(int field_no, Json_wrapper *json_wrapper) const {
  return down_cast<Field_json *>(field(field_no))->val_json(json_wrapper);
}

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////

Raw_new_record::Raw_new_record(TABLE *table) : Raw_record(table) {
  bitmap_set_all(m_table->write_set);
  bitmap_set_all(m_table->read_set);

  m_table->next_number_field = m_table->found_next_number_field;
  m_table->autoinc_field_has_explicit_non_null_value = true;

  restore_record(m_table, s->default_values);
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
    Create new record in SE

  @return true - on failure and error is reported.
  @return false - on success.
*/
bool Raw_new_record::insert() {
  DBUG_TRACE;

  int rc = m_table->file->ha_write_row(m_table->record[0]);

  if (rc) {
    m_table->file->print_error(rc, MYF(0));
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////

Object_id Raw_new_record::get_insert_id() const {
  Object_id id = m_table->file->insert_id_for_cur_row;

  // Objects without primary key should have still get INVALID_OBJECT_ID.
  return id ? id : dd::INVALID_OBJECT_ID;
}

///////////////////////////////////////////////////////////////////////////

void Raw_new_record::finalize() {
  if (!m_table) return;

  m_table->autoinc_field_has_explicit_non_null_value = false;
  m_table->file->ha_release_auto_increment();
  m_table->next_number_field = nullptr;

  m_table = nullptr;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
