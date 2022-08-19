/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_UPGRADE__GLOBAL_H_INCLUDED
#define DD_UPGRADE__GLOBAL_H_INCLUDED

#include <sys/types.h>

#include "sql/table.h"  // Table_check_intact

class THD;
class Time_zone;

namespace dd {

namespace upgrade_57 {

const String_type ISL_EXT = ".isl";
const String_type PAR_EXT = ".par";
const String_type OPT_EXT = ".opt";
const String_type NDB_EXT = ".ndb";
extern const char *TRN_EXT;
extern const char *TRG_EXT;

const String_type IBD_EXT = ".ibd";
const String_type index_stats = "innodb_index_stats";
const String_type index_stats_backup = "innodb_index_stats_backup57";
const String_type table_stats = "innodb_table_stats";
const String_type table_stats_backup = "innodb_table_stats_backup57";

/**
  THD::mem_root is only switched with the given mem_root and switched back
  on destruction. This does not free any mem_root.
 */
class Thd_mem_root_guard {
  THD *m_thd;
  MEM_ROOT *m_thd_prev_mem_root;

 public:
  Thd_mem_root_guard(THD *thd, MEM_ROOT *mem_root);
  ~Thd_mem_root_guard();
};

/**
   RAII for handling open and close of event and proc tables.
*/

class System_table_close_guard {
  THD *m_thd;
  TABLE *m_table;

 public:
  System_table_close_guard(THD *thd, TABLE *table);
  ~System_table_close_guard();
};

/**
  Class to check the system tables we are using from 5.7 are
  not corrupted before migrating the information to new DD.
*/
class Check_table_intact : public Table_check_intact {
 protected:
  void report_error(uint, const char *fmt, ...)
      MY_ATTRIBUTE((format(printf, 3, 4)));
};

}  // namespace upgrade_57
}  // namespace dd
#endif  // DD_UPGRADE__GLOBAL_H_INCLUDED
