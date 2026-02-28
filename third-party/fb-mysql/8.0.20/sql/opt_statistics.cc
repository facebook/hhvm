/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/opt_statistics.h"

#include <algorithm>

#include "my_base.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "sql/handler.h"
#include "sql/key.h"    // rec_per_key_t, KEY
#include "sql/table.h"  // TABLE

using std::max;

/**
  This code for computing a guestimate for records per key is based on
  code in Optimize_table_order::find_best_ref().

  Assume that the first key part matches 1% of the file and that the
  whole key matches 10 (duplicates) or 1 (unique) records. For small
  tables, ensure there are at least ten different key values.  Assume
  also that more key matches proportionally more records. This gives
  the formula:

    records = a - (x-1)/(c-1)*(a-b)

  where

    b = records matched by whole key
    a = records matched by first key part (1% of all records?)
    c = number of key parts in key
    x = used key parts (1 <= x <= c)

  @todo Change Optimize_table_order::find_best_ref() to use this function.
*/

rec_per_key_t guess_rec_per_key(const TABLE *const table, const KEY *const key,
                                uint used_keyparts) {
  DBUG_ASSERT(used_keyparts >= 1);
  DBUG_ASSERT(used_keyparts <= key->actual_key_parts);
  DBUG_ASSERT(!key->has_records_per_key(used_keyparts - 1));

  const ha_rows table_rows = table->file->stats.records;

  /*
    Make an estimates for how many records the whole key will match.
    If there exists index statistics for the whole key we use this.
    If not, we assume the whole key matches ten records for a non-unique
    index and 1 record for a unique index.
  */
  rec_per_key_t rec_per_key_all;
  if (key->has_records_per_key(key->user_defined_key_parts - 1))
    rec_per_key_all = key->records_per_key(key->user_defined_key_parts - 1);
  else {
    if (key->actual_flags & HA_NOSAME)
      rec_per_key_all = 1.0f;  // Unique index
    else {
      rec_per_key_all = 10.0f;  // Non-unique index

      /*
        Assume the index contains at least ten unique values. Need to
        adjust the records per key estimate for small tables. For an
        empty table we assume records per key is 1.
      */
      rec_per_key_all =
          std::min(rec_per_key_all, max(rec_per_key_t(table_rows) / 10, 1.0f));
    }
  }

  rec_per_key_t rec_per_key;

  // rec_per_key estimate for first key part (1% of records)
  const rec_per_key_t rec_per_key_first = table_rows * 0.01f;

  if (rec_per_key_first < rec_per_key_all) {
    rec_per_key = rec_per_key_all;
  } else {
    if (key->user_defined_key_parts > 1) {
      // See formula above
      rec_per_key =
          rec_per_key_first - (rec_per_key_t(used_keyparts - 1) /
                               (key->user_defined_key_parts - 1)) *
                                  (rec_per_key_first - rec_per_key_all);
    } else {
      // Single column index
      if (key->actual_flags & HA_NOSAME)
        rec_per_key = 1.0f;  // Unique index
      else
        rec_per_key = rec_per_key_first;  // Non-unique index
    }

    DBUG_ASSERT(rec_per_key >= rec_per_key_all);
  }

  return rec_per_key;
}
