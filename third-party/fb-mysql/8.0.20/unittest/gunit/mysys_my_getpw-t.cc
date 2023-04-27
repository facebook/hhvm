/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#ifdef HAVE_GETPWNAM
#include <gtest/gtest.h>
#include "my_getpwnam.h"

// Unit tests for mysys my_getpwnam() and my_getpwuid()

namespace mysys_my_getpw {

// Verify that a default-constructed PasswdValue is void
TEST(MysysMyGetpw, DefaultPasswdValue) {
  PasswdValue pv;
  EXPECT_TRUE(pv.IsVoid());
}
// Verify basic value operations
TEST(MysysMyGetpw, BasicValueOperations) {
  PasswdValue pv;
  pv.pw_name = "root";
  PasswdValue copy = pv;
  EXPECT_EQ(std::string{"root"}, copy.pw_name);
  PasswdValue moved = std::move(copy);
  EXPECT_EQ(std::string{"root"}, moved.pw_name);
  PasswdValue assigned;
  assigned = pv;
  EXPECT_EQ(std::string{"root"}, assigned.pw_name);
}

// Verify that it is possible to retrive entry for root using symbolic name
TEST(MysysMyGetpw, GetpwnamRoot) {
  PasswdValue rootpw = my_getpwnam("root");
  EXPECT_EQ(std::string{"root"}, rootpw.pw_name);
  EXPECT_EQ(0u, rootpw.pw_uid);
  EXPECT_EQ(0u, rootpw.pw_gid);
}

// Verify that it is possible to retrive entry for root using numeric id
TEST(MysysMyGetpw, GetpwuidRoot) {
  PasswdValue rootpw = my_getpwuid(0);
  EXPECT_EQ(std::string{"root"}, rootpw.pw_name);
  EXPECT_EQ(0u, rootpw.pw_uid);
  EXPECT_EQ(0u, rootpw.pw_gid);
}

// Verify that non-existent user give void value
TEST(MysysMyGetpw, NonExistentUser) {
  PasswdValue none = my_getpwnam("thereisnosuchuser___");
  EXPECT_TRUE(none.IsVoid());
}

}  // namespace mysys_my_getpw
#endif /* HAVE_GETPWNAM */
