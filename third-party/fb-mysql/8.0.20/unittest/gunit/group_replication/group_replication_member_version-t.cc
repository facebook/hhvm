/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "plugin/group_replication/include/member_version.h"

namespace member_version_unittest {

class MemberVersionTest : public ::testing::Test {
 protected:
  MemberVersionTest() {}

  virtual void SetUp() {
    version = new Member_version(0x010206);  // version: 1.2.6
  }

  virtual void TearDown() { delete version; }

  Member_version *version;
};

TEST_F(MemberVersionTest, AssertFullVersion) {
  ASSERT_EQ(0x010206, (int)this->version->get_version());
}

TEST_F(MemberVersionTest, AssertMajorVersion) {
  ASSERT_EQ(1, (int)this->version->get_major_version());
}

TEST_F(MemberVersionTest, AssertMinorVersion) {
  ASSERT_EQ(2, (int)this->version->get_minor_version());
}

TEST_F(MemberVersionTest, AssertPatchVersion) {
  ASSERT_EQ(6, (int)this->version->get_patch_version());
}

TEST_F(MemberVersionTest, AssertEqualsOperator) {
  Member_version another_version(0x010206);  // version: 1.2.6

  ASSERT_TRUE(*version == another_version);
}

TEST_F(MemberVersionTest, AssertLtOperator) {
  Member_version same_version(0x010206);  // version: 1.2.6

  ASSERT_FALSE(*version < same_version);

  Member_version major_major_version(0x020206);  // version: 2.2.6

  ASSERT_TRUE(*version < major_major_version);

  Member_version major_minor_version(0x010306);  // version: 1.3.6

  ASSERT_TRUE(*version < major_minor_version);

  Member_version major_patch_version(0x010207);  // version: 1.2.7

  ASSERT_TRUE(*version < major_patch_version);
}

TEST_F(MemberVersionTest, AssertGtOperator) {
  Member_version same_version(0x010206);  // version: 1.2.6

  ASSERT_FALSE(*version > same_version);

  Member_version minor_major_version(0x000206);  // version: 0.2.6

  ASSERT_TRUE(*version > minor_major_version);

  Member_version minor_minor_version(0x010106);  // version: 1.1.6

  ASSERT_TRUE(*version > minor_minor_version);

  Member_version minor_patch_version(0x010205);  // version: 1.2.5

  ASSERT_TRUE(*version > minor_patch_version);
}

TEST_F(MemberVersionTest, AssertGtEqualsOperator) {
  Member_version same_version(0x010206);  // version: 1.2.6

  ASSERT_TRUE(*version >= same_version);

  Member_version lower_version(0x010205);  // version: 1.2.5

  ASSERT_TRUE(*version >= lower_version);
}

TEST_F(MemberVersionTest, AssertLtEqualsOperator) {
  Member_version same_version(0x010206);  // version: 1.2.6

  ASSERT_TRUE(*version <= same_version);

  Member_version higher_version(0x010207);  // version: 1.2.7

  ASSERT_TRUE(*version <= higher_version);
}

}  // namespace member_version_unittest
