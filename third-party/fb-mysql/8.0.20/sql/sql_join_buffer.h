#ifndef SQL_JOIN_CACHE_INCLUDED
#define SQL_JOIN_CACHE_INCLUDED

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

class QEP_TAB;
struct MEM_ROOT;
struct MY_BITMAP;
struct TABLE;

/**
  Filter the base columns of virtual generated columns if using a covering index
  scan.

  Adjust table->read_set so that it only contains the columns that are needed in
  the join operation and afterwards.

  For a virtual generated column, all base columns are added to the read_set
  of the table. The storage engine will then copy all base column values so
  that the value of the GC can be calculated inside the executor.
  But when a virtual GC is fetched using a covering index, the actual GC
  value is fetched by the storage engine and the base column values are not
  needed. Code that looks at the read sets (join buffering, hash join, filesort)
  must not try to copy them.
  So, we eliminate from read_set those columns that are available from the
  covering index.

  Note that some iterators (DynamicRangeIterator and AlternativeIterator)
  may need to switch back to table scans. If so, they will adjust the table's
  read set before reading; see add_virtual_gcol_base_cols().
*/
void filter_virtual_gcol_base_cols(const QEP_TAB *qep_tab);

/**
  Create a read set that undoes the work of filter_virtual_gcol_base_cols();
  ie., for every virtual generated column that is part of the given table's read
  set, we also include the base tables. This is needed if we revert from
  a covering index back to a table scan (which doesn't store the virtual
  generated columns themselves, unlike the covering index). This happens in
  DynamicRangeIterator and AlternativeIterator.

  The new read set gets a new buffer, allocated on the given MEM_ROOT.
  table->read_set is not changed.
 */
void add_virtual_gcol_base_cols(TABLE *table, MEM_ROOT *mem_root,
                                MY_BITMAP *completed_read_set);

class JOIN_CACHE {
 public:
  /** Bits describing cache's type @sa setup_join_buffering() */
  enum enum_join_cache_type { ALG_NONE = 0, ALG_BNL = 1, ALG_BKA = 2 };
};

#endif /* SQL_JOIN_CACHE_INCLUDED */
