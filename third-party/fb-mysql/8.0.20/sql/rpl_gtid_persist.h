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

#ifndef RPL_GTID_PERSIST_H_
#define RPL_GTID_PERSIST_H_

#include <string.h>
#include <sys/types.h>
#include <atomic>
#include <string>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysqld_error.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_table_access.h"  // System_table_access
#include "sql/sql_class.h"         // Open_tables_backup
#include "sql/table.h"
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "thr_lock.h"

class Field;

class Gtid_table_access_context : public System_table_access {
 public:
  static const LEX_CSTRING DB_NAME;
  static const LEX_CSTRING TABLE_NAME;

  Gtid_table_access_context() : m_drop_thd_object(nullptr) {}
  virtual ~Gtid_table_access_context() {}

  /**
    Initialize the gtid_executed table access context as following:
      - Create a new THD if current_thd is NULL
      - Disable binlog temporarily if we are going to modify the table
      - Open and lock a table.

    @param[in,out] thd       Thread requesting to open the table
    @param[out]    table     We will store the open table here
    @param[in]     is_write  If true, the access will be for modifying the table

    @retval true  failed
    @retval false success
  */
  bool init(THD **thd, TABLE **table, bool is_write);
  /**
    De-initialize the gtid_executed table access context as following:
      - Close the table
      - Reenable binlog if needed
      - Destroy the created THD if needed.

    @param thd         Thread requesting to close the table
    @param table       Table to be closed
    @param error       If there was an error while updating the table
    @param need_commit Need to commit current transaction if it is true

    @retval true  failed
    @retval false success
  */
  bool deinit(THD *thd, TABLE *table, bool error, bool need_commit);
  /**
    Prepares before opening table.
    - set flags

    @param[in]  thd  Thread requesting to open the table
  */
  void before_open(THD *thd);
  /**
    Creates a new thread in the bootstrap process or in the mysqld startup,
    a thread is created in order to be able to access a table. And reset a
    new "statement".

    @returns THD* Pointer to thread structure
  */
  THD *create_thd();
  void drop_thd(THD *thd);

 private:
  /* Pointer to new created THD. */
  THD *m_drop_thd_object;
  /* Modify the table if it is true. */
  bool m_is_write;
  /* Save the lock info. */
  Open_tables_backup m_backup;
  /* Save binlog options. */
  ulonglong m_tmp_disable_binlog__save_options;
  /* Whether or not `THD::set_skip_readonly_check` was invoked during `THD`
     initialization */
  bool m_skip_readonly_set{false};

  /* Prevent user from invoking default assignment function. */
  Gtid_table_access_context &operator=(const Gtid_table_access_context &info);
  /* Prevent user from invoking default constructor function. */
  Gtid_table_access_context(const Gtid_table_access_context &info);
};

class Gtid_table_persistor {
 public:
  static const uint number_fields = 3;

  Gtid_table_persistor() {}
  virtual ~Gtid_table_persistor() {}

  /**
    Insert the gtid into table.

    @param thd  Thread requesting to save gtid into the table
    @param gtid holds the sidno and the gno.

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int save(THD *thd, const Gtid *gtid);
  /**
    Insert the gtid set into table.

    @param gtid_set  contains a set of gtid, which holds
                     the sidno and the gno.

    @param compress notify to compress gtid_executed table

    @retval
      0    OK
    @retval
      -1   Error
  */
  int save(const Gtid_set *gtid_set, bool compress = true);
  /**
    Delete all rows from the table.

    @param  thd Thread requesting to reset the table

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int reset(THD *thd);

  /**
    Fetch gtids from gtid_executed table and store them into
    gtid_executed set.

    @param[out]  gtid_set store gtids fetched from the gtid_executed table.

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int fetch_gtids(Gtid_set *gtid_set);
  /**
    Compress the gtid_executed table completely by employing one
    or more transactions.

    @param  thd Thread requesting to compress the table

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int compress(THD *thd);
  /**
    Push a warning to client if user is modifying the gtid_executed
    table explicitly by a non-XA transaction. Push an error to client
    if user is modifying it explicitly by a XA transaction.

    @param thd Thread requesting to access the table
    @param table The table is being accessed.

    @retval 0 No warning or error was pushed to the client.
    @retval 1 Push a warning to client.
    @retval 2 Push an error to client.
  */
  int warn_or_err_on_explicit_modification(THD *thd, TABLE_LIST *table) {
    DBUG_TRACE;

    if (!thd->is_operating_gtid_table_implicitly &&
        table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE &&
        !strcmp(table->table_name, Gtid_table_access_context::TABLE_NAME.str)) {
      if (thd->get_transaction()->xid_state()->has_state(
              XID_STATE::XA_ACTIVE)) {
        /*
          Push an error to client if user is modifying the gtid_executed
          table explicitly by a XA transaction.
        */
        thd->raise_error_printf(ER_ERROR_ON_MODIFYING_GTID_EXECUTED_TABLE,
                                table->table_name);
        return 2;
      } else {
        /*
          Push a warning to client if user is modifying the gtid_executed
          table explicitly by a non-XA transaction.
        */
        thd->raise_warning_printf(ER_WARN_ON_MODIFYING_GTID_EXECUTED_TABLE,
                                  table->table_name);
        return 1;
      }
    }

    return 0;
  }

 private:
  /* Count the append size of the table */
  std::atomic<int64> m_atomic_count{0};
  /**
    Compress the gtid_executed table, read each row by the
    PK(sid, gno_start) in increasing order, compress the first
    consecutive range of gtids within a single transaction.

    @param      thd          Thread requesting to compress the table
    @param[out] is_complete  True if the gtid_executed table is
                             compressd completely.

    @retval
      0    OK
    @retval
      1    The table was not found.
    @retval
      -1   Error
  */
  int compress_in_single_transaction(THD *thd, bool &is_complete);
  /**
    Read each row by the PK(sid, gno_start) in increasing order,
    compress the first consecutive range of gtids.
    For example,
      1 1
      2 2
      3 3
      6 6
      7 7
      8 8
    After the compression, the gtids in the table is compressed as following:
      1 3
      6 6
      7 7
      8 8

    @param      table        Reference to a table object.
    @param[out] is_complete  True if the gtid_executed table
                             is compressd completely.

    @retval 0    OK.
    @retval -1   Error.
  */
  int compress_first_consecutive_range(TABLE *table, bool &is_complete);
  /**
    Fill a gtid interval into fields of the gtid_executed table.

    @param  fields   Reference to table fileds.
    @param  sid      The source id of the gtid interval.
    @param  gno_start The first GNO of the gtid interval.
    @param  gno_end  The last GNO of the gtid interval.

    @retval 0    OK.
    @retval -1   Error.
  */
  int fill_fields(Field **fields, const char *sid, rpl_gno gno_start,
                  rpl_gno gno_end);
  /**
    Write a gtid interval into the gtid_executed table.

    @param  table    Reference to a table object.
    @param  sid      The source id of the gtid interval.
    @param  gno_start The first GNO of the gtid interval.
    @param  gno_end  The last GNO of the gtid interval.

    @retval 0    OK.
    @retval -1   Error.
  */
  int write_row(TABLE *table, const char *sid, rpl_gno gno_start,
                rpl_gno gno_end);
  /**
    Update a gtid interval in the gtid_executed table.
    - locate the gtid interval by primary key (sid, gno_start)
      to update it with the new_gno_end.

    @param  table        Reference to a table object.
    @param  sid          The source id of the gtid interval.
    @param  gno_start    The first GNO of the gtid interval.
    @param  new_gno_end  The new last GNO of the gtid interval.

    @retval 0    OK.
    @retval -1   Error.
  */
  int update_row(TABLE *table, const char *sid, rpl_gno gno_start,
                 rpl_gno new_gno_end);
  /**
    Delete all rows in the gtid_executed table.

    @param  table Reference to a table object.

    @retval 0    OK.
    @retval -1   Error.
  */
  int delete_all(TABLE *table);
  /**
    Encode the current row fetched from the table into gtid text.

    @param  table Reference to a table object.
    @retval Return the encoded gtid text.
  */
  std::string encode_gtid_text(TABLE *table);
  /**
    Get gtid interval from the the current row of the table.

    @param table          Reference to a table object.
    @param [out] sid      The source id of the gtid interval.
    @param [out] gno_start The first GNO of the gtid interval.
    @param [out] gno_end  The last GNO of the gtid interval.
  */
  void get_gtid_interval(TABLE *table, std::string &sid, rpl_gno &gno_start,
                         rpl_gno &gno_end);
  /**
    Insert the gtid set into table.

    @param table          The gtid_executed table.
    @param gtid_set       Contains a set of gtid, which holds
                          the sidno and the gno.

    @retval
      0    OK
    @retval
      -1   Error
  */
  int save(TABLE *table, const Gtid_set *gtid_set);
  /* Prevent user from invoking default assignment function. */
  Gtid_table_persistor &operator=(const Gtid_table_persistor &info);
  /* Prevent user from invoking default constructor function. */
  Gtid_table_persistor(const Gtid_table_persistor &info);
};

extern Gtid_table_persistor *gtid_table_persistor;
void create_compress_gtid_table_thread();
void terminate_compress_gtid_table_thread();

#endif /* RPL_GTID_PERSIST_H_ */
