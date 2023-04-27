/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
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

#include "unittest/gunit/xplugin/xcl/protocol_t.h"

namespace xcl {
namespace test {

class Xcl_protocol_impl_error_tests : public Xcl_protocol_impl_tests {
 public:
  const int expected_error_code = 2333;
  const char *expected_error_txt = "error text";
};

TEST_F(Xcl_protocol_impl_error_tests, execute_close) {
  XProtocol::Server_message_type_id out_smid;
  XProtocol::Client_message_type_id out_cmid{
      Mysqlx::ClientMessages::Type::ClientMessages_Type_CON_CLOSE};
  XError out_error;
  Mysqlx::Session::Reset reset;

  m_context->m_global_error = XError{expected_error_code, expected_error_txt};

  ASSERT_EQ(nullptr, m_sut->recv_single_message(&out_smid, &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(expected_error_code,
            m_sut->send(out_cmid, *static_cast<XProtocol::Message *>(&reset))
                .error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Session::AuthenticateContinue()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Session::Reset()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Session::Close()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Sql::StmtExecute()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Crud::Find()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Crud::Insert()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Crud::Update()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Crud::CreateView()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Crud::ModifyView()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Crud::DropView()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Expect::Open()).error());

  ASSERT_EQ(expected_error_code, m_sut->send(Mysqlx::Expect::Close()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Connection::CapabilitiesGet()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Connection::CapabilitiesSet()).error());

  ASSERT_EQ(expected_error_code,
            m_sut->send(Mysqlx::Connection::Close()).error());

  ASSERT_EQ(nullptr, m_sut->recv_resultset(&out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(expected_error_code, m_sut->recv_ok().error());

  ASSERT_EQ(expected_error_code, m_sut->execute_close().error());

  ASSERT_EQ(nullptr,
            m_sut->execute_with_resultset(out_cmid, reset, &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr,
            m_sut->execute_stmt(Mysqlx::Sql::StmtExecute(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr, m_sut->execute_find(Mysqlx::Crud::Find(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr, m_sut->execute_update(Mysqlx::Crud::Update(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr, m_sut->execute_insert(Mysqlx::Crud::Insert(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr, m_sut->execute_delete(Mysqlx::Crud::Delete(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr,
            m_sut->execute_prep_stmt(Mysqlx::Prepare::Execute(), &out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(nullptr, m_sut->execute_fetch_capabilities(&out_error));
  ASSERT_EQ(expected_error_code, out_error.error());
  ASSERT_STREQ(expected_error_txt, out_error.what());

  ASSERT_EQ(expected_error_code,
            m_sut->execute_set_capability(Mysqlx::Connection::CapabilitiesSet())
                .error());

  ASSERT_EQ(expected_error_code,
            m_sut->execute_authenticate("user", "pass", "db", "PLAIN").error());

  ASSERT_EQ(
      expected_error_code,
      m_sut->execute_authenticate("user", "pass", "db", "MYSQL41").error());
}

}  // namespace test
}  // namespace xcl
