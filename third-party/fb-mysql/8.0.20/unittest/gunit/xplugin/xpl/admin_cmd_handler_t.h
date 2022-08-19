/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_ADMIN_CMD_HANDLER_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_ADMIN_CMD_HANDLER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/admin_cmd_arguments.h"
#include "plugin/x/src/admin_cmd_handler.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

const Any::Object::Fld SCHEMA{"schema", "xtest"};
const Any::Object::Fld COLLECTION_NAME{"name", "test_coll"};

class Admin_command_handler_stub : public Admin_command_handler {
 public:
  explicit Admin_command_handler_stub(iface::Session *session)
      : Admin_command_handler(session) {}

  using Admin_command_handler::create_collection;
  using Admin_command_handler::ensure_collection;
  using Admin_command_handler::get_collection_options;
  using Admin_command_handler::modify_collection_options;
};

class Admin_command_handler_test : public ::testing::Test {
 public:
  void SetUp() {
    command.reset(new Admin_command_handler_stub(&mock_session));

    EXPECT_CALL(mock_session, data_context())
        .WillRepeatedly(ReturnRef(mock_data_context));
    EXPECT_CALL(mock_session, proto()).WillRepeatedly(ReturnRef(mock_encoder));
    EXPECT_CALL(mock_encoder, send_exec_ok()).WillRepeatedly(Return(true));

    EXPECT_CALL(mock_session, update_status(_));
  }

  void set_arguments(const Any &value) {
    m_list.Add()->CopyFrom(value);
    m_args.reset(new Admin_command_arguments_object(m_list));
  }

  void set_validation_details(const Any::Object &validation) {
    set_arguments(
        Any::Object{SCHEMA,
                    COLLECTION_NAME,
                    {"options", Any::Object{{"validation", validation}}}});
  }

  StrictMock<Mock_sql_data_context> mock_data_context;
  StrictMock<Mock_protocol_encoder> mock_encoder;
  StrictMock<Mock_session> mock_session;
  std::unique_ptr<Admin_command_handler_stub> command;
  Admin_command_arguments_object::List m_list;
  std::unique_ptr<Admin_command_arguments_object> m_args;
};

}  // namespace test
}  // namespace xpl

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_ADMIN_CMD_HANDLER_H_
