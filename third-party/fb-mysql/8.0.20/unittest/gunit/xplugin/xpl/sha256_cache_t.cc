/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/sha256_password_cache.h"

namespace xpl {

namespace test {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Expectation;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::SetArrayArgument;
using ::testing::SetErrnoAndReturn;
using ::testing::StrictMock;

class Sha256_cache_test_suite : public ::testing::Test {
 public:
  void SetUp() { m_cache.enable(); }

  Sha256_cache_test_suite() = default;

  xpl::SHA256_password_cache m_cache;
};

TEST_F(Sha256_cache_test_suite, add_entry_cache_enabled) {
  ASSERT_TRUE(m_cache.size() == 0);
  m_cache.upsert("user1", "host1", "value1");
  m_cache.upsert("user2", "host2", "value2");
  ASSERT_TRUE(m_cache.size() == 2);
}

TEST_F(Sha256_cache_test_suite, add_entry_cache_disabled) {
  ASSERT_TRUE(m_cache.size() == 0);
  m_cache.upsert("user1", "host1", "value1");
  ASSERT_TRUE(m_cache.size() == 1);
  m_cache.disable();
  ASSERT_TRUE(m_cache.size() == 0);
  m_cache.upsert("user2", "host2", "value2");
  m_cache.upsert("user3", "host3", "value3");
  ASSERT_TRUE(m_cache.size() == 0);
}

TEST_F(Sha256_cache_test_suite, update_entry) {
  m_cache.upsert("user", "host", "value1");
  m_cache.upsert("user", "host", "value2");
  ASSERT_TRUE(m_cache.size() == 1);
}

TEST_F(Sha256_cache_test_suite, get_existing_entry) {
  m_cache.upsert("user1", "host1", "value1");
  auto optional_entry = m_cache.get_entry("user1", "host1");
  ASSERT_TRUE(optional_entry.first);
  ASSERT_FALSE(optional_entry.second.empty());
}

TEST_F(Sha256_cache_test_suite, get_nonexistent_entry) {
  m_cache.upsert("user1", "host1", "value1");
  auto optional_entry = m_cache.get_entry("bogus_user", "host1");
  ASSERT_FALSE(optional_entry.first);
  ASSERT_TRUE(optional_entry.second.empty());
  optional_entry = m_cache.get_entry("user1", "bogus host");
  ASSERT_FALSE(optional_entry.first);
  ASSERT_TRUE(optional_entry.second.empty());
}

TEST_F(Sha256_cache_test_suite, get_entry_cache_disabled) {
  m_cache.upsert("user1", "host1", "value1");
  m_cache.disable();
  auto optional_entry = m_cache.get_entry("user1", "host1");
  ASSERT_FALSE(optional_entry.first);
  ASSERT_TRUE(optional_entry.second.empty());
}

TEST_F(Sha256_cache_test_suite, find_entry) {
  m_cache.upsert("user1", "host1", "value1");
  EXPECT_TRUE(m_cache.contains("user1", "host1", "value1"));
  EXPECT_FALSE(m_cache.contains("bogus user", "host1", "value1"));
  EXPECT_FALSE(m_cache.contains("user1", "bogus host", "value1"));
  EXPECT_FALSE(m_cache.contains("user1", "host1", "bogus value"));
}

TEST_F(Sha256_cache_test_suite, find_entry_cache_disabled) {
  m_cache.upsert("user1", "host1", "value1");
  m_cache.disable();
  EXPECT_FALSE(m_cache.contains("user1", "host1", "value1"));
  EXPECT_FALSE(m_cache.contains("bogus user", "host1", "value1"));
  EXPECT_FALSE(m_cache.contains("user1", "bogus host", "value1"));
  EXPECT_FALSE(m_cache.contains("user1", "host1", "bogus value"));
}

TEST_F(Sha256_cache_test_suite, check_entry_hashing) {
  m_cache.upsert("user1", "host1", "value1");
  auto optional_entry = m_cache.get_entry("user1", "host1");
  ASSERT_TRUE(optional_entry.first);
  ASSERT_NE("value1", optional_entry.second);
}

TEST_F(Sha256_cache_test_suite, remove_entry) {
  m_cache.upsert("user1", "host1", "value1");
  m_cache.upsert("user2", "host2", "value2");
  ASSERT_TRUE(m_cache.size() == 2);
  m_cache.remove("user1", "host1");
  ASSERT_TRUE(m_cache.size() == 1);
}

TEST_F(Sha256_cache_test_suite, clear_cache) {
  m_cache.upsert("user1", "host1", "value1");
  m_cache.upsert("user2", "host2", "value2");
  ASSERT_TRUE(m_cache.size() == 2);
  m_cache.clear();
  ASSERT_TRUE(m_cache.size() == 0);
}

}  // namespace test
}  // namespace xpl
