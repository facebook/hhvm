/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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
   This is a unit test for the 'meta data locking' classes.
   It is written to illustrate how we can use Google Test for unit testing
   of MySQL code.
   For documentation on Google Test, see http://code.google.com/p/googletest/
   and the contained wiki pages GoogleTestPrimer and GoogleTestAdvancedGuide.
   The code below should hopefully be (mostly) self-explanatory.
 */

#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysqld_error.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"
#include "sql/thr_malloc.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/test_mdl_context_owner.h"
#include "unittest/gunit/thread_utils.h"

/*
  Mock thd_wait_begin/end functions
*/

void thd_wait_begin(THD *, int) {}

void thd_wait_end(THD *) {}

/*
  A mock error handler.
*/
static uint expected_error = 0;
extern "C" void test_error_handler_hook(uint err, const char *str, myf) {
  EXPECT_EQ(expected_error, err) << str;
}

/*
  Mock away this global function.
  We don't need DEBUG_SYNC functionality in a unit test.
 */
void debug_sync(THD *, const char *sync_point_name MY_ATTRIBUTE((unused)),
                size_t) {
  DBUG_PRINT("debug_sync_point", ("hit: '%s'", sync_point_name));
  FAIL() << "Not yet implemented.";
}

/*
  Putting everything in a namespace prevents any (unintentional)
  name clashes with the code under test.
*/
namespace mdl_unittest {

using thread::Notification;
using thread::Thread;

const char db_name[] = "some_database";
const char table_name1[] = "some_table1";
const char table_name2[] = "some_table2";
const char table_name3[] = "some_table3";
const char table_name4[] = "some_table4";
const ulong zero_timeout = 0;
const ulong long_timeout = (ulong)3600L * 24L * 365L;
const ulonglong long_timeout_nsec =
    static_cast<ulonglong>(long_timeout) * 1000000000ULL;

class MDLTest : public ::testing::Test, public Test_MDL_context_owner {
 protected:
  MDLTest() : m_null_ticket(nullptr), m_null_request(nullptr) {}

  static void SetUpTestCase() {
    /* Save original and install our custom error hook. */
    m_old_error_handler_hook = error_handler_hook;
    error_handler_hook = test_error_handler_hook;
  }

  static void TearDownTestCase() {
    error_handler_hook = m_old_error_handler_hook;
  }

  void SetUp() {
    expected_error = 0;
    mdl_locks_unused_locks_low_water = MDL_LOCKS_UNUSED_LOCKS_LOW_WATER_DEFAULT;
    max_write_lock_count = ULONG_MAX;
    mdl_init();
    m_mdl_context.init(this);
    m_mdl_context.set_ignore_owner_thd(true);
    EXPECT_FALSE(m_mdl_context.has_locks());
    m_charset = system_charset_info;
    system_charset_info = &my_charset_utf8_bin;
    EXPECT_TRUE(system_charset_info != nullptr);

    MDL_REQUEST_INIT(&m_global_request, MDL_key::GLOBAL, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  }

  void TearDown() {
    system_charset_info = m_charset;
    m_mdl_context.destroy();
    mdl_destroy();
  }

  virtual void notify_shared_lock(MDL_context_owner *in_use,
                                  bool needs_thr_lock_abort) {
    in_use->notify_shared_lock(nullptr, needs_thr_lock_abort);
  }

  // A utility member for testing single lock requests.
  void test_one_simple_shared_lock(enum_mdl_type lock_type);

  const MDL_ticket *m_null_ticket;
  const MDL_request *m_null_request;
  MDL_context m_mdl_context;
  MDL_request m_request;
  MDL_request m_global_request;
  MDL_request_list m_request_list;

 private:
  CHARSET_INFO *m_charset;
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MDLTest);

  static void (*m_old_error_handler_hook)(uint, const char *, myf);
};

void (*MDLTest::m_old_error_handler_hook)(uint, const char *, myf);

/*
  Will grab a lock on table_name of given type in the run() function.
  The two notifications are for synchronizing with the main thread.
  Does *not* take ownership of the notifications.
*/
class MDL_thread : public Thread, public Test_MDL_context_owner {
 public:
  MDL_thread(const char *table_name, enum_mdl_type mdl_type,
             Notification *lock_grabbed, Notification *release_locks,
             Notification *lock_blocked, Notification *lock_released)
      : m_table_name(table_name),
        m_mdl_type(mdl_type),
        m_lock_grabbed(lock_grabbed),
        m_release_locks(release_locks),
        m_lock_blocked(lock_blocked),
        m_lock_released(lock_released),
        m_enable_release_on_notify(false) {
    m_mdl_context.init(this);
  }

  ~MDL_thread() { m_mdl_context.destroy(); }

  virtual void run();
  void enable_release_on_notify() { m_enable_release_on_notify = true; }

  virtual void notify_shared_lock(MDL_context_owner *in_use,
                                  bool needs_thr_lock_abort) {
    if (in_use)
      in_use->notify_shared_lock(nullptr, needs_thr_lock_abort);
    else if (m_enable_release_on_notify && m_release_locks)
      m_release_locks->notify();
  }

  virtual void enter_cond(mysql_cond_t *cond, mysql_mutex_t *mutex,
                          const PSI_stage_info *stage,
                          PSI_stage_info *old_stage, const char *src_function,
                          const char *src_file, int src_line) {
    Test_MDL_context_owner::enter_cond(cond, mutex, stage, old_stage,
                                       src_function, src_file, src_line);

    /*
      No extra checks needed here since MDL uses enter_con only when thread
      is blocked.
    */
    if (m_lock_blocked) m_lock_blocked->notify();

    return;
  }

  MDL_context &get_mdl_context() { return m_mdl_context; }

 private:
  const char *m_table_name;
  enum_mdl_type m_mdl_type;
  Notification *m_lock_grabbed;
  Notification *m_release_locks;
  Notification *m_lock_blocked;
  Notification *m_lock_released;
  bool m_enable_release_on_notify;
  MDL_context m_mdl_context;
};

void MDL_thread::run() {
  MDL_request request;
  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, m_table_name, m_mdl_type,
                   MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, m_table_name, m_mdl_type));

  // Tell the main thread that we have grabbed our locks.
  if (m_lock_grabbed) m_lock_grabbed->notify();

  // Hold on to locks until we are told to release them
  if (m_release_locks) m_release_locks->wait_for_notification();

  m_mdl_context.release_transactional_locks();

  // Tell the main thread that grabbed lock is released.
  if (m_lock_released) m_lock_released->notify();
}

// Google Test recommends DeathTest suffix for classes use in death tests.
typedef MDLTest MDLDeathTest;

/*
  Verifies that we die with a DBUG_ASSERT if we destry a non-empty MDL_context.
 */
#if GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)
TEST_F(MDLDeathTest, DieWhenMTicketsNonempty) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_DEATH(m_mdl_context.destroy(),
               ".*Assertion.*m_ticket_store.*is_empty.*");
  m_mdl_context.release_transactional_locks();
}
#endif  // GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)

/*
  The most basic test: just construct and destruct our test fixture.
 */
TEST_F(MDLTest, ConstructAndDestruct) {}

void MDLTest::test_one_simple_shared_lock(enum_mdl_type lock_type) {
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, lock_type,
                   MDL_TRANSACTION);

  EXPECT_EQ(lock_type, m_request.type);
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_NE(m_null_ticket, m_request.ticket);
  EXPECT_TRUE(m_mdl_context.has_locks());
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, lock_type));

  MDL_request request_2;
  MDL_REQUEST_INIT_BY_KEY(&request_2, &m_request.key, lock_type,
                          MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request_2));
  EXPECT_EQ(m_request.ticket, request_2.ticket);

  m_mdl_context.release_transactional_locks();
  EXPECT_FALSE(m_mdl_context.has_locks());
}

/*
  Acquires one lock of type MDL_SHARED.
 */
TEST_F(MDLTest, OneShared) { test_one_simple_shared_lock(MDL_SHARED); }

/*
  Acquires one lock of type MDL_SHARED_HIGH_PRIO.
 */
TEST_F(MDLTest, OneSharedHighPrio) {
  test_one_simple_shared_lock(MDL_SHARED_HIGH_PRIO);
}

/*
  Acquires one lock of type MDL_SHARED_READ.
 */
TEST_F(MDLTest, OneSharedRead) { test_one_simple_shared_lock(MDL_SHARED_READ); }

/*
  Acquires one lock of type MDL_SHARED_WRITE.
 */
TEST_F(MDLTest, OneSharedWrite) {
  test_one_simple_shared_lock(MDL_SHARED_WRITE);
}

/*
  Acquires one lock of type MDL_EXCLUSIVE.
 */
TEST_F(MDLTest, OneExclusive) {
  const enum_mdl_type lock_type = MDL_EXCLUSIVE;
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, lock_type,
                   MDL_TRANSACTION);
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  m_request_list.push_front(&m_request);
  m_request_list.push_front(&m_global_request);

  EXPECT_FALSE(
      m_mdl_context.acquire_locks_nsec(&m_request_list, long_timeout_nsec));

  EXPECT_NE(m_null_ticket, m_request.ticket);
  EXPECT_NE(m_null_ticket, m_global_request.ticket);
  EXPECT_TRUE(m_mdl_context.has_locks());
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, lock_type));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::GLOBAL, "", "", MDL_INTENTION_EXCLUSIVE));
  EXPECT_TRUE(m_request.ticket->is_upgradable_or_exclusive());

  m_mdl_context.release_transactional_locks();
  EXPECT_FALSE(m_mdl_context.has_locks());
}

/*
  Acquires two locks, on different tables, of type MDL_SHARED.
  Verifies that they are independent.
 */
TEST_F(MDLTest, TwoShared) {
  MDL_request request_2;
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_EXPLICIT);
  MDL_REQUEST_INIT(&request_2, MDL_key::TABLE, db_name, table_name2, MDL_SHARED,
                   MDL_EXPLICIT);

  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request_2));
  EXPECT_TRUE(m_mdl_context.has_locks());
  ASSERT_NE(m_null_ticket, m_request.ticket);
  ASSERT_NE(m_null_ticket, request_2.ticket);

  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name2, MDL_SHARED));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name3, MDL_SHARED));

  m_mdl_context.release_lock(m_request.ticket);
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.has_locks());

  m_mdl_context.release_lock(request_2.ticket);
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name2, MDL_SHARED));
  EXPECT_FALSE(m_mdl_context.has_locks());
}

/*
  Verifies that two different contexts can acquire a shared lock
  on the same table.
 */
TEST_F(MDLTest, SharedLocksBetweenContexts) {
  MDL_context mdl_context2;
  mdl_context2.init(this);
  MDL_request request_2;
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);
  MDL_REQUEST_INIT(&request_2, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_FALSE(mdl_context2.try_acquire_lock(&request_2));

  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_TRUE(mdl_context2.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));

  m_mdl_context.release_transactional_locks();
  mdl_context2.release_transactional_locks();
}

/*
  Verifies that we can upgrade a shared lock to exclusive.
 */
TEST_F(MDLTest, UpgradeSharedUpgradable) {
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);

  m_request_list.push_front(&m_request);
  m_request_list.push_front(&m_global_request);

  EXPECT_FALSE(
      m_mdl_context.acquire_locks_nsec(&m_request_list, long_timeout_nsec));
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));
  EXPECT_EQ(MDL_EXCLUSIVE, m_request.ticket->get_type());

  // Another upgrade should be a no-op.
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));
  EXPECT_EQ(MDL_EXCLUSIVE, m_request.ticket->get_type());

  m_mdl_context.release_transactional_locks();
}

/*
  Verfies that locks are released when we roll back to a savepoint.
 */
TEST_F(MDLTest, SavePoint) {
  MDL_request request_2;
  MDL_request request_3;
  MDL_request request_4;
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);
  MDL_REQUEST_INIT(&request_2, MDL_key::TABLE, db_name, table_name2, MDL_SHARED,
                   MDL_TRANSACTION);
  MDL_REQUEST_INIT(&request_3, MDL_key::TABLE, db_name, table_name3, MDL_SHARED,
                   MDL_TRANSACTION);
  MDL_REQUEST_INIT(&request_4, MDL_key::TABLE, db_name, table_name4, MDL_SHARED,
                   MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request_2));
  MDL_savepoint savepoint = m_mdl_context.mdl_savepoint();
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request_3));
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request_4));

  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name2, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name3, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name4, MDL_SHARED));

  m_mdl_context.rollback_to_savepoint(savepoint);
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name2, MDL_SHARED));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name3, MDL_SHARED));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name4, MDL_SHARED));

  m_mdl_context.release_transactional_locks();
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name2, MDL_SHARED));
}

/*
  Verifies that we can grab shared locks concurrently, in different threads.
 */
TEST_F(MDLTest, ConcurrentShared) {
  Notification lock_grabbed;
  Notification release_locks;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, &release_locks,
                        nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED));

  release_locks.notify();
  mdl_thread.join();

  m_mdl_context.release_transactional_locks();
}

/*
  Verifies that we cannot grab an exclusive lock on something which
  is locked with a shared lock in a different thread.
 */
TEST_F(MDLTest, ConcurrentSharedExclusive) {
  expected_error = ER_LOCK_WAIT_TIMEOUT;

  Notification lock_grabbed;
  Notification release_locks;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, &release_locks,
                        nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  m_request_list.push_front(&m_request);
  m_request_list.push_front(&m_global_request);

  // We should *not* be able to grab the lock here.
  EXPECT_TRUE(m_mdl_context.acquire_locks_nsec(&m_request_list, zero_timeout));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  release_locks.notify();
  mdl_thread.join();

  // Now we should be able to grab the lock.
  EXPECT_FALSE(m_mdl_context.acquire_locks_nsec(&m_request_list, zero_timeout));
  EXPECT_NE(m_null_ticket, m_request.ticket);

  m_mdl_context.release_transactional_locks();
}

/*
  Verifies that we cannot we cannot grab a shared lock on something which
  is locked exlusively in a different thread.
 */
TEST_F(MDLTest, ConcurrentExclusiveShared) {
  Notification lock_grabbed;
  Notification release_locks;
  MDL_thread mdl_thread(table_name1, MDL_EXCLUSIVE, &lock_grabbed,
                        &release_locks, nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);

  // We should *not* be able to grab the lock here.
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  release_locks.notify();

  // The other thread should eventually release its locks.
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));
  EXPECT_NE(m_null_ticket, m_request.ticket);

  mdl_thread.join();
  m_mdl_context.release_transactional_locks();
}

/*
  Verifies the following scenario:
  Thread 1: grabs a shared upgradable lock.
  Thread 2: grabs a shared lock.
  Thread 1: asks for an upgrade to exclusive (needs to wait for thread 2)
  Thread 2: gets notified, and releases lock.
  Thread 1: gets the exclusive lock.
 */
TEST_F(MDLTest, ConcurrentUpgrade) {
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);
  m_request_list.push_front(&m_request);
  m_request_list.push_front(&m_global_request);

  EXPECT_FALSE(
      m_mdl_context.acquire_locks_nsec(&m_request_list, long_timeout_nsec));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_SHARED_UPGRADABLE));
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  Notification lock_grabbed;
  Notification release_locks;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, &release_locks,
                        nullptr, nullptr);
  mdl_thread.enable_release_on_notify();
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  mdl_thread.join();
  m_mdl_context.release_transactional_locks();
}

TEST_F(MDLTest, UpgradableConcurrency) {
  MDL_request request_2;
  MDL_request_list request_list;
  Notification lock_grabbed;
  Notification release_locks;
  MDL_thread mdl_thread(table_name1, MDL_SHARED_UPGRADABLE, &lock_grabbed,
                        &release_locks, nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  // We should be able to take a SW lock.
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_WRITE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_NE(m_null_ticket, m_request.ticket);

  // But SHARED_UPGRADABLE is not compatible with itself
  expected_error = ER_LOCK_WAIT_TIMEOUT;
  MDL_REQUEST_INIT(&request_2, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);
  request_list.push_front(&m_global_request);
  request_list.push_front(&request_2);
  EXPECT_TRUE(m_mdl_context.acquire_locks_nsec(&request_list, zero_timeout));
  EXPECT_EQ(m_null_ticket, request_2.ticket);

  release_locks.notify();

  mdl_thread.join();
  m_mdl_context.release_transactional_locks();
}

/*
  Test compatibility matrice for MDL_SHARED_WRITE_LOW_PRIO lock.
*/

TEST_F(MDLTest, SharedWriteLowPrioCompatibility) {
  enum_mdl_type compatible[] = {
      MDL_SHARED,       MDL_SHARED_HIGH_PRIO,      MDL_SHARED_READ,
      MDL_SHARED_WRITE, MDL_SHARED_WRITE_LOW_PRIO, MDL_SHARED_UPGRADABLE};
  enum_mdl_type incompatible[] = {MDL_SHARED_READ_ONLY, MDL_SHARED_NO_WRITE,
                                  MDL_SHARED_NO_READ_WRITE, MDL_EXCLUSIVE};
  enum_mdl_type higher_prio[] = {MDL_SHARED_READ_ONLY, MDL_SHARED_NO_WRITE,
                                 MDL_SHARED_NO_READ_WRITE, MDL_EXCLUSIVE};
  Notification lock_grabbed;
  Notification release_lock;
  MDL_thread mdl_thread(table_name1, MDL_SHARED_WRITE_LOW_PRIO, &lock_grabbed,
                        &release_lock, nullptr, nullptr);
  uint i;

  // Start thread which will acquire SWLP lock and pause.
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  // We should be able to take all locks from compatible list
  for (i = 0; i < sizeof(compatible) / sizeof(enum_mdl_type); ++i) {
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     compatible[i], MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_NE(m_null_ticket, m_request.ticket);
    m_mdl_context.release_transactional_locks();
  }

  // But none of the locks from incompatible list
  for (i = 0; i < sizeof(incompatible) / sizeof(enum_mdl_type); ++i) {
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     incompatible[i], MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);
  }

  release_lock.notify();
  mdl_thread.join();

  // Check that SWLP lock can be acquired when any of compatible locks is active
  for (i = 0; i < sizeof(compatible) / sizeof(enum_mdl_type); ++i) {
    Notification second_grabbed;
    Notification second_release;
    MDL_thread mdl_thread2(table_name1, compatible[i], &second_grabbed,
                           &second_release, nullptr, nullptr);

    // Start thread that will acquire one of locks from compatible list
    mdl_thread2.start();
    second_grabbed.wait_for_notification();

    // Acquisition of SWLP should succeed
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_WRITE_LOW_PRIO, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_NE(m_null_ticket, m_request.ticket);
    m_mdl_context.release_transactional_locks();

    second_release.notify();
    mdl_thread2.join();
  }

  /*
    Check that SWLP lock can't be acquired when any of incompatible locks
    is active.
  */
  for (i = 0; i < sizeof(incompatible) / sizeof(enum_mdl_type); ++i) {
    Notification third_grabbed;
    Notification third_release;
    MDL_thread mdl_thread3(table_name1, incompatible[i], &third_grabbed,
                           &third_release, nullptr, nullptr);

    // Start thread that will acquire one of locks from incompatible list
    mdl_thread3.start();
    third_grabbed.wait_for_notification();

    // Acquisition of SWLP should fail.
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_WRITE_LOW_PRIO, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);

    third_release.notify();
    mdl_thread3.join();
  }

  /*
    Check that SWLP lock can't be acquired if one of higher-prio locks is
    pending.
  */
  for (i = 0; i < sizeof(higher_prio) / sizeof(enum_mdl_type); ++i) {
    Notification fourth_grabbed;
    Notification fourth_release;
    Notification fifth_blocked;
    MDL_thread mdl_thread4(table_name1, MDL_SHARED_WRITE, &fourth_grabbed,
                           &fourth_release, nullptr, nullptr);
    MDL_thread mdl_thread5(table_name1, higher_prio[i], nullptr, nullptr,
                           &fifth_blocked, nullptr);

    // Acquire SW lock on the table.
    mdl_thread4.start();
    fourth_grabbed.wait_for_notification();

    // Ensure that there is pending high-prio lock.
    mdl_thread5.start();
    fifth_blocked.wait_for_notification();

    // Acquisition of SWLP should fail because there is pending high-prio lock.
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_WRITE_LOW_PRIO, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);

    fourth_release.notify();
    mdl_thread4.join();
    mdl_thread5.join();
  }

  /*
    Check that higher-prio locks can be acquired even if there
    is pending SWLP lock.
  */
  for (i = 0; i < sizeof(higher_prio) / sizeof(enum_mdl_type); ++i) {
    Notification sixth_grabbed;
    Notification sixth_release;
    Notification seventh_blocked;
    Notification seventh_grabbed;
    Notification eighth_blocked;
    Notification eighth_release;
    MDL_thread mdl_thread6(table_name1, MDL_EXCLUSIVE, &sixth_grabbed,
                           &sixth_release, nullptr, nullptr);
    MDL_thread mdl_thread7(table_name1, higher_prio[i], &seventh_grabbed,
                           nullptr, &seventh_blocked, nullptr);
    MDL_thread mdl_thread8(table_name1, MDL_SHARED_WRITE_LOW_PRIO, nullptr,
                           &eighth_release, &eighth_blocked, nullptr);

    // Acquire X lock on the table.
    mdl_thread6.start();
    sixth_grabbed.wait_for_notification();

    // Ensure that there is pending high-prio lock.
    mdl_thread7.start();
    seventh_blocked.wait_for_notification();

    // Ensure that there is pending SWLP after it.
    mdl_thread8.start();
    eighth_blocked.wait_for_notification();

    // Release X lock.
    sixth_release.notify();
    mdl_thread6.join();

    /*
      This should unblock high-prio lock and not SWLP (otherwise we will
      wait for SWLP release.
    */
    seventh_grabbed.wait_for_notification();
    mdl_thread7.join();

    // After this SWLP lock will be granted and can be released
    eighth_release.notify();
    mdl_thread8.join();
  }
}

/*
  Test compatibility matrice for MDL_SHARED_READ_ONLY lock.
*/

TEST_F(MDLTest, SharedReadOnlyCompatibility) {
  enum_mdl_type compatible[] = {MDL_SHARED,           MDL_SHARED_HIGH_PRIO,
                                MDL_SHARED_READ,      MDL_SHARED_UPGRADABLE,
                                MDL_SHARED_READ_ONLY, MDL_SHARED_NO_WRITE};
  enum_mdl_type incompatible[] = {MDL_SHARED_WRITE, MDL_SHARED_WRITE_LOW_PRIO,
                                  MDL_SHARED_NO_READ_WRITE, MDL_EXCLUSIVE};
  enum_mdl_type higher_prio[] = {MDL_SHARED_WRITE, MDL_SHARED_NO_READ_WRITE,
                                 MDL_EXCLUSIVE};
  Notification lock_grabbed;
  Notification release_lock;
  MDL_thread mdl_thread(table_name1, MDL_SHARED_READ_ONLY, &lock_grabbed,
                        &release_lock, nullptr, nullptr);
  uint i;

  // Start thread which will acquire SRO lock and pause.
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  // We should be able to take all locks from compatible list
  for (i = 0; i < sizeof(compatible) / sizeof(enum_mdl_type); ++i) {
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     compatible[i], MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_NE(m_null_ticket, m_request.ticket);
    m_mdl_context.release_transactional_locks();
  }

  // But none of the locks from incompatible list
  for (i = 0; i < sizeof(incompatible) / sizeof(enum_mdl_type); ++i) {
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     incompatible[i], MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);
  }

  release_lock.notify();
  mdl_thread.join();

  // Check that SRO lock can be acquired when any of compatible locks is active
  for (i = 0; i < sizeof(compatible) / sizeof(enum_mdl_type); ++i) {
    Notification second_grabbed;
    Notification second_release;
    MDL_thread mdl_thread2(table_name1, compatible[i], &second_grabbed,
                           &second_release, nullptr, nullptr);

    // Start thread that will acquire one of locks from compatible list
    mdl_thread2.start();
    second_grabbed.wait_for_notification();

    // Acquisition of SRO should succeed
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_READ_ONLY, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_NE(m_null_ticket, m_request.ticket);
    m_mdl_context.release_transactional_locks();

    second_release.notify();
    mdl_thread2.join();
  }

  /*
    Check that SRO lock can't be acquired when any of incompatible locks
    is active.
  */
  for (i = 0; i < sizeof(incompatible) / sizeof(enum_mdl_type); ++i) {
    Notification third_grabbed;
    Notification third_release;
    MDL_thread mdl_thread3(table_name1, incompatible[i], &third_grabbed,
                           &third_release, nullptr, nullptr);

    // Start thread that will acquire one of locks from incompatible list
    mdl_thread3.start();
    third_grabbed.wait_for_notification();

    // Acquisition of SRO should fail.
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_READ_ONLY, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);

    third_release.notify();
    mdl_thread3.join();
  }

  /*
    Check that SRO lock can't be acquired if one of higher-prio locks is
    pending.
  */
  for (i = 0; i < sizeof(higher_prio) / sizeof(enum_mdl_type); ++i) {
    Notification fourth_grabbed;
    Notification fourth_release;
    Notification fifth_blocked;
    MDL_thread mdl_thread4(table_name1, MDL_SHARED_READ_ONLY, &fourth_grabbed,
                           &fourth_release, nullptr, nullptr);
    MDL_thread mdl_thread5(table_name1, higher_prio[i], nullptr, nullptr,
                           &fifth_blocked, nullptr);

    // Acquire SRO lock on the table.
    mdl_thread4.start();
    fourth_grabbed.wait_for_notification();

    // Ensure that there is pending high-prio lock.
    mdl_thread5.start();
    fifth_blocked.wait_for_notification();

    // Acquisition of SRO should fail because there is pending high-prio lock.
    MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_READ_ONLY, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
    EXPECT_EQ(m_null_ticket, m_request.ticket);

    fourth_release.notify();
    mdl_thread4.join();
    mdl_thread5.join();
  }

  // Check that SRO lock can be acquired if there is pending SWLP request.
  Notification sixth_grabbed;
  Notification sixth_release;
  Notification seventh_blocked;
  MDL_thread mdl_thread6(table_name1, MDL_SHARED_READ_ONLY, &sixth_grabbed,
                         &sixth_release, nullptr, nullptr);
  MDL_thread mdl_thread7(table_name1, MDL_SHARED_WRITE_LOW_PRIO, nullptr,
                         nullptr, &seventh_blocked, nullptr);

  // Acquire SRO lock on the table.
  mdl_thread6.start();
  sixth_grabbed.wait_for_notification();

  // Ensure that there is pending SWLP lock.
  mdl_thread7.start();
  seventh_blocked.wait_for_notification();

  // Acquisition of SRO should succeed despite pending SWLP is present
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_READ_ONLY, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_NE(m_null_ticket, m_request.ticket);
  m_mdl_context.release_transactional_locks();

  sixth_release.notify();
  mdl_thread6.join();
  mdl_thread7.join();

  /*
    Check that higher-prio locks can be acquired even if there
    is pending SRO lock.
  */
  for (i = 0; i < sizeof(higher_prio) / sizeof(enum_mdl_type); ++i) {
    Notification eighth_grabbed;
    Notification eighth_release;
    Notification nineth_blocked;
    Notification nineth_grabbed;
    Notification tenth_blocked;
    Notification tenth_release;
    MDL_thread mdl_thread8(table_name1, MDL_EXCLUSIVE, &eighth_grabbed,
                           &eighth_release, nullptr, nullptr);
    MDL_thread mdl_thread9(table_name1, higher_prio[i], &nineth_grabbed,
                           nullptr, &nineth_blocked, nullptr);
    MDL_thread mdl_thread10(table_name1, MDL_SHARED_READ_ONLY, nullptr,
                            &tenth_release, &tenth_blocked, nullptr);

    // Acquire X lock on the table.
    mdl_thread8.start();
    eighth_grabbed.wait_for_notification();

    // Ensure that there is pending high-prio lock.
    mdl_thread9.start();
    nineth_blocked.wait_for_notification();

    // Ensure that there is pending SRO after it.
    mdl_thread10.start();
    tenth_blocked.wait_for_notification();

    // Release X lock.
    eighth_release.notify();
    mdl_thread8.join();

    /*
      This should unblock high-prio lock and not SRO (otherwise we will
      wait for SRO release.
    */
    nineth_grabbed.wait_for_notification();
    mdl_thread9.join();

    // After this SRO lock will be granted and can be released
    tenth_release.notify();
    mdl_thread10.join();
  }

  // Check that SWLP lock can't be acquired if there is pending SRO request.
  Notification eleventh_grabbed;
  Notification eleventh_release;
  Notification twelveth_blocked;
  MDL_thread mdl_thread11(table_name1, MDL_SHARED_WRITE, &eleventh_grabbed,
                          &eleventh_release, nullptr, nullptr);
  MDL_thread mdl_thread12(table_name1, MDL_SHARED_READ_ONLY, nullptr, nullptr,
                          &twelveth_blocked, nullptr);

  // Acquire SW lock on the table.
  mdl_thread11.start();
  eleventh_grabbed.wait_for_notification();

  // Ensure that there is pending SRO lock.
  mdl_thread12.start();
  twelveth_blocked.wait_for_notification();

  // Acquisition of SWLP should fail.
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_WRITE_LOW_PRIO, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  eleventh_release.notify();
  mdl_thread11.join();
  mdl_thread12.join();
}

/*
  Verifies following scenario,
  Low priority lock requests starvation. Lock is granted to high priority
  lock request in wait queue always as max_write_lock_count is a large value.
  - max_write_lock_count == default value i.e ~(ulong)0L
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for SR lock on the table.
  - THREAD 3: Requests for SW lock on the table.
  - THREAD 4: Requests for SNRW on the table.
  - THREAD 1: Releases X lock.
  - THREAD 5: Requests for SNRW lock on the table.
  - THREAD 4: Releases SNRW lock.
  - THREAD 2,3: Check whether THREADs got lock on the table.
  Though, THREAD 2,3 requested lock before THREAD 4's SNRW lock and
  THREAD 5's SNRW lock, lock is granted for THREAD 4 and 5.
*/
TEST_F(MDLTest, HogLockTest1) {
  Notification thd_lock_grabbed[5];
  Notification thd_release_locks[5];
  Notification thd_lock_blocked[5];
  Notification thd_lock_released[5];

  /* Locks taken by the threads */
  enum { THD1_X, THD2_SR, THD3_SW, THD4_SNRW, THD5_SNRW };

  /*
    THREAD1:  Acquiring X lock on table.
    Lock Wait Queue: <empty>
    Lock granted: <empty>
  */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /*
    THREAD2:  Requesting SR lock on table.
    Lock Wait Queue: SR
    Lock granted: X
  */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD2_SR],
      &thd_release_locks[THD2_SR], &thd_lock_blocked[THD2_SR],
      &thd_lock_released[THD2_SR]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SR].wait_for_notification();

  /*
    THREAD3:  Requesting SW lock on table.
    Lock Wait Queue: SR<--SW
    Lock granted: X
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD3_SW],
      &thd_release_locks[THD3_SW], &thd_lock_blocked[THD3_SW],
      &thd_lock_released[THD3_SW]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SW].wait_for_notification();

  /*
    THREAD4:  Requesting SNRW lock on table.
    Lock Wait Queue: SR<--SW<--SNRW
    Lock granted: X
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD4_SNRW],
      &thd_release_locks[THD4_SNRW], &thd_lock_blocked[THD4_SNRW],
      &thd_lock_released[THD4_SNRW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SNRW].wait_for_notification();

  /* THREAD 1: Release X lock. */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();

  /*
    Lock Wait Queue: SR<--SW
    Lock granted: SNRW
  */
  thd_lock_grabbed[THD4_SNRW].wait_for_notification();

  /*
    THREAD 5: Requests SNRW lock on the table.
    Lock Wait Queue: SR<--SW<--SNRW
    Lock granted: SNRW
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD5_SNRW],
      &thd_release_locks[THD5_SNRW], &thd_lock_blocked[THD5_SNRW],
      &thd_lock_released[THD5_SNRW]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SNRW].wait_for_notification();

  /* THREAD 4: Release SNRW lock */
  thd_release_locks[THD4_SNRW].notify();
  thd_lock_released[THD4_SNRW].wait_for_notification();

  /* THREAD 2: Is Lock granted to me? */
  EXPECT_FALSE((mdl_thread2.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1, MDL_SHARED_READ));
  /* THREAD 3: Is Lock granted to me? */
  EXPECT_FALSE((mdl_thread3.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1, MDL_SHARED_WRITE));
  /*
    THREAD 5: Lock is granted to THREAD 5 as priority is higher.
    Lock Wait Queue: SR<--SW
    Lock granted: SNRW
  */
  thd_lock_grabbed[THD5_SNRW].wait_for_notification();
  thd_release_locks[THD5_SNRW].notify();
  thd_lock_released[THD5_SNRW].wait_for_notification();

  /*CLEANUP*/
  thd_lock_grabbed[THD2_SR].wait_for_notification();
  thd_release_locks[THD2_SR].notify();
  thd_lock_released[THD2_SR].wait_for_notification();

  thd_lock_grabbed[THD3_SW].wait_for_notification();
  thd_release_locks[THD3_SW].notify();
  thd_lock_released[THD3_SW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
}

/*
  Verifies following scenario,
  After granting max_write_lock_count(=1) number of times for high priority
  lock request, lock is granted to starving low priority lock request
  in wait queue.
  - max_write_lock_count= 1
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for SR lock on the table.
  - THREAD 3: Requests for SW lock on the table.
  - THREAD 4: Requests for SNRW on the table.
  - THREAD 1: Releases X lock. m_hog_lock_count= 1
  - THREAD 5: Requests for SNRW lock on the table.
  - THREAD 4: Releases SNRW lock.
  - THREAD 2,3: Release lock.
  While releasing X held by THREAD-1, m_hog_lock_count becomes 1 and while
  releasing SNRW lock in THREAD 4, lock is granted to starving low priority
  locks as m_hog_lock_count == max_write_lock_count.
  So THREAD 2, 3 gets lock here instead of THREAD 5.
*/
TEST_F(MDLTest, HogLockTest2) {
  Notification thd_lock_grabbed[5];
  Notification thd_release_locks[5];
  Notification thd_lock_blocked[5];
  Notification thd_lock_released[5];

  /* Locks taken by the threads */
  enum { THD1_X, THD2_SR, THD3_SW, THD4_SNRW, THD5_SNRW };

  max_write_lock_count = 1;

  /*
    THREAD1:  Acquiring X lock on table.
    Lock Wait Queue: <empty>
    Lock Granted: <empty>
  */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /*
    THREAD2:  Requesting SR lock on table.
    Lock Wait Queue: SR
    Lock Granted: X
  */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD2_SR],
      &thd_release_locks[THD2_SR], &thd_lock_blocked[THD2_SR],
      &thd_lock_released[THD2_SR]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SR].wait_for_notification();

  /*
    THREAD3:  Requesting SW lock on table.
    Lock Wait Queue: SR<--SW
    Lock Granted: X
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD3_SW],
      &thd_release_locks[THD3_SW], &thd_lock_blocked[THD3_SW],
      &thd_lock_released[THD3_SW]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SW].wait_for_notification();

  /*
    THREAD4:  Requesting SNRW lock on table.
    Lock Wait Queue: SR<--SW<--SNRW
    Lock Granted: X
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD4_SNRW],
      &thd_release_locks[THD4_SNRW], &thd_lock_blocked[THD4_SNRW],
      &thd_lock_released[THD4_SNRW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SNRW].wait_for_notification();

  /*
     THREAD 1: Release X lock.
     Lock Wait Queue: SR<--SW
     Lock Granted: SNRW
     m_hog_lock_count= 1
  */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();

  /* Lock is granted to THREAD 4 */
  thd_lock_grabbed[THD4_SNRW].wait_for_notification();

  /*
    THREAD 5: Requests SNRW lock on the table.
    Lock Wait Queue: SR<--SW<--SNRW
    Lock Granted: SNRW
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD5_SNRW],
      &thd_release_locks[THD5_SNRW], &thd_lock_blocked[THD5_SNRW],
      &thd_lock_released[THD5_SNRW]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SNRW].wait_for_notification();

  /* THREAD 4: Release SNRW lock */
  thd_release_locks[THD4_SNRW].notify();
  thd_lock_released[THD4_SNRW].wait_for_notification();

  /*
    THREAD 2: Since max_write_lock_count == m_hog_lock_count, Lock is granted to
              THREAD 2 and 3 instead of THREAD 5.
    Lock Wait Queue: SNRW
    Lock Granted: SR, SW
  */
  thd_lock_grabbed[THD2_SR].wait_for_notification();
  thd_lock_grabbed[THD3_SW].wait_for_notification();

  thd_release_locks[THD2_SR].notify();
  thd_lock_released[THD2_SR].wait_for_notification();

  thd_release_locks[THD3_SW].notify();
  thd_lock_released[THD3_SW].wait_for_notification();

  /* Cleanup */
  thd_lock_grabbed[THD5_SNRW].wait_for_notification();
  thd_release_locks[THD5_SNRW].notify();
  thd_lock_released[THD5_SNRW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
}

/*
  Verifies locks priorities,
  X has priority over--> S, SR, SW, SU, (SNW, SNRW)
  SNRW has priority over--> SR, SW
  SNW has priority over--> SW

  - max_write_lock_count contains default value i.e ~(ulong)0L
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for S lock on the table.
  - THREAD 3: Requests for SR lock on the table.
  - THREAD 4: Requests for SW lock on the table.
  - THREAD 5: Requests for SU lock on the table.
  - THREAD 6: Requests for SNRW on the table.
  - THREAD 1: Releases X lock.
              Lock is granted THREAD 2, THREAD 5.
  - THREAD 5: RELEASE SU lock.
              Lock is granted to THREAD 6.
  - THREAD 7: Requests for SNW lock on the table.
  - THREAD 6: Releases SNRW lock.
              Lock is granted to THREAD 4 & THREAD 7.
  - THREAD 4: Check whether THREAD got lock on the table.
  At each locks release, locks of equal priorities are granted.
  At the end only SW will be in wait queue as lock is granted to SNW
  lock request.
 */
TEST_F(MDLTest, LockPriorityTest) {
  Notification thd_lock_grabbed[7];
  Notification thd_release_locks[7];
  Notification thd_lock_blocked[7];
  Notification thd_lock_released[7];

  /* Locks taken by the threads */
  enum { THD1_X, THD2_S, THD3_SR, THD4_SW, THD5_SU, THD6_SNRW, THD7_SNW };

  /*THREAD1:  Acquiring X lock on table */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /*
    THREAD2:  Requesting S lock on table.
    Lock Wait Queue: S
    Lock Granted: X
  */
  MDL_thread mdl_thread2(table_name1, MDL_SHARED, &thd_lock_grabbed[THD2_S],
                         &thd_release_locks[THD2_S], &thd_lock_blocked[THD2_S],
                         &thd_lock_released[THD2_S]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_S].wait_for_notification();

  /*
    THREAD3:  Requesting SR lock on table.
    Lock Wait Queue: S<--SR
    Lock Granted: X
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD3_SR],
      &thd_release_locks[THD3_SR], &thd_lock_blocked[THD3_SR],
      &thd_lock_released[THD3_SR]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SR].wait_for_notification();

  /*
    THREAD4:  Requesting SW lock on table.
    Lock Wait Queue: S<--SR<--SW
    Lock Granted: X
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD4_SW],
      &thd_release_locks[THD4_SW], &thd_lock_blocked[THD4_SW],
      &thd_lock_released[THD4_SW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SW].wait_for_notification();

  /*
    THREAD5:  Requesting SU lock on table
    Lock Wait Queue: S<--SR<--SW<--SU
    Lock Granted: X
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_UPGRADABLE, &thd_lock_grabbed[THD5_SU],
      &thd_release_locks[THD5_SU], &thd_lock_blocked[THD5_SU],
      &thd_lock_released[THD5_SU]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SU].wait_for_notification();

  /*
    THREAD6:  Requesting SNRW lock on table
    Lock Wait Queue: S<--SR<--SW<--SU<--SNRW
    Lock Granted: X
  */
  MDL_thread mdl_thread6(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD6_SNRW],
      &thd_release_locks[THD6_SNRW], &thd_lock_blocked[THD6_SNRW],
      &thd_lock_released[THD6_SNRW]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_SNRW].wait_for_notification();

  /*
    Lock wait Queue status: S<--SR<--SW<--SU<--SNRW
    THREAD 1: Release X lock.
  */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();

  /*
    THREAD 5: Verify and Release lock.
    Lock wait Queue status: SR<--SW<--SNRW
    Lock Granted: S, SU
  */
  thd_lock_grabbed[THD2_S].wait_for_notification();
  thd_release_locks[THD2_S].notify();
  thd_lock_released[THD2_S].wait_for_notification();

  thd_lock_grabbed[THD5_SU].wait_for_notification();
  thd_release_locks[THD5_SU].notify();
  thd_lock_released[THD5_SU].wait_for_notification();

  /* Now Lock Granted to THREAD 6 SNRW lock type request*/
  thd_lock_grabbed[THD6_SNRW].wait_for_notification();

  /*
    THREAD 7: Requests SNW lock on the table.
    Lock wait Queue status: SR<--SW<--SNW
    Lock Granted: SNRW
  */
  MDL_thread mdl_thread7(
      table_name1, MDL_SHARED_NO_WRITE, &thd_lock_grabbed[THD7_SNW],
      &thd_release_locks[THD7_SNW], &thd_lock_blocked[THD7_SNW],
      &thd_lock_released[THD7_SNW]);
  mdl_thread7.start();
  thd_lock_blocked[THD7_SNW].wait_for_notification();

  /* THREAD 6: Release SNRW lock */
  thd_release_locks[THD6_SNRW].notify();
  thd_lock_released[THD6_SNRW].wait_for_notification();

  /* Now lock is granted to THREAD 3 & 7 */
  thd_lock_grabbed[THD7_SNW].wait_for_notification();
  thd_lock_grabbed[THD3_SR].wait_for_notification();

  /*
    THREAD 3: Release SR lock
    Lock wait Queue status: SW
    Lock Granted: SR, SNW
  */
  thd_release_locks[THD3_SR].notify();
  thd_lock_released[THD3_SR].wait_for_notification();

  /* THREAD 4: Verify whether lock is granted or not*/
  EXPECT_FALSE((mdl_thread4.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1, MDL_SHARED_WRITE));

  /*CLEANUP*/
  thd_release_locks[THD7_SNW].notify();
  thd_lock_released[THD7_SNW].wait_for_notification();

  thd_lock_grabbed[THD4_SW].wait_for_notification();
  thd_release_locks[THD4_SW].notify();
  thd_lock_released[THD4_SW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
  mdl_thread7.join();
}

/*
  Verifies locks priorities when max_write_lock_count= 1
  X has priority over--> S, SR, SW, SU, (SNW, SNRW)
  SNRW has priority over--> SR, SW
  SNW has priority over--> SW

  - max_write_lock_count= 1
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for S lock on the table.
  - THREAD 3: Requests for SR lock on the table.
  - THREAD 4: Requests for SW lock on the table.
  - THREAD 5: Requests for SU lock on the table.
  - THREAD 6: Requests for X on the table.
  - THREAD 1: Releases X lock.
              Lock is granted THREAD 6.
  - THREAD 7: Requests SNRW lock.
  - THREAD 6: Releases X lock.
              Lock is granted to THREAD 2,3,4,5.
  - THREAD 7: Check Whether lock is granted or not.
 */
TEST_F(MDLTest, HogLockTest3) {
  Notification thd_lock_grabbed[7];
  Notification thd_release_locks[7];
  Notification thd_lock_blocked[7];
  Notification thd_lock_released[7];

  enum { THD1_X, THD2_S, THD3_SR, THD4_SW, THD5_SU, THD6_X, THD7_SNRW };

  max_write_lock_count = 1;

  /* THREAD1: Acquiring X lock on table. */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /*
    THREAD2: Requesting S lock on table.
    Lock Wait Queue: S
    Lock Granted: X
  */
  MDL_thread mdl_thread2(table_name1, MDL_SHARED, &thd_lock_grabbed[THD2_S],
                         &thd_release_locks[THD2_S], &thd_lock_blocked[THD2_S],
                         &thd_lock_released[THD2_S]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_S].wait_for_notification();

  /*
    THREAD3: Requesting SR lock on table.
    Lock Wait Queue: S<--SR
    Lock Granted: X
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD3_SR],
      &thd_release_locks[THD3_SR], &thd_lock_blocked[THD3_SR],
      &thd_lock_released[THD3_SR]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SR].wait_for_notification();

  /*
    THREAD4: Requesting SW lock on table.
    Lock Wait Queue: S<--SR<--SW.
    Lock Granted: X
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD4_SW],
      &thd_release_locks[THD4_SW], &thd_lock_blocked[THD4_SW],
      &thd_lock_released[THD4_SW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SW].wait_for_notification();

  /*
    THREAD5: Requesting SU lock on table.
    Lock Wait Queue: S<--SR<--SW<--SU
    Lock Granted: X
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_UPGRADABLE, &thd_lock_grabbed[THD5_SU],
      &thd_release_locks[THD5_SU], &thd_lock_blocked[THD5_SU],
      &thd_lock_released[THD5_SU]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SU].wait_for_notification();

  /*
    THREAD6: Requesting X lock on table
    Lock Wait Queue: S<--SR<--SW<--SU<--X
    Lock Granted: X
  */
  MDL_thread mdl_thread6(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD6_X],
                         &thd_release_locks[THD6_X], &thd_lock_blocked[THD6_X],
                         &thd_lock_released[THD6_X]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_X].wait_for_notification();

  /*
    Lock wait Queue status: S<--SR<--SW<--SU<--X
    Lock Granted: X
    THREAD 1: Release X lock.
  */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();

  /* Lock is granted to THREAD 6*/
  thd_lock_grabbed[THD6_X].wait_for_notification();

  /*
    THREAD7:  Requesting SNRW lock on table
    Lock wait Queue status: S<--SR<--SW<--SU
    Lock Granted: X
  */
  MDL_thread mdl_thread7(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD7_SNRW],
      &thd_release_locks[THD7_SNRW], &thd_lock_blocked[THD7_SNRW],
      &thd_lock_released[THD7_SNRW]);
  mdl_thread7.start();
  thd_lock_blocked[THD7_SNRW].wait_for_notification();

  /* THREAD 6: Release X lock. */
  thd_release_locks[THD6_X].notify();
  thd_lock_released[THD6_X].wait_for_notification();

  /* Lock is granted to THREAD 2, 3, 4, 5*/
  thd_lock_grabbed[THD2_S].wait_for_notification();
  thd_lock_grabbed[THD3_SR].wait_for_notification();
  thd_lock_grabbed[THD4_SW].wait_for_notification();
  thd_lock_grabbed[THD5_SU].wait_for_notification();

  /*
    Lock wait Queue status: <empty>
    Lock Granted: <empty>
    THREAD 7: high priority SNRW lock is still waiting.
  */
  EXPECT_FALSE((mdl_thread7.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1,
                                                MDL_SHARED_NO_READ_WRITE));

  /* CLEAN UP */
  thd_release_locks[THD2_S].notify();
  thd_lock_released[THD2_S].wait_for_notification();

  thd_release_locks[THD3_SR].notify();
  thd_lock_released[THD3_SR].wait_for_notification();

  thd_release_locks[THD4_SW].notify();
  thd_lock_released[THD4_SW].wait_for_notification();

  thd_release_locks[THD5_SU].notify();
  thd_lock_released[THD5_SU].wait_for_notification();

  thd_lock_grabbed[THD7_SNRW].wait_for_notification();
  thd_release_locks[THD7_SNRW].notify();
  thd_lock_released[THD7_SNRW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
  mdl_thread7.join();
}

/*
  Verifies whether m_hog_lock_count is resets or not,
  when there are no low priority lock request.

  - max_write_lock_count= 1
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for SU lock on the table.
  - THREAD 3: Requests for X lock on the table.
  - THREAD 1: Releases X lock.
              Lock is granted to THREAD 3
              m_hog_lock_count= 1;
  - THREAD 3: Releases X lock.
              Lock is granted to THRED 2.
              m_hog_lock_count= 0;
  - THREAD 4: Requests for SNRW lock.
  - THREAD 5: Requests for R lock.
  - THREAD 2: Releases SU lock.
              Lock is granted to THREAD 4.
 */
TEST_F(MDLTest, HogLockTest4) {
  Notification thd_lock_grabbed[5];
  Notification thd_release_locks[5];
  Notification thd_lock_blocked[5];
  Notification thd_lock_released[5];

  /* Locks taken by the threads */
  enum { THD1_X, THD2_SU, THD3_X, THD4_SNRW, THD5_SR };

  max_write_lock_count = 1;

  /* THREAD1:  Acquiring X lock on table */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /* THREAD2:  Requesting SU lock on table */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_UPGRADABLE, &thd_lock_grabbed[THD2_SU],
      &thd_release_locks[THD2_SU], &thd_lock_blocked[THD2_SU],
      &thd_lock_released[THD2_SU]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SU].wait_for_notification();

  /* THREAD3:  Requesting X lock on table */
  MDL_thread mdl_thread3(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD3_X],
                         &thd_release_locks[THD3_X], &thd_lock_blocked[THD3_X],
                         &thd_lock_released[THD3_X]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_X].wait_for_notification();

  /*
    THREAD1: Release X lock.
    Lock Request Queue: SU<--X
    Lock Grant: X
    m_hog_lock_count= 1
  */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();
  /* Lock is granted to THREAD 3 */
  thd_lock_grabbed[THD3_X].wait_for_notification();

  /*
    THREAD3: Release X lock.
    Lock Request Queue: <empty>
    Lock Grant: SU
    m_hog_lock_count= 0
  */
  thd_release_locks[THD3_X].notify();
  thd_lock_released[THD3_X].wait_for_notification();
  /*Lock is granted to THREAD 2 */
  thd_lock_grabbed[THD2_SU].wait_for_notification();

  /*
    THREAD4: Requesting SNRW lock on table.
    Lock Request Queue: SNRW
    Lock Grant: SU
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD4_SNRW],
      &thd_release_locks[THD4_SNRW], &thd_lock_blocked[THD4_SNRW],
      &thd_lock_released[THD4_SNRW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SNRW].wait_for_notification();

  /*
    THREAD5: Requesting SR lock on table.
    Lock Request Queue: SNRW<--SR
    Lock Grant: SU
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD5_SR],
      &thd_release_locks[THD5_SR], &thd_lock_blocked[THD5_SR],
      &thd_lock_released[THD5_SR]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SR].wait_for_notification();

  /* THREAD 2: Release lock. */
  thd_release_locks[THD2_SU].notify();
  thd_lock_released[THD2_SU].wait_for_notification();

  /*
    Lock Request Queue: SR
    Lock Grant: SNRW
    Lock is granted to THREAD 5 if m_hog_lock_count is not reset.
  */
  thd_lock_grabbed[THD4_SNRW].wait_for_notification();

  /* THREAD5: Lock is not granted */
  EXPECT_FALSE((mdl_thread5.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1, MDL_SHARED_READ));

  /* CLEAN UP */
  thd_release_locks[THD4_SNRW].notify();
  thd_lock_released[THD4_SNRW].wait_for_notification();

  thd_lock_grabbed[THD5_SR].wait_for_notification();
  thd_release_locks[THD5_SR].notify();
  thd_lock_released[THD5_SR].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
}

/*
  Verifies resetting of m_hog_lock_count when only few of
  the waiting low priority locks are granted and queue has
  some more low priority lock requests in queue.
  m_hog_lock_count should not be reset to 0 when few low priority
  lock requests are granted.

  - max_write_lock_count= 1
  - THREAD 1: Acquires X lock on the table.
  - THREAD 2: Requests for SNW lock on the table.
  - THREAD 3: Requests for SR lock on the table.
  - THREAD 4: Requests for SW lock on the table.
  - THREAD 5: Requests for SU lock on the table.
  - THREAD 1: Releases X lock.
              Lock is granted THREAD 2, 3 as they are of same priority.
  - THREAD 6: Requests for SNRW lock.
  - THREAD 2: Releases SNW lock.
              Lock shoule be granted to THREAD 4, 5 as
              m_hog_lock_count == max_write_lock_count.
  - THREAD 3: Check Whether lock is granted or not.
 */
TEST_F(MDLTest, HogLockTest5) {
  Notification thd_lock_grabbed[6];
  Notification thd_release_locks[6];
  Notification thd_lock_blocked[6];
  Notification thd_lock_released[6];

  /* Locks taken by the threads */
  enum { THD1_X, THD2_SNW, THD3_SR, THD4_SW, THD5_SU, THD6_SNRW };
  max_write_lock_count = 1;

  /* THREAD1:  Acquiring X lock on table. */
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &thd_lock_grabbed[THD1_X],
                         &thd_release_locks[THD1_X], &thd_lock_blocked[THD1_X],
                         &thd_lock_released[THD1_X]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_X].wait_for_notification();

  /* THREAD2:  Requesting SNW lock on table. */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_NO_WRITE, &thd_lock_grabbed[THD2_SNW],
      &thd_release_locks[THD2_SNW], &thd_lock_blocked[THD2_SNW],
      &thd_lock_released[THD2_SNW]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SNW].wait_for_notification();

  /* THREAD3:  Requesting SR lock on table. */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD3_SR],
      &thd_release_locks[THD3_SR], &thd_lock_blocked[THD3_SR],
      &thd_lock_released[THD3_SR]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SR].wait_for_notification();

  /* THREAD4:  Requesting SW lock on table. */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD4_SW],
      &thd_release_locks[THD4_SW], &thd_lock_blocked[THD4_SW],
      &thd_lock_released[THD4_SW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SW].wait_for_notification();

  /* THREAD5:  Requesting SNW lock on table. */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_UPGRADABLE, &thd_lock_grabbed[THD5_SU],
      &thd_release_locks[THD5_SU], &thd_lock_blocked[THD5_SU],
      &thd_lock_released[THD5_SU]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SU].wait_for_notification();

  /*
    Lock wait Queue status: SNW<--SR<--SW<--SU
    Lock Granted: X
    THREAD 1: Release X lock.
  */
  thd_release_locks[THD1_X].notify();
  thd_lock_released[THD1_X].wait_for_notification();

  /*
    Lock wait Queue status: SW<--SU
    Lock Granted: SR, SNW
    Lock is granted for Thread 2, 3
  */
  thd_lock_grabbed[THD2_SNW].wait_for_notification();
  thd_lock_grabbed[THD3_SR].wait_for_notification();

  /*
    THREAD5:  Requesting SNRW lock on table.
    Lock wait Queue status: SW<--SU<--SNRW
    Lock Granted: SR, SNW
  */
  MDL_thread mdl_thread6(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD6_SNRW],
      &thd_release_locks[THD6_SNRW], &thd_lock_blocked[THD6_SNRW],
      &thd_lock_released[THD6_SNRW]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_SNRW].wait_for_notification();

  /* Thread 2: Release SNW lock */
  thd_release_locks[THD2_SNW].notify();
  thd_lock_released[THD2_SNW].wait_for_notification();

  /*
    Lock wait Queue status: SNRW
    Lock Granted: SR, SW, SU
    Lock is granted to Thread 4,5 instead of Thread 6
    THREAD6: Lock is not granted
  */
  EXPECT_FALSE((mdl_thread6.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1,
                                                MDL_SHARED_NO_READ_WRITE));

  thd_lock_grabbed[THD4_SW].wait_for_notification();
  thd_release_locks[THD4_SW].notify();
  thd_lock_released[THD4_SW].wait_for_notification();

  thd_lock_grabbed[THD5_SU].wait_for_notification();
  thd_release_locks[THD5_SU].notify();
  thd_lock_released[THD5_SU].wait_for_notification();

  /* CLEANUP */
  thd_release_locks[THD3_SR].notify();
  thd_lock_released[THD3_SR].wait_for_notification();
  thd_lock_grabbed[THD6_SNRW].wait_for_notification();
  thd_release_locks[THD6_SNRW].notify();
  thd_lock_released[THD6_SNRW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
}

/*
  Verifies that pending "obtrusive" lock correctly blocks later
  "unobtrusive" lock, which can't use "fast path" in this case.

  Also verifies that release of "fast path" "unobtrusive" lock
  unblocks waiting "obtrusive" lock and release of "obtrusive" lock
  unblocks pending "unobtrusive" lock.
*/

TEST_F(MDLTest, ConcurrentSharedExclusiveShared) {
  Notification first_shared_grabbed;
  Notification first_shared_release;
  Notification exclusive_blocked;
  Notification exclusive_grabbed;
  Notification exclusive_release;
  Notification second_shared_grabbed;
  Notification second_shared_blocked;
  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &first_shared_grabbed,
                         &first_shared_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name1, MDL_EXCLUSIVE, &exclusive_grabbed,
                         &exclusive_release, &exclusive_blocked, nullptr);
  MDL_thread mdl_thread3(table_name1, MDL_SHARED, &second_shared_grabbed,
                         nullptr, &second_shared_blocked, nullptr);

  /* Start thread which will acquire S lock. */
  mdl_thread1.start();
  first_shared_grabbed.wait_for_notification();

  /* Start thread which will try to acquire X lock and will block. */
  mdl_thread2.start();
  exclusive_blocked.wait_for_notification();

  /*
    Start thread which will try to acquire another S lock.
    It should not use "fast path" because of pending X lock
    and should be blocked.
  */
  mdl_thread3.start();
  second_shared_blocked.wait_for_notification();

  /*
    Unblock 2nd thread by releasing the first S lock.
    Here S lock acquiring on "fast path" should unblock
    pending "obtrusive" - X lock.
  */
  first_shared_release.notify();
  exclusive_grabbed.wait_for_notification();

  /*
    Release X lock and thus unblock the second S lock.
    This tests that release of X lock which uses "slow path"
    unblocks pending locks.
  */
  exclusive_release.notify();
  second_shared_grabbed.wait_for_notification();

  /* Wrap-up. */
  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
}

/*
  Verify that active X lock will block incoming X lock.
  This also covers general situation when "obtrusive" lock is
  acquired when other "obtrusive" lock is active.
  Also covers case when release of "obtrusive" lock
  unblocks another "obtrusive" lock.
*/

TEST_F(MDLTest, ConcurrentExclusiveExclusive) {
  Notification first_exclusive_grabbed;
  Notification first_exclusive_release;
  Notification second_exclusive_blocked;
  Notification second_exclusive_grabbed;
  MDL_thread mdl_thread1(table_name1, MDL_EXCLUSIVE, &first_exclusive_grabbed,
                         &first_exclusive_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name1, MDL_EXCLUSIVE, &second_exclusive_grabbed,
                         nullptr, &second_exclusive_blocked, nullptr);

  /* Start thread which will acquire X lock. */
  mdl_thread1.start();
  first_exclusive_grabbed.wait_for_notification();

  /* Start thread which will try to acquire X lock and will block. */
  mdl_thread2.start();
  second_exclusive_blocked.wait_for_notification();

  /* Releasing the first lock. This should unblock the second X request.*/
  first_exclusive_release.notify();
  second_exclusive_grabbed.wait_for_notification();

  /* Wrap-up. */
  mdl_thread1.join();
  mdl_thread2.join();
}

/*
  Verify that during rescheduling we correctly handle
  situation when still there are conflicting "fast path"
  lock (i.e. test that MDL_lock::can_grant_lock() correctly
  works in this case).
*/

TEST_F(MDLTest, ConcurrentSharedSharedExclusive) {
  Notification first_shared_grabbed;
  Notification first_shared_release;
  Notification second_shared_grabbed;
  Notification second_shared_release;
  Notification exclusive_blocked;
  Notification exclusive_grabbed;
  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &first_shared_grabbed,
                         &first_shared_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name1, MDL_SHARED, &second_shared_grabbed,
                         &second_shared_release, nullptr, nullptr);
  MDL_thread mdl_thread3(table_name1, MDL_EXCLUSIVE, &exclusive_grabbed,
                         nullptr, &exclusive_blocked, nullptr);

  /* Start two threads which will acquire S locks. */
  mdl_thread1.start();
  first_shared_grabbed.wait_for_notification();

  mdl_thread2.start();
  second_shared_grabbed.wait_for_notification();

  /* Start 3rd thread which will try to acquire X lock and will block. */
  mdl_thread3.start();
  exclusive_blocked.wait_for_notification();

  /* Release one S lock. */
  first_shared_release.notify();

  /*
    Rescheduling which happens in this case should still see that X lock
    is waiting.
  */
  EXPECT_FALSE((mdl_thread3.get_mdl_context())
                   .owns_equal_or_stronger_lock(MDL_key::TABLE, db_name,
                                                table_name1, MDL_EXCLUSIVE));

  /* Release the second S lock to unblock request for X lock. */
  second_shared_release.notify();

  /* Wrap-up. */
  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
}

/*
  Verify that we correctly handle situation when one thread tries
  consequitively to acquire locks which conflict with each other
  (this is again to check that MDL_lock::can_grant_lock() handles
  such situations correctly).
*/

TEST_F(MDLTest, SelfConflict) {
  /* The first scenario: "unobtrusive" lock first, then "obtrusive". */

  /* Acquire S lock, it will be acquired using "fast path" algorithm. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Acquire global IX lock to be able acquire X lock later. */
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));

  /*
    Acquire X lock on the same table. MDL subsystem should be able to detect
    that conflicting S lock belongs to the same context even though it was
    was acquired using "fast path".
  */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  m_mdl_context.release_transactional_locks();

  /*
    The second scenario: "obtrusive" lock first then "unobtrusive".

    Let us try to acquire global S lock.
  */
  MDL_REQUEST_INIT(&m_global_request, MDL_key::GLOBAL, "", "", MDL_SHARED,
                   MDL_TRANSACTION);
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));

  /*
    Now let us acquire IX lock. Note that S lock is not exactly stronger
    than IX lock, so IX lock can't be acquired using find_ticket()
    optimization.
  */
  MDL_REQUEST_INIT(&m_global_request, MDL_key::GLOBAL, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));

  m_mdl_context.release_transactional_locks();
}

/*
  Verifies that we correctly account for "fast path" locks in
  clone_ticket() operation.
*/

TEST_F(MDLTest, CloneSharedExclusive) {
  MDL_ticket *initial_ticket;
  Notification lock_blocked;
  MDL_thread mdl_thread(table_name1, MDL_EXCLUSIVE, nullptr, nullptr,
                        &lock_blocked, nullptr);

  /* Acquire SHARED lock, it will be acquired using "fast path" algorithm. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_EXPLICIT);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /*
    Save initial ticket and create its clone.
    This operation should correctly update MDL_lock's "fast path"
    counter.
  */
  initial_ticket = m_request.ticket;
  EXPECT_FALSE(m_mdl_context.clone_ticket(&m_request));

  /* Release the original lock. This will decrement "fast path" counter. */
  m_mdl_context.release_lock(initial_ticket);

  /*
    Now try to acquire EXCLUSIVE lock. This request should be blocked
    because "fast path" counter is non-zero.
  */
  mdl_thread.start();
  lock_blocked.wait_for_notification();

  /* Release remaining SHARED lock to unblock request for EXCLUSIVE lock. */
  m_mdl_context.release_lock(m_request.ticket);

  mdl_thread.join();
}

/*
  Verifies that we correctly account for "obtrusive" locks in
  clone_ticket() operation.
*/

TEST_F(MDLTest, CloneExclusiveShared) {
  MDL_ticket *initial_ticket;
  Notification lock_blocked;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, nullptr, nullptr,
                        &lock_blocked, nullptr);

  /* Acquire EXCLUSIVE lock, counter of "obtrusive" locks is increased. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_EXPLICIT);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /*
    Save initial ticket and create its clone.
    This operation should correctly increase counter of "obtrusive" locks.
  */
  initial_ticket = m_request.ticket;
  EXPECT_FALSE(m_mdl_context.clone_ticket(&m_request));

  /*
    Release the original lock. This will decrement count of "obtrusive" locks.
  */
  m_mdl_context.release_lock(initial_ticket);

  /*
    Now try to acquire SHARED lock. This request should be blocked
    because "obtrusive" counter still should be non-zero.
  */
  mdl_thread.start();
  lock_blocked.wait_for_notification();

  /* Release remaining EXCLUSIVE lock to unblock request for SHARED lock. */
  m_mdl_context.release_lock(m_request.ticket);

  mdl_thread.join();
}

/**
  Verify that we correctly notify owners of "unobtrusive" locks when "obtrusive"
  locks are acquired, even though the former can be initially acquired
  using "fast path".
*/
TEST_F(MDLTest, NotifyScenarios) {
  /*
    The first scenario: Check that notification works properly if for
                        "unobtrusive" lock owner set_needs_thr_lock_abort(true)
                        was called after lock acquisition.
  */
  Notification first_shared_grabbed, first_shared_release;
  Notification first_shared_released, first_exclusive_grabbed;
  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &first_shared_grabbed,
                         &first_shared_release, nullptr,
                         &first_shared_released);
  MDL_thread mdl_thread2(table_name1, MDL_EXCLUSIVE, &first_exclusive_grabbed,
                         nullptr, nullptr, nullptr);

  /* Acquire S lock which will be granted using "fast path". */
  mdl_thread1.enable_release_on_notify();
  mdl_thread1.start();
  mdl_thread1.get_mdl_context().set_ignore_owner_thd(true);
  first_shared_grabbed.wait_for_notification();

  /*
    In order for notification to work properly for such locks
    context should be marked as requiring lock abort.
  */
  mdl_thread1.get_mdl_context().set_needs_thr_lock_abort(true);

  /*
    Now try to acquire X lock. This attemptshould notify owner of S lock.
    In our unit test such notification causes S lock release, so X lock
    should be successfully granted after that.
  */
  mdl_thread2.start();
  mdl_thread2.get_mdl_context().set_ignore_owner_thd(true);
  first_shared_released.wait_for_notification();
  first_exclusive_grabbed.wait_for_notification();

  /* Wrap-up of the first scenario. */
  mdl_thread1.join();
  mdl_thread2.join();

  /*
    The second scenario: Check the same works fine when
                         set_needs_thr_lock_abort(true) was called before
                         lock acquisition.
  */
  Notification second_shared_grabbed, second_shared_release;
  Notification second_shared_released, second_exclusive_grabbed;
  MDL_thread mdl_thread3(table_name1, MDL_SHARED, &second_shared_grabbed,
                         &second_shared_release, nullptr,
                         &second_shared_released);
  MDL_thread mdl_thread4(table_name1, MDL_EXCLUSIVE, &second_exclusive_grabbed,
                         nullptr, nullptr, nullptr);

  /*
    In order for notification to work properly context should be marked
    as requiring lock abort.
  */
  mdl_thread3.get_mdl_context().set_needs_thr_lock_abort(true);
  mdl_thread3.enable_release_on_notify();
  /* Acquire S lock which will be granted using "fast path". */
  mdl_thread3.start();
  mdl_thread3.get_mdl_context().set_ignore_owner_thd(true);
  second_shared_grabbed.wait_for_notification();

  /*
    Now try to acquire X lock. This attemptshould notify owner of S lock.
    In our unit test such notification causes S lock release, so X lock
    should be successfully granted after that.
  */
  mdl_thread4.start();
  mdl_thread4.get_mdl_context().set_ignore_owner_thd(true);
  second_shared_released.wait_for_notification();
  second_exclusive_grabbed.wait_for_notification();

  /* Wrap-up of the second scenario. */
  mdl_thread3.join();
  mdl_thread4.join();
}

/*
  Verify that upgrade to "obtrusive" lock doesn't leave any wrong traces
  in MDL subsystem.
*/

TEST_F(MDLTest, UpgradeScenarios) {
  /*
    The first scenario: Upgrade to X lock and then its release, should not
                        leave traces blocking further S locks.
  */

  /* Acquire S lock to be upgraded. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Acquire IX lock to be able to upgrade. */
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));

  /* Upgrade S lock to X lock. */
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));

  /*
    Ensure that there is pending S lock, so release of X lock won't destroy
    MDL_lock object.
  */
  Notification first_blocked;
  Notification first_release;
  MDL_thread mdl_thread1(table_name1, MDL_SHARED, nullptr, &first_release,
                         &first_blocked, nullptr);
  mdl_thread1.start();
  first_blocked.wait_for_notification();

  /*
    Now release IX and X locks.
    The latter should clear "obtrusive" locks counter so it should be
    possible to acquire S lock using "fast path" after that.
  */
  m_mdl_context.release_transactional_locks();

  /* Check that we can acquire S lock. */
  Notification second_grabbed;
  MDL_thread mdl_thread2(table_name1, MDL_SHARED, &second_grabbed, nullptr,
                         nullptr, nullptr);
  mdl_thread2.start();
  second_grabbed.wait_for_notification();

  /* Wrap-up of the first scenario. */
  first_release.notify();
  mdl_thread1.join();
  mdl_thread2.join();

  /*
    The second scenario: Upgrade from S to X, should not block further X
                         requests if upgraded lock is released.
  */

  /* Acquire S lock to be upgraded. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Upgrade S lock to X lock. */
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));

  /*
    Try to acquire X lock, it will block.
  */
  Notification third_blocked;
  Notification third_grabbed;
  MDL_thread mdl_thread3(table_name1, MDL_EXCLUSIVE, &third_grabbed, nullptr,
                         &third_blocked, nullptr);
  mdl_thread3.start();
  third_blocked.wait_for_notification();

  /*
    Now release IX and X locks. Upgrade should not leave any traces
    from original S lock (e.g. no "fast path" counter), so this should
    unblock pending X lock.
  */
  m_mdl_context.release_transactional_locks();
  third_grabbed.wait_for_notification();

  /* Wrap-up of the second scenario. */
  mdl_thread3.join();

  /*
    The third scenario: After upgrade from SU to X lock and release of the
                        latter there should not be traces preventing "fast
                        path" acquisition of S lock. Main difference from
                        the first scenario is that we upgrade from "obtrusive"
                        lock, so different path is triggered.
  */
  /* Acquire SU lock to be upgraded. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Upgrade SU lock to X lock. */
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      m_request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));

  /*
    Ensure that there is pending S lock, so release of X lock won't destroy
    MDL_lock object.
  */
  Notification fourth_blocked;
  Notification fourth_release;
  MDL_thread mdl_thread4(table_name1, MDL_SHARED, nullptr, &fourth_release,
                         &fourth_blocked, nullptr);
  mdl_thread4.start();
  fourth_blocked.wait_for_notification();

  /*
    Now release IX and X locks.
    The latter should clear "obtrusive" locks counter so it should be
    possible to acquire S lock using "fast path" after that.
  */
  m_mdl_context.release_transactional_locks();

  /* Check that we can acquire S lock. */
  Notification fifth_grabbed;
  MDL_thread mdl_thread5(table_name1, MDL_SHARED, &fifth_grabbed, nullptr,
                         nullptr, nullptr);
  mdl_thread5.start();
  fifth_grabbed.wait_for_notification();

  /* Wrap-up of the third scenario. */
  fourth_release.notify();
  mdl_thread4.join();
  mdl_thread5.join();
}

/**
  Verify that MDL subsystem can correctly handle deadlock
  in which one of participated locks was initially granted
  using "fast path".
*/

TEST_F(MDLTest, Deadlock) {
  Notification lock_blocked;
  MDL_thread mdl_thread(table_name1, MDL_EXCLUSIVE, nullptr, nullptr,
                        &lock_blocked, nullptr);

  /* Acquire SR lock which will be granted using "fast path". */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_READ, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Now try to request X lock. This will blocked. */
  mdl_thread.start();
  lock_blocked.wait_for_notification();

  /*
    Try to acquire SW lock.
    Because of pending X lock this will lead to deadlock.

    Which should be correctly detected and reported even though SR
    lock was originally granted using "fast path".
  */
  expected_error = ER_LOCK_DEADLOCK;

  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_WRITE, MDL_TRANSACTION);
  EXPECT_TRUE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Wrap-up. */
  m_mdl_context.release_transactional_locks();
  mdl_thread.join();
}

/**
  Verify that downgrade correctly handles "obtrusive" locks by decrementing
  "obtrusive"-lock counter when necessary.
*/

TEST_F(MDLTest, DowngradeShared) {
  Notification lock_grabbed;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, nullptr,
                        nullptr, nullptr);

  /* Acquire global IX lock first to satisfy MDL asserts. */
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));

  /* Acquire "obtrusive" X lock. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /* Downgrade lock to S lock. */
  m_request.ticket->downgrade_lock(MDL_SHARED);

  /*
    It should be possible to acquire S lock now in another thread since
    downgrade operation should decrement counter of "obtrusive" locks.
  */
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  /* Wrap-up. */
  m_mdl_context.release_transactional_locks();
  mdl_thread.join();
}

/**
  Verify that rescheduling of "obtrusive" lock correctly handles
  "obtrusive" lock counter and further attempts to acquire "unobtrusive"
  locks are not blocked.
*/

TEST_F(MDLTest, RescheduleSharedNoWrite) {
  Notification shared_grabbed;
  Notification shared_release;
  Notification first_shared_write_grabbed;
  Notification first_shared_write_release;
  Notification shared_no_write_grabbed;
  Notification shared_no_write_blocked;
  Notification second_shared_write_grabbed;

  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &shared_grabbed,
                         &shared_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name1, MDL_SHARED_WRITE,
                         &first_shared_write_grabbed,
                         &first_shared_write_release, nullptr, nullptr);
  MDL_thread mdl_thread3(table_name1, MDL_SHARED_NO_WRITE,
                         &shared_no_write_grabbed, nullptr,
                         &shared_no_write_blocked, nullptr);
  MDL_thread mdl_thread4(table_name1, MDL_SHARED_WRITE,
                         &second_shared_write_grabbed, nullptr, nullptr,
                         nullptr);

  /* Start thread which will acquire S lock. */
  mdl_thread1.start();
  shared_grabbed.wait_for_notification();

  /* Start thread which will acquire SW lock. */
  mdl_thread2.start();
  first_shared_write_grabbed.wait_for_notification();

  /* Now start thread which will try to acquire SNW lock and will block. */
  mdl_thread3.start();
  shared_no_write_blocked.wait_for_notification();

  /*
    Now we will release the first SW lock. This should schedule SNW lock.
    In the process or rescheduling counter of waiting and granted "obtrusive"
    locks should stay the same. So release of SNW, which happens after that,
    will allow to acquire more SW locks.
  */
  first_shared_write_release.notify();

  /* It should be possible to acquire SW lock without problems now. */
  mdl_thread4.start();
  second_shared_write_grabbed.wait_for_notification();

  /* Wrap-up. */
  shared_release.notify();
  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
}

/**
  Verify that try_acquire_lock() operation for lock correctly
  cleans up after itself.
*/

TEST_F(MDLTest, ConcurrentSharedTryExclusive) {
  Notification first_grabbed, second_grabbed, third_grabbed;
  Notification first_release, second_release;
  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &first_grabbed,
                         &first_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name1, MDL_SHARED, &second_grabbed,
                         &second_release, nullptr, nullptr);
  MDL_thread mdl_thread3(table_name1, MDL_SHARED, &third_grabbed, nullptr,
                         nullptr, nullptr);

  /* Start the first thread which will acquire S lock. */
  mdl_thread1.start();
  first_grabbed.wait_for_notification();

  /* Acquire global IX lock to satisfy asserts in MDL subsystem. */
  EXPECT_FALSE(
      m_mdl_context.acquire_lock_nsec(&m_global_request, long_timeout_nsec));
  EXPECT_NE(m_null_ticket, m_global_request.ticket);

  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  /*
    Attempt to acquire X lock on table should fail.
    But it should correctly cleanup after itself.
  */
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  first_release.notify();
  mdl_thread1.join();

  /* After S lock is released. MDL_lock should be counted as unused. */
  EXPECT_EQ(1, mdl_get_unused_locks_count());

  /* Start the second thread which will acquire S lock again. */
  mdl_thread2.start();
  second_grabbed.wait_for_notification();

  /* Attempt to acquire X lock on table should fail again. */
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&m_request));
  EXPECT_EQ(m_null_ticket, m_request.ticket);

  /* Grabbing another S lock after that should not be a problem. */
  mdl_thread3.start();
  third_grabbed.wait_for_notification();

  second_release.notify();
  mdl_thread2.join();
  mdl_thread3.join();

  m_mdl_context.release_transactional_locks();
}

/**
  Basic test which checks that unused MDL_lock objects are freed at all.
*/

TEST_F(MDLTest, UnusedBasic) {
  mdl_locks_unused_locks_low_water = 0;
  EXPECT_EQ(0, mdl_get_unused_locks_count());
  test_one_simple_shared_lock(MDL_SHARED);
  EXPECT_EQ(0, mdl_get_unused_locks_count());
}

/**
  More complex test in which we test freeing couple of unused
  MDL_lock objects while having another one still used and around.
*/

TEST_F(MDLTest, UnusedConcurrentThree) {
  Notification first_grabbed, first_release, second_grabbed, second_release,
      third_grabbed, third_release;

  MDL_thread mdl_thread1(table_name1, MDL_SHARED, &first_grabbed,
                         &first_release, nullptr, nullptr);
  MDL_thread mdl_thread2(table_name2, MDL_SHARED, &second_grabbed,
                         &second_release, nullptr, nullptr);
  MDL_thread mdl_thread3(table_name3, MDL_SHARED_UPGRADABLE, &third_grabbed,
                         &third_release, nullptr, nullptr);

  mdl_locks_unused_locks_low_water = 0;

  /* Start thread which will use one MDL_lock object and will keep it used. */
  mdl_thread1.start();
  first_grabbed.wait_for_notification();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /*
    Start thread which will acquire lock on another table, i.e. will use
    another MDL_lock object.
  */
  mdl_thread2.start();
  second_grabbed.wait_for_notification();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Now let 2nd thread to release its lock and thus unuse MDL_lock object. */
  second_release.notify();
  mdl_thread2.join();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /*
    Start 3rd thread which will acquire "slow path" lock and release it.
    By doing this we test freeing of unused on "slow path" release.
  */
  mdl_thread3.start();
  third_grabbed.wait_for_notification();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Now let 3nd thread to release its lock and thus unuse MDL_lock object. */
  third_release.notify();
  mdl_thread3.join();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /*
    Let the 1st thread to release its lock and unuse its MDL_lock object
    as well.
  */
  first_release.notify();
  mdl_thread1.join();

  EXPECT_EQ(0, mdl_get_unused_locks_count());
}

/**
  Finally test which involves many threads using, unusing and
  freeing MDL_lock objects.
*/

TEST_F(MDLTest, UnusedConcurrentMany) {
  const uint THREADS = 50, TABLES = 10;
  const char *table_names_group_a[TABLES] = {"0", "1", "2", "3", "4",
                                             "5", "6", "7", "8", "9"};
  const char *table_names_group_b[TABLES] = {"a", "b", "c", "d", "e",
                                             "f", "g", "h", "i", "j"};
  MDL_thread *mdl_thread_group_a[THREADS];
  MDL_thread *mdl_thread_group_b[THREADS];
  Notification group_a_grabbed[THREADS], group_a_release,
      group_b_grabbed[THREADS], group_b_release;
  uint i;

  for (i = 0; i < THREADS; ++i)
    mdl_thread_group_a[i] = new MDL_thread(
        table_names_group_a[i % TABLES],
        /*
          To make things more interesting for the
          first 2 tables in the group on of threads
          will also acquire SU lock.
        */
        ((i % TABLES < 2) && (i % TABLES == i)) ? MDL_SHARED_UPGRADABLE
                                                : MDL_SHARED,
        &group_a_grabbed[i], &group_a_release, nullptr, nullptr);

  for (i = 0; i < THREADS; ++i)
    mdl_thread_group_b[i] = new MDL_thread(
        table_names_group_b[i % TABLES],
        /*
          To make things more interesting for the
          first 2 tables in the group on of threads
          will also acquire SU lock.
        */
        ((i % TABLES < 2) && (i % TABLES == i)) ? MDL_SHARED_UPGRADABLE
                                                : MDL_SHARED,
        &group_b_grabbed[i], &group_b_release, nullptr, nullptr);

  mdl_locks_unused_locks_low_water = 0;

  /* Start both groups of threads. */
  for (i = 0; i < THREADS; ++i) mdl_thread_group_a[i]->start();
  for (i = 0; i < THREADS; ++i) mdl_thread_group_b[i]->start();

  /* Wait until both groups acquire their locks. */
  for (i = 0; i < THREADS; ++i) group_a_grabbed[i].wait_for_notification();
  for (i = 0; i < THREADS; ++i) group_b_grabbed[i].wait_for_notification();

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /*
    Now let the second group to release its locks and free unused MDL_lock
    objects. The first group will keep its objects used.
  */
  group_b_release.notify();

  /* Wait until it is done. */
  for (i = 0; i < THREADS; ++i) {
    mdl_thread_group_b[i]->join();
    delete mdl_thread_group_b[i];
  }

  /*
    At this point we should not have more than
    TABLES*MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO/(1-MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO
    unused objects. It is OK to have less.
  */
  EXPECT_GE((TABLES * MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO /
             (1 - MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO)),
            mdl_get_unused_locks_count());

  /* Now let the first group go as well. */
  group_a_release.notify();
  for (i = 0; i < THREADS; ++i) {
    mdl_thread_group_a[i]->join();
    delete mdl_thread_group_a[i];
  }

  EXPECT_EQ(0, mdl_get_unused_locks_count());
}

/**
  Test that unused locks low water threshold works as expected,
  i.e. that we don't free unused objects unless their number
  exceeds threshold.
*/

TEST_F(MDLTest, UnusedLowWater) {
  const uint TABLES = 10;
  const char *table_names[TABLES] = {"0", "1", "2", "3", "4",
                                     "5", "6", "7", "8", "9"};
  MDL_request request;
  uint i;

  mdl_locks_unused_locks_low_water = 4;

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Acquire some locks and then release them all at once. */

  for (i = 0; i < TABLES; ++i) {
    MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_names[i],
                     MDL_SHARED, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  }

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Release all locks. */
  m_mdl_context.release_transactional_locks();

  /* Number of unused lock objects should not go below threshold. */
  EXPECT_EQ(4, mdl_get_unused_locks_count());
}

/**
  Test that we don't free unused MDL_lock objects if unused/total objects
  ratio is below MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO.
*/

TEST_F(MDLTest, UnusedMinRatio) {
  const uint TABLES = 10;
  const char *table_names_a[TABLES] = {"0", "1", "2", "3", "4",
                                       "5", "6", "7", "8", "9"};
  const char *table_names_b[TABLES] = {"0", "1", "2", "3", "4",
                                       "5", "6", "7", "8", "9"};
  MDL_request request;
  uint i;

  mdl_locks_unused_locks_low_water = 0;

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Acquire some locks. */

  for (i = 0; i < TABLES; ++i) {
    MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_names_a[i],
                     MDL_SHARED, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  }

  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /* Take a savepoint to be able to release part of the locks in future. */
  MDL_savepoint savepoint = m_mdl_context.mdl_savepoint();

  /* Acquire a few more locks. */
  for (i = 0; i < TABLES; ++i) {
    MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_names_b[i],
                     MDL_SHARED, MDL_TRANSACTION);
    EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  }

  /* Now release part of the locks we hold. */
  m_mdl_context.rollback_to_savepoint(savepoint);

  /*
    At this point we should not have more than
    TABLES*MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO/(1-MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO
    unused objects.

    This is equivalent to:
    "unused objects" <= (("used objects (i.e.TABLES)" + "unused objects") *
                         MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO).
  */
  EXPECT_GE((TABLES * MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO /
             (1 - MDL_LOCKS_UNUSED_LOCKS_MIN_RATIO)),
            mdl_get_unused_locks_count());

  /* Release all locks. */
  m_mdl_context.release_transactional_locks();

  /*
    There should be no unused MDL_lock objects after this
    since low water threshold is zero.
  */
  EXPECT_EQ(0, mdl_get_unused_locks_count());
}

/*
  Verifies following scenario,
  After granting max_write_lock_count(=1) number of times for SW
  lock request, lock is granted to starving SRO lock request
  in wait queue. After SRO lock has been granted SW locks again
  get higher priority than SRO locks until max_write_lock_count(=1)
  SW lock requests has been satisfied.

  - max_write_lock_count= 1
  - THREAD 1: Acquires SW lock on the table.
  - THREAD 2: Requests for SRO lock on the table, blocked.
  - THREAD 3: Acquires SW lock on the table (m_piglet_lock_count= 1).
  - THREAD 4: Requests for SW on the table, blocked.
  - THREAD 5: Requests for SRO lock on the table, blocked.
  - THREAD 1,3: Release SW locks.
  - THREAD 2,5: Get SRO locks, (m_piglet_lock_count= 0)
    THREAD 6: Requests SRO lock, blocked.
  - THREAD 2,5: Release SRO locks.
  - THREAD 4: Gets SW lock (m_piglet_lock_count= 1).
  - THREAD 7: Requests SW lock, blocked.
  - THREAD 4: Releases SW lock.
  - THREAD 6: Gets SRO lock. Releases it.
  - THREAD 7: Gets SW lock.
*/

TEST_F(MDLTest, PigletLockTest) {
  Notification thd_lock_grabbed[7];
  Notification thd_release_locks[7];
  Notification thd_lock_blocked[7];
  Notification thd_lock_released[7];

  /* Locks taken by the threads */
  enum { THD1_SW, THD2_SRO, THD3_SW, THD4_SW, THD5_SRO, THD6_SRO, THD7_SW };

  max_write_lock_count = 1;

  /*
    THREAD1:  Acquires SW lock on table.
    Lock Wait Queue: <empty>
    Lock Granted: SW
  */
  MDL_thread mdl_thread1(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD1_SW],
      &thd_release_locks[THD1_SW], &thd_lock_blocked[THD1_SW],
      &thd_lock_released[THD1_SW]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_SW].wait_for_notification();

  /*
    THREAD2:  Requesting SRO lock on table.
    Lock Wait Queue: SRO
    Lock Granted: SW
  */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD2_SRO],
      &thd_release_locks[THD2_SRO], &thd_lock_blocked[THD2_SRO],
      &thd_lock_released[THD2_SRO]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SRO].wait_for_notification();

  /*
    THREAD3:  Acquires SW lock on table. m_piglet_lock_count becomes 1.
    Lock Wait Queue: SRO
    Lock Granted: SW, SW
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD3_SW],
      &thd_release_locks[THD3_SW], &thd_lock_blocked[THD3_SW],
      &thd_lock_released[THD3_SW]);
  mdl_thread3.start();
  thd_lock_grabbed[THD3_SW].wait_for_notification();

  /*
    THREAD4:  Requesting SW lock on table.
    Blocks because m_piglet_lock_count == max_write_lock_count.

    Lock Wait Queue: SRO <-- SW
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD4_SW],
      &thd_release_locks[THD4_SW], &thd_lock_blocked[THD4_SW],
      &thd_lock_released[THD4_SW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SW].wait_for_notification();

  /*
    THREAD 5: Requests SRO lock on the table.
    Lock Wait Queue: SRO <-- SW <--SRO
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD5_SRO],
      &thd_release_locks[THD5_SRO], &thd_lock_blocked[THD5_SRO],
      &thd_lock_released[THD5_SRO]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SRO].wait_for_notification();

  /*
     THREAD 1, 3: Release SW locks. Both SRO locks are granted since
     m_piglet_lock_count == max_write_lock_count. After that
     m_piglet_lock_count is reset to 0.

     Lock Wait Queue: SW
     Lock Granted: SRO, SRO
  */
  thd_release_locks[THD1_SW].notify();
  thd_lock_released[THD1_SW].wait_for_notification();
  thd_release_locks[THD3_SW].notify();
  thd_lock_released[THD3_SW].wait_for_notification();

  /* Locks are granted to THREAD 2 and THREAD 5 */
  thd_lock_grabbed[THD2_SRO].wait_for_notification();
  thd_lock_grabbed[THD5_SRO].wait_for_notification();

  /*
    THREAD 6: Requests SRO lock on the table.
    Blocked because m_piglet_lock_count == 0.
    Lock Wait Queue: SW <-- SRO
    Lock Granted: SRO, SRO
  */
  MDL_thread mdl_thread6(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD6_SRO],
      &thd_release_locks[THD6_SRO], &thd_lock_blocked[THD6_SRO],
      &thd_lock_released[THD6_SRO]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_SRO].wait_for_notification();

  /* THREAD 2 and THREAD 5: Release SRO locks */
  thd_release_locks[THD2_SRO].notify();
  thd_lock_released[THD2_SRO].wait_for_notification();
  thd_release_locks[THD5_SRO].notify();
  thd_lock_released[THD5_SRO].wait_for_notification();

  /*
    THREAD 4: Gets SW lock.
    Since there is pending SRO, m_piglet_lock_count becomes 1.
    Lock Wait Queue: SRO
    Lock Granted: SW
  */
  thd_lock_grabbed[THD4_SW].wait_for_notification();

  /*
    THREAD 7: Requests SW lock on the table.
    Blocked because m_piglet_lock_count == max_write_lock_count.
    Lock Wait Queue: SRO <-- SW
    Lock Granted: SW
  */
  MDL_thread mdl_thread7(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD7_SW],
      &thd_release_locks[THD7_SW], &thd_lock_blocked[THD7_SW],
      &thd_lock_released[THD7_SW]);
  mdl_thread7.start();
  thd_lock_blocked[THD7_SW].wait_for_notification();

  /* THREAD 4: Release SW lock */
  thd_release_locks[THD4_SW].notify();
  thd_lock_released[THD4_SW].wait_for_notification();

  /* THREAD 6: Gets SRO lock. */
  thd_lock_grabbed[THD6_SRO].wait_for_notification();

  /* Cleanup */
  thd_release_locks[THD6_SRO].notify();
  thd_lock_released[THD6_SRO].wait_for_notification();

  thd_lock_grabbed[THD7_SW].wait_for_notification();
  thd_release_locks[THD7_SW].notify();
  thd_lock_released[THD7_SW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
  mdl_thread7.join();
}

/*
  Verifies interaction of "piglet" and "hog" lock requests.

  Check situation when we first reach limit on successive grants
  of "piglet" and then "hog" locks. Notice that once both these
  limits are reached we give a way to SRO locks and reset both
  counters even though there are still pending SW locks. This is
  allows to avoid stream of concurrent SRO and SW starving out
  "hog" locks.

  - max_write_lock_count= 1
  - THREAD 1: Acquires SW lock on the table.
  - THREAD 2: Requests for SRO lock on the table, blocked.
  - THREAD 3: Acquires SW lock on the table (m_piglet_lock_count= 1).
  - THREAD 4: Requests for SW on the table, blocked.
  - THREAD 5: Requests for SNRW lock on the table, blocked.
  - THREAD 6: Requests for SRO lock on the table, blocked.
  - THREAD 7: Requests for SNRW lock on the table, blocked.
  - THREAD 8: Requests for SW lock on the table, blocked.
  - THREAD 1, 3: Release SW locks.
  - THREAD 5: Gets SNRW lock on the table (m_hog_lock_count= 1).
  - THREAD 5: Releases SNRW lock.
  - THREAD 2, 6: Get SRO locks on the table. (m_piglet_lock_count= 0,
                                              m_hog_lock_count= 0)
  - THREAD 9: Requests for SRO lock on the table, blocked.
  - THREAD 2, 6: Release SRO locks on the table.
  - THREAD 7: Gets SNRW lock (m_hog_lock_count= 1).
  - THREAD 7: Releases SNRW lock.
  - THREAD 4: Gets SW lock on the table. (m_piglet_lock_count= 1)
  - THREAD 4: Releases SW lock.
  - THREAD 9: Gets SRO lock. (m_piglet_lock_count= 0,
                              m_hog_lock_count= 0).
  - THREAD 9: Releases SRO lock.
  - THREAD 8: Gets SW lock.
*/

TEST_F(MDLTest, PigletThenHogLockTest) {
  Notification thd_lock_grabbed[9];
  Notification thd_release_locks[9];
  Notification thd_lock_blocked[9];
  Notification thd_lock_released[9];

  /* Locks taken by the threads */
  enum {
    THD1_SW,
    THD2_SRO,
    THD3_SW,
    THD4_SW,
    THD5_SNRW,
    THD6_SRO,
    THD7_SNRW,
    THD8_SW,
    THD9_SRO
  };

  max_write_lock_count = 1;

  /*
    THREAD1:  Acquires SW lock on table.
    Lock Wait Queue: <empty>
    Lock Granted: SW
  */
  MDL_thread mdl_thread1(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD1_SW],
      &thd_release_locks[THD1_SW], &thd_lock_blocked[THD1_SW],
      &thd_lock_released[THD1_SW]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_SW].wait_for_notification();

  /*
    THREAD2:  Requesting SRO lock on table.
    Lock Wait Queue: SRO
    Lock Granted: SW
  */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD2_SRO],
      &thd_release_locks[THD2_SRO], &thd_lock_blocked[THD2_SRO],
      &thd_lock_released[THD2_SRO]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SRO].wait_for_notification();

  /*
    THREAD3:  Acquires SW lock on table. m_piglet_lock_count becomes 1.
    Lock Wait Queue: SRO
    Lock Granted: SW, SW
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD3_SW],
      &thd_release_locks[THD3_SW], &thd_lock_blocked[THD3_SW],
      &thd_lock_released[THD3_SW]);
  mdl_thread3.start();
  thd_lock_grabbed[THD3_SW].wait_for_notification();

  /*
    THREAD4:  Requesting SW lock on table.
    Blocks because m_piglet_lock_count == max_write_lock_count.

    Lock Wait Queue: SRO <-- SW
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD4_SW],
      &thd_release_locks[THD4_SW], &thd_lock_blocked[THD4_SW],
      &thd_lock_released[THD4_SW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SW].wait_for_notification();

  /*
    THREAD 5: Requests SNRW lock on the table.
    Lock Wait Queue: SRO <-- SW <-- SNRW
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD5_SNRW],
      &thd_release_locks[THD5_SNRW], &thd_lock_blocked[THD5_SNRW],
      &thd_lock_released[THD5_SNRW]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SNRW].wait_for_notification();

  /*
    THREAD 6: Requests SRO lock on the table.
    Blocked because m_hog_lock_count == 0.
    Lock Wait Queue: SRO <-- SW <-- SNRW <-- SRO
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread6(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD6_SRO],
      &thd_release_locks[THD6_SRO], &thd_lock_blocked[THD6_SRO],
      &thd_lock_released[THD6_SRO]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_SRO].wait_for_notification();

  /*
    THREAD 7: Requests SNRW lock on the table.
    Lock Wait Queue: SRO <-- SW <-- SNRW <-- SRO <-- SNRW
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread7(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD7_SNRW],
      &thd_release_locks[THD7_SNRW], &thd_lock_blocked[THD7_SNRW],
      &thd_lock_released[THD7_SNRW]);
  mdl_thread7.start();
  thd_lock_blocked[THD7_SNRW].wait_for_notification();

  /*
    THREAD 8: Requests SW lock on the table.
    Blocked because of pending SRO and SNRW locks.
    Lock Wait Queue: SRO <-- SW <-- SNRW <-- SRO <-- SNRW <-- SW
    Lock Granted: SW, SW
    m_piglet_lock_count == 1
  */
  MDL_thread mdl_thread8(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD8_SW],
      &thd_release_locks[THD8_SW], &thd_lock_blocked[THD8_SW],
      &thd_lock_released[THD8_SW]);
  mdl_thread8.start();
  thd_lock_blocked[THD8_SW].wait_for_notification();

  /*
    THREAD 1, 3: Release SW locks. The first SNRW lock is granted.

    Lock Wait Queue: SRO <-- SW <-- SRO <-- SNRW <-- SW
    Lock Granted: SNRW
    m_piglet_lock_count == 1
    m_hog_lock_count == 1
  */
  thd_release_locks[THD1_SW].notify();
  thd_lock_released[THD1_SW].wait_for_notification();
  thd_release_locks[THD3_SW].notify();
  thd_lock_released[THD3_SW].wait_for_notification();

  /* Lock is granted to THREAD 5 */
  thd_lock_grabbed[THD5_SNRW].wait_for_notification();

  /*
    THREAD 5: Release SNRW lock.
    Since m_piglet_lock_count == 1 and m_hog_lock_count == 1 this
    ublocks SRO locks.
  */
  thd_release_locks[THD5_SNRW].notify();
  thd_lock_released[THD5_SNRW].wait_for_notification();

  /*
    THREAD 2,6: Get SRO locks.
    m_piglet_lock_count and m_hog_lock_count are reset to 0.

    Lock Wait Queue: SW <-- SNRW <-- SW
    Lock Granted: SRO, SRO
    m_piglet_lock_count == 0
    m_hog_lock_count == 0
  */
  thd_lock_grabbed[THD2_SRO].wait_for_notification();
  thd_lock_grabbed[THD6_SRO].wait_for_notification();

  /*
    THREAD 9: Requests SRO lock on the table.
    Blocked because m_piglet_lock_count == 0 and m_hog_lock_count == 0.
    Lock Wait Queue: SW <-- SNRW <-- SW <-- SRO
    Lock Granted: SRO, SRO
    m_piglet_lock_count == 0
    m_hog_lock_count == 0
  */
  MDL_thread mdl_thread9(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD9_SRO],
      &thd_release_locks[THD9_SRO], &thd_lock_blocked[THD9_SRO],
      &thd_lock_released[THD9_SRO]);
  mdl_thread9.start();
  thd_lock_blocked[THD9_SRO].wait_for_notification();

  /*
    THREAD 2,6: Release SRO locks.
    Since m_piglet_lock_count == 0 and m_hog_lock_count == 0 this
    unblocks SNRW lock request,
  */
  thd_release_locks[THD2_SRO].notify();
  thd_lock_released[THD2_SRO].wait_for_notification();
  thd_release_locks[THD6_SRO].notify();
  thd_lock_released[THD6_SRO].wait_for_notification();

  /*
    THREAD 7: Gets SNRW lock.
    m_hog_lock_count is set to 1 since there is pending SW and SRO locks.

    Lock Wait Queue: SW <-- SW <-- SRO
    Lock Granted: SNW
    m_piglet_lock_count == 0
    m_hog_lock_count == 1
  */
  thd_lock_grabbed[THD7_SNRW].wait_for_notification();

  /* THREAD 7: Releases SNRW lock. This will unblock one of SW locks. */
  thd_release_locks[THD7_SNRW].notify();
  thd_lock_released[THD7_SNRW].wait_for_notification();

  /*
    THREAD 4: Gets SW locks.
    m_piglet_lock_count is set to 1 since there is pending SRO lock.

    Lock Wait Queue: SW <-- SRO
    Lock Granted: SW
    m_piglet_lock_count == 1
    m_hog_lock_count == 0
  */
  thd_lock_grabbed[THD4_SW].wait_for_notification();

  /*
    THREAD 4: Release SW locks.
    Since m_piglet_lock_count == 1 this unblocks SRO lock request,
  */
  thd_release_locks[THD4_SW].notify();
  thd_lock_released[THD4_SW].wait_for_notification();

  /*
    THREAD 9: Gets SRO lock.
    m_piglet_lock_count is reset to 0.

    Lock Wait Queue:  SW
    Lock Granted: SRO
    m_piglet_lock_count == 0
    m_hog_lock_count == 0
  */
  thd_lock_grabbed[THD9_SRO].wait_for_notification();

  /*
    THREAD 9: Release SRO lock.
    Since m_hog_lock_count == 1 this unblocks SW lock request.
  */
  thd_release_locks[THD9_SRO].notify();
  thd_lock_released[THD9_SRO].wait_for_notification();

  /* THREAD 8: Gets SW lock. */
  thd_lock_grabbed[THD8_SW].wait_for_notification();

  /* Cleanup */
  thd_release_locks[THD8_SW].notify();
  thd_lock_released[THD8_SW].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
  mdl_thread7.join();
  mdl_thread8.join();
  mdl_thread9.join();
}

/*
  Another test for interaction of "piglet" and "hog" lock requests.

  Check situation when we first reach limit on successive grants
  of "hog" and then "piglet" locks. Again once both these limits
  are reached we give a way to SRO locks and reset both counters
  even though there are still pending SW locks in order to avoid
  starvation of "hog" requests.

  - max_write_lock_count= 1
  - THREAD 1: Acquires SW lock on the table.
  - THREAD 2: Requests for SNRW lock on the table, blocked.
  - THREAD 3: Requests for SW on the table, blocked.
  - THREAD 1: Releases SW lock.
  - THREAD 2: Gets SNRW lock (m_hog_lock_count= 1).
  - THREAD 4: Requests for SNRW lock on the table, blocked.
  - THREAD 5: Requests for SRO lock on the table, blocked.
  - THREAD 2: Releases SNRW lock.
  - THREAD 3: Gets SW lock (m_piglet_lock_count= 1).
  - THREAD 6: Requests for SW lock on the table, blocked.
  - THREAD 3: Releases SW lock.
  - THREAD 5: Gets SRO lock (m_piglet_lock_count=0, m_hog_lock_count= 0).
  - THREAD 7: Requests SR lock on the table, blocked.
  - THREAD 5: Releases SRO lock.
  - THREAD 4: Gets SNRW lock (m_hog_lock_count= 1)
  - THREAD 4: Releases SNRW lock.
  - THREAD 6,7: Get SW and SR locks.
*/

TEST_F(MDLTest, HogThenPigletLockTest) {
  Notification thd_lock_grabbed[7];
  Notification thd_release_locks[7];
  Notification thd_lock_blocked[7];
  Notification thd_lock_released[7];

  /* Locks taken by the threads */
  enum { THD1_SW, THD2_SNRW, THD3_SW, THD4_SNRW, THD5_SRO, THD6_SW, THD7_SR };

  max_write_lock_count = 1;

  /*
    THREAD1:  Acquires SW lock on table.

    Lock Wait Queue: <empty>
    Lock Granted: SW
  */
  MDL_thread mdl_thread1(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD1_SW],
      &thd_release_locks[THD1_SW], &thd_lock_blocked[THD1_SW],
      &thd_lock_released[THD1_SW]);
  mdl_thread1.start();
  thd_lock_grabbed[THD1_SW].wait_for_notification();

  /*
    THREAD2:  Requesting SNRW lock on table.

    Lock Wait Queue: SNRW
    Lock Granted: SW
  */
  MDL_thread mdl_thread2(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD2_SNRW],
      &thd_release_locks[THD2_SNRW], &thd_lock_blocked[THD2_SNRW],
      &thd_lock_released[THD2_SNRW]);
  mdl_thread2.start();
  thd_lock_blocked[THD2_SNRW].wait_for_notification();

  /*
    THREAD3:  Requesting SW lock on table.

    Lock Wait Queue: SNRW <-- SW
    Lock Granted: SW
  */
  MDL_thread mdl_thread3(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD3_SW],
      &thd_release_locks[THD3_SW], &thd_lock_blocked[THD3_SW],
      &thd_lock_released[THD3_SW]);
  mdl_thread3.start();
  thd_lock_blocked[THD3_SW].wait_for_notification();

  /*
    THREAD 1: Release SW lock. The SNRW lock is granted.
    m_hog_lock_count is set to 1.

    Lock Wait Queue: SW
    Lock Granted: SNRW
    m_piglet_lock_count == 0
    m_hog_lock_count == 1
  */
  thd_release_locks[THD1_SW].notify();
  thd_lock_released[THD1_SW].wait_for_notification();

  /* THREAD 2: Gets SNRW. */
  thd_lock_grabbed[THD2_SNRW].wait_for_notification();

  /*
    THREAD4:  Requesting SNRW lock on table.
    Blocks because m_hog_lock_count == 1.

    Lock Wait Queue: SW <-- SNRW
    Lock Granted: SNRW
    m_piglet_lock_count == 0
    m_hog_lock_count == 1
  */
  MDL_thread mdl_thread4(
      table_name1, MDL_SHARED_NO_READ_WRITE, &thd_lock_grabbed[THD4_SNRW],
      &thd_release_locks[THD4_SNRW], &thd_lock_blocked[THD4_SNRW],
      &thd_lock_released[THD4_SNRW]);
  mdl_thread4.start();
  thd_lock_blocked[THD4_SNRW].wait_for_notification();

  /*
    THREAD 5: Requests SRO lock on the table.

    Lock Wait Queue: SW <-- SNRW <-- SRO
    Lock Granted: SNRW
    m_piglet_lock_count == 0
    m_hog_lock_count == 1
  */
  MDL_thread mdl_thread5(
      table_name1, MDL_SHARED_READ_ONLY, &thd_lock_grabbed[THD5_SRO],
      &thd_release_locks[THD5_SRO], &thd_lock_blocked[THD5_SRO],
      &thd_lock_released[THD5_SRO]);
  mdl_thread5.start();
  thd_lock_blocked[THD5_SRO].wait_for_notification();

  /*
    THREAD 2: Releases SNRW lock. This unblocks SW lock
    since m_hog_lock_count == 1.
  */
  thd_release_locks[THD2_SNRW].notify();
  thd_lock_released[THD2_SNRW].wait_for_notification();

  /*
    THREAD 3: Gets SW lock on the table.
    m_piglet_lock_count is set to 1.

    Lock Wait Queue: SNRW <-- SRO
    Lock Granted: SW
    m_piglet_lock_count == 1
    m_hog_lock_count == 1
  */
  thd_lock_grabbed[THD3_SW].wait_for_notification();

  /*
    THREAD 6: Requests SW lock on the table.
    Blocked because m_piglet_lock_count == 1.

    Lock Wait Queue: SNRW <-- SRO <-- SW
    Lock Granted: SW
    m_piglet_lock_count == 1
    m_hog_lock_count == 1
  */
  MDL_thread mdl_thread6(
      table_name1, MDL_SHARED_WRITE, &thd_lock_grabbed[THD6_SW],
      &thd_release_locks[THD6_SW], &thd_lock_blocked[THD6_SW],
      &thd_lock_released[THD6_SW]);
  mdl_thread6.start();
  thd_lock_blocked[THD6_SW].wait_for_notification();

  /*
    THREAD 3: Releases SW lock. SRO lock is granted.
    Both m_piglet_lock_count and m_hog_lock_count are reset to 0.

    Lock Wait Queue: SNRW <-- SW
    Lock Granted: SRO
    m_piglet_lock_count == 0
    m_hog_lock_count == 0
  */
  thd_release_locks[THD3_SW].notify();
  thd_lock_released[THD3_SW].wait_for_notification();

  /* THREAD 5: Gets SRO lock. */
  thd_lock_grabbed[THD5_SRO].wait_for_notification();

  /*
    THREAD 7: Requests SR lock on the table. Blocked.

    Lock Wait Queue: SNRW <-- SW <-- SR
    Lock Granted: SRO
    m_piglet_lock_count == 0
    m_hog_lock_count == 0
  */
  MDL_thread mdl_thread7(
      table_name1, MDL_SHARED_READ, &thd_lock_grabbed[THD7_SR],
      &thd_release_locks[THD7_SR], &thd_lock_blocked[THD7_SR],
      &thd_lock_released[THD7_SR]);
  mdl_thread7.start();
  thd_lock_blocked[THD7_SR].wait_for_notification();

  /* THREAD 5: Releases SRO lock. */
  thd_release_locks[THD5_SRO].notify();
  thd_lock_released[THD5_SRO].wait_for_notification();

  /*
    THREAD 4: Gets SNRW lock. m_hog_lock_count is set to 1.

    Lock Wait Queue: SW <-- SR
    Lock Granted: SNRW
    m_piglet_lock_count == 0
    m_hog_lock_count == 1
  */
  thd_lock_grabbed[THD4_SNRW].wait_for_notification();

  /* THREAD 4: Releases SNRW lock. */
  thd_release_locks[THD4_SNRW].notify();
  thd_lock_released[THD4_SNRW].wait_for_notification();

  /* THREAD 6, 8: Get SW and SR locks. */
  thd_lock_grabbed[THD6_SW].wait_for_notification();
  thd_lock_grabbed[THD7_SR].wait_for_notification();

  /* Cleanup */
  thd_release_locks[THD6_SW].notify();
  thd_lock_released[THD6_SW].wait_for_notification();
  thd_release_locks[THD7_SR].notify();
  thd_lock_released[THD7_SR].wait_for_notification();

  mdl_thread1.join();
  mdl_thread2.join();
  mdl_thread3.join();
  mdl_thread4.join();
  mdl_thread5.join();
  mdl_thread6.join();
  mdl_thread7.join();
}

/**
  Auxiliary thread class which simulates connection which does
  LOCK TABLES t1 WRITE, t1 READ/UNLOCK TABLES in a loop.
*/

class MDL_SRO_SNRW_thread : public Thread, public Test_MDL_context_owner {
 public:
  MDL_SRO_SNRW_thread() { m_mdl_context.init(this); }

  ~MDL_SRO_SNRW_thread() { m_mdl_context.destroy(); }

  virtual void run();

  virtual void notify_shared_lock(MDL_context_owner *, bool) {}

 private:
  MDL_context m_mdl_context;
};

void MDL_SRO_SNRW_thread::run() {
  for (int i = 0; i < 100; ++i) {
    MDL_request request1, request2;
    MDL_request_list request_list;

    MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_NO_READ_WRITE, MDL_TRANSACTION);
    MDL_REQUEST_INIT(&request2, MDL_key::TABLE, db_name, table_name1,
                     MDL_SHARED_READ_ONLY, MDL_TRANSACTION);
    /*
      Simulate LOCK TABLES t1 WRITE, t1 READ, by putting SRO lock
      to the front of the list and SNRW to its back.
    */
    request_list.push_front(&request1);
    request_list.push_front(&request2);

    /*
      Lock acquisition should succeed and never deadlock. I.e. we should
      always acquire SNRW first and only then SRO.
    */
    EXPECT_FALSE(
        m_mdl_context.acquire_locks_nsec(&request_list, long_timeout_nsec));
    m_mdl_context.release_transactional_locks();
  }
}

/*
  Check that MDL_context::acquire_locks() takes type of lock requested
  into account when it sorts requests in order to avoid/reduce number
  of deadlocks.
  Also provides additional coverage for case when we concurrently create
  and destroy MDL_lock instances for the same object.
*/

TEST_F(MDLTest, AcquireLocksTypeOrder) {
  MDL_SRO_SNRW_thread thread1, thread2;

  /* Force immediate destruction of unused MDL_lock object. */
  mdl_locks_unused_locks_low_water = 0;

  thread1.start();
  thread2.start();
  thread1.join();
  thread2.join();
}

/**
  Thread class for testing MDL_context::set_force_dml_deadlock_weight()
  method.
*/

class MDL_weight_thread : public Thread, public Test_MDL_context_owner {
 public:
  MDL_weight_thread(Notification *first_grabbed, Notification *go_for_second,
                    Notification *lock_blocked)
      : m_first_grabbed(first_grabbed),
        m_go_for_second(go_for_second),
        m_lock_blocked(lock_blocked) {
    m_mdl_context.init(this);
  }

  ~MDL_weight_thread() { m_mdl_context.destroy(); }

  virtual void run();

  virtual void notify_shared_lock(MDL_context_owner *, bool) {}

  virtual void enter_cond(mysql_cond_t *cond, mysql_mutex_t *mutex,
                          const PSI_stage_info *stage,
                          PSI_stage_info *old_stage, const char *src_function,
                          const char *src_file, int src_line) {
    Test_MDL_context_owner::enter_cond(cond, mutex, stage, old_stage,
                                       src_function, src_file, src_line);

    m_lock_blocked->notify();
    return;
  }

 private:
  MDL_context m_mdl_context;
  Notification *m_first_grabbed;
  Notification *m_go_for_second;
  Notification *m_lock_blocked;
};

void MDL_weight_thread::run() {
  MDL_request request1, request2;

  MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_NO_READ_WRITE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request1, long_timeout_nsec));

  m_first_grabbed->notify();
  m_go_for_second->wait_for_notification();

  MDL_REQUEST_INIT(&request2, MDL_key::TABLE, db_name, table_name2,
                   MDL_SHARED_NO_READ_WRITE, MDL_TRANSACTION);

  /* Mark current thread as preferred deadlock victim. */
  m_mdl_context.set_force_dml_deadlock_weight(true);

  /*
    Wait for the second table should end-up in a deadlock with
    thread being chosen as victim.
  */
  expected_error = ER_LOCK_DEADLOCK;
  EXPECT_TRUE(m_mdl_context.acquire_lock_nsec(&request2, long_timeout_nsec));

  m_mdl_context.release_transactional_locks();
}

/**
  Test coverage for MDL_context::set_force_dml_deadlock_weight() method.
*/

TEST_F(MDLTest, ForceDMLDeadlockWeight) {
  Notification first_grabbed, go_for_second, second_blocked;
  MDL_weight_thread thread(&first_grabbed, &go_for_second, &second_blocked);

  /*
    Start the concurrent thread and wait until it acquires SNRW lock on
    table_name1 and gets suspended.
  */
  thread.start();
  first_grabbed.wait_for_notification();

  /* Now let us grab SNRW lock table_name2. */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name2,
                   MDL_SHARED_NO_READ_WRITE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  /*
    Resume the concurrent thread. It should try to acquire SNRW lock on
    table_name2. Wait for it to get blocked.
  */
  go_for_second.notify();
  second_blocked.wait_for_notification();

  /*
    Now let us try to grab SNRW lock table_name1.
    This should lead to deadlock.

    Normally this context will be chosen as a victim since all waits
    happen for the same type request - SNRW and it has joined waiters
    graph last.

    But since another thread uses MDL_context::set_force_dml_deadlock_weight()
    method it will be chosen as a victim instead.
  */
  MDL_REQUEST_INIT(&m_request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_NO_READ_WRITE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&m_request, long_timeout_nsec));

  m_mdl_context.release_transactional_locks();

  thread.join();
}

/** Simple MDL_context_visitor which stores pointer to context visited. */

class MDLTestContextVisitor : public MDL_context_visitor {
 public:
  MDLTestContextVisitor() : m_visited_ctx(nullptr) {}
  virtual void visit_context(const MDL_context *ctx) { m_visited_ctx = ctx; }
  const MDL_context *get_visited_ctx() { return m_visited_ctx; }

 private:
  const MDL_context *m_visited_ctx;
};

/**
  Test coverage for MDL_context::find_lock_owner() method.
*/

TEST_F(MDLTest, FindLockOwner) {
  Notification first_grabbed, first_release;
  Notification second_blocked, second_grabbed, second_release;
  MDL_thread thread1(table_name1, MDL_EXCLUSIVE, &first_grabbed, &first_release,
                     nullptr, nullptr);
  MDL_thread thread2(table_name1, MDL_EXCLUSIVE, &second_grabbed,
                     &second_release, &second_blocked, nullptr);
  MDL_key mdl_key(MDL_key::TABLE, db_name, table_name1);

  /* There should be no lock owner before we have started any threads. */
  MDLTestContextVisitor visitor1;
  EXPECT_FALSE(m_mdl_context.find_lock_owner(&mdl_key, &visitor1));
  const MDL_context *null_context = nullptr;
  EXPECT_EQ(null_context, visitor1.get_visited_ctx());

  /*
    Start the first thread and wait until it grabs the lock.
    This thread should be found as lock owner.
  */
  thread1.start();
  first_grabbed.wait_for_notification();
  EXPECT_FALSE(m_mdl_context.find_lock_owner(&mdl_key, &visitor1));
  EXPECT_EQ(&thread1.get_mdl_context(), visitor1.get_visited_ctx());

  /*
    Start the second thread and wait unti it is blocked waiting for the lock.
    The first thread still should be reported as lock owner.
  */
  MDLTestContextVisitor visitor2;
  thread2.start();
  second_blocked.wait_for_notification();
  EXPECT_FALSE(m_mdl_context.find_lock_owner(&mdl_key, &visitor2));
  EXPECT_EQ(&thread1.get_mdl_context(), visitor2.get_visited_ctx());

  /*
    Let the first thread to release lock and wait until the second thread
    acquires it. The second thread should be reported as lock owner now.
  */
  MDLTestContextVisitor visitor3;
  first_release.notify();
  second_grabbed.wait_for_notification();
  EXPECT_FALSE(m_mdl_context.find_lock_owner(&mdl_key, &visitor3));
  EXPECT_EQ(&thread2.get_mdl_context(), visitor3.get_visited_ctx());

  /* Release the lock. Wait until both threads have stopped. */
  second_release.notify();
  thread1.join();
  thread2.join();

  /* There should be no lock owners again. */
  MDLTestContextVisitor visitor4;
  EXPECT_FALSE(m_mdl_context.find_lock_owner(&mdl_key, &visitor4));
  EXPECT_EQ(null_context, visitor4.get_visited_ctx());
}

/** Test class for SE notification testing. */

class MDLHtonNotifyTest : public MDLTest {
 protected:
  MDLHtonNotifyTest() {}

  void SetUp() {
    MDLTest::SetUp();
    reset_counts_and_keys();
  }

  void TearDown() { MDLTest::TearDown(); }

  virtual bool notify_hton_pre_acquire_exclusive(const MDL_key *mdl_key,
                                                 bool *victimized) {
    *victimized = false;
    m_pre_acquire_count++;
    m_pre_acquire_key.mdl_key_init(mdl_key);
    return m_refuse_acquire;
  }
  virtual void notify_hton_post_release_exclusive(const MDL_key *mdl_key) {
    m_post_release_key.mdl_key_init(mdl_key);
    m_post_release_count++;
  }

  uint pre_acquire_count() const { return m_pre_acquire_count; }

  uint post_release_count() const { return m_post_release_count; }

  const MDL_key &pre_acquire_key() const { return m_pre_acquire_key; }

  const MDL_key &post_release_key() const { return m_post_release_key; }

  void reset_counts_and_keys() {
    m_pre_acquire_count = 0;
    m_post_release_count = 0;
    m_pre_acquire_key.reset();
    m_post_release_key.reset();
    m_refuse_acquire = false;
  }

  void set_refuse_acquire() { m_refuse_acquire = true; }

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MDLHtonNotifyTest);

  uint m_pre_acquire_count, m_post_release_count;
  MDL_key m_pre_acquire_key, m_post_release_key;
  bool m_refuse_acquire;
};

/**
  Test that SE notification is performed only for prescribed namespaces and
  not others.
*/

TEST_F(MDLHtonNotifyTest, NotifyNamespaces) {
  bool notify_or_not[] = {
      false,  // GLOBAL
      false,  // BACKUP_LOCK
      true,   // TABLESPACE
      true,   // SCHEMA
      true,   // TABLE
      true,   // FUNCTION
      true,   // PROCEDURE
      true,   // TRIGGER
      true,   // EVENT
      false,  // COMMIT
      false,  // USER_LEVEL_LOCK
      false,  // LOCKING_SERVICE
      false,  // SRID
      false,  // ACL_CACHE
      false,  // COLUMN_STATISTICS
      false,  // RESOURCE_GROUPS
      false,  // FOREIGN_KEY
      false   // CHECK_CONSTRAINT
  };
  static_assert(
      sizeof(notify_or_not) == MDL_key::NAMESPACE_END,
      "Initializer list for notify_or_not[] has the wrong number of elements!");

  for (uint i = 0; i < static_cast<uint>(MDL_key::NAMESPACE_END); i++) {
    MDL_request request;
    if (static_cast<MDL_key::enum_mdl_namespace>(i) == MDL_key::FUNCTION ||
        static_cast<MDL_key::enum_mdl_namespace>(i) == MDL_key::PROCEDURE ||
        static_cast<MDL_key::enum_mdl_namespace>(i) == MDL_key::TRIGGER ||
        static_cast<MDL_key::enum_mdl_namespace>(i) == MDL_key::EVENT ||
        static_cast<MDL_key::enum_mdl_namespace>(i) ==
            MDL_key::RESOURCE_GROUPS) {
      MDL_key mdl_key;
      mdl_key.mdl_key_init(static_cast<MDL_key::enum_mdl_namespace>(i), "", "",
                           0, "");
      MDL_REQUEST_INIT_BY_KEY(&request, &mdl_key, MDL_EXCLUSIVE,
                              MDL_TRANSACTION);
    } else {
      MDL_REQUEST_INIT(&request, static_cast<MDL_key::enum_mdl_namespace>(i),
                       "",
                       "",  // To work with GLOBAL/COMMIT spaces
                       MDL_EXCLUSIVE, MDL_TRANSACTION);
    }
    EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
    m_mdl_context.release_transactional_locks();

    if (notify_or_not[i]) {
      EXPECT_EQ(1U, pre_acquire_count());
      EXPECT_EQ(1U, post_release_count());
      reset_counts_and_keys();
    } else {
      EXPECT_EQ(0U, pre_acquire_count());
      EXPECT_EQ(0U, post_release_count());
    }
  }
}

/**
  Test that SE notification is performed only for X requests and not others.
*/

TEST_F(MDLHtonNotifyTest, NotifyLockTypes) {
  MDL_request request;

  // IX type can only be used for scoped locks. Doesn't cause notification.
  MDL_REQUEST_INIT(&request, MDL_key::SCHEMA, db_name, "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  m_mdl_context.release_transactional_locks();

  EXPECT_EQ(0U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());

  /*
    All other lock types can be used with tables and don't cause
    notification except X requests.
  */
  for (uint i = static_cast<uint>(MDL_INTENTION_EXCLUSIVE) + 1;
       i < static_cast<uint>(MDL_EXCLUSIVE); i++) {
    MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                     static_cast<enum_mdl_type>(i), MDL_TRANSACTION);

    EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
    m_mdl_context.release_transactional_locks();

    EXPECT_EQ(0U, pre_acquire_count());
    EXPECT_EQ(0U, post_release_count());
  }

  // X lock causes notifications.
  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  m_mdl_context.release_transactional_locks();

  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());

  // There are no other lock types!
  DBUG_ASSERT(static_cast<uint>(MDL_EXCLUSIVE) + 1 ==
              static_cast<uint>(MDL_TYPE_END));
}

/**
  Test for SE notification in some acquire/release scenarios
  including case when SE refuses lock acquisition.
*/

TEST_F(MDLHtonNotifyTest, NotifyAcquireRelease) {
  MDL_request request1, request2;

  // Straightforward acquire/release cycle.
  MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request1, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request1.key));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  m_mdl_context.release_transactional_locks();
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(post_release_key().is_equal(&request1.key));

  reset_counts_and_keys();

  // Case when we try to acquire lock several times.
  MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  MDL_REQUEST_INIT(&request2, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request1, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request1.key));

  // The second acquisition should not cause notification.
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request2, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());

  // On release there is only one notification call.
  m_mdl_context.release_transactional_locks();
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(post_release_key().is_equal(&request1.key));

  reset_counts_and_keys();

  // Case when lock acquisition is refused by SE.
  MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  expected_error = ER_LOCK_REFUSED_BY_ENGINE;
  set_refuse_acquire();

  EXPECT_TRUE(m_mdl_context.acquire_lock_nsec(&request1, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  // Nothing to release and nothing to notify about.
  m_mdl_context.release_transactional_locks();
  EXPECT_EQ(0U, post_release_count());
}

/**
  Test for SE notification in some scenarios when we successfully perform
  SE notification but then fail to acquire lock for some reason.
*/

TEST_F(MDLHtonNotifyTest, NotifyAcquireFail) {
  Notification lock_grabbed, release_lock;
  MDL_request request;

  // Acquire S lock on the table in another thread.
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, &release_lock,
                        nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  /*
    Try to acquire X lock on the table using try_acquire_lock().
  */
  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  EXPECT_FALSE(m_mdl_context.try_acquire_lock(&request));
  EXPECT_EQ(m_null_ticket, request.ticket);
  /*
    We treat failure to acquire X lock after successful pre-acquire
    notification in the same way as lock release.
  */
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request.key));
  EXPECT_TRUE(post_release_key().is_equal(&request.key));

  reset_counts_and_keys();

  /*
    Now do the same thing using acquire_lock() and zero timeout,
  */
  expected_error = ER_LOCK_WAIT_TIMEOUT;

  EXPECT_TRUE(m_mdl_context.acquire_lock_nsec(&request, zero_timeout));
  /*
    Again we treat failure to acquire X lock after successful pre-acquire
    notification in the same way as lock release.
  */
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request.key));
  EXPECT_TRUE(post_release_key().is_equal(&request.key));

  release_lock.notify();
  mdl_thread.join();
}

/**
  Test for SE notification in lock upgrade scenarios.
*/

TEST_F(MDLHtonNotifyTest, NotifyUpgrade) {
  MDL_request request;

  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);

  // Check that we notify SE about upgrade.
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));

  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request.key));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  // Second attempt to upgrade should not cause additional notification.
  EXPECT_FALSE(m_mdl_context.upgrade_shared_lock_nsec(
      request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());

  m_mdl_context.release_transactional_locks();
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(post_release_key().is_equal(&request.key));

  reset_counts_and_keys();

  // Case when lock upgrade is refused by SE.
  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));

  expected_error = ER_LOCK_REFUSED_BY_ENGINE;
  set_refuse_acquire();

  EXPECT_TRUE(m_mdl_context.upgrade_shared_lock_nsec(
      request.ticket, MDL_EXCLUSIVE, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_FALSE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  // Nothing to notify about during release.
  m_mdl_context.release_transactional_locks();
  EXPECT_EQ(0U, post_release_count());

  reset_counts_and_keys();

  /*
    Now case when notification is successful but we fail to upgrade for some
    other reason.
  */

  // Acquire S lock on the table in another thread.
  Notification lock_grabbed, release_lock;
  MDL_thread mdl_thread(table_name1, MDL_SHARED, &lock_grabbed, &release_lock,
                        nullptr, nullptr);
  mdl_thread.start();
  lock_grabbed.wait_for_notification();

  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_SHARED_UPGRADABLE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));

  expected_error = ER_LOCK_WAIT_TIMEOUT;

  EXPECT_TRUE(m_mdl_context.upgrade_shared_lock_nsec(
      request.ticket, MDL_EXCLUSIVE, zero_timeout));

  /*
    Failure to upgrade lock after successful notification is treated as release.
  */
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request.key));
  EXPECT_TRUE(post_release_key().is_equal(&request.key));

  m_mdl_context.release_transactional_locks();

  // No additional notifications!
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());

  release_lock.notify();
  mdl_thread.join();
}

/**
  Test for SE notification in lock downgrade scenarios.
*/

TEST_F(MDLHtonNotifyTest, NotifyDowngrade) {
  MDL_request request;

  MDL_REQUEST_INIT(&request, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  // Acquire X lock.
  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request.key));

  // First try no-op downgrade. No notification should be done.
  request.ticket->downgrade_lock(MDL_EXCLUSIVE);

  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());

  // Now downgrade to SNRW lock.
  request.ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);

  // We should notify SE as if doing lock release.
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
  EXPECT_TRUE(post_release_key().is_equal(&request.key));

  m_mdl_context.release_transactional_locks();

  // No additional notifications!
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(1U, post_release_count());
}

/**
  Test for SE notification in scenarios involving clone operation.
*/

TEST_F(MDLHtonNotifyTest, NotifyClone) {
  MDL_request request1, request2, request3, request4;

  // Acquire X lock.
  MDL_REQUEST_INIT(&request1, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);

  EXPECT_FALSE(m_mdl_context.acquire_lock_nsec(&request1, long_timeout_nsec));
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request1.key));
  EXPECT_TRUE(m_mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db_name, table_name1, MDL_EXCLUSIVE));

  // Now try to clone it to S lock.
  MDL_REQUEST_INIT(&request2, MDL_key::TABLE, db_name, table_name1, MDL_SHARED,
                   MDL_EXPLICIT);
  request2.ticket = request1.ticket;
  EXPECT_FALSE(m_mdl_context.clone_ticket(&request2));

  // There should be no additional notification in this case
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());

  reset_counts_and_keys();

  // Now try to clone it to another X lock instance.
  MDL_REQUEST_INIT(&request3, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_EXPLICIT);
  request3.ticket = request1.ticket;
  EXPECT_FALSE(m_mdl_context.clone_ticket(&request3));

  // This should cause additional SE notification
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request1.key));

  reset_counts_and_keys();

  // Finally try to clone it X lock and see what happens when SE refuses.
  MDL_REQUEST_INIT(&request4, MDL_key::TABLE, db_name, table_name1,
                   MDL_EXCLUSIVE, MDL_EXPLICIT);

  request4.ticket = request1.ticket;
  expected_error = ER_LOCK_REFUSED_BY_ENGINE;
  set_refuse_acquire();

  EXPECT_TRUE(m_mdl_context.clone_ticket(&request4));

  // This should cause additional SE notification (which fails).
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(0U, post_release_count());
  EXPECT_TRUE(pre_acquire_key().is_equal(&request1.key));

  // Release all locks and see how much SE notifications it will cause.
  m_mdl_context.release_transactional_locks();
  m_mdl_context.release_lock(request2.ticket);
  m_mdl_context.release_lock(request3.ticket);

  // All locks are released.
  EXPECT_FALSE(m_mdl_context.has_locks());

  // Two instances of X locks were released so 2 post-release calls.
  EXPECT_EQ(1U, pre_acquire_count());
  EXPECT_EQ(2U, post_release_count());
  EXPECT_TRUE(post_release_key().is_equal(&request1.key));
}

/** Test class for MDL_key class testing. Doesn't require MDL initialization. */

class MDLKeyTest : public ::testing::Test {
 protected:
  MDLKeyTest() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(MDLKeyTest);
};

// Google Test recommends DeathTest suffix for classes use in death tests.
typedef MDLKeyTest MDLKeyDeathTest;

/*
  Verifies that debug build dies with a DBUG_ASSERT if we try to construct
  MDL_key with too long database or object names.
*/

#if GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)
TEST_F(MDLKeyDeathTest, DieWhenNamesAreTooLong) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  /* We need a name which is longer than NAME_LEN = 64*3 = 192.*/
  const char *too_long_name =
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789";

  EXPECT_DEATH(MDL_key key0(MDL_key::TABLE, too_long_name, ""),
               ".*Assertion.*strlen.*");
  EXPECT_DEATH(MDL_key key1(MDL_key::TABLE, "", too_long_name),
               ".*Assertion.*strlen.*");

  MDL_key key2;

  EXPECT_DEATH(key2.mdl_key_init(MDL_key::TABLE, too_long_name, ""),
               ".*Assertion.*strlen.*");
  EXPECT_DEATH(key2.mdl_key_init(MDL_key::TABLE, "", too_long_name),
               ".*Assertion.*strlen.*");
}
#endif  // GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)

/*
  Verifies that for production build we allow construction of
  MDL_key with too long database or object names, but they are
  truncated.
*/

#if defined(DBUG_OFF)
TEST_F(MDLKeyTest, TruncateTooLongNames) {
  /* We need a name which is longer than NAME_LEN = 64*3 = 192.*/
  const char *too_long_name =
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789012345678901234567890123456789012345678901234567890123"
      "0123456789";

  MDL_key key(MDL_key::TABLE, too_long_name, too_long_name);

  const char *db_name = key.db_name();
  const char *name = key.name();

  EXPECT_LE(strlen(db_name), (uint)NAME_LEN);
  EXPECT_TRUE(strncmp(db_name, too_long_name, NAME_LEN) == 0);
  EXPECT_LE(strlen(name), (uint)NAME_LEN);
  EXPECT_TRUE(strncmp(name, too_long_name, NAME_LEN) == 0);
}

struct Mock_MDL_context_owner : public Test_MDL_context_owner {
  void notify_shared_lock(MDL_context_owner *in_use,
                          bool needs_thr_lock_abort) override final {
    in_use->notify_shared_lock(NULL, needs_thr_lock_abort);
  }
};

using Name_vec = std::vector<std::string>;

/*
  Helper function for benchmark.
 */
inline size_t read_from_env(const char *var, size_t def) {
  const char *val = getenv(var);
  return val ? static_cast<size_t>(atoi(val)) : def;
}

/*
  Helper function for benchmark.
 */
static Name_vec make_name_vec(size_t t) {
  std::vector<std::string> names;
  names.reserve(t);
  static std::stringstream str;
  for (size_t i = 0; i < t; ++i) {
    str << "t_" << i;
    names.push_back(str.str());
    str.str("");
  }
  return names;
}

/*
  Helper function for benchmark.
 */
static void lock_bench(MDL_context &ctx, const Name_vec &names) {
  for (auto &name : names) {
    MDL_request request;
    MDL_REQUEST_INIT(&request, MDL_key::TABLE, "S", name.c_str(),
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
    ctx.acquire_lock_nsec(&request, 2000000000ULL);
  }
  ctx.release_transactional_locks();
}

/**
  Microbenchmark which tests the performance of acquire_lock (find_ticket)
*/
static void BM_FindTicket(size_t num_iterations) {
  StopBenchmarkTiming();
  system_charset_info = &my_charset_utf8_bin;
  mdl_init();
  MDL_context ctx;
  Mock_MDL_context_owner owner;
  ctx.init(&owner);
  size_t ntickets = read_from_env("NTICKETS", 512);

  std::cout << "Tickets: " << ntickets << "\n";

  Name_vec names = make_name_vec(ntickets);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    lock_bench(ctx, names);
  }

  StopBenchmarkTiming();
  mdl_destroy();
}

BENCHMARK(BM_FindTicket)

#endif  // defined(DBUG_OFF)

}  // namespace mdl_unittest
