/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef LOCKED_TABLES_LIST_INCLUDED
#define LOCKED_TABLES_LIST_INCLUDED

#include <sys/types.h>
#include <vector>
#include "my_alloc.h"

struct MYSQL_LOCK;
class MDL_context;
class MDL_ticket;
struct TABLE;
struct TABLE_LIST;
class THD;

/**
  Type of locked tables mode.
  See comment for THD::locked_tables_mode for complete description.
  While adding new enum values add them to the getter method for this enum
  declared below and defined in binlog.cc as well.
*/

enum enum_locked_tables_mode {
  LTM_NONE = 0,
  LTM_LOCK_TABLES,
  LTM_PRELOCKED,
  LTM_PRELOCKED_UNDER_LOCK_TABLES
};

#ifndef DBUG_OFF
/**
  Getter for the enum enum_locked_tables_mode
  @param locked_tables_mode enum for types of locked tables mode

  @return The string represantation of that enum value
*/
const char *get_locked_tables_mode_name(
    enum_locked_tables_mode locked_tables_mode);
#endif

/**
  Tables that were locked with LOCK TABLES statement.

  Encapsulates a list of TABLE_LIST instances for tables
  locked by LOCK TABLES statement, memory root for metadata locks,
  and, generally, the context of LOCK TABLES statement.

  In LOCK TABLES mode, the locked tables are kept open between
  statements.
  Therefore, we can't allocate metadata locks on execution memory
  root -- as well as tables, the locks need to stay around till
  UNLOCK TABLES is called.
  The locks are allocated in the memory root encapsulated in this
  class.

  Some SQL commands, like FLUSH TABLE or ALTER TABLE, demand that
  the tables they operate on are closed, at least temporarily.
  This class encapsulates a list of TABLE_LIST instances, one
  for each base table from LOCK TABLES list,
  which helps conveniently close the TABLEs when it's necessary
  and later reopen them.

*/

class Locked_tables_list {
 private:
  MEM_ROOT m_locked_tables_root;
  TABLE_LIST *m_locked_tables;
  TABLE_LIST **m_locked_tables_last;
  /** An auxiliary array used only in reopen_tables(). */
  TABLE **m_reopen_array;
  /**
    Count the number of tables in m_locked_tables list. We can't
    rely on thd->lock->table_count because it excludes
    non-transactional temporary tables. We need to know
    an exact number of TABLE objects.
  */
  size_t m_locked_tables_count;

  struct MDL_ticket_pair {
    MDL_ticket *m_src;
    MDL_ticket *m_dst;
  };
  using MDL_ticket_pairs = std::vector<MDL_ticket_pair>;

  MDL_ticket_pairs m_rename_tablespace_mdls;

 public:
  Locked_tables_list();

  void unlock_locked_tables(THD *thd);
  ~Locked_tables_list() {
    unlock_locked_tables(nullptr);
    DBUG_ASSERT(m_rename_tablespace_mdls.empty());
  }
  bool init_locked_tables(THD *thd);
  TABLE_LIST *locked_tables() const { return m_locked_tables; }
  void unlink_from_list(const THD *thd, TABLE_LIST *table_list,
                        bool remove_from_locked_tables);
  void unlink_all_closed_tables(THD *thd, MYSQL_LOCK *lock,
                                size_t reopen_count);
  bool reopen_tables(THD *thd);
  void rename_locked_table(TABLE_LIST *old_table_list, const char *new_db,
                           const char *new_table_name,
                           MDL_ticket *target_mdl_ticket);

  void add_rename_tablespace_mdls(MDL_ticket *src, MDL_ticket *dst);
  void adjust_renamed_tablespace_mdls(MDL_context *mctx);
  void discard_renamed_tablespace_mdls() { m_rename_tablespace_mdls.clear(); }
};

#endif /*not defined LOCK_TABLES_LIST_INCLUDED */
