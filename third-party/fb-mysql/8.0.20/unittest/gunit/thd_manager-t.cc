/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
  This is unit test for the Global_THD_manager class.
*/

#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "my_inttypes.h"
#include "sql/mysqld.h"
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/rpl_master.h"
#include "sql/sql_class.h"
#include "unittest/gunit/thread_utils.h"

using thread::Notification;
using thread::Thread;

namespace thd_manager_unittest {

class ThreadManagerTest : public ::testing::Test {
 protected:
  ThreadManagerTest() {}

  void SetUp() {
    Global_THD_manager::create_instance();
    thd_manager = Global_THD_manager::get_instance();
    thd_manager->set_unit_test();
    init_slave_list();
  }

  void TearDown() { end_slave_list(); }

  Global_THD_manager *thd_manager;

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(ThreadManagerTest);
};

enum TEST_TYPE { TEST_WAIT = 0, TEST_TIMED_WAIT = 1 };

/*
  Verify add_thd(), remove_thd() methods
*/
TEST_F(ThreadManagerTest, AddRemoveTHDWithGuard) {
  THD thd1(false), thd2(false);
  thd1.server_id = 1;
  thd1.set_new_thread_id();
  thd2.server_id = 2;
  thd2.set_new_thread_id();

  EXPECT_EQ(0U, thd_manager->get_thd_count());
  thd_manager->add_thd(&thd1);
  EXPECT_EQ(1U, thd_manager->get_thd_count());
  thd_manager->add_thd(&thd2);

  thd_manager->remove_thd(&thd1);
  EXPECT_EQ(1U, thd_manager->get_thd_count());
  thd_manager->remove_thd(&thd2);
  EXPECT_EQ(0U, thd_manager->get_thd_count());
}

TEST_F(ThreadManagerTest, IncDecThreadRunning) {
  EXPECT_EQ(0, thd_manager->get_num_thread_running());
  thd_manager->inc_thread_running();
  EXPECT_EQ(1, thd_manager->get_num_thread_running());
  thd_manager->dec_thread_running();
  EXPECT_EQ(0, thd_manager->get_num_thread_running());
}

TEST_F(ThreadManagerTest, IncThreadCreated) {
  EXPECT_EQ(0U, thd_manager->get_num_thread_created());
  thd_manager->inc_thread_created();
  EXPECT_EQ(1U, thd_manager->get_num_thread_created());
}

/*
  Test function to validate do_for_all_thd, do_for_all_thd_copy.
  It emulates counter function to count number of thds in thd list.
*/
class TestFunc1 : public Do_THD_Impl {
 private:
  int cnt;

 public:
  TestFunc1() : cnt(0) {}
  int get_count() { return cnt; }
  void reset_count() { cnt = 0; }
  void operator()(THD *) { cnt = cnt + 1; }
};

TEST_F(ThreadManagerTest, TestTHDCopyDoFunc) {
  THD thd1(false), thd2(false);
  thd1.server_id = 1;
  thd1.set_new_thread_id();
  thd2.server_id = 2;
  thd2.set_new_thread_id();
  // Add two THD into thd list.
  thd_manager->add_thd(&thd1);
  thd_manager->add_thd(&thd2);

  int cnt = 0;
  TestFunc1 testFunc1;
  thd_manager->do_for_all_thd_copy(&testFunc1);
  cnt = testFunc1.get_count();
  EXPECT_EQ(2, cnt);

  testFunc1.reset_count();
  thd_manager->do_for_all_thd(&testFunc1);
  cnt = testFunc1.get_count();
  EXPECT_EQ(2, cnt);

  // Cleanup - Remove added THD.
  thd_manager->remove_thd(&thd1);
  thd_manager->remove_thd(&thd2);
}

/*
  Test class to verify find_thd()
*/
class TestFunc2 : public Find_THD_Impl {
 public:
  TestFunc2() : search_value(0) {}
  bool operator()(THD *thd) {
    if (thd->server_id == search_value) {
      return true;
    }
    return false;
  }
  void set_search_value(uint val) { search_value = val; }

 private:
  uint search_value;
};

/*
  Test class to verify do_all_for_thd() function.
  Counts all thd whose server_id value is less than or equal to 2.
*/
class TestFunc3 : public Do_THD_Impl {
 public:
  TestFunc3() : count(0) {}
  void operator()(THD *thd) {
    if (thd->server_id <= 2) {
      count++;
    }
  }
  int get_count() { return count; }

 private:
  int count;
};

TEST_F(ThreadManagerTest, TestTHDFindFunc) {
  THD thd1(false), thd2(false);
  thd1.server_id = 1;
  thd1.set_new_thread_id();
  thd2.server_id = 2;
  thd2.set_new_thread_id();
  thd_manager->add_thd(&thd1);
  thd_manager->add_thd(&thd2);
  TestFunc2 testFunc2;
  testFunc2.set_search_value(2);
  THD *thd = thd_manager->find_thd(&testFunc2);
  /* Returns the last thd which matches. */
  EXPECT_EQ(2U, thd->server_id);

  testFunc2.set_search_value(6);
  thd = thd_manager->find_thd(&testFunc2);
  /* Find non existing thd with server_id value 6. Expected to return NULL. */
  const THD *null_thd = nullptr;
  EXPECT_EQ(null_thd, thd);

  // Cleanup - Remove added THD.
  thd_manager->remove_thd(&thd1);
  thd_manager->remove_thd(&thd2);
}

TEST_F(ThreadManagerTest, TestTHDCountFunc) {
  THD thd1(false), thd2(false), thd3(false);
  thd1.server_id = 1;
  thd1.set_new_thread_id();
  thd2.server_id = 2;
  thd2.set_new_thread_id();
  thd3.server_id = 3;
  thd3.set_new_thread_id();
  thd_manager->add_thd(&thd1);
  thd_manager->add_thd(&thd2);
  thd_manager->add_thd(&thd3);

  TestFunc3 testFunc3;
  thd_manager->do_for_all_thd(&testFunc3);
  int ret = testFunc3.get_count();
  // testFunc3 counts for thd->server_id values, 1 and 2.
  EXPECT_EQ(2, ret);

  // Cleanup - Remove added THD.
  thd_manager->remove_thd(&thd1);
  thd_manager->remove_thd(&thd2);
  thd_manager->remove_thd(&thd3);
}

TEST_F(ThreadManagerTest, ThreadID) {
  // Code assumes that the size of my_thread_id is 32 bit.
  ASSERT_EQ(4U, sizeof(my_thread_id));

  // Reset the thread ID counter
  thd_manager->set_thread_id_counter(1);
  EXPECT_EQ(1U, thd_manager->get_thread_id());

  // The counter is incremented after ID is assigned.
  EXPECT_EQ(1U, thd_manager->get_new_thread_id());
  EXPECT_EQ(2U, thd_manager->get_thread_id());

  // Two increments in a row
  EXPECT_EQ(2U, thd_manager->get_new_thread_id());
  EXPECT_EQ(3U, thd_manager->get_new_thread_id());
  EXPECT_EQ(4U, thd_manager->get_thread_id());

  // Force wrap of the counter
  thd_manager->set_thread_id_counter(UINT_MAX32);
  EXPECT_EQ(UINT_MAX32, thd_manager->get_new_thread_id());

  // We should not use the value reserved for temporary THDs (0).
  // The next available value should be 4.
  EXPECT_EQ(4U, thd_manager->get_new_thread_id());
  EXPECT_EQ(5U, thd_manager->get_thread_id());

  // Release thread ID 3 and reset counter.
  thd_manager->release_thread_id(3);
  thd_manager->set_thread_id_counter(1U);
  EXPECT_EQ(3U, thd_manager->get_new_thread_id());

  // Releasing the reserved thread ID is allowed - multiple times.
  thd_manager->release_thread_id(Global_THD_manager::reserved_thread_id);
  thd_manager->release_thread_id(Global_THD_manager::reserved_thread_id);

  // Cleanup
  thd_manager->release_thread_id(1);
  thd_manager->release_thread_id(2);
  thd_manager->release_thread_id(3);
  thd_manager->release_thread_id(4);
  thd_manager->release_thread_id(UINT_MAX32);
}

#if !defined(DBUG_OFF)
TEST_F(ThreadManagerTest, ThreadIDDeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  my_thread_id thread_id = thd_manager->get_new_thread_id();
  thd_manager->release_thread_id(thread_id);
  // Releasing the same ID twice should assert.
  EXPECT_DEATH_IF_SUPPORTED(thd_manager->release_thread_id(thread_id),
                            ".*Assertion .*1 == num_erased.*");
}
#endif

}  // namespace thd_manager_unittest
