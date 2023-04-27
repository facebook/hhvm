/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

#include "unittest/gunit/xplugin/xcl/session_t.h"

namespace xcl {
namespace test {

TEST_F(Xcl_session_impl_tests, get_protocol_returns_mock) {
  ASSERT_EQ(m_mock_protocol, &m_sut->get_protocol());
}

TEST_F(Xcl_session_impl_tests, get_client_id) {
  const bool local_notice = false;
  const XProtocol::Client_id expected_client_id = 1220;
  std::string payload;

  encode_session_state_change(
      Mysqlx::Notice::SessionStateChanged::CLIENT_ID_ASSIGNED,
      expected_client_id, &payload);

  /* Connection was not started, session_id must
   be not initialized. */
  ASSERT_EQ(XCL_CLIENT_ID_NOT_VALID, m_sut->client_id());

  /* Update the session_id */
  ASSERT_EQ(Handler_result::Consumed,
            m_out_message_handler(m_mock_protocol, local_notice,
                                  Mysqlx::Notice::Frame::SESSION_STATE_CHANGED,
                                  payload.c_str(),
                                  static_cast<uint32_t>(payload.length())));

  /* Verify that the session object reports the new ID */
  ASSERT_EQ(expected_client_id, m_sut->client_id());
}

}  // namespace test
}  // namespace xcl
