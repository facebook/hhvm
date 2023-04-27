/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "mysql_com.h"

#include "plugin/x/src/cache_based_verification.h"
#include "plugin/x/src/native_plain_verification.h"
#include "plugin/x/src/native_verification.h"
#include "plugin/x/src/sha256_password_cache.h"
#include "plugin/x/src/sha256_plain_verification.h"
#include "plugin/x/src/sha2_plain_verification.h"
#include "sha1.h"  // for SHA1_HASH_SIZE
#include "unittest/gunit/xplugin/xpl/mock/sha256_password_cache.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class User_password_verification : public ::testing::Test {
 public:
  void SetUp() {
    m_cache_mock.reset(new Mock_sha256_password_cache());
    m_cached_value = {std::begin(CACHED_VALUE_TABLE),
                      std::end(CACHED_VALUE_TABLE)};
  }

  std::unique_ptr<Mock_sha256_password_cache> m_cache_mock;

  const char *const EMPTY = "";
  const char *const EXPECTED_NATIVE_HASH =
      "*BF201911C951DCC0264E2C7577977E0A3EF06695";
  const char *const EXPECTED_SHA256_HASH =
      "$5$1S> j#F2}Vz3yqu`fC8X$HrURSrHutEhr6orwomWpNiRquOS/xy9DzQFj5TuVHn0";
  const char *const WRONG_PASSWD = "ALA_MA_KACA";
  const char *const GOOD_PASSWD = "ALA_MA_KOTA";

  const std::string SHA256_MEMORY_CLIENT_STRING =
      "9F85587C3992CA48B17B727FAAC84299093B40E83797497980A58931342DA5BE";
  const std::string MADE_UP_SALT = std::string(20, 'x');
  const unsigned char CACHED_VALUE_TABLE[64] = {
      0x22, 0xf5, 0xa3, 0xa4, 0x64, 0x7f, 0x33, 0xe5, 0x3a, 0xe8, 0x2c,
      0x18, 0x1a, 0x51, 0x33, 0xe6, 0x2c, 0x47, 0x56, 0x21, 0x65, 0xb0,
      0x62, 0xd0, 0x37, 0x7b, 0x1e, 0x3c, 0x79, 0x57, 0xc2, 0xde};
  std::string m_cached_value;
};

TEST_F(User_password_verification, native_plain_verification_get_salt) {
  Native_plain_verification verificator{m_cache_mock.get()};
  ASSERT_STREQ(EMPTY, verificator.get_salt().c_str());
}

TEST_F(User_password_verification, native_plain_verification_pass) {
  EXPECT_CALL(*m_cache_mock.get(), contains("user", "host", GOOD_PASSWD));
  EXPECT_CALL(*m_cache_mock.get(), upsert("user", "host", GOOD_PASSWD));
  Native_plain_verification verificator{m_cache_mock.get()};
  ASSERT_TRUE(verificator.verify_authentication_string(
      "user", "host", GOOD_PASSWD, EXPECTED_NATIVE_HASH));
}

TEST_F(User_password_verification, native_plain_verification_fail) {
  EXPECT_CALL(*m_cache_mock.get(), contains("user", "host", WRONG_PASSWD));
  EXPECT_CALL(*m_cache_mock.get(), upsert(_, _, _)).Times(0);
  Native_plain_verification verificator{m_cache_mock.get()};
  ASSERT_FALSE(verificator.verify_authentication_string(
      "user", "host", WRONG_PASSWD, EXPECTED_NATIVE_HASH));
}

TEST_F(User_password_verification, native_verification_get_salt) {
  Native_verification verificator{m_cache_mock.get()};
  ASSERT_STRNE(EMPTY, verificator.get_salt().c_str());
}

std::string get_hash(const std::string &salt, const std::string &user_string) {
  char scrambled[SCRAMBLE_LENGTH + 1] = {0};
  char hash[2 * SHA1_HASH_SIZE + 2] = {0};
  ::scramble(scrambled, salt.c_str(), user_string.c_str());
  ::make_password_from_salt(hash, reinterpret_cast<const uint8_t *>(scrambled));
  return hash;
}

TEST_F(User_password_verification, native_verification_pass) {
  Native_verification verificator{m_cache_mock.get()};
  EXPECT_CALL(*m_cache_mock.get(), contains(_, _, _)).Times(0);
  EXPECT_CALL(*m_cache_mock.get(), upsert(_, _, _)).Times(0);
  ASSERT_TRUE(verificator.verify_authentication_string(
      "", "", get_hash(verificator.get_salt(), GOOD_PASSWD),
      EXPECTED_NATIVE_HASH));
}

TEST_F(User_password_verification, native_verification_fail) {
  Native_verification verificator{m_cache_mock.get()};
  EXPECT_CALL(*m_cache_mock.get(), contains(_, _, _)).Times(0);
  EXPECT_CALL(*m_cache_mock.get(), upsert(_, _, _)).Times(0);
  ASSERT_FALSE(verificator.verify_authentication_string(
      "", "", get_hash(verificator.get_salt(), WRONG_PASSWD),
      EXPECTED_NATIVE_HASH));
}

TEST_F(User_password_verification, sha256_plain_verification_get_salt) {
  Sha256_plain_verification verificator{m_cache_mock.get()};
  ASSERT_STREQ(EMPTY, verificator.get_salt().c_str());
}

TEST_F(User_password_verification, sha256_plain_verification_pass) {
  EXPECT_CALL(*m_cache_mock.get(), contains("user", "host", GOOD_PASSWD));
  EXPECT_CALL(*m_cache_mock.get(), upsert("user", "host", GOOD_PASSWD));
  Sha256_plain_verification verificator{m_cache_mock.get()};
  ASSERT_TRUE(verificator.verify_authentication_string(
      "user", "host", GOOD_PASSWD, EXPECTED_SHA256_HASH));
}

TEST_F(User_password_verification, sha256_plain_verification_fail) {
  EXPECT_CALL(*m_cache_mock.get(), contains("user", "host", WRONG_PASSWD));
  EXPECT_CALL(*m_cache_mock.get(), upsert(_, _, _)).Times(0);
  Sha256_plain_verification verificator{m_cache_mock.get()};
  ASSERT_FALSE(verificator.verify_authentication_string(
      "user", "host", WRONG_PASSWD, EXPECTED_SHA256_HASH));
}

TEST_F(User_password_verification, sha256_memory_verification_get_salt) {
  Cache_based_verification verificator{m_cache_mock.get()};
  ASSERT_STRNE(EMPTY, verificator.get_salt().c_str());
}

TEST_F(User_password_verification, sha256_memory_verification_pass) {
  EXPECT_CALL(*m_cache_mock.get(), get_entry("user", "host"))
      .WillOnce(Return(std::pair<bool, std::string>{true, m_cached_value}));
  Mock_cache_based_verification verificator{m_cache_mock.get()};
  EXPECT_CALL(verificator, get_salt()).WillRepeatedly(ReturnRef(MADE_UP_SALT));
  ASSERT_TRUE(verificator.verify_authentication_string(
      "user", "host", SHA256_MEMORY_CLIENT_STRING, ""));
}

TEST_F(User_password_verification, sha256_memory_verification_no_entry) {
  EXPECT_CALL(*m_cache_mock.get(), get_entry("user", "host"))
      .WillOnce(Return(std::pair<bool, std::string>{false, ""}));
  Mock_cache_based_verification verificator{m_cache_mock.get()};
  EXPECT_CALL(verificator, get_salt()).Times(0);
  ASSERT_FALSE(verificator.verify_authentication_string(
      "user", "host", SHA256_MEMORY_CLIENT_STRING, ""));
}

TEST_F(User_password_verification, sha256_memory_verification_fail) {
  const std::string bogus_entry(32, 'z');
  EXPECT_CALL(*m_cache_mock.get(), get_entry("user", "host"))
      .WillOnce(Return(std::pair<bool, std::string>{true, bogus_entry}));
  Mock_cache_based_verification verificator{m_cache_mock.get()};
  EXPECT_CALL(verificator, get_salt()).WillRepeatedly(ReturnRef(MADE_UP_SALT));
  ASSERT_FALSE(verificator.verify_authentication_string(
      "user", "host", SHA256_MEMORY_CLIENT_STRING, ""));
}

}  // namespace test
}  // namespace xpl
