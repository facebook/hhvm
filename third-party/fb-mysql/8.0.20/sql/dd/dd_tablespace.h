/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_TABLESPACE_INCLUDED
#define DD_TABLESPACE_INCLUDED

#include "sql/lock.h"  // Tablespace_hash_set
#include "sql/thr_malloc.h"

struct MEM_ROOT;
class THD;
class st_alter_tablespace;
struct handlerton;

namespace dd {

/**
  Fill Tablespace_hash_set with tablespace names used by the
  given db_name.table_name.

  @param thd            - Thread invoking the function.
  @param db_name        - Database name.
  @param table_name     - Table name.
  @param tablespace_set - (OUT) hash set where tablespace names
                          are filled.

  @return true - On failure.
  @return false - On success.
*/
bool fill_table_and_parts_tablespace_names(THD *thd, const char *db_name,
                                           const char *table_name,
                                           Tablespace_hash_set *tablespace_set);

/**
  Read tablespace name of a tablespace_id.

  @param thd                   - Thread invoking this call.
  @param obj                   - Table/Partition object whose
                                 tablespace name is being read.
  @param tablespace_name (OUT) - Tablespace name of table.
  @param mem_root              - Memroot where tablespace name
                                 should be stored.

  @return true  - On failure.
  @return false - On success.
*/
template <typename T>
bool get_tablespace_name(THD *thd, const T *obj, const char **tablespace_name,
                         MEM_ROOT *mem_root);

}  // namespace dd
#endif  // DD_TABLESPACE_INCLUDED
