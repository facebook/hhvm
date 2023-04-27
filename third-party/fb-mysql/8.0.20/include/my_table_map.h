/*
   Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_TABLE_MAP_INCLUDED
#define MY_TABLE_MAP_INCLUDED

#include "my_dbug.h"
#include "my_inttypes.h"

using table_map = uint64_t;    // Used for table bits in join.
using nesting_map = uint64_t;  // Used for flags of nesting constructs.
using qep_tab_map = uint64_t;  // Used for indexing QEP_TABs in a JOIN.

// Returns a bitmap representing the semi-open interval [start, end).
static inline uint64_t TablesBetween(unsigned start, unsigned end) {
  DBUG_ASSERT(end >= start);
  return (uint64_t{1} << end) - (uint64_t{1} << start);
}

// Test whether "map" contains the given table.
static inline bool ContainsTable(uint64_t map, unsigned idx) {
  return map & (uint64_t{1} << idx);
}

#endif  // MY_TABLE_MAP_INCLUDED
