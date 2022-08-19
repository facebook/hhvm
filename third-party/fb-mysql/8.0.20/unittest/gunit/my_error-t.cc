/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <errno.h>
#include <gtest/gtest.h>
#include <string.h>

#include "m_string.h"  // native_strcasecmp
#include "my_base.h"   // HA_ERR_KEY_NOT_FOUND
#include "my_config.h"
#include "my_sys.h"  // my_strerror()

namespace my_error_unittest {

TEST(MyErrorTest, MyStrErrorSystem) {
  char buf[512];
  const char *msg;

  msg = my_strerror(buf, sizeof(buf) - 1, 9999);
  EXPECT_TRUE(!native_strcasecmp("unknown error", msg) ||
              !native_strcasecmp("unknown error: 9999", msg) ||
              !native_strcasecmp("unknown error 9999", msg))
      << "msg<" << msg << ">";

  // try a proper error number
  msg = my_strerror(buf, sizeof(buf) - 1, EPERM);
  const char *os_msg = strerror(EPERM);
  EXPECT_STREQ(os_msg, msg) << "msg<" << msg << ">";
}

TEST(MyErrorTest, MyStrErrorHandlerPlugin) {
  char buf[512];
  const char *msg;

  // try a HA error number
  msg = my_strerror(buf, sizeof(buf) - 1, HA_ERR_KEY_NOT_FOUND);
  EXPECT_STREQ("Didn't find key on read or update", msg);
}

TEST(MyErrorTest, MyGetErrMsgUninitialized) {
  const char *msg;

  msg = my_get_err_msg(HA_ERR_KEY_NOT_FOUND);
  EXPECT_TRUE(msg == nullptr);
}

const char *faux_errmsgs[] = {"alpha", "beta", nullptr, "delta"};

static const int faux_error_first = 8000;
static const int faux_error_last = 8003;

const char *get_faux_errmsg(int nr) {
  return faux_errmsgs[nr - faux_error_first];
}

TEST(MyErrorTest, MyGetErrMsgInitialized) {
  const char *msg;

  EXPECT_EQ(
      0, my_error_register(get_faux_errmsg, faux_error_first, faux_error_last));

  // flag error when trying to register overlapping area
  EXPECT_NE(0, my_error_register(get_faux_errmsg, faux_error_first + 2,
                                 faux_error_last + 2));

  msg = my_get_err_msg(faux_error_first);
  EXPECT_STREQ("alpha", msg);

  msg = my_get_err_msg(faux_error_first + 1);
  EXPECT_STREQ("beta", msg);

  // within range. gives NULL here. higher level function will
  // substitute a default string before printing.
  msg = my_get_err_msg(faux_error_first + 2);
  EXPECT_TRUE(msg == nullptr);

  // out of range
  msg = my_get_err_msg(faux_error_first - 1);
  EXPECT_TRUE(msg == nullptr);

  msg = my_get_err_msg(faux_error_last);
  EXPECT_STREQ("delta", msg);

  // out of range
  msg = my_get_err_msg(faux_error_last + 1);
  EXPECT_TRUE(msg == nullptr);

  EXPECT_FALSE(my_error_unregister(faux_error_first, faux_error_last));

  // flag error when trying to unregister twice
  EXPECT_TRUE(my_error_unregister(faux_error_first, faux_error_last));
}

}  // namespace my_error_unittest
