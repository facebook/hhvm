/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "m_string.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"

namespace security_context_unittest {

/*
  Testing accessor functions of string type data members of class
  Security_context.
*/
TEST(Security_context, string_data_member) {
  Security_context sctx;

  // Case 1: Initialize Security context and check the values set.
  EXPECT_EQ(sctx.user().length, (size_t)0);
  EXPECT_EQ(sctx.host().length, (size_t)0);
  EXPECT_EQ(sctx.ip().length, (size_t)0);
  EXPECT_EQ(sctx.external_user().length, (size_t)0);
  EXPECT_EQ(strcmp(sctx.host_or_ip().str, "connecting host"), 0);
  EXPECT_EQ(sctx.priv_user().length, (size_t)0);
  EXPECT_EQ(sctx.proxy_user().length, (size_t)0);
  EXPECT_EQ(sctx.priv_host().length, (size_t)0);

  // Case 2: Set the empty string to Securtiy context members and check values.
  sctx.set_user_ptr("", 0);
  sctx.set_host_ptr("", 0);
  sctx.set_ip_ptr("", 0);
  sctx.set_host_or_ip_ptr();
  sctx.set_external_user_ptr("", 0);
  sctx.assign_priv_user("", 0);
  sctx.assign_proxy_user("", 0);
  sctx.assign_priv_host("", 0);

  EXPECT_EQ(sctx.user().length, (size_t)0);
  EXPECT_EQ(sctx.host().length, (size_t)0);
  EXPECT_EQ(sctx.ip().length, (size_t)0);
  EXPECT_EQ(sctx.external_user().length, (size_t)0);
  EXPECT_EQ(sctx.host_or_ip().length, (size_t)0);
  EXPECT_EQ(sctx.priv_user().length, (size_t)0);
  EXPECT_EQ(sctx.proxy_user().length, (size_t)0);
  EXPECT_EQ(sctx.priv_host().length, (size_t)0);

  // using method assign_xxxx();
  sctx.assign_user("", 0);
  sctx.assign_host("", 0);
  sctx.assign_ip("", 0);
  sctx.assign_external_user("", 0);

  EXPECT_EQ(sctx.user().length, (size_t)0);
  EXPECT_EQ(sctx.host().length, (size_t)0);
  EXPECT_EQ(sctx.ip().length, (size_t)0);
  EXPECT_EQ(sctx.external_user().length, (size_t)0);
  EXPECT_EQ(sctx.host_or_ip().length, (size_t)0);
  EXPECT_EQ(sctx.priv_user().length, (size_t)0);
  EXPECT_EQ(sctx.proxy_user().length, (size_t)0);
  EXPECT_EQ(sctx.priv_host().length, (size_t)0);

  // using  method assign_host() but passing the nullptr to it
  sctx.assign_host(nullptr, 0);
  EXPECT_EQ(sctx.host().length, (size_t)0);

  // Case 3: Set non-empty string to Securtiy context members and check values.
  sctx.set_user_ptr(STRING_WITH_LEN("user_test"));
  sctx.set_host_ptr(STRING_WITH_LEN("localhost"));
  sctx.set_ip_ptr(STRING_WITH_LEN("127.0.0.1"));
  sctx.set_host_or_ip_ptr();
  sctx.set_external_user_ptr(STRING_WITH_LEN("ext_user_test"));
  sctx.assign_priv_user(STRING_WITH_LEN("priv_user"));
  sctx.assign_proxy_user(STRING_WITH_LEN("proxy_user"));
  sctx.assign_priv_host(STRING_WITH_LEN("localhost"));

  EXPECT_EQ(0, strcmp(sctx.user().str, "user_test"));
  EXPECT_EQ(0, strcmp(sctx.host().str, "localhost"));
  EXPECT_EQ(0, strcmp(sctx.ip().str, "127.0.0.1"));
  EXPECT_EQ(0, strcmp(sctx.external_user().str, "ext_user_test"));
  EXPECT_EQ(0, strcmp(sctx.host_or_ip().str, "localhost"));
  EXPECT_EQ(0, strcmp(sctx.priv_user().str, "priv_user"));
  EXPECT_EQ(0, strcmp(sctx.proxy_user().str, "proxy_user"));
  EXPECT_EQ(0, strcmp(sctx.priv_host().str, "localhost"));

  sctx.set_host_or_ip_ptr(sctx.ip().str, sctx.ip().length);
  EXPECT_EQ(0, strcmp(sctx.host_or_ip().str, "127.0.0.1"));

  // Case 4: Change members with non-empty string and check values.
  sctx.set_user_ptr(STRING_WITH_LEN("user_test_1"));
  sctx.set_host_ptr(STRING_WITH_LEN("localhost_1"));
  sctx.set_ip_ptr(STRING_WITH_LEN("127.0.0.2"));
  sctx.set_host_or_ip_ptr();
  sctx.set_external_user_ptr(STRING_WITH_LEN("ext_user_test_1"));
  sctx.assign_priv_user(STRING_WITH_LEN("priv_user_1"));
  sctx.assign_proxy_user(STRING_WITH_LEN("proxy_user_1"));
  sctx.assign_priv_host(STRING_WITH_LEN("localhost_1"));

  EXPECT_EQ(0, strcmp(sctx.user().str, "user_test_1"));
  EXPECT_EQ(0, strcmp(sctx.host().str, "localhost_1"));
  EXPECT_EQ(0, strcmp(sctx.ip().str, "127.0.0.2"));
  EXPECT_EQ(0, strcmp(sctx.external_user().str, "ext_user_test_1"));
  EXPECT_EQ(0, strcmp(sctx.host_or_ip().str, "localhost_1"));
  EXPECT_EQ(0, strcmp(sctx.priv_user().str, "priv_user_1"));
  EXPECT_EQ(0, strcmp(sctx.proxy_user().str, "proxy_user_1"));
  EXPECT_EQ(0, strcmp(sctx.priv_host().str, "localhost_1"));

  // Case 5: Change members with non-empty string members with copy option.
  sctx.assign_user(STRING_WITH_LEN("user_test"));
  sctx.assign_host(STRING_WITH_LEN("localhost"));
  sctx.assign_ip(STRING_WITH_LEN("127.0.0.1"));
  sctx.set_host_or_ip_ptr();
  sctx.assign_external_user(STRING_WITH_LEN("ext_user_test"));
  sctx.assign_priv_user(STRING_WITH_LEN("priv_user"));
  sctx.assign_proxy_user(STRING_WITH_LEN("proxy_user"));
  sctx.assign_priv_host(STRING_WITH_LEN("localhost"));

  EXPECT_EQ(0, strcmp(sctx.user().str, "user_test"));
  EXPECT_EQ(0, strcmp(sctx.host().str, "localhost"));
  EXPECT_EQ(0, strcmp(sctx.ip().str, "127.0.0.1"));
  EXPECT_EQ(0, strcmp(sctx.external_user().str, "ext_user_test"));
  EXPECT_EQ(0, strcmp(sctx.host_or_ip().str, "localhost"));
  EXPECT_EQ(0, strcmp(sctx.priv_user().str, "priv_user"));
  EXPECT_EQ(0, strcmp(sctx.proxy_user().str, "proxy_user"));
  EXPECT_EQ(0, strcmp(sctx.priv_host().str, "localhost"));
}

}  // namespace security_context_unittest
