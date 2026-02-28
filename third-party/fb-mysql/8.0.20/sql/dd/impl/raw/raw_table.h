/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__RAW_TABLE_INCLUDED
#define DD__RAW_TABLE_INCLUDED

#include <memory>

#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/table.h"           // TABLE_LIST
#include "thr_lock.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_key;
class Raw_new_record;
class Raw_record;
class Raw_record_set;

///////////////////////////////////////////////////////////////////////////

class Raw_table {
 public:
  Raw_table(thr_lock_type lock_type, const String_type &name);

  virtual ~Raw_table() {}

 public:
  TABLE *get_table() { return m_table_list.table; }

  TABLE_LIST *get_table_list() { return &m_table_list; }

 public:
  bool find_record(const Object_key &key, std::unique_ptr<Raw_record> &r);

  bool find_last_record(const Object_key &key, std::unique_ptr<Raw_record> &r);

  bool prepare_record_for_update(const Object_key &key,
                                 std::unique_ptr<Raw_record> &r);

  Raw_new_record *prepare_record_for_insert();

 public:
  bool open_record_set(const Object_key *key,
                       std::unique_ptr<Raw_record_set> &rs);

 protected:
  TABLE_LIST m_table_list;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__RAW_TABLE_INCLUDED
