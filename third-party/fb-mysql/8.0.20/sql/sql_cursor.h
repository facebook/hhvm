/* Copyright (c) 2005, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _sql_cursor_h_
#define _sql_cursor_h_

#include <stddef.h>
#include <sys/types.h>
#include <new>

#include "sql/sql_class.h" /* Query_arena */

class JOIN;
class Query_result;
struct MEM_ROOT;

/**
  @file

  Declarations for implementation of server side cursors. Only
  read-only non-scrollable cursors are currently implemented.
*/

/**
  Server_side_cursor -- an interface for materialized
  implementation of cursors. All cursors are self-contained
  (created in their own memory root).  For that reason they must
  be deleted only using a pointer to Server_side_cursor, not to
  its base class.
*/

class Server_side_cursor {
 protected:
  Query_arena m_arena;
  /** Row destination used for fetch */
  Query_result *result;

 public:
  Server_side_cursor(MEM_ROOT *mem_root_arg, Query_result *result_arg)
      : m_arena(mem_root_arg, Query_arena::STMT_INITIALIZED),
        result(result_arg) {}

  virtual bool is_open() const = 0;

  virtual int open(THD *thd, JOIN *top_level_join) = 0;
  virtual bool fetch(ulong num_rows) = 0;
  virtual void close() = 0;
  virtual ~Server_side_cursor();

  static void operator delete(void *ptr, size_t size);
  static void operator delete(
      void *, MEM_ROOT *, const std::nothrow_t &)noexcept { /* never called */
  }
};

bool mysql_open_cursor(THD *thd, Query_result *result,
                       Server_side_cursor **res);

#endif /* _sql_cusor_h_ */
