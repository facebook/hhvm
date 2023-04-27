/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TC_LOG_H
#define TC_LOG_H

#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"  // my_msync
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/psi/mysql_cond.h"

class THD;

typedef ulonglong my_xid;

#define TC_LOG_MIN_PAGES 6

/**
  Transaction Coordinator Log.

  A base abstract class for three different implementations of the
  transaction coordinator.

  The server uses the transaction coordinator to order transactions
  correctly and there are three different implementations: one using
  an in-memory structure, one dummy that does not do anything, and one
  using the binary log for transaction coordination.
*/
class TC_LOG {
 public:
  /**
    Perform heuristic recovery, if --tc-heuristic-recover was used.

    @note no matter whether heuristic recovery was successful or not
    mysqld must exit. So, return value is the same in both cases.

    @retval false  no heuristic recovery was requested
    @retval true   heuristic recovery was performed
  */
  bool using_heuristic_recover();

  TC_LOG() {}
  virtual ~TC_LOG() {}

  enum enum_result { RESULT_SUCCESS, RESULT_ABORTED, RESULT_INCONSISTENT };

  /**
    Initialize and open the coordinator log.
    Do recovery if necessary. Called during server startup.

    @param opt_name  Name of logfile.

    @retval 0  sucess
    @retval 1  failed
  */
  virtual int open(const char *opt_name) = 0;

  /**
    Close the transaction coordinator log and free any resources.
    Called during server shutdown.
  */
  virtual void close() = 0;

  /**
     Log a commit record of the transaction to the transaction
     coordinator log.

     When the function returns, the transaction commit is properly
     logged to the transaction coordinator log and can be committed in
     the storage engines.

     @param thd Session to log transaction for.
     @param all @c True if this is a "real" commit, @c false if it is a
     "statement" commit.

     @return Error code on failure, zero on success.
   */
  virtual enum_result commit(THD *thd, bool all) = 0;

  /**
     Log a rollback record of the transaction to the transaction
     coordinator log.

     When the function returns, the transaction have been aborted in
     the transaction coordinator log.

     @param thd Session to log transaction record for.

     @param all @c true if an explicit commit or an implicit commit
     for a statement, @c false if an internal commit of the statement.

     @return Error code on failure, zero on success.
   */
  virtual int rollback(THD *thd, bool all) = 0;

  /**
     Log a prepare record of the transaction to the storage engines.

     @param thd Session to log transaction record for.

     @param all @c true if an explicit commit or an implicit commit
     for a statement, @c false if an internal commit of the statement.

     @return Error code on failure, zero on success.
   */
  virtual int prepare(THD *thd, bool all) = 0;
};

class TC_LOG_DUMMY : public TC_LOG  // use it to disable the logging
{
 public:
  TC_LOG_DUMMY() {}
  int open(const char *) { return 0; }
  void close() {}
  enum_result commit(THD *thd, bool all);
  int rollback(THD *thd, bool all);
  int prepare(THD *thd, bool all);
};

class TC_LOG_MMAP : public TC_LOG {
 public:  // only to keep Sun Forte on sol9x86 happy
  typedef enum {
    PS_POOL,   // page is in pool
    PS_ERROR,  // last sync failed
    PS_DIRTY   // new xids added since last sync
  } PAGE_STATE;

 private:
  struct PAGE {
    PAGE *next;           // pages are linked in a fifo queue
    my_xid *start, *end;  // usable area of a page
    my_xid *ptr;          // next xid will be written here
    int size, free;    // max and current number of free xid slots on the page
    int waiters;       // number of waiters on condition
    PAGE_STATE state;  // see above
    /**
      Signalled when syncing of this page is done or when
      this page is in "active" slot and syncing slot just
      became free.
    */
    mysql_cond_t cond;
  };

  char logname[FN_REFLEN];
  File fd;
  my_off_t file_length;
  uint npages, inited;
  uchar *data;
  PAGE *pages, *syncing, *active, *pool, **pool_last_ptr;
  /*
    LOCK_tc is used to protect access both to data members 'syncing',
    'active', 'pool' and to the content of PAGE objects.
  */
  mysql_mutex_t LOCK_tc;
  /**
    Signalled when active PAGE is moved to syncing state,
    thus member "active" becomes 0.
  */
  mysql_cond_t COND_active;
  /**
    Signalled when one more page becomes available in the
    pool which we might select as active.
  */
  mysql_cond_t COND_pool;

 public:
  TC_LOG_MMAP() : inited(0) {}
  int open(const char *opt_name);
  void close();
  enum_result commit(THD *thd, bool all);
  int rollback(THD *thd, bool all);
  int prepare(THD *thd, bool all);
  int recover();
  uint size() const;

 private:
  ulong log_xid(my_xid xid);
  void unlog(ulong cookie, my_xid xid);
  PAGE *get_active_from_pool();
  bool sync();
  void overflow();

 protected:
  // We want to mock away syncing to disk in unit tests.
  virtual int do_msync_and_fsync(int fd_arg, void *addr, size_t len,
                                 int flags) {
    return my_msync(fd_arg, addr, len, flags);
  }

 private:
  /**
    Find empty slot in the page and write xid value there.

    @param   xid    value of xid to store in the page
    @param   p      pointer to the page where to store xid
    @param   data_arg   pointer to the top of the mapped to memory file
                    to calculate offset value (cookie)

    @return  offset value from the top of the page where the xid was stored.
  */
  ulong store_xid_in_empty_slot(my_xid xid, PAGE *p, uchar *data_arg) {
    /* searching for an empty slot */
    while (*p->ptr) {
      p->ptr++;
      DBUG_ASSERT(p->ptr < p->end);  // because p->free > 0
    }

    /* found! store xid there and mark the page dirty */
    ulong cookie = (ulong)((uchar *)p->ptr - data_arg);  // can never be zero
    *p->ptr++ = xid;
    p->free--;
    p->state = PS_DIRTY;

    return cookie;
  }

  /**
    Wait for until page data will be written to the disk.

    @param   p   pointer to the PAGE to store to the disk

    @retval false   Success
    @retval true    Failure
  */
  bool wait_sync_completion(PAGE *p) {
    p->waiters++;
    while (p->state == PS_DIRTY && syncing) {
      mysql_cond_wait(&p->cond, &LOCK_tc);
    }
    p->waiters--;

    return p->state == PS_ERROR;
  }

  /*
    the following friend declaration is to grant access from TCLogMMapTest
    to methods log_xid()/unlog() that are private.
  */
  friend class TCLogMMapTest;
};

extern TC_LOG *tc_log;
extern TC_LOG_MMAP tc_log_mmap;
extern TC_LOG_DUMMY tc_log_dummy;

#endif  // TC_LOG_H
