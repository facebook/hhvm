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

#include "sql/opt_explain_format.h"

#include "m_ctype.h"
#include "my_dbug.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"

bool qep_row::mem_root_str::is_empty() {
  if (deferred) {
    StringBuffer<128> buff(system_charset_info);
    if (deferred->eval(&buff) || set(buff)) {
      DBUG_ASSERT(!"OOM!");
      return true;  // ignore OOM
    }
    deferred = nullptr;  // prevent double evaluation, if any
  }
  return str == nullptr;
}

bool qep_row::mem_root_str::set(const char *str_arg, size_t length_arg) {
  deferred = nullptr;
  if (!(str = strndup_root(current_thd->mem_root, str_arg, length_arg)))
    return true; /* purecov: inspected */
  length = length_arg;
  return false;
}
