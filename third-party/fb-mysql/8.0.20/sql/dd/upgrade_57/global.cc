/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/upgrade_57/global.h"

#include <stdarg.h>
#include <stdio.h>

#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "sql/handler.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "sql/table.h"

namespace dd {
namespace upgrade_57 {

const char *TRN_EXT = ".TRN";
const char *TRG_EXT = ".TRG";

Thd_mem_root_guard::Thd_mem_root_guard(THD *thd, MEM_ROOT *mem_root) {
  m_thd = thd;
  m_thd_prev_mem_root = m_thd->mem_root;
  m_thd->mem_root = mem_root;
}

Thd_mem_root_guard::~Thd_mem_root_guard() {
  m_thd->mem_root = m_thd_prev_mem_root;
}

System_table_close_guard::System_table_close_guard(THD *thd, TABLE *table)
    : m_thd(thd), m_table(table) {}

System_table_close_guard::~System_table_close_guard() {
  if (m_table->file->inited) (void)m_table->file->ha_index_end();
  close_thread_tables(m_thd);
}

void Check_table_intact::report_error(uint, const char *fmt, ...) {
  va_list args;
  char buff[MYSQL_ERRMSG_SIZE];
  va_start(args, fmt);
  vsnprintf(buff, sizeof(buff), fmt, args);
  va_end(args);

  LogErr(ERROR_LEVEL, ER_DD_UPGRADE_TABLE_INTACT_ERROR, buff);
}

}  // namespace upgrade_57
}  // namespace dd
