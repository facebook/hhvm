/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  Unit tests for MDL subsystem which require DEBUG_SYNC facility
  (mostly because they are very sensitive to thread scheduling).
*/

#include "my_config.h"

#include <gtest/gtest.h>

#include "sql/debug_sync.h"
#include "sql/mdl.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"
#include "unittest/gunit/thread_utils.h"

#if defined(ENABLED_DEBUG_SYNC)
namespace mdl_sync_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;
using thread::Notification;
using thread::Thread;

class MDLSyncTest : public ::testing::Test {
 protected:
  MDLSyncTest() {}

  void SetUp() {
    /* Set debug sync timeout of 60 seconds. */
    opt_debug_sync_timeout = 60;
    debug_sync_init();
    /* Force immediate destruction of unused MDL_lock objects. */
    mdl_locks_unused_locks_low_water = 0;
    max_write_lock_count = ULONG_MAX;
    mdl_init();

    m_initializer.SetUp();
    m_thd = m_initializer.thd();
    table_def_init();
  }

  void TearDown() {
    m_initializer.TearDown();
    mdl_destroy();
    debug_sync_end();
    opt_debug_sync_timeout = 0;
    mdl_locks_unused_locks_low_water = MDL_LOCKS_UNUSED_LOCKS_LOW_WATER_DEFAULT;
    table_def_free();
  }

  Server_initializer m_initializer;
  THD *m_thd;
};

/** Wrapper function which simplify usage of debug_sync_set_action(). */

static bool debug_sync_set_action(THD *thd, const char *sync) {
  return debug_sync_set_action(thd, sync, strlen(sync));
}

/**
  Set sync point and acquires and then releases the specified type of lock
  on the table.
  Allows to pause thread execution after lock acquisition and before its
  release by using notifications.
*/

class MDLSyncThread : public Thread {
 public:
  MDLSyncThread(const char *main_table_arg, enum_mdl_type main_mdl_type_arg,
                const char *sync_arg, Notification *grabbed_arg,
                Notification *release_arg)
      : m_main_table(main_table_arg),
        m_main_mdl_type(main_mdl_type_arg),
        m_sync(sync_arg),
        m_lock_grabbed(grabbed_arg),
        m_lock_release(release_arg),
        m_pre_table(nullptr),
        m_pre_mdl_type(MDL_INTENTION_EXCLUSIVE),
        m_pre_sync(nullptr) {}

  MDLSyncThread(const char *main_table_arg, enum_mdl_type main_mdl_type_arg,
                const char *sync_arg, Notification *grabbed_arg,
                Notification *release_arg, const char *pre_table_arg,
                enum_mdl_type pre_mdl_type_arg, const char *pre_sync_arg)
      : m_main_table(main_table_arg),
        m_main_mdl_type(main_mdl_type_arg),
        m_sync(sync_arg),
        m_lock_grabbed(grabbed_arg),
        m_lock_release(release_arg),
        m_pre_table(pre_table_arg),
        m_pre_mdl_type(pre_mdl_type_arg),
        m_pre_sync(pre_sync_arg) {}

  virtual void run() {
    m_initializer.SetUp();
    m_thd = m_initializer.thd();
    m_mdl_context = &m_thd->mdl_context;

    if (m_pre_table) {
      Mock_error_handler error_handler(m_thd, 0);

      if (m_pre_sync) {
        EXPECT_FALSE(debug_sync_set_action(m_thd, m_pre_sync));
      }

      /* Pre-acquire lock on auxiliary table if requested. */
      MDL_request request;
      MDL_REQUEST_INIT(&request, MDL_key::TABLE, "db", m_pre_table,
                       m_pre_mdl_type, MDL_TRANSACTION);

      EXPECT_FALSE(
          m_mdl_context->acquire_lock_nsec(&request, 3600000000000ULL));

      /* The above should not generate any warnings (e.g. about timeouts). */
      EXPECT_EQ(0, error_handler.handle_called());
    }

    /*
      Use a block to ensure that Mock_error_handler dtor is called
      before m_initalizer.TearDown().
    */
    {
      Mock_error_handler error_handler(m_thd, 0);

      if (m_sync) {
        EXPECT_FALSE(debug_sync_set_action(m_thd, m_sync));
      }

      MDL_request request;
      MDL_REQUEST_INIT(&request, MDL_key::TABLE, "db", m_main_table,
                       m_main_mdl_type, MDL_TRANSACTION);

      EXPECT_FALSE(
          m_mdl_context->acquire_lock_nsec(&request, 3600000000000ULL));
      EXPECT_TRUE(m_mdl_context->owns_equal_or_stronger_lock(
          MDL_key::TABLE, "db", m_main_table, m_main_mdl_type));

      if (m_lock_grabbed) m_lock_grabbed->notify();
      if (m_lock_release) m_lock_release->wait_for_notification();

      m_mdl_context->release_transactional_locks();

      /* The above should not generate any warnings (e.g. about timeouts). */
      EXPECT_EQ(0, error_handler.handle_called());
    }

    m_initializer.TearDown();
  }

 private:
  Server_initializer m_initializer;
  THD *m_thd;
  MDL_context *m_mdl_context;
  const char *m_main_table;
  enum_mdl_type m_main_mdl_type;
  const char *m_sync;
  Notification *m_lock_grabbed;
  Notification *m_lock_release;
  const char *m_pre_table;
  enum_mdl_type m_pre_mdl_type;
  const char *m_pre_sync;
};

/*
  Checks that "fast path" lock acquisition correctly handles MDL_lock objects
  which already have been marked as destroyed but still present in MDL_map.
*/

TEST_F(MDLSyncTest, IsDestroyedFastPath) {
  MDLSyncThread thread1("table", MDL_SHARED,
                        "mdl_remove_random_unused_after_is_destroyed_set "
                        "SIGNAL marked WAIT_FOR resume_removal",
                        nullptr, nullptr);
  MDLSyncThread thread2("table", MDL_SHARED,
                        "mdl_acquire_lock_is_destroyed_fast_path "
                        "SIGNAL resume_removal",
                        nullptr, nullptr);

  /*
    Start the first thread which acquires S lock on a table and immediately
    releases it. As result the MDL_lock object for the table becomes unused
    and attempt to destroy this unused object is made. MDL_lock is marked
    as destroyed, but the thread hits sync point before the object is removed
    from the MDL_map hash.
  */
  thread1.start();

  /* Wait until MDL_lock object is marked as destroyed. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR marked"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Start the second thread which will try to acquire S lock on the same
    table. It should notice that corresponding MDL_lock object is marked
    as destroyed. Sync point in code responsible for handling this situation
    will emit signal which should unblock the first thread.
  */
  thread2.start();

  /* Check that both threads finish. */
  thread1.join();
  thread2.join();
}

/*
  Checks that "slow path" lock acquisition correctly handles MDL_lock objects
  which already have been marked as destroyed but still present in MDL_map.
*/

TEST_F(MDLSyncTest, IsDestroyedSlowPath) {
  MDLSyncThread thread1("table", MDL_SHARED,
                        "mdl_remove_random_unused_after_is_destroyed_set "
                        "SIGNAL marked WAIT_FOR resume_removal",
                        nullptr, nullptr);
  MDLSyncThread thread2("table", MDL_SHARED_NO_READ_WRITE,
                        "mdl_acquire_lock_is_destroyed_slow_path "
                        "SIGNAL resume_removal",
                        nullptr, nullptr);

  /*
    Start the first thread which acquires S lock on a table and immediately
    releases it. As result the MDL_lock object for the table becomes unused
    and attempt to destroy this unused object is made. MDL_lock is marked
    as destroyed, but the thread hits sync point before the object is removed
    from the MDL_map hash.
  */
  thread1.start();

  /* Wait until MDL_lock object is marked as destroyed. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR marked"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Start the second thread which will try to acquire SNRW lock on the same
    table. It should notice that corresponding MDL_lock object is marked
    as destroyed. Sync point in code responsible for handling this situation
    will emit signal which should unblock the first thread.
  */
  thread2.start();

  /* Check that both threads finish. */
  thread1.join();
  thread2.join();
}

/*
  Checks that code responsible for destroying of random unused MDL_lock
  object correctly handles situation then it fails to find such an object.
*/

TEST_F(MDLSyncTest, DoubleDestroyTakeOne) {
  MDLSyncThread thread1("table", MDL_SHARED,
                        "mdl_remove_random_unused_before_search "
                        "SIGNAL before_search WAIT_FOR start_search",
                        nullptr, nullptr);
  MDLSyncThread thread2("table", MDL_SHARED, nullptr, nullptr, nullptr);

  /*
    Start the first thread which acquires S lock on a table and immediately
    releases it. As result the MDL_lock object for the table becomes unused
    and attempt to destroy random unused object is made. Thread hits sync
    point before it starts search for unused object.
  */
  thread1.start();

  /* Wait until thread stops before searching for unused object. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR before_search"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Start the second thread which will acquire and release S lock.
    As result MDL_lock object will become unused and destroyed.
    There should be no unused MDL_lock objects after its completion.
  */
  thread2.start();
  thread2.join();
  EXPECT_EQ(0, mdl_get_unused_locks_count());

  /*
    Resume the first thread. At this point it should find no unused objects.
    Check that it finishes succesfully.
  */
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now SIGNAL start_search"));
  thread1.join();
}

/*
  Checks that code responsible for destroying of random unused MDL_lock
  object correctly handles situation then it discovers that object
  which was chosen for destruction is already destroyed.
*/

TEST_F(MDLSyncTest, DoubleDestroyTakeTwo) {
  MDLSyncThread thread1("table", MDL_SHARED,
                        "mdl_remove_random_unused_after_search "
                        "SIGNAL found WAIT_FOR resume_destroy",
                        nullptr, nullptr);
  MDLSyncThread thread2("table", MDL_SHARED,
                        "mdl_remove_random_unused_after_is_destroyed_set "
                        "SIGNAL resume_destroy",
                        nullptr, nullptr);

  /*
    Start the first thread which acquires S lock on a table and immediately
    releases it. As result the MDL_lock object for the table becomes unused
    and attempt to destroy random unused object is made. Thread hits sync
    point after it has found the only unused MDL_lock object, but before
    it has acquired its MDL_lock::m_rwlock.
  */
  thread1.start();

  /* Wait until thread finds unused object. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR found"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Start the second thread which will acquire and release S lock.
    As result MDL_lock object will become unused and we will try
    to destroy it again.
    Thread will hit the sync point once the object is marked as destroyed.
    As result signal unblocking the first thread will be emitted.

  */
  thread2.start();

  /*
    The first thread should discover that the only unused MDL_lock object
    already has been marked as destroyed and still complete successfully.
  */
  thread1.join();

  /* So does the second thread. */
  thread2.join();
}

/*
  Checks that code responsible for destroying of random unused MDL_lock
  object correctly handles situation then it discovers that object
  which was chosen for destruction is re-used again and can't be destroyed.
*/

TEST_F(MDLSyncTest, DestroyUsed) {
  Notification lock_grabbed, lock_release;
  MDLSyncThread thread1("table", MDL_SHARED,
                        "mdl_remove_random_unused_after_search "
                        "SIGNAL found WAIT_FOR resume_destroy",
                        nullptr, nullptr);
  MDLSyncThread thread2("table", MDL_SHARED, nullptr, &lock_grabbed,
                        &lock_release);

  /*
    Start the first thread which acquires S lock on a table and immediately
    releases it. As result the MDL_lock object for the table becomes unused
    and attempt to destroy random unused object is made. Thread hits sync
    point after it has found the only unused MDL_lock object, but before
    it has acquired its MDL_lock::m_rwlock.
  */
  thread1.start();

  /* Wait until thread finds unused object. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR found"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Start the second thread which will acquire S lock and re-use
    the same MDL_lock object.
  */
  thread2.start();
  /* Wait until lock is acquired. */
  lock_grabbed.wait_for_notification();

  /*
    Resume the first thread. It should discover that the MDL_lock object
    it has found is used again and can't be destroyed.
  */
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now SIGNAL resume_destroy"));

  /* Still it should complete successfully.*/
  thread1.join();

  /* So does the second thread after it releases S lock. */
  lock_release.notify();
  thread2.join();
}

/*
  Test that shows that reschedule of waiters in cases when we acquire
  "hog" locks without waiting and this changes priority matrice is
  important.
*/

TEST_F(MDLSyncTest, PriorityDeadlock) {
  Notification first_grabbed, first_release;
  MDLSyncThread thread1("alpha", MDL_SHARED_READ, nullptr, &first_grabbed,
                        &first_release);
  MDLSyncThread thread2("alpha", MDL_SHARED_NO_READ_WRITE,
                        "mdl_acquire_lock_wait SIGNAL snrw_blocked ", nullptr,
                        nullptr);
  MDLSyncThread thread3("alpha", MDL_SHARED_READ,
                        "mdl_acquire_lock_wait SIGNAL sr_blocked "
                        "WAIT_FOR sr_resume",
                        nullptr, nullptr, "beta", MDL_SHARED_WRITE, nullptr);
  MDLSyncThread thread4("beta", MDL_SHARED_NO_WRITE, nullptr, nullptr, nullptr,
                        "alpha", MDL_SHARED_NO_WRITE, nullptr);

  /*
    Set "max_write_lock_count" to low value to ensure that switch of priority
    matrices happens early.
  */
  max_write_lock_count = 1;

  /* Start the first thread which acquires SR lock on "alpha". */
  thread1.start();
  first_grabbed.wait_for_notification();

  /* Start the second thread which will try to acquire SNRW on "alpha". */
  thread2.start();
  /* Wait until the thread gets blocked creating pending SNRW lock. */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR snrw_blocked"));
    EXPECT_EQ(0, error_handler.handle_called());
  }

  /*
    Start the third thread. It will acquire SW lock on table "beta",
    and will try to acquire SR lock on "alpha". It will be blocked
    because of pending SNRW lock on it.
  */
  thread3.start();
  /*
    Pause execution of the thread after it get blocked, but before
    deadlock detection is done.
  */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR sr_blocked"));
    EXPECT_EQ(0, error_handler.handle_called());
  }

  /*
    Start the fourth thread. It will acquire SNW lock on "alpha".
    This will switch priority matrice for this table and unblock pending
    SR lock request (because max_write_lock_count will be reached).
    After that it will try to acquire SNW lock on "beta" and block
    due to active SW lock from thread 3.
  */
  thread4.start();

  /*
    Resume thread 3.
    It should not wait because priority matrice has been changed and SR lock
    can now be granted (Reporting deadlock is acceptable outcome as well).
  */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now SIGNAL sr_resume"));
    EXPECT_EQ(0, error_handler.handle_called());
  }
  thread3.join();

  /*
    After SR lock has been acquired on "alpha" by thread 3.
    It will release all locks including SW lock on "beta".
    So thread 4 should be able to proceed.
  */
  thread4.join();

  /* Wrap-up. */
  first_release.notify();
  thread1.join();
  thread2.join();
}

/*
  Test that shows that reschedule is also important when priority
  matrice is changed due to "hog" lock being granted after wait.
*/

TEST_F(MDLSyncTest, PriorityDeadlock2) {
  Notification first_grabbed, first_release, second_grabbed, second_release;
  MDLSyncThread thread1("alpha", MDL_SHARED_WRITE, nullptr, &first_grabbed,
                        &first_release);
  MDLSyncThread thread2("alpha", MDL_SHARED_READ, nullptr, &second_grabbed,
                        &second_release);
  MDLSyncThread thread3("alpha", MDL_SHARED_NO_READ_WRITE,
                        "mdl_acquire_lock_wait SIGNAL snrw_blocked ", nullptr,
                        nullptr);
  MDLSyncThread thread4("alpha", MDL_SHARED_READ,
                        "mdl_acquire_lock_wait SIGNAL sr_blocked "
                        "WAIT_FOR sr_resume",
                        nullptr, nullptr, "beta", MDL_SHARED_WRITE, nullptr);
  MDLSyncThread thread5("beta", MDL_SHARED_NO_WRITE, nullptr, nullptr, nullptr,
                        "alpha", MDL_SHARED_NO_WRITE,
                        "mdl_acquire_lock_wait SIGNAL snw_blocked");

  /*
    Set "max_write_lock_count" to low value to ensure that switch of priority
    matrices happens early.
  */
  max_write_lock_count = 1;

  /* Start the first thread which acquires SW lock on "alpha". */
  thread1.start();
  first_grabbed.wait_for_notification();
  /* Start the second thread which acquires SR lock on "alpha". */
  thread2.start();
  second_grabbed.wait_for_notification();

  /* Start the third thread which will try to acquire SNRW on "alpha". */
  thread3.start();
  /* Wait until the thread gets blocked creating pending SNRW lock. */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR snrw_blocked"));
    EXPECT_EQ(0, error_handler.handle_called());
  }

  /*
    Start the fourth thread. It will acquire SW lock on table "beta",
    and will try to acquire SR lock on "alpha". It will be blocked
    because of pending SNRW lock on it.
  */
  thread4.start();
  /*
    Pause execution of the thread after it get blocked, but before
    deadlock detection is done.
  */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR sr_blocked"));
    EXPECT_EQ(0, error_handler.handle_called());
  }

  /*
    Start the fifth thread. It will try to acquire SNW lock on "alpha".
    It will get blocked because of active SW lock.
  */
  thread5.start();

  /*
    Wait until the thread gets blocked and SNW request is added to waiters
    queue.
  */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR snw_blocked"));
    EXPECT_EQ(0, error_handler.handle_called());
  }

  /*
    Release SW lock. This will unblock the fifth thread. SNW lock will
    be granted.

    This will switch priority matrice for this table and unblock pending
    SR lock request (because max_write_lock_count will be reached).
    After that it will try to acquire SNW lock on "beta" and block
    due to active SW lock from thread 3.
  */
  first_release.notify();
  thread1.join();

  /*
    Resume thread 4.
    It should not wait because priority matrice has been changed and SR lock
    can now be granted (Reporting deadlock is acceptable outcome as well).
  */
  {
    Mock_error_handler error_handler(m_thd, 0);
    EXPECT_FALSE(debug_sync_set_action(m_thd, "now SIGNAL sr_resume"));
    EXPECT_EQ(0, error_handler.handle_called());
  }
  thread4.join();

  /*
    After SR lock has been acquired on "alpha" by thread 4.
    It will release all locks including SW lock on "beta".
    So thread 5 should be able to proceed.
  */
  thread5.join();

  /* Wrap-up. */
  second_release.notify();
  thread2.join();
  thread3.join();
}

/** Dummy MDL_context_visitor which does nothing. */

class MDLSyncContextVisitor : public MDL_context_visitor {
 public:
  virtual void visit_context(const MDL_context *) {}
};

/*
  Checks that MDL_context::find_lock_owner() correctly handles MDL_lock
  objects which already have been marked as destroyed but still present
  in MDL_map.
*/

TEST_F(MDLSyncTest, IsDestroyedFindLockOwner) {
  MDLSyncThread thread1("table", MDL_EXCLUSIVE,
                        "mdl_remove_random_unused_after_is_destroyed_set "
                        "SIGNAL marked WAIT_FOR resume_removal",
                        nullptr, nullptr);
  MDL_key mdl_key(MDL_key::TABLE, "db", "table");
  MDLSyncContextVisitor dummy_visitor;

  /*
    Start a thread which acquires X lock on a table and immediately releases
    it. As result the MDL_lock object for the table becomes unused and
    attempt to destroy this unused object is made. MDL_lock is marked
    as destroyed, but the thread hits sync point before the object is
    removed from the MDL_map hash.
  */
  thread1.start();

  /* Wait until MDL_lock object is marked as destroyed. */
  Mock_error_handler error_handler(m_thd, 0);
  EXPECT_FALSE(debug_sync_set_action(m_thd, "now WAIT_FOR marked"));
  /* The above should not generate warnings about timeouts. */
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    Ensure that once MDL_context::find_lock_owner() sees object marked
    as destroyed it resumes the above thread.
  */
  EXPECT_FALSE(debug_sync_set_action(
      m_thd, "mdl_find_lock_owner_is_destroyed SIGNAL resume_removal"));
  EXPECT_EQ(0, error_handler.handle_called());

  /*
    MDL_context::find_lock_owner() should see MDL_lock object which is
    marked as destroyed and unblock thread performing its destruction.
  */
  EXPECT_FALSE(m_thd->mdl_context.find_lock_owner(&mdl_key, &dummy_visitor));

  /* Check that thread performing destruction of MDL_lock object has finished.
   */
  thread1.join();
}

}  // namespace mdl_sync_unittest
#endif
