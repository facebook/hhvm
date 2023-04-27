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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_MESSAGE_HELPERS_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_MESSAGE_HELPERS_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <type_traits>

#include "plugin/x/client/mysqlxclient/xmessage.h"
#include "plugin/x/client/mysqlxclient/xprotocol.h"

namespace xcl {
namespace test {

template <typename Message_type>
class Message_from_str {
  Message_type m_protobuf_message;

 public:
  explicit Message_from_str(const std::string &text_message) {
    EXPECT_TRUE(::google::protobuf::TextFormat::ParseFromString(
        text_message, &m_protobuf_message));
  }
  Message_type &get() { return m_protobuf_message; }
  const Message_type &get() const { return m_protobuf_message; }
};

template <typename M>
class Message_compare {
 public:
  explicit Message_compare(const std::string &text_message)
      : m_expected_text_reformated(reformat_text_message(text_message)) {}

  explicit Message_compare(const M &message) {
    ::google::protobuf::TextFormat::PrintToString(message,
                                                  &m_expected_text_reformated);
  }

  bool operator==(const xcl::XProtocol::Message &msg) const {
    std::string msg_text;
    ::google::protobuf::TextFormat::PrintToString(msg, &msg_text);

    return msg_text == m_expected_text_reformated;
  }

  std::string explain_failure(const xcl::XProtocol::Message &msg) const {
    std::string msg_text;

    ::google::protobuf::TextFormat::PrintToString(msg, &msg_text);

    std::string result = "\nFollowing message doesn't match ";
    result += "with expectations:\n" + msg_text;
    result += "\nExpected message:\n " + m_expected_text_reformated;

    return result;
  }

 private:
  std::string reformat_text_message(const std::string &text_message) const {
    M m;
    std::string text_message_reformated;

    EXPECT_TRUE(
        ::google::protobuf::TextFormat::ParseFromString(text_message, &m));
    ::google::protobuf::TextFormat::PrintToString(m, &text_message_reformated);

    return text_message_reformated;
  }

  // User entered message reformatted by protobuf
  std::string m_expected_text_reformated;
};

// If Your code requires fields from this template, You need
// to create a specialization of this class for concrete message
struct Message_encoder {
  static std::string encode(const XProtocol::Message &msg) {
    std::string result;

    msg.SerializeToString(&result);

    return result;
  }
};

template <typename Msg>
struct Client_message {};

template <>
struct Client_message<::Mysqlx::Session::AuthenticateStart> {
  enum { id = ::Mysqlx::ClientMessages::SESS_AUTHENTICATE_START };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Session::AuthenticateStart make_required() {
    ::Mysqlx::Session::AuthenticateStart msg;
    msg.set_mech_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Session::AuthenticateContinue> {
  enum { id = ::Mysqlx::ClientMessages::SESS_AUTHENTICATE_CONTINUE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Session::AuthenticateContinue make_required() {
    ::Mysqlx::Session::AuthenticateContinue msg;
    msg.set_auth_data("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Session::Reset> {
  enum { id = ::Mysqlx::ClientMessages::SESS_RESET };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Session::Reset make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Session::Close> {
  enum { id = ::Mysqlx::ClientMessages::SESS_CLOSE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Session::Close make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Sql::StmtExecute> {
  enum { id = ::Mysqlx::ClientMessages::SQL_STMT_EXECUTE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Sql::StmtExecute make_required() {
    ::Mysqlx::Sql::StmtExecute msg;
    msg.set_stmt("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::Find> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_FIND };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::Find make_required() {
    ::Mysqlx::Crud::Find msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::Insert> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_INSERT };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::Insert make_required() {
    ::Mysqlx::Crud::Insert msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::Update> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_UPDATE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::Update make_required() {
    ::Mysqlx::Crud::Update msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::Delete> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_DELETE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::Delete make_required() {
    ::Mysqlx::Crud::Delete msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::CreateView> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_CREATE_VIEW };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::CreateView make_required() {
    ::Mysqlx::Crud::CreateView msg;
    msg.mutable_collection()->set_name("");
    msg.mutable_stmt()->mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::ModifyView> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_MODIFY_VIEW };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::ModifyView make_required() {
    ::Mysqlx::Crud::ModifyView msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Crud::DropView> {
  enum { id = ::Mysqlx::ClientMessages::CRUD_DROP_VIEW };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Crud::DropView make_required() {
    ::Mysqlx::Crud::DropView msg;
    msg.mutable_collection()->set_name("");
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Expect::Open> {
  enum { id = ::Mysqlx::ClientMessages::EXPECT_OPEN };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Expect::Open make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Expect::Close> {
  enum { id = ::Mysqlx::ClientMessages::EXPECT_CLOSE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Expect::Close make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Connection::CapabilitiesGet> {
  enum { id = ::Mysqlx::ClientMessages::CON_CAPABILITIES_GET };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Connection::CapabilitiesGet make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Connection::CapabilitiesSet> {
  enum { id = ::Mysqlx::ClientMessages::CON_CAPABILITIES_SET };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Connection::CapabilitiesSet make_required() {
    ::Mysqlx::Connection::CapabilitiesSet msg;
    msg.mutable_capabilities();
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Connection::Close> {
  enum { id = ::Mysqlx::ClientMessages::CON_CLOSE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Connection::Close make_required() { return {}; }
};

template <>
struct Client_message<::Mysqlx::Prepare::Prepare> {
  enum { id = ::Mysqlx::ClientMessages::PREPARE_PREPARE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Prepare::Prepare make_required() {
    ::Mysqlx::Prepare::Prepare msg;
    msg.set_stmt_id(1);
    auto stmt = msg.mutable_stmt();
    stmt->set_type(Mysqlx::Prepare::Prepare_OneOfMessage::FIND);
    *stmt->mutable_find() =
        Client_message<Mysqlx::Crud::Find>().make_required();
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Prepare::Execute> {
  enum { id = ::Mysqlx::ClientMessages::PREPARE_EXECUTE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Prepare::Execute make_required() {
    ::Mysqlx::Prepare::Execute msg;
    msg.set_stmt_id(1);
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Prepare::Deallocate> {
  enum { id = ::Mysqlx::ClientMessages::PREPARE_DEALLOCATE };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Prepare::Deallocate make_required() {
    ::Mysqlx::Prepare::Deallocate msg;
    msg.set_stmt_id(1);
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Cursor::Open> {
  enum { id = ::Mysqlx::ClientMessages::CURSOR_OPEN };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Cursor::Open make_required() {
    ::Mysqlx::Cursor::Open msg;
    msg.set_cursor_id(1);
    auto statement = msg.mutable_stmt();
    statement->set_type(Mysqlx::Cursor::Open::OneOfMessage::PREPARE_EXECUTE);
    *statement->mutable_prepare_execute() =
        Client_message<Mysqlx::Prepare::Execute>().make_required();
    return msg;
  }
};

template <>
struct Client_message<::Mysqlx::Cursor::Fetch> {
  enum { id = ::Mysqlx::ClientMessages::CURSOR_FETCH };

  static ::Mysqlx::ClientMessages::Type get_id() {
    return static_cast<::Mysqlx::ClientMessages::Type>(id);
  }

  static ::Mysqlx::Cursor::Fetch make_required() {
    ::Mysqlx::Cursor::Fetch msg;
    msg.set_cursor_id(1);
    return msg;
  }
};

template <typename Msg>
struct Server_message {};

template <>
struct Server_message<::Mysqlx::Ok> {
  enum { id = ::Mysqlx::ServerMessages::OK };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Ok make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Error> {
  enum { id = ::Mysqlx::ServerMessages::ERROR };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Error make_required() {
    ::Mysqlx::Error result;
    result.set_code(1);
    result.set_sql_state("");
    result.set_msg("");
    return result;
  }
};

template <>
struct Server_message<::Mysqlx::Connection::Capabilities> {
  enum { id = ::Mysqlx::ServerMessages::CONN_CAPABILITIES };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Connection::Capabilities make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Session::AuthenticateContinue> {
  enum { id = ::Mysqlx::ServerMessages::SESS_AUTHENTICATE_CONTINUE };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Session::AuthenticateContinue make_required() {
    ::Mysqlx::Session::AuthenticateContinue result;
    result.set_auth_data("123456789012345678901234567890");
    return result;
  }
};

template <>
struct Server_message<::Mysqlx::Session::AuthenticateOk> {
  enum { id = ::Mysqlx::ServerMessages::SESS_AUTHENTICATE_OK };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Session::AuthenticateOk make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Notice::Frame> {
  enum { id = ::Mysqlx::ServerMessages::NOTICE };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Notice::Frame make_required() {
    ::Mysqlx::Notice::Frame result;
    result.set_type(::Mysqlx::Notice::Frame_Type_WARNING);
    return result;
  }
};

template <>
struct Server_message<::Mysqlx::Resultset::ColumnMetaData> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_COLUMN_META_DATA };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::ColumnMetaData make_required() {
    ::Mysqlx::Resultset::ColumnMetaData result;
    result.set_type(Mysqlx::Resultset::ColumnMetaData_FieldType_DECIMAL);
    return {};
  }
};

template <>
struct Server_message<::Mysqlx::Resultset::Row> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_ROW };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::Row make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Resultset::FetchDone> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_FETCH_DONE };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::FetchDone make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Resultset::FetchSuspended> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_FETCH_SUSPENDED };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::FetchSuspended make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Resultset::FetchDoneMoreResultsets> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_FETCH_DONE_MORE_RESULTSETS };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::FetchDoneMoreResultsets make_required() {
    return {};
  }
};

template <>
struct Server_message<::Mysqlx::Sql::StmtExecuteOk> {
  enum { id = ::Mysqlx::ServerMessages::SQL_STMT_EXECUTE_OK };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Sql::StmtExecuteOk make_required() { return {}; }
};

template <>
struct Server_message<::Mysqlx::Resultset::FetchDoneMoreOutParams> {
  enum { id = ::Mysqlx::ServerMessages::RESULTSET_FETCH_DONE_MORE_OUT_PARAMS };

  static ::Mysqlx::ServerMessages::Type get_id() {
    return static_cast<::Mysqlx::ServerMessages::Type>(id);
  }

  static ::Mysqlx::Resultset::FetchDoneMoreOutParams make_required() {
    return {};
  }
};

// parameter `n` can be either a string or XMessage
MATCHER_P(Cmp_msg, n, "") {
  Message_compare<typename std::decay<decltype(arg)>::type> comparer(n);
  const bool result = comparer == arg;

  if (!result) {
    *result_listener << comparer.explain_failure(arg);
  }

  return result;
}

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_MESSAGE_HELPERS_H_
