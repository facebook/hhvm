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
#include "my_getpwnam.h"  // PasswdValue

// Unit tests for functions in mysqld.cc.
extern bool opt_initialize;
namespace mysqld_funcs_unit_test {
PasswdValue check_user_drv(const char *user);

TEST(MysqldFuncs, CheckUser) {
  EXPECT_TRUE(check_user_drv("root").IsVoid());

  if (geteuid() == 0) {
    // Running as root
    EXPECT_FALSE(check_user_drv("0").IsVoid());
    EXPECT_FALSE(check_user_drv("1").IsVoid());
    EXPECT_FALSE(check_user_drv("bin").IsVoid());
  } else {
    // These would trigger unireg_abort if run as root, and
    // unireg_abort currently triggers crash if run in a unit test
    EXPECT_TRUE(check_user_drv(nullptr).IsVoid());
    EXPECT_TRUE(check_user_drv("thereisnosuchuser___").IsVoid());
    EXPECT_TRUE(check_user_drv("0").IsVoid());
    EXPECT_TRUE(check_user_drv("0abc").IsVoid());
    EXPECT_TRUE(check_user_drv("1").IsVoid());
    EXPECT_TRUE(check_user_drv("bin").IsVoid());
  }
}
}  // namespace mysqld_funcs_unit_test
#endif /* HAVE_GETPWNAM */
