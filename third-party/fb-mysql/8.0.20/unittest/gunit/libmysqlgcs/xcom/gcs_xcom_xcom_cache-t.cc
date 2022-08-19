/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <string>
#include <vector>

#include "gcs_base_test.h"

#include "app_data.h"
#include "synode_no.h"
#include "xcom_base.h"
#include "xcom_cache.h"
#include "xcom_cfg.h"

namespace gcs_xcom_xcom_unittest {

void setup_cache(uint64_t increment) {
  set_length_increment(increment);
  set_size_decrement(increment);
  init_cache();
}

void cleanup_cache() { deinit_cache(); }

/**
 * Mocks the XCom maintenance task.
 */
void *cache_task(void *ptr) {
  bool *run = (bool *)ptr;
  while (*run) {
    do_cache_maintenance();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return nullptr;
}

class GcsXComXComCache : public GcsBaseTest {
 public:
  const std::string *m_addr;
  char m_payload[512];
  u_int m_payload_size;
  node_address *m_na;
  site_def *m_sd;
  app_data m_a;
  synode_no m_synode;
  My_xp_thread_impl *m_thread;
  bool m_run;

 protected:
  GcsXComXComCache()
      : m_addr(nullptr),
        m_payload_size(0),
        m_na(nullptr),
        m_sd(nullptr),
        m_thread(nullptr),
        m_run(false) {}
  ~GcsXComXComCache() {}

  virtual void SetUp() {
    m_synode = {1, 1, 0};
    m_addr = new std::string("127.0.0.1:12345");
    char *names[] = {const_cast<char *>(m_addr->c_str())};
    m_na = new_node_address(1, names);
    m_sd = new_site_def();
    init_site_def(1, m_na, m_sd);
    push_site_def(m_sd);
    m_payload_size = 512 - sizeof(pax_msg) - sizeof(app_data);
    init_app_msg(&m_a, m_payload, m_payload_size);
    init_cfg_app_xcom();
  }

  virtual void TearDown() {
    m_run = false;
    if (m_thread) {
      m_thread->join(nullptr);
      delete m_thread;
    }
    push_site_def(nullptr);
    free_site_defs();
    delete_node_address(1, m_na);
    cleanup_cache();
    deinit_cfg_app_xcom();
    delete m_addr;
  }

  virtual void cache_msg(synode_no synode) {
    pax_machine *pm = nullptr;
    pm = get_cache(synode);
    ASSERT_TRUE(pm != nullptr);
    ASSERT_TRUE(synode_eq(pm->synode, synode));
    unchecked_replace_pax_msg(&pm->proposer.msg, pax_msg_new(synode, m_sd));
    pm->proposer.msg->a = clone_app_data(&m_a);
    add_cache_size(pm);
  }

  virtual void cache_bulk(size_t num_msg) {
    while (m_synode.msgno <= num_msg) {
      cache_msg(m_synode);
      m_synode.msgno++;
    }
  }

  virtual size_t msg_size() {
    return sizeof(pax_msg) + sizeof(app_data) + m_payload_size;
  }

  virtual void basic_test_generic(size_t increment, size_t target_occupation) {
    setup_cache(increment);
    ASSERT_EQ(get_xcom_cache_length(), increment);
    ASSERT_EQ(get_xcom_cache_occupation(), 0);
    ASSERT_EQ(get_xcom_cache_size(), 0);

    cache_bulk(target_occupation);

    ASSERT_EQ(get_xcom_cache_occupation(), target_occupation);
    ASSERT_EQ(get_xcom_cache_size(), target_occupation * (msg_size()));
    size_t expected_length =
        (floor((double)target_occupation / (double)increment) + 1) * increment;
    ASSERT_EQ(get_xcom_cache_length(), expected_length);
  }
};

/**
 * Basic test to verify that cache grows as needed.
 */
TEST_F(GcsXComXComCache, XComCacheTestDefaults) {
  basic_test_generic(50000, 500000);
  ASSERT_EQ(get_xcom_cache_length(), 550000);
}

/**
 * Checks the boundaries of increment: cache only grows in 50k increments and
 * the increment is triggered when the last empty slot is occupied. In the
 * previous test the cache reached the limit when it hit the 500k slots and was
 * incremented by 50k. In this test the cache will not be incremented because
 * it still has one slots left.
 */
TEST_F(GcsXComXComCache, XComCacheTestIncrementBelow) {
  basic_test_generic(50000, 499999);
  ASSERT_EQ(get_xcom_cache_length(), 500000);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

TEST_F(GcsXComXComCache, XComCacheTestIncrementAbove) {
  basic_test_generic(50000, 500001);
  ASSERT_EQ(get_xcom_cache_length(), 550000);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Stress test with a large cache.
 */
TEST_F(GcsXComXComCache, XComCacheTestDefaultsLargeCache) {
  basic_test_generic(50000, 3000000);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Test to exert some extra stress on the system by having a large number of
 * small hash tables.
 */
TEST_F(GcsXComXComCache, XComCacheTestLargeCacheSmallConfig) {
  basic_test_generic(100, 3000000);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Same as above, but with even smaller hash tables.
 */
TEST_F(GcsXComXComCache, XComCacheTestLargeCacheSmallConfig2) {
  basic_test_generic(10, 3000000);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Iterates the cache starting with the oldest message. Simulates the recovery
 * of an unreachable node that just got back into the group.
 */
TEST_F(GcsXComXComCache, XComCacheTestIterateForward) {
  basic_test_generic(50000, 3000000);
  synode_no synode = {1, 1, 0};
  while (synode.msgno < m_synode.msgno) {
    pax_machine *pm = nullptr;
    pm = get_cache(synode);
    ASSERT_TRUE(pm != nullptr);
    ASSERT_TRUE(synode_eq(pm->synode, synode));
    synode.msgno++;
  }
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * For performance comparison with the test above, just does 5M accesses to
 * the most recent message.
 */
TEST_F(GcsXComXComCache, XComCacheTestAccessRecent) {
  basic_test_generic(50000, 3000000);
  u_int iterations = 3000000;
  while (iterations > 0) {
    pax_machine *pm = nullptr;
    pm = get_cache(m_synode);
    ASSERT_TRUE(pm != nullptr);
    ASSERT_TRUE(synode_eq(pm->synode, m_synode));
    iterations--;
  }
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Tests the length decrease process by gradually changing configuration
 * parameters. The management itself is made manually in the test by explicitly
 * calling the management functions.
 */
TEST_F(GcsXComXComCache, XComCacheTestLengthDecrease) {
  size_t target_occupation = 450000;  // 450K
  basic_test_generic(50000, target_occupation);
  ASSERT_EQ(get_xcom_cache_occupation(), 450000);
  ASSERT_EQ(get_xcom_cache_length(), 500000);
  /*
    Cache refuses to decrease before it reaches 500K slots
  */
  ASSERT_EQ(check_decrease(), 1);

  target_occupation = 3000000;  // 3M
  cache_bulk(target_occupation);
  ASSERT_EQ(get_xcom_cache_occupation(), 3000000);
  ASSERT_EQ(get_xcom_cache_length(), 3050000);
  /*
    Fails because there are no empty hash tables in the stack
  */
  ASSERT_EQ(check_decrease(), 2);

  size_t count = 0;
  while (above_cache_limit()) {
    count += shrink_cache();
  }
  /**
    The size of the cache decreasead from 1536000000 to below 1000000000.
    With a message size of 512, shrink_cache() had to remove 3046875 messages:
      -> 1536000000 - 1000000000 = 536000000
      -> 536000000 / 512 = 1046875
   */
  ASSERT_EQ(count, 1046875);
  ASSERT_EQ(get_xcom_cache_occupation(), 1953125);  // init occupation - count
  ASSERT_EQ(get_xcom_cache_length(), 3050000);      // Length did not change
  ASSERT_EQ(check_decrease(), 0);
  ASSERT_EQ(get_xcom_cache_length(), 3000000);
  while (!check_decrease()) {
  }
  /*
    Fails because occupation >= cache_length * min_target_occupation
    (1953125 / 0.7) == 2790178. Since the cache decreases in 50k slots,
    decrease will stop once cache_length hits the lowest 50k decrement
    that is lower than 3892564, which is 2750000
  */
  ASSERT_EQ(check_decrease(), 3);
  ASSERT_EQ(get_xcom_cache_occupation(), 1953125);
  ASSERT_EQ(get_xcom_cache_length(), 2750000);

  set_min_target_occupation(1.0);  // Force previous test to pass
  while (!check_decrease()) {
  }
  /*
   * Now it fails because
   * (cache_length - BUCKETS) * min_occupation <= occupation):
   * 1953125 / 0.9 = 2170139 ; 2170139 + 50000 = 2220139
   *
   */
  ASSERT_EQ(check_decrease(), 4);
  ASSERT_EQ(get_xcom_cache_length(), 2200000);
  set_min_length_threshold(1);  // Force previous test to pass
  ASSERT_EQ(check_decrease(), 0);
  set_max_cache_size(2000000000);
  /*
    Verify that decrease fails because cache_size is still far from the limit.
  */
  ASSERT_EQ(check_decrease(), 5);
  // Reset vars
  set_min_target_occupation(MIN_TARGET_OCCUPATION);
  set_min_length_threshold(MIN_LENGTH_THRESHOLD);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

/**
 * Similar to the previous test, but instead of calling the management functions
 * directly, sets up a background task mocking the management task of the XCom
 * cache.
 */
TEST_F(GcsXComXComCache, XComCacheTestLengthDecreaseWithTask) {
  size_t target_occupation = 3000000;
  basic_test_generic(50000, target_occupation);
  ASSERT_EQ(get_xcom_cache_size(), target_occupation * (msg_size()));
  ASSERT_EQ(get_xcom_cache_occupation(), 3000000);
  ASSERT_EQ(get_xcom_cache_length(), 3050000);
  m_thread = new My_xp_thread_impl();
  m_run = true;
  m_thread->create(PSI_NOT_INSTRUMENTED, nullptr, cache_task, (void *)&m_run);
  /*
    Wait for the task to decrease the size
  */
  while (above_cache_limit()) {
    sleep(1);
  }

  ASSERT_EQ(get_xcom_cache_occupation(), 1953125);
  /*
    After XCom decrease the size, it should start decreasing the length.
    Decrease stops when (occupation >= cache_length * min_target_occupation)
    (1953125 / 0.7) == 2790178. Since the cache decreases in 50k slots,
    decrease will stop once cache_length hits the lowest 50k decrement
    that is lower than 3892564, which is 2750000
  */
  while (get_xcom_cache_length() != 2750000) {
    sleep(1);
  }

  ASSERT_EQ(get_xcom_cache_occupation(), 1953125);
  ASSERT_EQ(get_xcom_cache_length(), 2750000);  // Redundant, but let's do it!
  ASSERT_EQ(check_decrease(), 3);

  set_min_target_occupation(1.0);  // Force previous test to pass
  /*
    Now it fails because
    (cache_length - BUCKETS) * min_occupation <= occupation):
    1953125 / 0.9 = 2170139 ; 2170139 + 50000 = 2220139
  */
  while (get_xcom_cache_length() != 2200000) {
    sleep(1);
  }

  ASSERT_EQ(get_xcom_cache_length(), 2200000);
  ASSERT_EQ(check_decrease(), 4);

  /*
   * Increase the max cache size before "removing" the min length threshold
   */
  set_max_cache_size(2000000000);

  set_min_length_threshold(1);  // Force previous test to pass
  // Check that decrease still has no effect, but for a different reason
  ASSERT_EQ(check_decrease(), 5);
  // Restore max cache size
  set_max_cache_size(1000000000);

  while (get_xcom_cache_length() != 2050000) {
    sleep(1);
  }

  // The decrease stops once the length gets to 2050000...
  ASSERT_EQ(get_xcom_cache_length(), 2050000);
  // ...because we have no more empty hash tables at the end of the stack
  ASSERT_EQ(check_decrease(), 2);
  deinit_cache();
  ASSERT_EQ(get_xcom_cache_length(), 0);
  ASSERT_EQ(get_xcom_cache_occupation(), 0);
  ASSERT_EQ(get_xcom_cache_size(), 0);
}

}  // namespace gcs_xcom_xcom_unittest
