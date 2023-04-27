/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/prepare_command_handler.h"
#include "plugin/x/src/xpl_client.h"
#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrictMock;

class Cursor_test_suite : public ::testing::Test {
 public:
  void SetUp() {
    EXPECT_CALL(m_mock_session, proto())
        .WillRepeatedly(ReturnRef(m_mock_encoder));
    EXPECT_CALL(m_mock_session, get_notice_output_queue())
        .WillRepeatedly(ReturnRef(m_mock_notice_output_queue));
    EXPECT_CALL(m_mock_session, get_notice_configuration())
        .WillRepeatedly(ReturnRef(m_mock_notice_configuration));
  }

  ngs::Metadata_builder m_meta_builder;
  StrictMock<Mock_protocol_encoder> m_mock_encoder;
  StrictMock<Mock_session> m_mock_session;
  StrictMock<Mock_notice_output_queue> m_mock_notice_output_queue;
  StrictMock<Mock_notice_configuration> m_mock_notice_configuration;
  StrictMock<Mock_sql_data_context> m_mock_data_context;
  Prepare_command_handler m_handler{&m_mock_session};
};

TEST_F(Cursor_test_suite, add_cursor_more_rows_after_open) {
  const auto stmt_id = 42;
  const auto cursor_id = 1;
  const auto resultset_stmt_id = 2;

  Mysqlx::Cursor::Open open_msg;
  open_msg.set_cursor_id(cursor_id);
  ::Mysqlx::Prepare::Execute *prepare_execute =
      new ::Mysqlx::Prepare::Execute();
  prepare_execute->set_stmt_id(stmt_id);
  ::Mysqlx::Cursor::Open_OneOfMessage *one_of_message =
      new ::Mysqlx::Cursor::Open_OneOfMessage();
  one_of_message->set_allocated_prepare_execute(prepare_execute);
  open_msg.set_allocated_stmt(one_of_message);

  const auto type = Mysqlx::Prepare::Prepare::OneOfMessage::STMT;
  m_handler.insert_prepared_statement(
      stmt_id, Prepare_command_handler::Prepared_stmt_info{
                   resultset_stmt_id, type, {}, 1, true, false, cursor_id});

  EXPECT_CALL(m_mock_encoder, get_metadata_builder())
      .WillOnce(Return(&m_meta_builder));
  EXPECT_CALL(m_mock_session, data_context())
      .WillRepeatedly(ReturnRef(m_mock_data_context));
  EXPECT_CALL(m_mock_data_context,
              execute_prep_stmt(resultset_stmt_id, true, _, _, _))
      .WillRepeatedly(Return(ngs::Success()));
  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_open));
  EXPECT_CALL(m_mock_encoder, send_exec_ok());

  ASSERT_ERROR_CODE(ER_X_SUCCESS, m_handler.execute_cursor_open(open_msg));
  EXPECT_EQ(1, m_handler.cursors_count());
  EXPECT_TRUE(nullptr != m_handler.get_cursor_if_allocated(cursor_id));
}

TEST_F(Cursor_test_suite,
       add_cursor_already_allocated_and_more_rows_after_open) {
  const auto stmt_id = 42;
  const auto cursor_id = 1;
  const auto resultset_stmt_id = 2;
  Mysqlx::Cursor::Open open_msg;
  open_msg.set_cursor_id(cursor_id);
  ::Mysqlx::Prepare::Execute *prepare_execute =
      new ::Mysqlx::Prepare::Execute();
  prepare_execute->set_stmt_id(stmt_id);
  ::Mysqlx::Cursor::Open_OneOfMessage *one_of_message =
      new ::Mysqlx::Cursor::Open_OneOfMessage();
  one_of_message->set_allocated_prepare_execute(prepare_execute);
  open_msg.set_allocated_stmt(one_of_message);

  ngs::Message_request request;
  request.reset(Mysqlx::ClientMessages::CURSOR_OPEN, &open_msg);

  EXPECT_CALL(m_mock_encoder, get_metadata_builder())
      .WillRepeatedly(Return(&m_meta_builder));

  const auto type = Mysqlx::Prepare::Prepare::OneOfMessage::STMT;
  m_handler.insert_prepared_statement(
      stmt_id, Prepare_command_handler::Prepared_stmt_info{
                   resultset_stmt_id, type, {}, 1, true, false, cursor_id});
  m_handler.insert_cursor(cursor_id, stmt_id, false, false);

  EXPECT_CALL(m_mock_session, data_context())
      .WillRepeatedly(ReturnRef(m_mock_data_context));
  EXPECT_CALL(m_mock_data_context,
              execute_prep_stmt(resultset_stmt_id, true, _, _, _))
      .WillRepeatedly(Return(ngs::Success()));
  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_open));
  EXPECT_CALL(m_mock_encoder, send_exec_ok());

  EXPECT_EQ(m_handler.cursors_count(), 1);
  ASSERT_ERROR_CODE(ER_X_SUCCESS, m_handler.execute_cursor_open(open_msg));
  EXPECT_EQ(m_handler.cursors_count(), 1);
}

TEST_F(Cursor_test_suite, add_cursor_no_existing_statement) {
  Mysqlx::Cursor::Open open_msg;
  open_msg.set_cursor_id(1);
  ::Mysqlx::Prepare::Execute *prepare_execute =
      new ::Mysqlx::Prepare::Execute();
  prepare_execute->set_stmt_id(42);
  ::Mysqlx::Cursor::Open_OneOfMessage *one_of_message =
      new ::Mysqlx::Cursor::Open_OneOfMessage();
  one_of_message->set_allocated_prepare_execute(prepare_execute);
  open_msg.set_allocated_stmt(one_of_message);

  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_open));

  ASSERT_ERROR_CODE(ER_X_BAD_STATEMENT_ID,
                    m_handler.execute_cursor_open(open_msg));

  EXPECT_EQ(m_handler.cursors_count(), 0);
}

TEST_F(Cursor_test_suite,
       add_cursor_statement_has_cursor_already_and_more_rows_after_open) {
  const auto stmt_id = 42;
  const auto cursor_id = 1;
  const auto resultset_stmt_id = 2;
  Mysqlx::Cursor::Open open_msg;
  open_msg.set_cursor_id(cursor_id);
  ::Mysqlx::Prepare::Execute *prepare_execute =
      new ::Mysqlx::Prepare::Execute();
  prepare_execute->set_stmt_id(stmt_id);
  ::Mysqlx::Cursor::Open_OneOfMessage *one_of_message =
      new ::Mysqlx::Cursor::Open_OneOfMessage();
  one_of_message->set_allocated_prepare_execute(prepare_execute);
  open_msg.set_allocated_stmt(one_of_message);

  ngs::Message_request request;
  request.reset(Mysqlx::ClientMessages::CURSOR_OPEN, &open_msg);

  const auto type = Mysqlx::Prepare::Prepare::OneOfMessage::STMT;
  m_handler.insert_prepared_statement(
      stmt_id, Prepare_command_handler::Prepared_stmt_info{
                   resultset_stmt_id, type, {}, 1, true, true, cursor_id});

  EXPECT_CALL(m_mock_encoder, get_metadata_builder())
      .WillOnce(Return(&m_meta_builder));
  EXPECT_CALL(m_mock_session, data_context())
      .WillRepeatedly(ReturnRef(m_mock_data_context));
  EXPECT_CALL(m_mock_data_context,
              execute_prep_stmt(resultset_stmt_id, true, _, _, _))
      .WillRepeatedly(Return(ngs::Success()));
  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_open));
  EXPECT_CALL(m_mock_encoder, send_exec_ok());

  ASSERT_ERROR_CODE(ER_X_SUCCESS, m_handler.execute_cursor_open(open_msg));
  EXPECT_EQ(m_handler.cursors_count(), 1);
}

TEST_F(Cursor_test_suite, close_cursor) {
  const auto cursor_id = 1;
  const auto stmt_id = 42;
  Mysqlx::Cursor::Close close_msg;
  close_msg.set_cursor_id(cursor_id);

  EXPECT_CALL(m_mock_encoder, get_metadata_builder())
      .WillOnce(Return(&m_meta_builder));

  m_handler.insert_cursor(cursor_id, stmt_id, false, false);

  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_close));
  EXPECT_CALL(m_mock_encoder, send_ok());
  EXPECT_TRUE(nullptr != m_handler.get_cursor_if_allocated(cursor_id));
  EXPECT_EQ(m_handler.cursors_count(), 1);
  ASSERT_ERROR_CODE(ER_X_SUCCESS, m_handler.execute_cursor_close(close_msg));
  EXPECT_EQ(m_handler.cursors_count(), 0);
  EXPECT_FALSE(nullptr != m_handler.get_cursor_if_allocated(cursor_id));
}

TEST_F(Cursor_test_suite, close_cursor_wrong_cursor_id) {
  const auto cursor_id = 1;
  Mysqlx::Cursor::Close close_msg;
  close_msg.set_cursor_id(cursor_id);

  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_close));

  ASSERT_ERROR_CODE(ER_X_BAD_CURSOR_ID,
                    m_handler.execute_cursor_close(close_msg));
}

TEST_F(Cursor_test_suite, fetch_cursor) {
  const auto cursor_id = 1;
  const auto stmt_id = 42;
  const auto resultset_stmt_id = 2;
  const auto count = 1;

  Mysqlx::Cursor::Fetch fetch_msg;
  fetch_msg.set_cursor_id(cursor_id);
  fetch_msg.set_fetch_rows(count);

  const auto type = Mysqlx::Prepare::Prepare::OneOfMessage::STMT;
  m_handler.insert_prepared_statement(
      stmt_id, Prepare_command_handler::Prepared_stmt_info{
                   resultset_stmt_id, type, {}, 1, true, true, cursor_id});

  EXPECT_CALL(m_mock_encoder, get_metadata_builder())
      .WillOnce(Return(&m_meta_builder));
  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_fetch));
  EXPECT_CALL(m_mock_session, data_context())
      .WillRepeatedly(ReturnRef(m_mock_data_context));
  EXPECT_CALL(m_mock_data_context, fetch_cursor(resultset_stmt_id, count, _))
      .WillRepeatedly(Return(ngs::Error_code{}));
  m_handler.insert_cursor(cursor_id, stmt_id, false, false);

  EXPECT_CALL(m_mock_encoder, send_exec_ok());
  ASSERT_ERROR_CODE(ER_X_SUCCESS, m_handler.execute_cursor_fetch(fetch_msg));
}

TEST_F(Cursor_test_suite, fetch_cursor_wrong_cursor_id) {
  const auto cursor_id = 1;
  Mysqlx::Cursor::Fetch fetch_msg;
  fetch_msg.set_cursor_id(cursor_id);

  EXPECT_CALL(m_mock_session,
              update_status(&ngs::Common_status_variables::m_cursor_fetch));
  ASSERT_ERROR_CODE(ER_X_BAD_CURSOR_ID,
                    m_handler.execute_cursor_fetch(fetch_msg));
}

}  // namespace test
}  // namespace xpl
