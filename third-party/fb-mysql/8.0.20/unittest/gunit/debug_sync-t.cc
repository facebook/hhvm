/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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
  This is a unit test written to demonstrate a race issue in debug_sync
  functionality which resulted in wait thread getting timed out. This
  happens because we use a single global variable to identify the thread
  that has signalled. Before the wait could process the signal from the
  signal thread, another signal thread can overwrite this global signal
  value thus making the wait thread to lose this signal and  timeout
  waiting for a signal that it has already lost.
*/

#include <gtest/gtest.h>
#include <string>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/debug_sync.h"
#include "unittest/gunit/test_utils.h"
#include "unittest/gunit/thread_utils.h"

#if defined(ENABLED_DEBUG_SYNC)
extern uchar *debug_sync_value_ptr(THD *thd);

namespace debug_sync_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;
using thread::Notification;
using thread::Thread;

class DebugSyncTest : public ::testing::Test {
 protected:
  DebugSyncTest() {}

  void SetUp() {
    // set debug sync timeout of 60 seconds.
    opt_debug_sync_timeout = 60;
    debug_sync_init();
  }

  void TearDown() {
    debug_sync_end();
    opt_debug_sync_timeout = 0;
  }
};

/*
  Set up a debug sync action thread. Depending on type of action, the  thread
  serves different purpose. In the case where it involves wait_for action, it
  serves the purpose of a waiting thread that wait at a 'sync_point' in it's
  thread of control for named signal event. The debug sync action could
  be a signal event, in which case it assumes the role of signalling thread
  that signals an event identified by signal name to wake a waiting thread.
*/
class DebugSyncThread : public Thread {
 public:
  DebugSyncThread(Notification *go, bool wait_thread, std::string action)
      : m_go(go), m_wait_thread(wait_thread), m_action(action) {}

  virtual void run() {
    m_initializer.SetUp();
    m_thd = m_initializer.thd();
    m_go->wait_for_notification();

    if (m_wait_thread) {
      std::string sync_point("sync_point ");
      std::string action_str = sync_point + m_action;
      Mock_error_handler error_handler(m_thd, 0);

      debug_sync_set_action(m_thd, action_str.c_str(), action_str.length());
      wait_point();

      // The above should not generate any warnings.
      EXPECT_EQ(0, error_handler.handle_called());
    } else {
      debug_sync_set_action(m_thd, m_action.c_str(), m_action.length());
    }

    m_initializer.TearDown();
  }

 private:
  Notification *m_go;
  Server_initializer m_initializer;
  THD *m_thd;            // THD context required to hold debug_sync context.
  bool m_wait_thread;    // indicate if it is a wait thread.
  std::string m_action;  // indicate type of action WAIT_FOR or SIGNAL.

  void wait_point() { DEBUG_SYNC(m_thd, "sync_point"); }
};

/*
  Start a wait thread waiting for signal X1  and two signal threads
  one signalling event X1 and other signalling an event X2. Notify
  all of them to start at once.
*/
TEST_F(DebugSyncTest, DebugSyncSignalWaitTests) {
  Notification go[3];

  /* wait thread */
  DebugSyncThread wait_thread(&go[0], true, std::string("WAIT_FOR X1"));
  /* signal thread */
  DebugSyncThread signal_thread1(&go[1], false, std::string("now signal X1"));
  DebugSyncThread signal_thread2(&go[2], false, std::string("now signal X2"));

  wait_thread.start();
  signal_thread1.start();
  signal_thread2.start();

  // Notify all threads
  go[0].notify();
  go[1].notify();
  go[2].notify();

  // Wait for all threads to finish.
  wait_thread.join();
  signal_thread1.join();
  signal_thread2.join();
}

// Test debug_sync_value_ptr
TEST_F(DebugSyncTest, DebugSyncValuesTest) {
  std::string action;
  Server_initializer server_initializer;
  THD *thd;

  server_initializer.SetUp();
  thd = server_initializer.thd();

  // Ensure that we have a empty signal list at startup.
  EXPECT_STREQ("ON - signals: ''",
               reinterpret_cast<char *>(debug_sync_value_ptr(thd)));

  // Set up signalling actions and ensure the signals list reflect it.
  action = "now signal x1";
  debug_sync_set_action(thd, action.c_str(), action.length());
  EXPECT_STREQ("ON - signals: 'x1'",
               reinterpret_cast<char *>(debug_sync_value_ptr(thd)));

  action = "now signal x2";
  debug_sync_set_action(thd, action.c_str(), action.length());
  EXPECT_STREQ("ON - signals: 'x1,x2'",
               reinterpret_cast<char *>(debug_sync_value_ptr(thd)));

  action = "now signal x3";
  debug_sync_set_action(thd, action.c_str(), action.length());
  EXPECT_STREQ("ON - signals: 'x1,x2,x3'",
               reinterpret_cast<char *>(debug_sync_value_ptr(thd)));

  // Ensure the signal list is empty after reset.
  action = "reset";
  debug_sync_set_action(thd, action.c_str(), action.length());
  EXPECT_STREQ("ON - signals: ''",
               reinterpret_cast<char *>(debug_sync_value_ptr(thd)));

  server_initializer.TearDown();
}

}  // namespace debug_sync_unittest
#endif
