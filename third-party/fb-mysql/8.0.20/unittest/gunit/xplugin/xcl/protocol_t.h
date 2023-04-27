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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_PROTOCOL_T_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_PROTOCOL_T_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "errmsg.h"       // NOLINT(build/include_subdir)
#include "my_compiler.h"  // NOLINT(build/include_subdir)
#include "my_inttypes.h"
#include "plugin/x/client/context/xconnection_config.h"
#include "plugin/x/client/context/xssl_config.h"
#include "plugin/x/client/xprotocol_impl.h"
#include "unittest/gunit/xplugin/xcl/message_helpers.h"
#include "unittest/gunit/xplugin/xcl/mock/connection.h"
#include "unittest/gunit/xplugin/xcl/mock/connection_state.h"
#include "unittest/gunit/xplugin/xcl/mock/factory.h"
#include "unittest/gunit/xplugin/xcl/mock/message_handler.h"
#include "unittest/gunit/xplugin/xcl/mock/query_result.h"

namespace xcl {
namespace test {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrictMock;
using ::testing::Test;

class Xcl_protocol_impl_tests : public Test {
 protected:
  std::array<uint8_t, 4> extract_header_from_message(const uchar *data) {
    std::array<uint8_t, 4> header = {{data[0], data[1], data[2], data[3]}};

#ifdef WORDS_BIGENDIAN
    std::swap(header[0], header[3]);
    std::swap(header[1], header[2]);
#endif
    return header;
  }

 public:
  void SetUp() override {
    m_mock_connection = new StrictMock<Mock_connection>();

    EXPECT_CALL(m_mock_factory, create_connection_raw(_))
        .WillOnce(Return(m_mock_connection));

    m_sut.reset(new Protocol_impl(m_context, &m_mock_factory));
  }

  XQuery_result *expect_factory_new_result() {
    Mock_query_result *result = new Mock_query_result();
    static XQuery_result::Metadata metadata;

    EXPECT_CALL(m_mock_factory, create_result_raw(_, _, _))
        .WillOnce(Return(result));
    EXPECT_CALL(*result, get_metadata(_)).WillOnce(ReturnRef(metadata));

    return result;
  }

  void expect_write_header(const XProtocol::Client_message_type_id id,
                           const uint32_t expected_payload_size,
                           const int32_t error_code = 0) {
    EXPECT_CALL(*m_mock_connection, write(_, 5))
        .WillOnce(Invoke(
            [this, id, expected_payload_size, error_code](
                const uchar *data,
                const std::size_t size MY_ATTRIBUTE((unused))) -> XError {
              EXPECT_EQ(data[4], id);
              auto header = this->extract_header_from_message(data);

              EXPECT_EQ(expected_payload_size,
                        *reinterpret_cast<int32_t *>(header.data()) - 1);
              return XError{error_code, ""};
            }));
  }

  template <typename Message_type>
  void expect_write_message_without_payload(
      const Message_from_str<Message_type> &message,
      const int32_t error_code = 0) {
    const Message_type &message_base = message;

    expect_write_message_without_payload(message_base, error_code);
  }

  template <typename Message_type>
  void expect_write_message_without_payload(
      const Message_type &message, const int32_t expected_error_code = 0) {
    auto message_binary = Message_encoder::encode(message);
    expect_write_header(Client_message<Message_type>::get_id(),
                        static_cast<uint32>(message_binary.size()),
                        expected_error_code);
  }

  template <typename Message_type>
  void expect_write_message(const Message_from_str<Message_type> &message,
                            const int32_t error_code = 0) {
    const Message_type &message_base = message;

    expect_write_message(message_base, error_code);
  }

  template <typename Message_type>
  void expect_write_message(const Message_type &message,
                            const int32_t error_code = 0) {
    auto message_binary = Message_encoder::encode(message);

    const uint8_t payload_size = message_binary.size();
    const auto id = Client_message<Message_type>::get_id();

    expect_write_payload(id, payload_size, error_code);
  }

  void expect_write_payload(const uint8_t id, const uint32_t payload_size,
                            const int32 error_code = 0) {
    const std::uint8_t header_size = 5;

    EXPECT_CALL(*m_mock_connection, write(_, header_size + payload_size))
        .WillOnce(Invoke(
            [this, id, payload_size, error_code](
                const uchar *data,
                const std::size_t size MY_ATTRIBUTE((unused))) -> XError {
              EXPECT_EQ(data[4], id);
              auto header = this->extract_header_from_message(data);

              EXPECT_EQ(payload_size,
                        *reinterpret_cast<int32_t *>(header.data()) - 1);
              return XError{error_code, ""};
            }));
  }

  template <typename Message_type_id>
  void expect_read_header(const Message_type_id id, const int32_t payload_size,
                          const int32_t error_code = 0) {
    const int32_t expected_header_size = 5;

    EXPECT_CALL(*m_mock_connection, read(_, expected_header_size))
        .WillOnce(
            Invoke([id, payload_size, error_code](
                       uchar *data, const std::size_t data_length MY_ATTRIBUTE(
                                        (unused))) -> XError {
              // 1byte(type)+ payload_size-bytes(protobuf-msg-payload)
              *reinterpret_cast<int32_t *>(data) = 1 + payload_size;

#ifdef WORDS_BIGENDIAN
              std::swap(data[0], data[3]);
              std::swap(data[1], data[2]);
#endif

              data[4] = static_cast<uchar>(id);

              return XError{error_code, ""};
            }));
  }

  template <typename Message_type>
  void expect_read_message_without_payload(const Message_type &message
                                               MY_ATTRIBUTE((unused)),
                                           const int32_t error_code = 0) {
    expect_read_header(Server_message<Message_type>::get_id(), 0, error_code);
  }

  template <typename Message_type>
  void expect_read_message(const Message_from_str<Message_type> &message) {
    const Message_type &message_base = message;

    expect_read_message(message_base);
  }

  template <typename Message_type>
  void expect_read_message(const Message_type &message) {
    const std::string message_payload = Message_encoder::encode(message);

    expect_read_header(Server_message<Message_type>::get_id(),
                       static_cast<int32>(message_payload.size()));

    EXPECT_CALL(*m_mock_connection, read(_, message_payload.size()))
        .WillOnce(
            Invoke([message_payload](uchar *data,
                                     const std::size_t data_length MY_ATTRIBUTE(
                                         (unused))) -> XError {
              std::copy(message_payload.begin(), message_payload.end(), data);
              return {};
            }));
  }

  template <typename Message_type>
  void assert_read_message(const Message_type &message) {
    XProtocol::Server_message_type_id out_id;
    XError out_error;

    expect_read_message(message);

    auto result = m_sut->recv_single_message(&out_id, &out_error);
    ASSERT_FALSE(out_error);
    ASSERT_TRUE(result.get());
    ASSERT_EQ(message.GetTypeName(), result->GetTypeName());
    ASSERT_EQ(Message_compare<Message_type>(message), *result);
  }

  StrictMock<Mock_connection> *m_mock_connection;
  StrictMock<Mock_factory> m_mock_factory;
  StrictMock<Mock_handlers> m_mock_handlers;

  std::shared_ptr<Context> m_context{std::make_shared<Context>()};

  std::shared_ptr<Protocol_impl> m_sut;
};

template <typename T>
class Xcl_protocol_impl_tests_with_msg : public Xcl_protocol_impl_tests {
 public:
  using Msg = T;
  using Msg_ptr = std::unique_ptr<Msg>;
  using Msg_descriptor = Client_message<T>;
  using Type = Xcl_protocol_impl_tests_with_msg<T>;

 public:
  Msg_ptr m_message;

  void setup_required_fields_in_message() {
    m_message.reset(new Msg(Msg_descriptor::make_required()));
  }

  XError assert_write_size(const uchar *data, const uint32_t size) {
    EXPECT_EQ(Msg_descriptor::get_id(), data[4]);
    auto header = extract_header_from_message(data);

    const uint8_t expected_header_size = 5;
    const uint32_t payload_size =
        *reinterpret_cast<uint32_t *>(header.data()) - 1;
    EXPECT_EQ(size, (payload_size + expected_header_size));

    return {};
  }

  void assert_multiple_handlers(const Handler_result first_handler_consumed) {
    StrictMock<Mock_handlers> mock_handlers[2];

    this->setup_required_fields_in_message();

    const auto id1 = this->m_sut->add_send_message_handler(
        this->m_mock_handlers.get_mock_lambda_send_message_handler());
    const auto id2 = this->m_sut->add_send_message_handler(
        mock_handlers[0].get_mock_lambda_send_message_handler());
    const auto id3 = this->m_sut->add_send_message_handler(
        mock_handlers[1].get_mock_lambda_send_message_handler());

    {
      InSequence s;
      // Sequence of pushed handlers is important
      // Lets verify if the execution is done in sequence
      // from last pushed handler to first hander
      EXPECT_CALL(mock_handlers[1],
                  send_message_handler(_, Msg_descriptor::get_id(),
                                       Ref(*this->m_message.get())))
          .WillOnce(Return(first_handler_consumed));
      EXPECT_CALL(mock_handlers[0],
                  send_message_handler(_, Msg_descriptor::get_id(),
                                       Ref(*this->m_message.get())))
          .WillOnce(Return(Handler_result::Continue));
      EXPECT_CALL(this->m_mock_handlers,
                  send_message_handler(_, Msg_descriptor::get_id(),
                                       Ref(*this->m_message.get())))
          .WillOnce(Return(Handler_result::Continue));

      EXPECT_CALL(*this->m_mock_connection, write(_, _))
          .WillOnce(Invoke(
              [this](const uchar *data, const std::size_t size) -> XError {
                return this->assert_write_size(data, size);
              }));
    }

    auto error = this->m_sut->send(*this->m_message.get());

    ASSERT_FALSE(error);

    this->m_sut->remove_send_message_handler(id1);
    this->m_sut->remove_send_message_handler(id2);
    this->m_sut->remove_send_message_handler(id3);
  }
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_PROTOCOL_T_H_
