/* Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef FILESORT_INCLUDED
#define FILESORT_INCLUDED

#include <stddef.h>
#include <sys/types.h>

#include "my_base.h" /* ha_rows */
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/sort_param.h"

class Addon_fields;
class Field;
class JOIN;
class RowIterator;
class Sort_result;
class THD;
struct ORDER;
struct TABLE;
struct st_sort_field;

enum class Addon_fields_status;

/**
  Sorting related info.
*/
class Filesort {
 public:
  THD *m_thd;
  /// The table we are sorting.
  TABLE *const table;
  /// If true, do not free the filesort buffers (use if you expect to sort many
  /// times, like in an uncacheable subquery).
  const bool keep_buffers;
  /// Maximum number of rows to return
  ha_rows limit;
  /// ORDER BY list with some precalculated info for filesort
  st_sort_field *sortorder;
  /// true means we are using Priority Queue for order by with limit.
  bool using_pq;
  /// true means force stable sorting
  bool m_force_stable_sort;
  bool m_remove_duplicates;
  // If true, we will always sort references to rows on table (and crucially,
  // the result iterators used will always position the underlying table on
  // the original row before returning from Read()).
  bool m_force_sort_positions;
  // TODO: Consider moving this into private members of Filesort.
  Sort_param m_sort_param;

  Filesort(THD *thd, TABLE *table, bool keep_buffers, ORDER *order,
           ha_rows limit_arg, bool force_stable_sort, bool remove_duplicates,
           bool force_sort_positions);

  Addon_fields *get_addon_fields(TABLE *table,
                                 Addon_fields_status *addon_fields_status,
                                 uint *plength, uint *ppackable_length);

  // Number of elements in the sortorder array.
  uint sort_order_length() const { return m_sort_order_length; }

  /// Whether we are using addon fields (sort entire rows) or not (sort row
  /// IDs). Note that on the first call, this actually makes Sort_param
  /// compute the decision and cache it, so it cannot be called before the sort
  /// order is properly set up.
  bool using_addon_fields();

 private:
  /* Prepare ORDER BY list for sorting. */
  uint make_sortorder(ORDER *order);

  uint m_sort_order_length;
};

bool filesort(THD *thd, Filesort *fsort, RowIterator *source_iterator,
              Filesort_info *fs_info, Sort_result *sort_result,
              ha_rows *found_rows);
void filesort_free_buffers(TABLE *table, bool full);
void change_double_for_sort(double nr, uchar *to);

/// Declared here so we can unit test it.
uint sortlength(THD *thd, st_sort_field *sortorder, uint s_length);

/// Declared here for Item_func_weight_string.
longlong get_int_sort_key_for_item(Item *item);

// Avoid pulling in sql/field.h.
template <bool Is_big_endian>
void copy_integer(uchar *to, size_t to_length, const uchar *from,
                  size_t from_length, bool is_unsigned);

static inline void copy_native_longlong(uchar *to, size_t to_length,
                                        longlong val, bool is_unsigned) {
#ifdef WORDS_BIGENDIAN
  constexpr bool Is_big_endian = true;
#else
  constexpr bool Is_big_endian = false;
#endif
  copy_integer<Is_big_endian>(to, to_length,
                              static_cast<uchar *>(static_cast<void *>(&val)),
                              sizeof(longlong), is_unsigned);
}

#endif /* FILESORT_INCLUDED */
