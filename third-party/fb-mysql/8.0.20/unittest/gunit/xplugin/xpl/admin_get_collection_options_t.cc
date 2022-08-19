/*
 * Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/ngs/include/ngs/protocol/page_pool.h"
#include "plugin/x/protocol/encoders/encoding_buffer.h"
#include "plugin/x/protocol/encoders/encoding_pool.h"
#include "plugin/x/protocol/encoders/encoding_xmessages.h"
#include "plugin/x/protocol/encoders/encoding_xrow.h"
#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/admin_cmd_handler_t.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"

namespace xpl {
namespace test {

using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::ReturnRef;
using ::testing::StrEq;

class Admin_cmd_get_collection_options_test
    : public Admin_command_handler_test {};

TEST_F(Admin_cmd_get_collection_options_test, empty_schema) {
  set_arguments(Any::Object{
      {"schema", ""}, COLLECTION_NAME, {"options", Any::Array{"validation"}}});
  ASSERT_ERROR_CODE(ER_X_BAD_SCHEMA,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, missing_schema) {
  set_arguments(Any::Object{COLLECTION_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, empty_collection_name) {
  set_arguments(
      Any::Object{SCHEMA, {"name", ""}, {"options", Any::Array{"validation"}}});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, missing_collection_name) {
  set_arguments(Any::Object{SCHEMA});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, invalid_object) {
  set_arguments(Any::Object{SCHEMA, COLLECTION_NAME, {"foo:", "bar"}});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, empty_option_array) {
  set_arguments(
      Any::Object{SCHEMA, COLLECTION_NAME, {"options", Any::Array{}}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, one_invalid_option_array) {
  set_arguments(
      Any::Object{SCHEMA, COLLECTION_NAME, {"options", Any::Array{"foo"}}});
  ASSERT_ERROR_CODE(ER_X_COLLECTION_OPTION_DOESNT_EXISTS,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, valid_and_invalid_option_array) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION_NAME, {"options", Any::Array{"foo", "validation"}}});
  ASSERT_ERROR_CODE(ER_X_COLLECTION_OPTION_DOESNT_EXISTS,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test,
       get_validation_user_has_no_access) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION_NAME, {"options", Any::Array{"validation"}}});
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT 1 FROM `xtest`.`test_coll` LIMIT 1"), _, _))
      .WillOnce(Return(ngs::Error_code{ER_TABLEACCESS_DENIED_ERROR, ""}));
  ASSERT_ERROR_CODE(ER_TABLEACCESS_DENIED_ERROR,
                    command->get_collection_options(m_args.get()));
}

TEST_F(Admin_cmd_get_collection_options_test, get_validation_ok) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION_NAME, {"options", Any::Array{"validation"}}});
  EXPECT_CALL(
      mock_data_context,
      execute(StrEq("SELECT 1 FROM `xtest`.`test_coll` LIMIT 1"), _, _));
  EXPECT_CALL(mock_data_context,
              execute(StrEq("SELECT @@lower_case_table_names"), _, _));
  EXPECT_CALL(
      mock_data_context,
      execute(HasSubstr("SELECT IF(COUNT(*),\"strict\",\"off\") FROM "
                        "information_schema.TABLE_CONSTRAINTS WHERE "
                        "TABLE_SCHEMA='xtest' AND TABLE_NAME='test_coll'"),
              _, _));
  EXPECT_CALL(
      mock_data_context,
      execute(StrEq("SELECT GENERATION_EXPRESSION FROM "
                    "information_schema.COLUMNS WHERE TABLE_SCHEMA='xtest' AND "
                    "TABLE_NAME='test_coll' AND COLUMN_NAME='_json_schema';"),
              _, _));
  EXPECT_CALL(mock_encoder, send_column_metadata(_));
  EXPECT_CALL(mock_encoder, start_row());
  ngs::Memory_block_pool memory_block_pool{{10, 4096}};
  protocol::Encoding_pool pool{10, &memory_block_pool};
  protocol::Encoding_buffer xproto_buffer{&pool};
  protocol::XMessage_encoder xproto_encoder{&xproto_buffer};
  protocol::XRow_encoder row_builder{&xproto_encoder};
  EXPECT_CALL(mock_encoder, row_builder()).WillOnce(Return(&row_builder));
  EXPECT_CALL(mock_encoder, send_row());
  EXPECT_CALL(mock_encoder, send_result_fetch_done());
  ASSERT_ERROR_CODE(ER_X_SUCCESS,
                    command->get_collection_options(m_args.get()));
}

}  // namespace test
}  // namespace xpl
