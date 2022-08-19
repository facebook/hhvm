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

#include "sql/dd/impl/raw/raw_table.h"

#include <stddef.h>
#include <algorithm>
#include <new>

#include "m_string.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/udf_registration_types.h"
#include "sql/dd/impl/object_key.h"          // dd::Object_key
#include "sql/dd/impl/raw/raw_key.h"         // dd::Raw_key
#include "sql/dd/impl/raw/raw_record.h"      // dd::Raw_record
#include "sql/dd/impl/raw/raw_record_set.h"  // dd::Raw_record_set
#include "sql/handler.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

Raw_table::Raw_table(thr_lock_type lock_type, const String_type &name)
    : m_table_list(STRING_WITH_LEN("mysql"), name.c_str(), name.length(),
                   name.c_str(), lock_type) {
  m_table_list.is_dd_ctx_table = true;
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
  Find record and populate raw_record.

  @param key - Pointer to Raw_record after moving to next row.
  @param r - row on which data is updated.

  @return false - On success. 1) We found a row.
                              2) OR Either we don't have any matching rows
  @return true - On failure and error is reported.
*/
bool Raw_table::find_record(const Object_key &key,
                            std::unique_ptr<Raw_record> &r) {
  DBUG_TRACE;

  TABLE *table = get_table();
  std::unique_ptr<Raw_key> k(key.create_access_key(this));

  int rc;
  if (!table->file->inited &&
      (rc = table->file->ha_index_init(k->index_no, true))) {
    table->file->print_error(rc, MYF(0));
    return true;
  }

  rc = table->file->ha_index_read_idx_map(
      table->record[0], k->index_no, k->key, k->keypart_map,
      (k->keypart_map == HA_WHOLE_KEY) ? HA_READ_KEY_EXACT : HA_READ_PREFIX);

  if (table->file->inited)
    table->file->ha_index_end();  // Close the scan over the index

  // Row not found.
  if (rc == HA_ERR_KEY_NOT_FOUND || rc == HA_ERR_END_OF_FILE) {
    r.reset(nullptr);
    return false;
  }

  // Got unexpected error.
  if (rc) {
    table->file->print_error(rc, MYF(0));
    r.reset(nullptr);
    return true;
  }

  r.reset(new Raw_record(table));
  return false;
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
  Write modified data into row buffer.

  @param key - Pointer to Raw_record after moving to next row.
  @param r - row on which data is updated.

  @return false - On success.
  @return true - On failure and error is reported.
*/
bool Raw_table::prepare_record_for_update(const Object_key &key,
                                          std::unique_ptr<Raw_record> &r) {
  DBUG_TRACE;

  TABLE *table = get_table();

  // Setup row buffer for update
  table->use_all_columns();
  bitmap_set_all(table->write_set);
  bitmap_set_all(table->read_set);

  if (find_record(key, r)) return true;

  store_record(table, record[1]);

  return false;
}

///////////////////////////////////////////////////////////////////////////

Raw_new_record *Raw_table::prepare_record_for_insert() {
  return new (std::nothrow) Raw_new_record(get_table());
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
   Initiate table scan operation for the given key.

  @return false - on success.
  @return true - on failure and error is reported.
*/
bool Raw_table::open_record_set(const Object_key *key,
                                std::unique_ptr<Raw_record_set> &rs) {
  DBUG_TRACE;

  Raw_key *access_key = nullptr;

  // Create specific access key if submitted.
  if (key) {
    restore_record(get_table(), s->default_values);
    access_key = key->create_access_key(this);
  }

  std::unique_ptr<Raw_record_set> rs1(
      new (std::nothrow) Raw_record_set(get_table(), access_key));

  if (rs1->open()) return true;

  rs = std::move(rs1);

  return false;
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
  Find last record in table and populate raw_record.

  @param key - Pointer to Raw_record after moving to next row.
  @param r - row on which data is updated.

  @return false - On success. 1) We found a row.
                              2) OR Either we don't have any matching rows
  @return true - On failure and error is reported.
*/
/* purecov: begin deadcode */
bool Raw_table::find_last_record(const Object_key &key,
                                 std::unique_ptr<Raw_record> &r) {
  DBUG_TRACE;

  TABLE *table = get_table();
  std::unique_ptr<Raw_key> k(key.create_access_key(this));

  int rc;
  if (!table->file->inited &&
      (rc = table->file->ha_index_init(k->index_no, true))) {
    table->file->print_error(rc, MYF(0));
    return true;
  }

  rc = table->file->ha_index_read_idx_map(table->record[0], k->index_no, k->key,
                                          k->keypart_map,
                                          HA_READ_PREFIX_LAST_OR_PREV);

  if (table->file->inited)
    table->file->ha_index_end();  // Close the scan over the index

  // Row not found.
  if (rc == HA_ERR_KEY_NOT_FOUND || rc == HA_ERR_END_OF_FILE) {
    r.reset(nullptr);
    return false;
  }

  // Got unexpected error.
  if (rc) {
    table->file->print_error(rc, MYF(0));
    r.reset(nullptr);
    return true;
  }

  r.reset(new Raw_record(table));

  return false;
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
