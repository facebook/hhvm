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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_PROTOCOL_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_PROTOCOL_H_

#include <gmock/gmock.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "plugin/x/client/mysqlxclient/xprotocol.h"

namespace xcl {
namespace test {

class Mock_protocol : public XProtocol {
 public:
  MOCK_METHOD3(add_notice_handler,
               Handler_id(Notice_handler handler, const Handler_position,
                          const Handler_priority));
  MOCK_METHOD3(add_received_message_handler,
               Handler_id(Server_message_handler handler,
                          const Handler_position, const Handler_priority));
  MOCK_METHOD3(add_send_message_handler,
               Handler_id(Client_message_handler handler,
                          const Handler_position, const Handler_priority));
  MOCK_METHOD1(remove_notice_handler, void(Handler_id handler));
  MOCK_METHOD1(remove_received_message_handler, void(Handler_id handler));
  MOCK_METHOD1(remove_send_message_handler, void(Handler_id handler));
  MOCK_METHOD0(get_connection, XConnection &());
  MOCK_METHOD3(recv, XError(Header_message_type_id *out_mid, uint8_t **buffer,
                            std::size_t *buffer_size));
  MOCK_METHOD4(deserialize_received_message_raw,
               Message *(const Header_message_type_id mid,
                         const uint8_t *payload, const std::size_t payload_size,
                         XError *out_error));
  MOCK_METHOD2(recv_single_message_raw,
               Message *(Server_message_type_id *out_mid, XError *out_error));

  MOCK_METHOD2(send_compressed_frame,
               XError(const Client_message_type_id mid, const Message &msg));
  MOCK_METHOD1(send_compressed_multiple_frames,
               XError(const std::vector<std::pair<Client_message_type_id,
                                                  const Message *>> &messages));
  MOCK_METHOD2(send,
               XError(const Client_message_type_id mid, const Message &msg));
  MOCK_METHOD3(send, XError(const Header_message_type_id mid,
                            const uint8_t *buffer, const std::size_t length));
  MOCK_METHOD1(send, XError(const Mysqlx::Session::AuthenticateStart &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Session::AuthenticateContinue &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Session::Reset &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Session::Close &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Sql::StmtExecute &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::Find &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::Insert &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::Update &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::Delete &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::CreateView &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::ModifyView &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Crud::DropView &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Expect::Open &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Expect::Close &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Connection::CapabilitiesGet &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Connection::CapabilitiesSet &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Connection::Close &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Cursor::Open &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Cursor::Close &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Cursor::Fetch &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Prepare::Prepare &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Prepare::Execute &m));
  MOCK_METHOD1(send, XError(const Mysqlx::Prepare::Deallocate &m));
  MOCK_METHOD0(recv_ok, XError());
  MOCK_METHOD0(execute_close, XError());
  MOCK_METHOD1(execute_fetch_capabilities_raw,
               Capabilities *(XError *out_error));
  MOCK_METHOD1(execute_set_capability,
               XError(const ::Mysqlx::Connection::CapabilitiesSet &m));
  MOCK_METHOD4(execute_authenticate,
               XError(const std::string &, const std::string &,
                      const std::string &, const std::string &));
  MOCK_METHOD0(recv_resultset_raw, XQuery_result *());
  MOCK_METHOD1(recv_resultset_raw, XQuery_result *(XError *out_error));
  MOCK_METHOD3(execute_with_resultset_raw,
               XQuery_result *(const Client_message_type_id mid,
                               const Message &msg, XError *out_error));
  MOCK_METHOD2(execute_stmt_raw,
               XQuery_result *(const Mysqlx::Sql::StmtExecute &m,
                               XError *out_error));
  MOCK_METHOD2(execute_find_raw,
               XQuery_result *(const Mysqlx::Crud::Find &m, XError *out_error));
  MOCK_METHOD2(execute_update_raw,
               XQuery_result *(const Mysqlx::Crud::Update &m,
                               XError *out_error));
  MOCK_METHOD2(execute_insert_raw,
               XQuery_result *(const Mysqlx::Crud::Insert &m,
                               XError *out_error));
  MOCK_METHOD2(execute_delete_raw,
               XQuery_result *(const Mysqlx::Crud::Delete &m,
                               XError *out_error));
  MOCK_METHOD2(execute_prep_stmt_raw,
               XQuery_result *(const Mysqlx::Prepare::Execute &m,
                               XError *out_error));
  MOCK_METHOD2(execute_cursor_open_raw,
               XQuery_result *(const Mysqlx::Cursor::Open &m,
                               XError *out_error));
  MOCK_METHOD3(execute_cursor_fetch_raw,
               XQuery_result *(const Mysqlx::Cursor::Fetch &m,
                               std::unique_ptr<XQuery_result> &cursor_open_res,
                               XError *out_error));

  MOCK_METHOD1(use_compression, void(const Compression_algorithm algo));
  MOCK_METHOD2(use_compression,
               void(const Compression_algorithm algo, const int32_t level));

 private:
  using XQuery_result_ptr = std::unique_ptr<XQuery_result>;
  using Message_ptr = std::unique_ptr<Message>;
  using Capabilities_ptr = std::unique_ptr<Capabilities>;

  Message_ptr deserialize_received_message(const Header_message_type_id mid,
                                           const uint8_t *payload,
                                           const std::size_t payload_size,
                                           XError *out_error) override {
    return Message_ptr(deserialize_received_message_raw(
        mid, payload, payload_size, out_error));
  }

  Capabilities_ptr execute_fetch_capabilities(XError *out_error) override {
    return Capabilities_ptr(execute_fetch_capabilities_raw(out_error));
  }

  Message_ptr recv_single_message(Server_message_type_id *out_mid,
                                  XError *out_error) override {
    return Message_ptr(recv_single_message_raw(out_mid, out_error));
  }

  XQuery_result_ptr recv_resultset() override {
    return XQuery_result_ptr(recv_resultset_raw());
  }

  XQuery_result_ptr recv_resultset(XError *out_error) override {
    return XQuery_result_ptr(recv_resultset_raw(out_error));
  }

  XQuery_result_ptr execute_with_resultset(const Client_message_type_id mid,
                                           const Message &msg,
                                           XError *out_error) override {
    return XQuery_result_ptr(execute_with_resultset_raw(mid, msg, out_error));
  }

  XQuery_result_ptr execute_stmt(const Mysqlx::Sql::StmtExecute &m,
                                 XError *out_error) override {
    return XQuery_result_ptr(execute_stmt_raw(m, out_error));
  }

  XQuery_result_ptr execute_find(const Mysqlx::Crud::Find &m,
                                 XError *out_error) override {
    return XQuery_result_ptr(execute_find_raw(m, out_error));
  }

  XQuery_result_ptr execute_cursor_open(const Mysqlx::Cursor::Open &m,
                                        XError *out_error) override {
    return XQuery_result_ptr(execute_cursor_open_raw(m, out_error));
  }

  XQuery_result_ptr execute_cursor_fetch(const Mysqlx::Cursor::Fetch &m,
                                         XQuery_result_ptr cursor_open_result,
                                         XError *out_error) override {
    return XQuery_result_ptr(
        execute_cursor_fetch_raw(m, cursor_open_result, out_error));
  }

  XQuery_result_ptr execute_update(const Mysqlx::Crud::Update &m,
                                   XError *out_error) override {
    return XQuery_result_ptr(execute_update_raw(m, out_error));
  }

  XQuery_result_ptr execute_insert(const Mysqlx::Crud::Insert &m,
                                   XError *out_error) override {
    return XQuery_result_ptr(execute_insert_raw(m, out_error));
  }

  XQuery_result_ptr execute_delete(const Mysqlx::Crud::Delete &m,
                                   XError *out_error) override {
    return XQuery_result_ptr(execute_delete_raw(m, out_error));
  }

  XQuery_result_ptr execute_prep_stmt(const Mysqlx::Prepare::Execute &m,
                                      XError *out_error) override {
    return XQuery_result_ptr(execute_prep_stmt_raw(m, out_error));
  }
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_MOCK_PROTOCOL_H_
