/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

TEST_F(Xcl_protocol_impl_tests, execute_close) {
  using Send_desc = Client_message<::Mysqlx::Connection::Close>;
  using Recv_desc = Server_message<::Mysqlx::Ok>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  expect_write_message_without_payload(msg_send);
  expect_read_message_without_payload(msg_recv);

  auto error = m_sut->execute_close();

  ASSERT_FALSE(error);
}

TEST_F(Xcl_protocol_impl_tests, execute_close_fail_at_write) {
  const uint32_t expected_header_size = 5;
  const int expected_error_code = 1123;

  EXPECT_CALL(*m_mock_connection, write(_, expected_header_size))
      .WillOnce(Return(XError{expected_error_code, ""}));
  auto error = m_sut->execute_close();

  ASSERT_EQ(expected_error_code, error.error());
}

TEST_F(Xcl_protocol_impl_tests, execute_close_fail_at_read) {
  const uint32_t expected_header_size = 5;
  const int expected_error_code = 1124;

  EXPECT_CALL(*m_mock_connection, write(_, expected_header_size))
      .WillOnce(Return(XError()));
  EXPECT_CALL(*m_mock_connection, read(_, _))
      .WillOnce(Return(XError{expected_error_code, ""}));
  auto error = m_sut->execute_close();

  ASSERT_EQ(expected_error_code, error.error());
}

TEST_F(Xcl_protocol_impl_tests, execute_close_failed_because_of_error_msg) {
  using Send_desc = Client_message<::Mysqlx::Connection::Close>;
  using Recv_desc = Server_message<::Mysqlx::Error>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  const uint32_t expected_error_code = 1124;
  msg_recv.set_code(expected_error_code);

  expect_write_message_without_payload(msg_send);
  expect_read_message(msg_recv);

  auto error = m_sut->execute_close();

  ASSERT_EQ(expected_error_code, error.error());
}

TEST_F(Xcl_protocol_impl_tests, execute_close_failed_because_recv_wrong_msg) {
  using Send_desc = Client_message<::Mysqlx::Connection::Close>;
  using Recv_desc = Server_message<::Mysqlx::Session::AuthenticateContinue>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  expect_write_message_without_payload(msg_send);
  expect_read_message(msg_recv);

  auto error = m_sut->execute_close();

  ASSERT_EQ(CR_MALFORMED_PACKET, error.error());
}

TEST_F(Xcl_protocol_impl_tests, execute_with_resultset_msg_without_payload) {
  using Send_desc = Client_message<::Mysqlx::Session::Close>;
  auto msg_send = Send_desc::make_required();
  XError out_error;

  XQuery_result *expected_result = expect_factory_new_result();
  expect_write_message_without_payload(msg_send);

  auto result =
      m_sut->execute_with_resultset(Send_desc::get_id(), msg_send, &out_error);

  ASSERT_FALSE(out_error);
  ASSERT_EQ(expected_result, result.get());
}

TEST_F(Xcl_protocol_impl_tests, execute_with_resultset_msg_with_payload) {
  using Send_desc = Client_message<::Mysqlx::Crud::CreateView>;
  auto msg_send = Send_desc::make_required();
  XError out_error;

  XQuery_result *expected_result = expect_factory_new_result();
  expect_write_message(msg_send);

  auto result =
      m_sut->execute_with_resultset(Send_desc::get_id(), msg_send, &out_error);

  ASSERT_FALSE(out_error);
  ASSERT_EQ(expected_result, result.get());
}

TEST_F(Xcl_protocol_impl_tests, execute_with_resultset_fails_at_header_write) {
  using Close_desc = Client_message<::Mysqlx::Session::Close>;

  auto msg_close = Close_desc::make_required();
  const int expected_error_code = 23324;
  XError out_error;

  expect_write_message_without_payload(msg_close, expected_error_code);

  auto result = m_sut->execute_with_resultset(Close_desc::get_id(), msg_close,
                                              &out_error);

  ASSERT_EQ(expected_error_code, out_error.error());
}

template <typename Msg>
class xcl_protocol_impl_tests_execute_msg
    : public Xcl_protocol_impl_tests_with_msg<Msg> {
 public:
  void SetUp() override {
    Xcl_protocol_impl_tests_with_msg<Msg>::SetUp();
    this->setup_required_fields_in_message();
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Sql::StmtExecute &m,
                                            XError *out_error) {
    return this->m_sut->execute_stmt(m, out_error);
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Crud::Find &m,
                                            XError *out_error) {
    return this->m_sut->execute_find(m, out_error);
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Crud::Update &m,
                                            XError *out_error) {
    return this->m_sut->execute_update(m, out_error);
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Crud::Insert &m,
                                            XError *out_error) {
    return this->m_sut->execute_insert(m, out_error);
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Crud::Delete &m,
                                            XError *out_error) {
    return this->m_sut->execute_delete(m, out_error);
  }

  std::unique_ptr<XQuery_result> do_execute(const Mysqlx::Cursor::Open &m,
                                            XError *out_error) {
    return this->m_sut->execute_cursor_open(m, out_error);
  }
};

TEST_F(Xcl_protocol_impl_tests, cursor_fetch_msg_with_payload) {
  using Send_desc = Client_message<::Mysqlx::Cursor::Fetch>;
  auto msg_send = Send_desc::make_required();
  XError out_error;

  Mock_query_result *expected_result = new Mock_query_result();
  expect_write_message(msg_send);

  XQuery_result::Metadata metadata;
  std::unique_ptr<Mock_query_result> open_result(new Mock_query_result());
  EXPECT_CALL(*open_result.get(), get_metadata(_))
      .WillOnce(ReturnRef(metadata));
  EXPECT_CALL(m_mock_factory, create_result_raw(_, _, _))
      .WillOnce(Return(expected_result));
  EXPECT_CALL(*expected_result, set_metadata(_));
  auto result =
      m_sut->execute_cursor_fetch(msg_send, std::move(open_result), &out_error);

  ASSERT_FALSE(out_error);
  ASSERT_EQ(expected_result, result.get());
}

using Msg_vs_method_types =
    ::testing::Types<::Mysqlx::Sql::StmtExecute, ::Mysqlx::Crud::Find,
                     ::Mysqlx::Crud::Insert, ::Mysqlx::Crud::Update,
                     ::Mysqlx::Crud::Delete, ::Mysqlx::Cursor::Open>;

TYPED_TEST_CASE(xcl_protocol_impl_tests_execute_msg, Msg_vs_method_types);

TYPED_TEST(xcl_protocol_impl_tests_execute_msg, msg_with_payload) {
  XError out_error;

  XQuery_result *expected_result = this->expect_factory_new_result();
  this->expect_write_message(*this->m_message);

  auto result = this->do_execute(*this->m_message, &out_error);

  ASSERT_FALSE(out_error);
  ASSERT_EQ(expected_result, result.get());
}

TYPED_TEST(xcl_protocol_impl_tests_execute_msg, fails_at_message_write) {
  const uint32_t expected_error_code = 23324;
  XError out_error;

  this->expect_write_message(*this->m_message, expected_error_code);

  auto result = this->do_execute(*this->m_message, &out_error);

  ASSERT_EQ(expected_error_code, out_error.error());
}

TEST_F(Xcl_protocol_impl_tests, execute_fetch_capabilities) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesGet>;
  using Recv_desc = Server_message<::Mysqlx::Connection::Capabilities>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();
  XError out_error;

  auto cap_value = msg_recv.mutable_capabilities()->Add();
  cap_value->set_name("CapName");
  auto any = cap_value->mutable_value();
  any->set_type(::Mysqlx::Datatypes::Any_Type_SCALAR);
  auto scalar = any->mutable_scalar();
  scalar->set_type(::Mysqlx::Datatypes::Scalar_Type_V_BOOL);
  scalar->set_v_bool(true);

  expect_write_message_without_payload(msg_send);
  expect_read_message(msg_recv);

  auto result = m_sut->execute_fetch_capabilities(&out_error);

  ASSERT_FALSE(out_error);
  ASSERT_EQ(Message_compare<::Mysqlx::Connection::Capabilities>(msg_recv),
            *result);
}

TEST_F(Xcl_protocol_impl_tests,
       execute_fetch_capabilities_fails_at_header_write) {
  const uint32_t expected_error = 2300;
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesGet>;
  auto msg_send = Send_desc::make_required();
  XError out_error;

  expect_write_message_without_payload(msg_send, expected_error);

  auto result = m_sut->execute_fetch_capabilities(&out_error);

  ASSERT_EQ(expected_error, out_error.error());
  ASSERT_FALSE(result.get());
}

TEST_F(Xcl_protocol_impl_tests, execute_fetch_capabilities_fails_at_read) {
  const uint32_t expected_error = 2301;
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesGet>;
  using Recv_desc = Server_message<::Mysqlx::Connection::Capabilities>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();
  XError out_error;

  expect_write_message_without_payload(msg_send);
  expect_read_message_without_payload(msg_recv, expected_error);

  auto result = m_sut->execute_fetch_capabilities(&out_error);

  ASSERT_EQ(expected_error, out_error.error());
  ASSERT_FALSE(result.get());
}

TEST_F(Xcl_protocol_impl_tests,
       execute_fetch_capabilities_fails_because_recv_error_msg) {
  const uint32_t expected_error = 2302;
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesGet>;
  using Recv_desc = Server_message<::Mysqlx::Error>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();
  XError out_error;

  msg_recv.set_code(expected_error);
  expect_write_message_without_payload(msg_send);
  expect_read_message(msg_recv);

  auto result = m_sut->execute_fetch_capabilities(&out_error);

  ASSERT_EQ(expected_error, out_error.error());
  ASSERT_FALSE(result.get());
}

TEST_F(Xcl_protocol_impl_tests,
       execute_fetch_capabilities_fails_because_wrong_msg) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesGet>;
  using Recv_desc = Server_message<::Mysqlx::Ok>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();
  XError out_error;

  expect_write_message_without_payload(msg_send);
  expect_read_message_without_payload(msg_recv);

  auto result = m_sut->execute_fetch_capabilities(&out_error);

  ASSERT_EQ(CR_MALFORMED_PACKET, out_error.error());
  ASSERT_FALSE(result.get());
}

TEST_F(Xcl_protocol_impl_tests, execute_set_capability) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesSet>;
  using Recv_desc = Server_message<::Mysqlx::Ok>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  expect_write_message(msg_send);
  expect_read_message_without_payload(msg_recv);

  auto error = m_sut->execute_set_capability(msg_send);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_protocol_impl_tests,
       execute_set_capability_failed_because_of_recv_error_msg) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesSet>;
  using Recv_desc = Server_message<::Mysqlx::Error>;

  const uint32_t expected_error_code = 1001;
  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  msg_recv.set_code(expected_error_code);
  expect_write_message(msg_send);
  expect_read_message(msg_recv);

  auto error = m_sut->execute_set_capability(msg_send);

  ASSERT_EQ(expected_error_code, error.error());
}

TEST_F(Xcl_protocol_impl_tests,
       execute_set_capability_failed_because_of_send_error) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesSet>;

  const uint32_t expected_error_code = 1002;
  auto msg_send = Send_desc::make_required();

  expect_write_message(msg_send, expected_error_code);

  auto error = m_sut->execute_set_capability(msg_send);

  ASSERT_EQ(expected_error_code, error.error());
}

TEST_F(Xcl_protocol_impl_tests,
       execute_set_capability_failed_because_of_wrong_msg_recv) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesSet>;
  using Recv_desc = Server_message<::Mysqlx::Session::AuthenticateContinue>;

  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  expect_write_message(msg_send);
  expect_read_message(msg_recv);

  auto error = m_sut->execute_set_capability(msg_send);

  ASSERT_EQ(CR_MALFORMED_PACKET, error.error());
}

TEST_F(Xcl_protocol_impl_tests,
       execute_set_capability_failed_because_of_io_error_recv) {
  using Send_desc = Client_message<::Mysqlx::Connection::CapabilitiesSet>;
  using Recv_desc = Server_message<::Mysqlx::Ok>;

  const uint32_t expected_error_code = 1003;
  auto msg_send = Send_desc::make_required();
  auto msg_recv = Recv_desc::make_required();

  expect_write_message(msg_send);
  expect_read_message_without_payload(msg_recv, expected_error_code);

  auto error = m_sut->execute_set_capability(msg_send);

  ASSERT_EQ(expected_error_code, error.error());
}

}  // namespace test
}  // namespace xcl
