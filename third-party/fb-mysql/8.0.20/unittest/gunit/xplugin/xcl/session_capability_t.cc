/*
 *
 Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <gtest/gtest.h>
#include <memory>

#include "unittest/gunit/xplugin/xcl/session_t.h"

namespace xcl {
namespace test {

TEST_F(Xcl_session_impl_tests, xsession_set_capability_successful) {
  const bool value_bool = true;
  const Argument_uobject value_object;

  ASSERT_FALSE(m_sut->set_capability(
      XSession::Capability_can_handle_expired_password, value_bool));

  ASSERT_FALSE(m_sut->set_capability(XSession::Capability_session_connect_attrs,
                                     value_object));
}

TEST_F(Xcl_session_impl_tests, xsession_set_capability_unsuccessful) {
  const char *value_pchar = "pchar";
  const std::string value_string = "std::string";
  const int64_t value_int = 10;
  const Argument_uobject value_object;
  const bool value_bool = true;

  ASSERT_TRUE(m_sut->set_capability(
      XSession::Capability_can_handle_expired_password, value_pchar));
  ASSERT_TRUE(m_sut->set_capability(
      XSession::Capability_can_handle_expired_password, value_string));
  ASSERT_TRUE(m_sut->set_capability(
      XSession::Capability_can_handle_expired_password, value_int));
  ASSERT_TRUE(m_sut->set_capability(
      XSession::Capability_can_handle_expired_password, value_object));

  ASSERT_TRUE(m_sut->set_capability(XSession::Capability_session_connect_attrs,
                                    value_pchar));
  ASSERT_TRUE(m_sut->set_capability(XSession::Capability_session_connect_attrs,
                                    value_string));
  ASSERT_TRUE(m_sut->set_capability(XSession::Capability_session_connect_attrs,
                                    value_int));
  ASSERT_TRUE(m_sut->set_capability(XSession::Capability_session_connect_attrs,
                                    value_bool));
}

}  // namespace test
}  // namespace xcl
