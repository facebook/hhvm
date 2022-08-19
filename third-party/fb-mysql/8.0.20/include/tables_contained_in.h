/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TABLES_CONTAINED_IN_H_
#define TABLES_CONTAINED_IN_H_

#include <string.h>

#include "my_inttypes.h"
#include "sql/sql_optimizer.h"

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_BitScanForward64)
#endif

// A utility class to loop over all set bits in the given map.
// Use as:
//
//   qep_tab_map = ...;
//   for (QEP_TAB *qep_tab : TablesContainedIn(join, qep_tab_map)) {
//      ...
//   }
class TablesContainedIn {
 public:
  class iterator {
   private:
    const JOIN *const m_join;
    qep_tab_map m_bits_left;

   public:
    iterator(const JOIN *join, qep_tab_map map)
        : m_join(join), m_bits_left(map) {}
    bool operator==(const iterator &other) const {
      DBUG_ASSERT(m_join == other.m_join);
      return m_bits_left == other.m_bits_left;
    }
    bool operator!=(const iterator &other) const {
      DBUG_ASSERT(m_join == other.m_join);
      return m_bits_left != other.m_bits_left;
    }
    QEP_TAB *operator*() const {
      // Find the QEP_TAB that corresponds to the lowest set bit.
      DBUG_ASSERT(m_bits_left != 0);
#ifdef _MSC_VER
      unsigned long idx;
      _BitScanForward64(&idx, m_bits_left);
#else
      size_t idx = ffsll(m_bits_left) - 1;
#endif
      DBUG_ASSERT(idx < m_join->tables);
      return &m_join->qep_tab[idx];
    }
    iterator &operator++() {
      // Clear the lowest set bit.
      DBUG_ASSERT(m_bits_left != 0);
      m_bits_left &= (m_bits_left - 1);
      return *this;
    }
  };

  TablesContainedIn(const JOIN *join, qep_tab_map map)
      : m_join(join), m_initial_map(map) {}

  iterator begin() const { return {m_join, m_initial_map}; }
  iterator end() const { return {m_join, 0}; }

 private:
  const JOIN *const m_join;
  const qep_tab_map m_initial_map;
};

#endif  // TABLES_CONTAINED_IN_H_
