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

#include "plugin/x/protocol/stream/compression/compression_algorithm_interface.h"

namespace xcl {
namespace test {

using Msg_types = ::testing::Types<
    ::Mysqlx::Session::AuthenticateStart,
    ::Mysqlx::Session::AuthenticateContinue, ::Mysqlx::Session::Reset,
    ::Mysqlx::Session::Close, ::Mysqlx::Sql::StmtExecute, ::Mysqlx::Crud::Find,
    ::Mysqlx::Crud::Insert, ::Mysqlx::Crud::Update, ::Mysqlx::Crud::Delete,
    ::Mysqlx::Crud::CreateView, ::Mysqlx::Crud::ModifyView,
    ::Mysqlx::Crud::DropView, ::Mysqlx::Expect::Open, ::Mysqlx::Expect::Close,
    ::Mysqlx::Connection::CapabilitiesGet,
    ::Mysqlx::Connection::CapabilitiesSet, ::Mysqlx::Connection::Close,
    ::Mysqlx::Prepare::Prepare, ::Mysqlx::Prepare::Execute,
    ::Mysqlx::Prepare::Deallocate>;

TYPED_TEST_CASE(Xcl_protocol_impl_tests_with_msg, Msg_types);

TYPED_TEST(Xcl_protocol_impl_tests_with_msg,
           connection_send_successful_send_handler_consumed_msg_was_ignored) {
  this->setup_required_fields_in_message();

  const auto id = this->m_sut->add_send_message_handler(
      this->m_mock_handlers.get_mock_lambda_send_message_handler());

  {
    InSequence s;
    EXPECT_CALL(this->m_mock_handlers,
                send_message_handler(_, TestFixture::Msg_descriptor::get_id(),
                                     Ref(*this->m_message.get())))
        .WillOnce(Return(Handler_result::Consumed));

    EXPECT_CALL(*this->m_mock_connection, write(_, _))
        .WillOnce(
            Invoke([this](const uchar *data, const std::size_t size) -> XError {
              return this->assert_write_size(data, size);
            }));
  }

  auto error = this->m_sut->send(*this->m_message.get());

  ASSERT_FALSE(error);

  this->m_sut->remove_send_message_handler(id);
}

TYPED_TEST(Xcl_protocol_impl_tests_with_msg,
           connection_send_successful_send_handler_ignored_msg) {
  this->setup_required_fields_in_message();

  const auto id = this->m_sut->add_send_message_handler(
      this->m_mock_handlers.get_mock_lambda_send_message_handler());

  {
    InSequence s;
    EXPECT_CALL(this->m_mock_handlers,
                send_message_handler(_, TestFixture::Msg_descriptor::get_id(),
                                     Ref(*this->m_message.get())))
        .WillOnce(Return(Handler_result::Continue));

    EXPECT_CALL(*this->m_mock_connection, write(_, _))
        .WillOnce(
            Invoke([this](const uchar *data, const std::size_t size) -> XError {
              return this->assert_write_size(data, size);
            }));
  }

  auto error = this->m_sut->send(*this->m_message.get());

  ASSERT_FALSE(error);

  this->m_sut->remove_send_message_handler(id);
}

TYPED_TEST(Xcl_protocol_impl_tests_with_msg,
           connection_send_successful_multiple_send_handlers_called) {
  const auto first_handler_didnt_consume = Handler_result::Continue;

  this->assert_multiple_handlers(first_handler_didnt_consume);
}

TYPED_TEST(Xcl_protocol_impl_tests_with_msg,
           connection_send_successful_first_handler_doesnt_blok_others) {
  const auto first_handler_consumed = Handler_result::Consumed;

  this->assert_multiple_handlers(first_handler_consumed);
}

TYPED_TEST(Xcl_protocol_impl_tests_with_msg, connection_send_successful) {
  this->setup_required_fields_in_message();

  {
    InSequence s;

    EXPECT_CALL(*this->m_mock_connection, write(_, _))
        .WillOnce(
            Invoke([this](const uchar *data, const std::size_t size) -> XError {
              return this->assert_write_size(data, size);
            }));
  }

  auto error = this->m_sut->send(*this->m_message.get());

  ASSERT_FALSE(error);
}

TEST_F(Xcl_protocol_impl_tests, recv_fails_at_header) {
  const int64_t expected_error_code = 3000;
  const int32_t expected_payload_size = 10;
  XProtocol::Server_message_type_id out_id;
  XError out_error;

  expect_read_header(::Mysqlx::ServerMessages::OK, expected_payload_size,
                     expected_error_code);
  auto result = m_sut->recv_single_message(&out_id, &out_error);
  ASSERT_FALSE(result.get());
  ASSERT_EQ(expected_error_code, out_error.error());
}

TEST_F(Xcl_protocol_impl_tests, recv_fails_at_payload) {
  const int expected_error_code = 3000;
  const int32_t expected_payload_size = 10;
  XProtocol::Server_message_type_id out_id;
  XError out_error;

  expect_read_header(::Mysqlx::ServerMessages::OK, expected_payload_size);

  EXPECT_CALL(*m_mock_connection, read(_, expected_payload_size))
      .WillOnce(Return(XError(expected_error_code, "")));
  auto result = m_sut->recv_single_message(&out_id, &out_error);
  ASSERT_FALSE(result.get());
  ASSERT_EQ(expected_error_code, out_error.error());
}

TEST_F(Xcl_protocol_impl_tests, recv_large_message) {
  std::string message_payload(1024 * 64, 'x');
  int offset = 0;
  expect_read_header(::Mysqlx::ServerMessages::OK,
                     static_cast<int32>(message_payload.size()));

  {
    InSequence s;
    const int k_internal_buffer = 4 * 1024;
    size_t count_data_in_reads = 0;

    while (count_data_in_reads < message_payload.size()) {
      EXPECT_CALL(*m_mock_connection, read(_, k_internal_buffer))
          .WillOnce(Invoke(
              [&offset, message_payload](
                  uchar *data, const std::size_t data_length MY_ATTRIBUTE(
                                   (unused))) -> XError {
                auto i_start = message_payload.begin() + offset;
                std::copy(i_start, i_start + data_length, data);
                offset += data_length;
                return {};
              }));
      count_data_in_reads += k_internal_buffer;
    }
  }
  XProtocol::Server_message_type_id out_id;
  XError out_error;
  auto result = m_sut->recv_single_message(&out_id, &out_error);
  ASSERT_TRUE(result.get());
  ASSERT_EQ(0, out_error.error());
}

TEST_F(Xcl_protocol_impl_tests, recv_unknown_msg_type) {
  const int32_t invalid_message_id = 255;
  const int32_t expected_payload_size = 10;
  XProtocol::Server_message_type_id out_id;
  XError out_error;

  expect_read_header(invalid_message_id, expected_payload_size);

  EXPECT_CALL(*m_mock_connection, read(_, expected_payload_size))
      .WillOnce(Return(XError()));
  auto result = m_sut->recv_single_message(&out_id, &out_error);
  ASSERT_FALSE(result.get());
  ASSERT_EQ(CR_MALFORMED_PACKET, out_error.error());
}

TEST_F(Xcl_protocol_impl_tests, recv_in_single_read_op) {
  const int32_t expected_payload_size = 0;
  const std::string expected_message_name = "Mysqlx.Ok";
  XProtocol::Server_message_type_id out_id;
  XError out_error;

  expect_read_header(::Mysqlx::ServerMessages::OK, expected_payload_size);

  auto result = m_sut->recv_single_message(&out_id, &out_error);
  ASSERT_TRUE(result.get());
  ASSERT_EQ(expected_message_name, result->GetTypeName());
  ASSERT_FALSE(out_error);
}

TEST_F(Xcl_protocol_impl_tests, recv_in_two_read_op) {
  using Auth_continue = ::Mysqlx::Session::AuthenticateContinue;
  const auto message = Server_message<Auth_continue>::make_required();

  assert_read_message(message);
}

TEST_F(Xcl_protocol_impl_tests, recv_in_two_read_op_call_one_handler) {
  using Auth_continue_desc =
      Server_message<::Mysqlx::Session::AuthenticateContinue>;

  const auto message = Auth_continue_desc::make_required();

  m_sut->add_received_message_handler(
      m_mock_handlers.get_mock_lambda_received_message_handler());
  EXPECT_CALL(m_mock_handlers,
              received_message_handler(_, Auth_continue_desc::get_id(),
                                       Cmp_msg(message)))
      .WillOnce(Return(Handler_result::Continue));

  assert_read_message(message);
}

TEST_F(Xcl_protocol_impl_tests, recv_in_two_read_op_call_multiple_handlers) {
  using Auth_continue_desc =
      Server_message<::Mysqlx::Session::AuthenticateContinue>;

  const auto message = Auth_continue_desc::make_required();
  StrictMock<Mock_handlers> mock_handlers[2];

  m_sut->add_received_message_handler(
      m_mock_handlers.get_mock_lambda_received_message_handler());
  m_sut->add_received_message_handler(
      mock_handlers[0].get_mock_lambda_received_message_handler());
  m_sut->add_received_message_handler(
      mock_handlers[1].get_mock_lambda_received_message_handler());

  // The call sequence must be opposite to push sequence
  {
    InSequence s;
    EXPECT_CALL(mock_handlers[1],
                received_message_handler(_, Auth_continue_desc::get_id(),
                                         Cmp_msg(message)))
        .WillOnce(Return(Handler_result::Continue));
    EXPECT_CALL(mock_handlers[0],
                received_message_handler(_, Auth_continue_desc::get_id(),
                                         Cmp_msg(message)))
        .WillOnce(Return(Handler_result::Continue));
    EXPECT_CALL(m_mock_handlers,
                received_message_handler(_, Auth_continue_desc::get_id(),
                                         Cmp_msg(message)))
        .WillOnce(Return(Handler_result::Continue));
  }

  assert_read_message(message);
}

TEST_F(
    Xcl_protocol_impl_tests,
    recv_in_two_read_op_call_first_handler_consumes_next_msg_on_all_handlers) {
  using Auth_continue_desc =
      Server_message<::Mysqlx::Session::AuthenticateContinue>;
  using Error_desc = Server_message<::Mysqlx::Error>;

  const auto msg_continue = Auth_continue_desc::make_required();
  const auto msg_error = Error_desc::make_required();
  StrictMock<Mock_handlers> mock_handlers[2];

  m_sut->add_received_message_handler(
      m_mock_handlers.get_mock_lambda_received_message_handler());
  m_sut->add_received_message_handler(
      mock_handlers[0].get_mock_lambda_received_message_handler());
  m_sut->add_received_message_handler(
      mock_handlers[1].get_mock_lambda_received_message_handler());

  // The call sequence must be opposite to push sequence
  {
    InSequence s1;
    // Get first transmitted message
    // and consume it inside the handler
    EXPECT_CALL(mock_handlers[1],
                received_message_handler(_, Auth_continue_desc::get_id(),
                                         Cmp_msg(msg_continue)))
        .WillOnce(Return(Handler_result::Consumed));

    // Message was consumed, thus read_single_message
    // is going to read next one from XConnection.
    // We are expecting that next message is going to
    // processed on all handlers without consuming it !
    EXPECT_CALL(
        mock_handlers[1],
        received_message_handler(_, Error_desc::get_id(), Cmp_msg(msg_error)))
        .WillOnce(Return(Handler_result::Continue));
    EXPECT_CALL(
        mock_handlers[0],
        received_message_handler(_, Error_desc::get_id(), Cmp_msg(msg_error)))
        .WillOnce(Return(Handler_result::Continue));
    EXPECT_CALL(
        m_mock_handlers,
        received_message_handler(_, Error_desc::get_id(), Cmp_msg(msg_error)))
        .WillOnce(Return(Handler_result::Continue));
  }

  {
    InSequence s2;
    expect_read_message(msg_continue);  // Message to be read at first read op
    assert_read_message(msg_error);     // Message to be read at second read op
  }
}

TEST_F(Xcl_protocol_impl_tests, recv_ok_fails_on_other_msg) {
  using Auth_continue_desc =
      Server_message<::Mysqlx::Session::AuthenticateContinue>;

  const auto msg_continue = Auth_continue_desc::make_required();

  expect_read_message(msg_continue);
  auto error = m_sut->recv_ok();

  ASSERT_EQ(CR_MALFORMED_PACKET, error.error());
}

TEST_F(Xcl_protocol_impl_tests, recv_ok_fails_error_msg) {
  using Error_desc = Server_message<::Mysqlx::Error>;

  const uint32_t expected_error_code = 23332;
  const char *expected_msg = "expected error message";
  const char *expected_sql_state = "expected sql state";

  auto msg_error = Error_desc::make_required();

  msg_error.set_code(expected_error_code);
  msg_error.set_msg(expected_msg);
  msg_error.set_sql_state(expected_sql_state);
  msg_error.set_severity(::Mysqlx::Error::FATAL);

  expect_read_message(msg_error);
  XError error = m_sut->recv_ok();

  ASSERT_EQ(expected_error_code, error.error());
  ASSERT_STREQ(expected_msg, error.what());
  ASSERT_STREQ(expected_sql_state, error.sql_state());
  ASSERT_TRUE(error.is_fatal());
}

TEST_F(Xcl_protocol_impl_tests, recv_ok) {
  using Ok_desc = Server_message<::Mysqlx::Ok>;

  auto msg_ok = Ok_desc::make_required();

  expect_read_message_without_payload(msg_ok);
  auto error = m_sut->recv_ok();

  ASSERT_FALSE(error);
}

TEST_F(Xcl_protocol_impl_tests, recv_resultset) {
  Mock_query_result *query_result = new Mock_query_result();
  static XQuery_result::Metadata metadata;

  EXPECT_CALL(m_mock_factory, create_result_raw(_, _, _))
      .WillOnce(Return(query_result));

  auto result = m_sut->recv_resultset();

  ASSERT_EQ(query_result, result.get());
}

TEST_F(Xcl_protocol_impl_tests, send_compressed) {
  m_sut->use_compression(Compression_algorithm::k_deflate, 1);
  expect_write_payload(Mysqlx::ClientMessages::COMPRESSION, 17, 0);

  auto result = m_sut->send_compressed_frame(
      Mysqlx::ClientMessages::EXPECT_OPEN, Mysqlx::Expect::Open());

  ASSERT_EQ(0, result.error());
}

}  // namespace test
}  // namespace xcl
