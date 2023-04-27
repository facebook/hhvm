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

#ifndef ACL_TABLE_BASE_INCLUDED
#define ACL_TABLE_BASE_INCLUDED

#include "my_base.h"
#include "my_dbug.h"

class THD;
struct TABLE;

namespace acl_table {

typedef int Table_op_error_code;

enum class Acl_table_op_status { OP_ERROR_CRITICAL = 0, OP_OK, OP_ERROR_ROW };
enum class Acl_table_operation { OP_INSERT = 0, OP_UPDATE, OP_DELETE, OP_READ };

/**
  Base class to handle ACL table manipulation
*/

class Acl_table {
 public:
  /* Constructor & Destructor */
  Acl_table(THD *thd, TABLE *table, Acl_table_operation operation)
      : m_thd(thd), m_table(table), m_operation(operation), m_error(0) {
    DBUG_ASSERT(m_table);
  }
  virtual ~Acl_table() {}

  /* Don't allow copy */
  Acl_table(const Acl_table &) = delete;
  const Acl_table &operator=(const Acl_table &) = delete;
  Acl_table(const Acl_table &&) = delete;
  const Acl_table &operator=(const Acl_table &&) = delete;

  /* Finish a reade/write operation on given table */
  virtual Acl_table_op_status finish_operation(Table_op_error_code &error) = 0;
  Acl_table_operation get_operation_mode() { return m_operation; }

 protected:
  Acl_table_op_status convert_table_op_error_code() {
    if (!m_error) return Acl_table_op_status::OP_OK;

    if (m_error == HA_ERR_KEY_NOT_FOUND || m_error == HA_ERR_END_OF_FILE)
      return Acl_table_op_status::OP_ERROR_ROW;

    return Acl_table_op_status::OP_ERROR_CRITICAL;
  }

  /* Thread handle */
  THD *m_thd;
  /* Table handle */
  TABLE *m_table;
  /* Mode - INSERT/UPDATE/DELETE/READ */
  Acl_table_operation m_operation;
  /* Table operation status */
  Table_op_error_code m_error;
};

}  // namespace acl_table

#endif /* ACL_TABLE_BASE_INCLUDED */
