/* Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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
/**
  @file table_id.h

  @brief Contains the class Table_id, mainly used for row based replication.
*/

#ifndef TABLE_ID_INCLUDED
#define TABLE_ID_INCLUDED
#include <stdint.h>
#include "wrapper_functions.h"

/**
  @class Table_id

  @brief Each table share has a table id, it is mainly used for row based
  replication. Meanwhile it is used as table's version too.
*/
class Table_id {
 private:
  /* In table map event and rows events, table id is 6 bytes.*/
  static const unsigned long long TABLE_ID_MAX = (~0ULL >> 16);
  uint64_t m_id;

 public:
  Table_id() : m_id(0) {}
  explicit Table_id(unsigned long long id) : m_id(id) {}

  unsigned long long id() const { return m_id; }
  bool is_valid() const { return m_id <= TABLE_ID_MAX; }

  Table_id &operator=(unsigned long long id) {
    m_id = id;
    return *this;
  }

  bool operator==(const Table_id &tid) const { return m_id == tid.m_id; }
  bool operator!=(const Table_id &tid) const { return m_id != tid.m_id; }

  /* Support implicit type converting from Table_id to unsigned long long */
  operator unsigned long long() const { return m_id; }

  Table_id operator++(int) {
    Table_id id(m_id);

    /* m_id is reset to 0, when it exceeds the max value. */
    m_id = (m_id == TABLE_ID_MAX ? 0 : m_id + 1);

    BAPI_ASSERT(m_id <= TABLE_ID_MAX);
    return id;
  }
};

#endif
