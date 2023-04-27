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

#include "sql/tc_log.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <algorithm>

#include "map_helpers.h"
#include "my_alloc.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/handler.h"
#include "sql/log.h"
#include "sql/mysqld.h"          // mysql_data_home
#include "sql/psi_memory_key.h"  // key_memory_TC_LOG_MMAP_pages
#include "sql/sql_class.h"       // THD
#include "sql/sql_const.h"
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "thr_mutex.h"

TC_LOG::enum_result TC_LOG_DUMMY::commit(THD *thd, bool all) {
  return ha_commit_low(thd, all) ? RESULT_ABORTED : RESULT_SUCCESS;
}

int TC_LOG_DUMMY::rollback(THD *thd, bool all) {
  return ha_rollback_low(thd, all);
}

int TC_LOG_DUMMY::prepare(THD *thd, bool all) {
  return ha_prepare_low(thd, all);
}

/********* transaction coordinator log for 2pc - mmap() based solution *******/

/*
  the log consists of a file, mmapped to a memory.
  file is divided on pages of tc_log_page_size size.
  (usable size of the first page is smaller because of log header)
  there's PAGE control structure for each page
  each page (or rather PAGE control structure) can be in one of three
  states - active, syncing, pool.
  there could be only one page in active or syncing states,
  but many in pool - pool is fifo queue.
  usual lifecycle of a page is pool->active->syncing->pool
  "active" page - is a page where new xid's are logged.
  the page stays active as long as syncing slot is taken.
  "syncing" page is being synced to disk. no new xid can be added to it.
  when the sync is done the page is moved to a pool and an active page
  becomes "syncing".

  the result of such an architecture is a natural "commit grouping" -
  If commits are coming faster than the system can sync, they do not
  stall. Instead, all commit that came since the last sync are
  logged to the same page, and they all are synced with the next -
  one - sync. Thus, thought individual commits are delayed, throughput
  is not decreasing.

  when a xid is added to an active page, the thread of this xid waits
  for a page's condition until the page is synced. when syncing slot
  becomes vacant one of these waiters is awaken to take care of syncing.
  it syncs the page and signals all waiters that the page is synced.
  PAGE::waiters is used to count these waiters, and a page may never
  become active again until waiters==0 (that is all waiters from the
  previous sync have noticed the sync was completed)

  note, that the page becomes "dirty" and has to be synced only when a
  new xid is added into it. Removing a xid from a page does not make it
  dirty - we don't sync removals to disk.
*/

ulong tc_log_page_waits = 0;

#define TC_LOG_HEADER_SIZE (sizeof(tc_log_magic) + 1)

static const char tc_log_magic[] = {(char)254, 0x23, 0x05, 0x74};

ulong tc_log_max_pages_used = 0, tc_log_page_size = 0,
      tc_log_cur_pages_used = 0;

int TC_LOG_MMAP::open(const char *opt_name) {
  uint i;
  bool crashed = false;
  PAGE *pg;

  DBUG_ASSERT(total_ha_2pc > 1);
  DBUG_ASSERT(opt_name && opt_name[0]);

  tc_log_page_size = my_getpagesize();

  fn_format(logname, opt_name, mysql_data_home, "", MY_UNPACK_FILENAME);
  if ((fd = mysql_file_open(key_file_tclog, logname, O_RDWR, MYF(0))) < 0) {
    if (my_errno() != ENOENT) goto err;
    if (using_heuristic_recover()) return 1;
    if ((fd = mysql_file_create(key_file_tclog, logname, CREATE_MODE, O_RDWR,
                                MYF(MY_WME))) < 0)
      goto err;
    inited = 1;
    file_length = opt_tc_log_size;
    if (mysql_file_chsize(fd, file_length, 0, MYF(MY_WME))) goto err;
  } else {
    inited = 1;
    crashed = true;
    LogErr(INFORMATION_LEVEL, ER_TC_RECOVERING_AFTER_CRASH_USING, opt_name);
    if (tc_heuristic_recover != TC_HEURISTIC_NOT_USED) {
      LogErr(ERROR_LEVEL, ER_TC_CANT_AUTO_RECOVER_WITH_TC_HEURISTIC_RECOVER);
      goto err;
    }
    file_length = mysql_file_seek(fd, 0L, MY_SEEK_END, MYF(MY_WME + MY_FAE));
    if (file_length == MY_FILEPOS_ERROR || file_length % tc_log_page_size)
      goto err;
  }

  data = (uchar *)my_mmap(nullptr, (size_t)file_length, PROT_READ | PROT_WRITE,
                          MAP_NOSYNC | MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    set_my_errno(errno);
    goto err;
  }
  inited = 2;

  npages = (uint)file_length / tc_log_page_size;
  DBUG_ASSERT(npages >= 3);  // to guarantee non-empty pool
  if (!(pages = (PAGE *)my_malloc(key_memory_TC_LOG_MMAP_pages,
                                  npages * sizeof(PAGE),
                                  MYF(MY_WME | MY_ZEROFILL))))
    goto err;
  inited = 3;
  for (pg = pages, i = 0; i < npages; i++, pg++) {
    pg->next = pg + 1;
    pg->waiters = 0;
    pg->state = PS_POOL;
    mysql_cond_init(key_PAGE_cond, &pg->cond);
    pg->size = pg->free = tc_log_page_size / sizeof(my_xid);
    pg->start = (my_xid *)(data + i * tc_log_page_size);
    pg->end = pg->start + pg->size;
    pg->ptr = pg->start;
  }
  pages[0].size = pages[0].free =
      (tc_log_page_size - TC_LOG_HEADER_SIZE) / sizeof(my_xid);
  pages[0].start = pages[0].end - pages[0].size;
  pages[npages - 1].next = nullptr;
  inited = 4;

  if (crashed && recover()) goto err;

  memcpy(data, tc_log_magic, sizeof(tc_log_magic));
  data[sizeof(tc_log_magic)] = (uchar)total_ha_2pc;
  my_msync(fd, data, tc_log_page_size, MS_SYNC);
  inited = 5;

  mysql_mutex_init(key_LOCK_tc, &LOCK_tc, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_active, &COND_active);
  mysql_cond_init(key_COND_pool, &COND_pool);

  inited = 6;

  syncing = nullptr;
  active = pages;
  pool = pages + 1;
  pool_last_ptr = &pages[npages - 1].next;

  return 0;

err:
  close();
  return 1;
}

/**
  Get the total amount of potentially usable slots for XIDs in TC log.
*/

uint TC_LOG_MMAP::size() const {
  return (tc_log_page_size - TC_LOG_HEADER_SIZE) / sizeof(my_xid) +
         (npages - 1) * (tc_log_page_size / sizeof(my_xid));
}

/**
  there is no active page, let's got one from the pool.

  Two strategies here:
    -# take the first from the pool
    -# if there're waiters - take the one with the most free space.

  @todo
    TODO page merging. try to allocate adjacent page first,
    so that they can be flushed both in one sync

  @returns Pointer to qualifying page or NULL if no page in the
           pool can be made active.
*/

TC_LOG_MMAP::PAGE *TC_LOG_MMAP::get_active_from_pool() {
  PAGE **best_p = &pool;

  if ((*best_p)->waiters != 0 || (*best_p)->free == 0) {
    /* if the first page can't be used try second strategy */
    int best_free = 0;
    PAGE **p = &pool;
    for (p = &(*p)->next; *p; p = &(*p)->next) {
      if ((*p)->waiters == 0 && (*p)->free > best_free) {
        best_free = (*p)->free;
        best_p = p;
      }
    }
    if (*best_p == nullptr || best_free == 0) return nullptr;
  }

  PAGE *new_active = *best_p;
  if (new_active->free == new_active->size)  // we've chosen an empty page
  {
    tc_log_cur_pages_used++;
    tc_log_max_pages_used =
        std::max(tc_log_max_pages_used, tc_log_cur_pages_used);
  }

  *best_p = (*best_p)->next;
  if (!*best_p) pool_last_ptr = best_p;

  return new_active;
}

/**
  @todo
  perhaps, increase log size ?
*/
void TC_LOG_MMAP::overflow() {
  /*
    simple overflow handling - just wait
    TODO perhaps, increase log size ?
    let's check the behaviour of tc_log_page_waits first
  */
  ulong old_log_page_waits = tc_log_page_waits;

  mysql_cond_wait(&COND_pool, &LOCK_tc);

  if (old_log_page_waits == tc_log_page_waits) {
    /*
      When several threads are waiting in overflow() simultaneously
      we want to increase counter only once and not for each thread.
    */
    tc_log_page_waits++;
  }
}

/**
  Commit the transaction.

  @note When the TC_LOG inteface was changed, this function was added
  and uses the functions that were there with the old interface to
  implement the logic.
 */
TC_LOG::enum_result TC_LOG_MMAP::commit(THD *thd, bool all) {
  DBUG_TRACE;
  ulong cookie = 0;
  my_xid xid = thd->get_transaction()->xid_state()->get_xid()->get_my_xid();

  if (all && xid)
    if (!(cookie = log_xid(xid)))
      return RESULT_ABORTED;  // Failed to log the transaction

  if (ha_commit_low(thd, all))
    return RESULT_INCONSISTENT;  // Transaction logged, but not committed

  /* If cookie is non-zero, something was logged */
  if (cookie) unlog(cookie, xid);

  return RESULT_SUCCESS;
}

int TC_LOG_MMAP::rollback(THD *thd, bool all) {
  return ha_rollback_low(thd, all);
}

int TC_LOG_MMAP::prepare(THD *thd, bool all) {
  return ha_prepare_low(thd, all);
}

/**
  Record that transaction XID is committed on the persistent storage.

    This function is called in the middle of two-phase commit:
    First all resources prepare the transaction, then tc_log->log() is called,
    then all resources commit the transaction, then tc_log->unlog() is called.

    All access to active page is serialized but it's not a problem, as
    we're assuming that fsync() will be a main bottleneck.
    That is, parallelizing writes to log pages we'll decrease number of
    threads waiting for a page, but then all these threads will be waiting
    for a fsync() anyway

   If tc_log == MYSQL_BIN_LOG then tc_log writes transaction to binlog and
   records XID in a special Xid_log_event.
   If tc_log = TC_LOG_MMAP then xid is written in a special memory-mapped
   log.

  @returns "cookie", a number that will be passed as an argument
    to unlog() call. tc_log can define it any way it wants,
    and use for whatever purposes. TC_LOG_MMAP sets it
    to the position in memory where xid was logged to.
  @retval
    0  error
*/

ulong TC_LOG_MMAP::log_xid(my_xid xid) {
  mysql_mutex_lock(&LOCK_tc);

  while (true) {
    /* If active page is full - just wait... */
    while (unlikely(active && active->free == 0))
      mysql_cond_wait(&COND_active, &LOCK_tc);

    /* no active page ? take one from the pool. */
    if (active == nullptr) {
      active = get_active_from_pool();

      /* There are no pages with free slots? Wait and retry. */
      if (active == nullptr) {
        overflow();
        continue;
      }
    }

    break;
  }

  PAGE *p = active;
  ulong cookie = store_xid_in_empty_slot(xid, p, data);
  bool err;

  if (syncing) {  // somebody's syncing. let's wait
    err = wait_sync_completion(p);
    if (p->state != PS_DIRTY)  // page was synced
    {
      if (p->waiters == 0)
        mysql_cond_broadcast(&COND_pool);  // in case somebody's waiting
      mysql_mutex_unlock(&LOCK_tc);
      goto done;  // we're done
    }
  }  // page was not synced! do it now
  DBUG_ASSERT(active == p && syncing == nullptr);
  syncing = p;                         // place is vacant - take it
  active = nullptr;                    // page is not active anymore
  mysql_cond_broadcast(&COND_active);  // in case somebody's waiting
  mysql_mutex_unlock(&LOCK_tc);
  err = sync();

done:
  return err ? 0 : cookie;
}

/**
  Write the page data being synchronized to the disk.

  @retval false   Success
  @retval true    Failure
*/
bool TC_LOG_MMAP::sync() {
  /*
    sit down and relax - this can take a while...
    note - no locks are held at this point
  */

  int err = do_msync_and_fsync(fd, syncing->start,
                               syncing->size * sizeof(my_xid), MS_SYNC);

  mysql_mutex_lock(&LOCK_tc);
  DBUG_ASSERT(syncing != active);

  /* Page is synced. Let's move it to the pool. */
  *pool_last_ptr = syncing;
  pool_last_ptr = &(syncing->next);
  syncing->next = nullptr;
  syncing->state = err ? PS_ERROR : PS_POOL;
  mysql_cond_broadcast(&COND_pool);  // in case somebody's waiting

  /* Wake-up all threads which are waiting for syncing of the same page. */
  mysql_cond_broadcast(&syncing->cond);

  /* Mark syncing slot as free and wake-up new syncer. */
  syncing = nullptr;
  if (active) mysql_cond_signal(&active->cond);

  mysql_mutex_unlock(&LOCK_tc);
  return err != 0;
}

/**
  erase xid from the page, update page free space counters/pointers.
  cookie points directly to the memory where xid was logged.
*/

void TC_LOG_MMAP::unlog(ulong cookie, my_xid xid MY_ATTRIBUTE((unused))) {
  PAGE *p = pages + (cookie / tc_log_page_size);
  my_xid *x = (my_xid *)(data + cookie);

  DBUG_ASSERT(*x == xid);
  DBUG_ASSERT(x >= p->start && x < p->end);

  mysql_mutex_lock(&LOCK_tc);
  *x = 0;
  p->free++;
  DBUG_ASSERT(p->free <= p->size);
  p->ptr = std::min(p->ptr, x);
  if (p->free == p->size)  // the page is completely empty
    tc_log_cur_pages_used--;
  if (p->waiters == 0)                 // the page is in pool and ready to rock
    mysql_cond_broadcast(&COND_pool);  // ping ... for overflow()
  mysql_mutex_unlock(&LOCK_tc);
}

void TC_LOG_MMAP::close() {
  uint i;
  switch (inited) {
    case 6:
      mysql_mutex_destroy(&LOCK_tc);
      mysql_cond_destroy(&COND_pool);
      // Fall through.
    case 5:
      data[0] = 'A';  // garble the first (signature) byte, in case
                      // mysql_file_delete fails
      // Fall through.
    case 4:
      for (i = 0; i < npages; i++) {
        if (pages[i].ptr == nullptr) break;
        mysql_cond_destroy(&pages[i].cond);
      }
      // Fall through.
    case 3:
      my_free(pages);
      // Fall through.
    case 2:
      my_munmap((char *)data, (size_t)file_length);
      // Fall through.
    case 1:
      mysql_file_close(fd, MYF(0));
  }
  if (inited >= 5)  // cannot do in the switch because of Windows
    mysql_file_delete(key_file_tclog, logname, MYF(MY_WME));
  inited = 0;
}

int TC_LOG_MMAP::recover() {
  PAGE *p = pages, *end_p = pages + npages;

  if (memcmp(data, tc_log_magic, sizeof(tc_log_magic))) {
    LogErr(ERROR_LEVEL, ER_TC_BAD_MAGIC_IN_TC_LOG);
    goto err1;
  }

  /*
    the first byte after magic signature is set to current
    number of storage engines on startup
  */
  if (data[sizeof(tc_log_magic)] != total_ha_2pc) {
    LogErr(ERROR_LEVEL, ER_TC_NEED_N_SE_SUPPORTING_2PC_FOR_RECOVERY,
           data[sizeof(tc_log_magic)]);
    goto err1;
  }

  {
    MEM_ROOT mem_root(PSI_INSTRUMENT_ME, tc_log_page_size / 3);
    xid_to_gtid_container xids(&mem_root);

    for (; p < end_p; p++) {
      for (my_xid *x = p->start; x < p->end; x++) {
        if (*x) {
          Gtid current_gtid;
          current_gtid.clear();
          xids.emplace(*x, current_gtid);
        }
      }
    }

    bool err = ha_recover(&xids);
    if (err) goto err1;
  }

  memset(data, 0, (size_t)file_length);
  return 0;

err1:
  LogErr(ERROR_LEVEL, ER_TC_RECOVERY_FAILED_THESE_ARE_YOUR_OPTIONS);
  return 1;
}

TC_LOG *tc_log;
TC_LOG_DUMMY tc_log_dummy;
TC_LOG_MMAP tc_log_mmap;

bool TC_LOG::using_heuristic_recover() {
  if (tc_heuristic_recover == TC_HEURISTIC_NOT_USED) return false;

  LogErr(INFORMATION_LEVEL, ER_TC_HEURISTIC_RECOVERY_MODE);
  if (ha_recover(nullptr)) LogErr(ERROR_LEVEL, ER_TC_HEURISTIC_RECOVERY_FAILED);
  LogErr(INFORMATION_LEVEL, ER_TC_RESTART_WITHOUT_TC_HEURISTIC_RECOVER);
  return true;
}
