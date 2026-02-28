/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_TABLE_ACCESS_H_
#define RPL_TABLE_ACCESS_H_

#include <sys/types.h>

#include "lex_string.h"
#include "thr_lock.h"  // thr_lock_type

class Open_tables_backup;
class THD;
struct TABLE;

/**
  A base class for accessing a system table.
*/

class System_table_access {
 public:
  virtual ~System_table_access() {}

  /**
    Opens and locks a system table.

    It's assumed that the caller knows what they are doing:
    - whether it was necessary to reset-and-backup the open tables state
    - whether the requested lock does not lead to a deadlock
    - whether this open mode would work under LOCK TABLES, or inside a
    stored function or trigger.

    Note that if the table can't be locked successfully this operation will
    close it. Therefore it provides guarantee that it either opens and locks
    table or fails without leaving any tables open.

    @param[in]  thd           Thread requesting to open the table
    @param[in]  dbstr         Database where the table resides
    @param[in]  tbstr         Table to be openned
    @param[in]  max_num_field Maximum number of fields
    @param[in]  lock_type     How to lock the table
    @param[out] table         We will store the open table here
    @param[out] backup        Save the lock info. here

    @retval true open and lock failed - an error message is pushed into the
                                          stack
    @retval false success
  */
  bool open_table(THD *thd, const LEX_CSTRING dbstr, const LEX_CSTRING tbstr,
                  uint max_num_field, enum thr_lock_type lock_type,
                  TABLE **table, Open_tables_backup *backup);
  /**
    Prepares before opening table.

    @param[in]  thd  Thread requesting to open the table
  */
  virtual void before_open(THD *thd) = 0;
  /**
    Commits the changes, unlocks the table and closes it. This method
    needs to be called even if the open_table fails, in order to ensure
    the lock info is properly restored.

    @param[in] thd    Thread requesting to close the table
    @param[in] table  Table to be closed
    @param[in] backup Restore the lock info from here
    @param[in] error  If there was an error while updating
                      the table
    @param[in] need_commit Need to commit current transaction
                           if it is true.

    @retval  true   failed
    @retval  false  success

    If there is an error, rolls back the current statement. Otherwise,
    commits it. However, if a new thread was created and there is an
    error, the transaction must be rolled back. Otherwise, it must be
    committed. In this case, the changes were not done on behalf of
    any user transaction and if not finished, there would be pending
    changes.
  */
  bool close_table(THD *thd, TABLE *table, Open_tables_backup *backup,
                   bool error, bool need_commit);
  /**
    Creates a new thread in the bootstrap process or in the mysqld startup,
    a thread is created in order to be able to access a table.

    @return THD* Pointer to thread structure
  */
  THD *create_thd();
  /**
    Destroys the created thread and restores the
    system_thread information.

    @param thd Thread requesting to be destroyed
  */
  void drop_thd(THD *thd);

  /* Flags for opening table */
  uint m_flags;
};

#endif /* RPL_TABLE_ACCESS_H_ */
