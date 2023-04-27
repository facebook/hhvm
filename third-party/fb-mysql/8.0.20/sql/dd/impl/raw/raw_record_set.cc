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

#include "sql/dd/impl/raw/raw_record_set.h"

#include <sys/types.h>

#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/udf_registration_types.h"
#include "sql/dd/impl/raw/raw_key.h"  // dd::Raw_key
#include "sql/handler.h"
#include "sql/table.h"  // TABLE

namespace dd {

///////////////////////////////////////////////////////////////////////////

/**
  @brief
    Initializes the table scan operation.
    If there is no key supplied, then we do a sorted index full scan.

  @return true - on failure and error is reported.
  @return false - on success.
*/
bool Raw_record_set::open() {
  DBUG_TRACE;
  uint index_no = 0;

  // Use specific index if key submitted.
  if (m_key) index_no = m_key->index_no;

  int rc = m_table->file->ha_index_init(index_no, true);

  if (rc) {
    m_table->file->print_error(rc, MYF(0));
    return true;
  }

  if (m_key)
    rc = m_table->file->ha_index_read_idx_map(
        m_table->record[0], m_key->index_no, m_key->key, m_key->keypart_map,
        HA_READ_KEY_EXACT);
  else
    rc = m_table->file->ha_index_first(m_table->record[0]);

  // Row not found.
  if (rc == HA_ERR_KEY_NOT_FOUND || rc == HA_ERR_END_OF_FILE) {
    DBUG_ASSERT(!m_current_record);
    return false;
  }

  // Got unexpected error.
  if (rc) {
    m_table->file->print_error(rc, MYF(0));
    return true;
  }

  m_current_record = this;

  return false;
}

///////////////////////////////////////////////////////////////////////////

/**
  @brief
  Move to next record in DD table that matches the supplied key.
  If there is no key supplied, then we do a sorted index full scan.

  @param r - Pointer to Raw_record after moving to next row.


  @return false - On success. 1) We found a row.
                              2) OR Either we don't have any matching rows
  @return true - On failure and my_error() is invoked.
*/
bool Raw_record_set::next(Raw_record *&r) {
  DBUG_TRACE;
  int rc;

  if (!m_current_record) {
    m_current_record = nullptr;
    r = nullptr;
    return false;
  }

  if (m_key)
    rc = m_table->file->ha_index_next_same(m_table->record[0], m_key->key,
                                           m_key->key_len);
  else
    rc = m_table->file->ha_index_next(m_table->record[0]);

  // Row not found.
  if (rc == HA_ERR_KEY_NOT_FOUND || rc == HA_ERR_END_OF_FILE) {
    m_current_record = nullptr;
    r = nullptr;
    return false;
  }

  // Got unexpected error.
  if (rc) {
    m_table->file->print_error(rc, MYF(0));
    m_current_record = nullptr;
    r = nullptr;
    return true;
  }

  m_current_record = this;
  r = this;

  return false;
}

///////////////////////////////////////////////////////////////////////////

Raw_record_set::~Raw_record_set() {
  if (m_table->file->inited != handler::NONE) {
    int rc = m_table->file->ha_index_end();

    if (rc) {
      /* purecov: begin inspected */
      m_table->file->print_error(rc, MYF(ME_ERRORLOG));
      DBUG_ASSERT(false);
      /* purecov: end */
    }
  }

  delete m_key;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
