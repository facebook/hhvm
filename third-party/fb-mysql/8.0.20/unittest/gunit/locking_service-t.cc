/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "mysqld_error.h"
#include "sql/locking_service.h"
#include "sql/mdl.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"
#include "unittest/gunit/thread_utils.h"

static constexpr const ulonglong sec3600 = 3600000000000ULL;

/*
  Putting everything in a namespace prevents any (unintentional)
  name clashes with the code under test.
*/
namespace locking_service {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;
using thread::Notification;
using thread::Thread;

const char namespace1[] = "namespace1";
const char namespace2[] = "namespace2";
const char lock_name1[] = "lock1";
const char lock_name2[] = "lock2";
const char lock_name3[] = "lock3";
const char lock_name4[] = "lock4";

class LockingServiceTest : public ::testing::Test {
 protected:
  LockingServiceTest() {}

  static void SetUpTestCase() {
    m_old_error_handler_hook = error_handler_hook;
    // Make sure my_error() ends up calling my_message_sql so that
    // Mock_error_handler is actually triggered.
    error_handler_hook = my_message_sql;
    table_def_init();
  }

  static void TearDownTestCase() {
    error_handler_hook = m_old_error_handler_hook;
    table_def_free();
  }

  virtual void SetUp() {
    mdl_init();
    m_initializer.SetUp();
    m_thd = m_initializer.thd();
    // Slight hack: Makes THD::is_connected() return true.
    // This prevents MDL_context::acquire_lock() from thinking
    // the connection has died and the wait should be aborted.
    m_thd->system_thread = SYSTEM_THREAD_SLAVE_IO;
  }

  virtual void TearDown() {
    m_initializer.TearDown();
    mdl_destroy();
  }

  Server_initializer m_initializer;
  THD *m_thd;

  static void (*m_old_error_handler_hook)(uint, const char *, myf);
};

void (*LockingServiceTest::m_old_error_handler_hook)(uint, const char *, myf);

/**
  Test acquire and release of several read or write locks.
*/
TEST_F(LockingServiceTest, AcquireAndRelease) {
  // Take two read locks
  const char *names1[] = {lock_name1, lock_name2};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 2, LOCKING_SERVICE_READ, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));

  // Release the locks
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));

  // Take one write lock
  const char *names2[] = {lock_name3};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(m_thd, namespace1, names2, 1,
                                                  LOCKING_SERVICE_WRITE, 3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name3, MDL_EXCLUSIVE));

  // Take another write lock
  const char *names3[] = {lock_name4};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names3, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name3, MDL_EXCLUSIVE));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name4, MDL_EXCLUSIVE));

  // Take the read locks again
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 2, LOCKING_SERVICE_READ, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name3, MDL_EXCLUSIVE));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name4, MDL_EXCLUSIVE));

  // Release all locks
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name3, MDL_SHARED));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name4, MDL_SHARED));
}

/**
  Test that names are case sensitive
*/
TEST_F(LockingServiceTest, CaseSensitive) {
  const char *lower[] = {"test"};
  const char *upper[] = {"TEST"};

  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, lower, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, "test", MDL_EXCLUSIVE));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, "TEST", MDL_EXCLUSIVE));
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, "test", upper, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, "test", "TEST", MDL_EXCLUSIVE));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, "TEST", "TEST", MDL_EXCLUSIVE));
  EXPECT_FALSE(release_locking_service_locks(m_thd, "test"));
}

/**
  Test verfication of name lengths.
*/
TEST_F(LockingServiceTest, ValidNames) {
  const char *ok_name[] = {"test"};
  {
    const char *null = nullptr;
    const char *names1[] = {null};
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_WRONG_NAME);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names1, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, null, ok_name, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(release_locking_service_locks(m_thd, null));
    EXPECT_EQ(3, error_handler.handle_called());
  }

  {
    const char *empty = "";
    const char *names2[] = {empty};
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_WRONG_NAME);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names2, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, empty, ok_name, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(release_locking_service_locks(m_thd, empty));
    EXPECT_EQ(3, error_handler.handle_called());
  }

  {
    const char *long65 =
        "12345678901234567890123456789012345678901234567890123456789012345";
    const char *names3[] = {long65};
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_WRONG_NAME);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names3, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, long65, ok_name, 1, LOCKING_SERVICE_READ, sec3600));
    EXPECT_TRUE(release_locking_service_locks(m_thd, long65));
    EXPECT_EQ(3, error_handler.handle_called());
  }
}

/**
  Test interaction (or lack of it) with transactional locks.
*/
TEST_F(LockingServiceTest, TransactionInteraction) {
  // Releasing transactional locks should not affect lock service locks.
  const char *names1[] = {lock_name1};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_READ, sec3600));

  m_thd->mdl_context.release_transactional_locks();
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));

  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_EXCLUSIVE));
  m_thd->mdl_context.release_transactional_locks();
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_EXCLUSIVE));

  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

  // Releasing lock service locks should not affect transactional locks.
  MDL_request fake_request;
  MDL_REQUEST_INIT(&fake_request, MDL_key::SCHEMA, "db", "table", MDL_EXCLUSIVE,
                   MDL_TRANSACTION);
  EXPECT_FALSE(
      m_thd->mdl_context.acquire_lock_nsec(&fake_request, 3600000000000ULL));

  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_READ, sec3600));

  m_thd->mdl_context.release_transactional_locks();
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));

  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));
}

/**
  Utility thread for acquiring lock service locks with notification of
  acquire and release.
*/
class LockServiceThread : public Thread {
 public:
  LockServiceThread(const char **names, size_t num,
                    enum_locking_service_lock_type lock_type,
                    Notification *grabbed_arg, Notification *release_arg)
      : m_names(names),
        m_num(num),
        m_lock_type(lock_type),
        m_lock_grabbed(grabbed_arg),
        m_lock_release(release_arg) {}

  virtual void run() {
    Server_initializer m_initializer;
    m_initializer.SetUp();
    THD *m_thd = m_initializer.thd();

    EXPECT_FALSE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, m_names, m_num, m_lock_type, sec3600));
    if (m_lock_grabbed) m_lock_grabbed->notify();
    if (m_lock_release) m_lock_release->wait_for_notification();

    EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

    m_initializer.TearDown();
  }

 private:
  const char **m_names;
  size_t m_num;
  enum_locking_service_lock_type m_lock_type;
  Notification *m_lock_grabbed;
  Notification *m_lock_release;
};

/**
  Test that read locks are compatible with read locks but
  incompatible with write locks.
*/
TEST_F(LockingServiceTest, ReadCompatibility) {
  const char *names1[] = {lock_name1};
  Notification lock_grabbed, lock_release;
  LockServiceThread thread(names1, 1, LOCKING_SERVICE_READ, &lock_grabbed,
                           &lock_release);
  thread.start();
  lock_grabbed.wait_for_notification();

  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_READ, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

  {
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_TIMEOUT);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, 2000000000ULL));
    // Wait 2 seconds here so that we hit the "abs_timeout is far away"
    // code path in MDL_context::acquire_lock() on all platforms.
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
    EXPECT_EQ(1, error_handler.handle_called());
  }

  lock_release.notify();
  thread.join();
}

/**
  Test that write locks are incompatible with write locks.
*/
TEST_F(LockingServiceTest, WriteCompatibility) {
  const char *names1[] = {lock_name1};
  Notification lock_grabbed, lock_release;
  LockServiceThread thread(names1, 1, LOCKING_SERVICE_WRITE, &lock_grabbed,
                           &lock_release);
  thread.start();
  lock_grabbed.wait_for_notification();

  {
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_TIMEOUT);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, 1000000000ULL));
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
    EXPECT_EQ(1, error_handler.handle_called());
  }

  lock_release.notify();
  thread.join();
}

/**
  Test that if acquisition of multiple locks fails because of one
  lock conflicts, no locks are acquired.
*/
TEST_F(LockingServiceTest, AtomicAcquire) {
  const char *names1[] = {lock_name1};
  Notification lock_grabbed, lock_release;
  LockServiceThread thread(names1, 1, LOCKING_SERVICE_READ, &lock_grabbed,
                           &lock_release);
  thread.start();
  lock_grabbed.wait_for_notification();

  {
    // Conflict on lock_name1, lock_name2 should not be acquired.
    const char *names2[] = {lock_name1, lock_name2};
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_TIMEOUT);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names2, 2, LOCKING_SERVICE_WRITE, 1000000000ULL));
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));
    EXPECT_EQ(1, error_handler.handle_called());
  }

  {
    // Reverse order of lock names - should give same result.
    const char *names2[] = {lock_name2, lock_name1};
    Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_TIMEOUT);
    EXPECT_TRUE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names2, 2, LOCKING_SERVICE_WRITE, 1000000000ULL));
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
    EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_SHARED));
    EXPECT_EQ(1, error_handler.handle_called());
  }

  lock_release.notify();
  thread.join();
}

/**
  Test that namespaces are independent.
*/
TEST_F(LockingServiceTest, Namespaces) {
  const char *names1[] = {lock_name1};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_READ, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace2, lock_name1, MDL_SHARED));

  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace2));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_SHARED));
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

  // Take write lock with namespace1 in a separate thread.
  Notification lock_grabbed, lock_release;
  LockServiceThread thread(names1, 1, LOCKING_SERVICE_WRITE, &lock_grabbed,
                           &lock_release);
  thread.start();
  lock_grabbed.wait_for_notification();

  // We should be able to take a write lock in namespace2.
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace2, names1, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace2, lock_name1, MDL_EXCLUSIVE));
  EXPECT_FALSE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_EXCLUSIVE));

  lock_release.notify();
  thread.join();
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace2));
}

/**
  Thread which acquires a write lock on lock_name1 and then disconnects.
*/
class LockServiceDisconnectThread : public Thread {
 public:
  LockServiceDisconnectThread() {}

  virtual void run() {
    Server_initializer m_initializer;
    m_initializer.SetUp();
    THD *m_thd = m_initializer.thd();

    const char *names1[] = {lock_name1};
    EXPECT_FALSE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, sec3600));
    m_initializer.TearDown();
  }
};

/**
  Test that locks are released automatically on disconnect.
*/
TEST_F(LockingServiceTest, Disconnect) {
  LockServiceDisconnectThread thread;
  thread.start();
  thread.join();

  // Check that we now can acquire a write lock on name1.
  const char *names1[] = {lock_name1};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, sec3600));
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));
}

/**
  Utility thread for deadlock tests. Acquires two locks in order.
*/
class LockServiceDeadlockThread : public Thread {
 public:
  LockServiceDeadlockThread(Notification *grabbed1_arg, Notification *wait_arg)
      : m_lock_grabbed1(grabbed1_arg), m_wait(wait_arg) {}

  virtual void run() {
    Server_initializer m_initializer;
    m_initializer.SetUp();
    THD *m_thd = m_initializer.thd();

    const char *names1[] = {lock_name1};
    EXPECT_FALSE(acquire_locking_service_locks_nsec(
        m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, sec3600));
    m_lock_grabbed1->notify();

    m_wait->wait_for_notification();

    {
      // The deadlock should be resolved by aborting the wait for the read lock
      // We should therefore fail.
      const char *names2[] = {lock_name2};
      Mock_error_handler error_handler(m_thd, ER_LOCKING_SERVICE_DEADLOCK);
      EXPECT_TRUE(acquire_locking_service_locks_nsec(
          m_thd, namespace1, names2, 1, LOCKING_SERVICE_READ, sec3600));
      EXPECT_EQ(1, error_handler.handle_called());
    }

    // We still have the first lock
    EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
        MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_EXCLUSIVE));

    // Release locks so that the other thread can continue.
    EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));

    m_initializer.TearDown();
  }

 private:
  Notification *m_lock_grabbed1;
  Notification *m_wait;
};

/**
  Test that deadlock is detected and lock acquisition fails.
  The wait for a read lock should be aborted in preference for
  aborting the wait for a write lock.
*/
TEST_F(LockingServiceTest, DeadlockRead) {
  // Start a thread which acquires a write lock on name1
  Notification lock_grabbed1, wait;
  LockServiceDeadlockThread thread(&lock_grabbed1, &wait);
  thread.start();
  lock_grabbed1.wait_for_notification();

  // Acquire write lock on name 2
  const char *names2[] = {lock_name2};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names2, 1, LOCKING_SERVICE_WRITE, sec3600));

  // Signal the other thread to continue, taking write lock on name1
  wait.notify();

  // The other thread will be aborted so that we will acquire the lock.
  const char *names1[] = {lock_name1};
  EXPECT_FALSE(acquire_locking_service_locks_nsec(
      m_thd, namespace1, names1, 1, LOCKING_SERVICE_WRITE, sec3600));

  // Both locks should now be held
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name1, MDL_EXCLUSIVE));
  EXPECT_TRUE(m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::LOCKING_SERVICE, namespace1, lock_name2, MDL_EXCLUSIVE));

  thread.join();
  EXPECT_FALSE(release_locking_service_locks(m_thd, namespace1));
}
}  // namespace locking_service
