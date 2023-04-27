#ifndef SQL_RECORDS_H
#define SQL_RECORDS_H
/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <sys/types.h>
#include <memory>
#include <string>

#include "my_alloc.h"
#include "my_base.h"
#include "sql/basic_row_iterators.h"
#include "sql/composite_iterators.h"
#include "sql/ref_row_iterators.h"
#include "sql/row_iterator.h"
#include "sql/sorting_iterator.h"

class QEP_TAB;
class THD;
struct TABLE;

unique_ptr_destroy_only<RowIterator> create_table_iterator(
    THD *thd, TABLE *table, QEP_TAB *qep_tab, bool disable_rr_cache,
    bool ignore_not_found_rows, ha_rows *examined_rows, bool *using_table_scan);

/**
  Calls create_table_iterator(), then calls Init() on the resulting iterator.
  Returns nullptr on failure.
 */
unique_ptr_destroy_only<RowIterator> init_table_iterator(
    THD *thd, TABLE *table, QEP_TAB *qep_tab, bool disable_rr_cache,
    bool ignore_not_found_rows);

unique_ptr_destroy_only<RowIterator> create_table_iterator_idx(
    THD *thd, TABLE *table, uint idx, bool reverse, QEP_TAB *qep_tab);

#endif /* SQL_RECORDS_H */
