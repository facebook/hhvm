/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__RAW_RECORD_SET_INCLUDED
#define DD__RAW_RECORD_SET_INCLUDED

#include <stddef.h>

#include "sql/dd/impl/raw/raw_record.h"  // dd::Raw_record

struct TABLE;

namespace dd {

///////////////////////////////////////////////////////////////////////////

struct Raw_key;

///////////////////////////////////////////////////////////////////////////

class Raw_record_set : private Raw_record {
 public:
  ~Raw_record_set();

  Raw_record *current_record() { return m_current_record; }

  bool next(Raw_record *&r);

 private:
  // Note: The 'key' supplied will be freed by Raw_record_set
  Raw_record_set(TABLE *table, Raw_key *key)
      : Raw_record(table), m_key(key), m_current_record(nullptr) {}

  bool open();

  friend class Raw_table;

 private:
  // Raw_record_set owns m_key.
  Raw_key *m_key;

  Raw_record *m_current_record;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__RAW_RECORD_SET_INCLUDED
