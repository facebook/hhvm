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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "plugin/x/src/admin_cmd_arguments.h"
#include "plugin/x/src/admin_cmd_index.h"
#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"
#include "unittest/gunit/xplugin/xpl/one_row_resultset.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::StrictMock;

class Admin_command_index_stub : public Admin_command_index {
 public:
  explicit Admin_command_index_stub(iface::Session *session)
      : Admin_command_index(session) {}
  using Admin_command_index::is_table_support_virtual_columns;
};

class Admin_command_index_test : public ::testing::Test {
 public:
  using Sql = ngs::PFS_string;

  void SetUp() {
    command.reset(new Admin_command_index_stub(&session));

    EXPECT_CALL(session, data_context())
        .WillRepeatedly(ReturnRef(data_context));
    EXPECT_CALL(session, proto()).WillRepeatedly(ReturnRef(encoder));
    EXPECT_CALL(encoder, send_exec_ok()).WillRepeatedly(Return(true));
  }

  void set_arguments(const Any &value) {
    list.Add()->CopyFrom(value);
    args.reset(new Admin_command_arguments_object(list));
  }

  StrictMock<Mock_sql_data_context> data_context;
  StrictMock<Mock_client> client;
  StrictMock<Mock_protocol_encoder> encoder;
  StrictMock<Mock_session> session;
  std::unique_ptr<Admin_command_index_stub> command;
  Admin_command_arguments_object::List list;
  std::unique_ptr<Admin_command_arguments_object> args;
};

namespace {
#define ALPHA "alpha"
#define BETA "beta"
#define GAMMA "gamma"
const Any::Object::Fld SCHEMA{"schema", ALPHA};
const Any::Object::Fld COLLECTION{"collection", BETA};
const Any::Object::Fld INDEX_NAME{"name", GAMMA};
const Any::Object::Fld UNIQUE{"unique", true};
const Any::Object::Fld NOT_UNIQUE{"unique", false};
const Any::Object::Fld INDEX_TYPE_PLAIN{"type", "INDEX"};
const Any::Object::Fld INDEX_TYPE_SPATIAL{"type", "SPATIAL"};
const Any::Object::Fld INDEX_TYPE_FULLTEXT{"type", "FULLTEXT"};
#define PATH "$.path"
#define PATH_HASH "6EA549FAA434CCD150A7DB5FF9C0AEC77C4F5D25"
const Any::Object::Fld MEMBER{"member", PATH};
const Any::Object::Fld REQUIRED{"required", true};
const Any::Object::Fld NOT_REQUIRED{"required", false};
const Any::Object DECIMAL_FIELD{MEMBER, {"type", "DECIMAL"}, REQUIRED};
const Any::Object TEXT_FIELD{MEMBER, {"type", "TEXT"}, NOT_REQUIRED};
const Any::Object GEOJSON_FIELD{MEMBER, {"type", "GEOJSON"}, REQUIRED};
const Any::Object FULLTEXT_FIELD{MEMBER, {"type", "FULLTEXT"}, NOT_REQUIRED};
const One_row_resultset TABLE_WITH_INNODB_ENGINE{BETA,
                                                 "table with ENGINE=InnoDB"};
const One_row_resultset TABLE_WITH_MYISAM_ENGINE{BETA,
                                                 "table with ENGINE=MyISAM"};
#define QUOTE(var) "'" var "'"
#define IDENT(var) "`" var "`"
#define GET_INDEX_COLUMNS \
  "SELECT column_name, COUNT(index_name) AS count FROM "              \
  "information_schema.statistics WHERE table_name=" QUOTE(BETA)       \
  " AND table_schema=" QUOTE(ALPHA)                                   \
  " AND column_name IN (SELECT BINARY column_name"                    \
  " FROM information_schema.statistics WHERE table_name=" QUOTE(BETA) \
  " AND table_schema=" QUOTE(ALPHA) " AND index_name=" QUOTE(GAMMA)   \
  " AND column_name RLIKE '^\\\\$ix_[[:alnum:]_]+[[:xdigit:]]+$') "   \
  "GROUP BY column_name HAVING count = 1"
#define ALTER_TABLE "ALTER TABLE " IDENT(ALPHA) "." IDENT(BETA)
#define DROP_INDEX " DROP INDEX " IDENT(GAMMA)
#define ADD_INDEX(itype, iargs) " ADD " itype " " IDENT(GAMMA) " (" iargs ")"
#define DROP_COLUMN(cname) ", DROP COLUMN " IDENT(cname)
#define ADD_COLUMN(cname, ctype, cgen, cmod)                          \
  " ADD COLUMN " IDENT(cname) " " ctype " GENERATED ALWAYS AS (" cgen \
                              ") " cmod ","
#define SHOW_CREATE_TABLE "SHOW CREATE TABLE " IDENT(ALPHA) "." IDENT(BETA)
#define DECIMAL_COLUMN "$ix_xd_r_" PATH_HASH
#define EXTRACT_PATH "JSON_EXTRACT(doc, " QUOTE(PATH) ")"
#define SHOW_COLUMNS(carg)                                                   \
  "SHOW COLUMNS FROM " IDENT(ALPHA) "." IDENT(BETA) " WHERE Field = " QUOTE( \
      carg)

}  // namespace

TEST_F(Admin_command_index_test, drop_empty_schema) {
  set_arguments(Any::Object{{"schema", ""}, COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_BAD_SCHEMA, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_missing_schema) {
  set_arguments(Any::Object{COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_empty_collection) {
  set_arguments(Any::Object{SCHEMA, {"collection", ""}, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_missing_collection) {
  set_arguments(Any::Object{SCHEMA, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_empty_index_name) {
  set_arguments(Any::Object{SCHEMA, COLLECTION, {"name", ""}});
  ASSERT_ERROR_CODE(ER_X_MISSING_ARGUMENT, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_missing_index_name) {
  set_arguments(Any::Object{SCHEMA, COLLECTION});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_no_schema) {
  EXPECT_CALL(data_context, is_sql_mode_set(_)).WillOnce(Return(false));

  EXPECT_CALL(data_context, execute(Eq(Sql(GET_INDEX_COLUMNS)), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(data_context, execute(Eq(Sql(ALTER_TABLE DROP_INDEX)), _, _))
      .WillOnce(Return(ngs::Error_code(ER_BAD_DB_ERROR, "bad db error")));

  set_arguments(Any::Object{SCHEMA, COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_no_collection) {
  EXPECT_CALL(data_context, is_sql_mode_set(_)).WillOnce(Return(false));

  EXPECT_CALL(data_context, execute(Eq(Sql(GET_INDEX_COLUMNS)), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(data_context, execute(Eq(Sql(ALTER_TABLE DROP_INDEX)), _, _))
      .WillOnce(Return(ngs::Error_code(ER_NO_SUCH_TABLE, "no such table")));

  set_arguments(Any::Object{SCHEMA, COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_no_virtual_column_no_index) {
  EXPECT_CALL(data_context, is_sql_mode_set(_)).WillOnce(Return(false));

  EXPECT_CALL(data_context, execute(Eq(Sql(GET_INDEX_COLUMNS)), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(data_context, execute(Eq(Sql(ALTER_TABLE DROP_INDEX)), _, _))
      .WillOnce(Return(ngs::Error_code(ER_NO_SUCH_INDEX, "no such index")));

  set_arguments(Any::Object{SCHEMA, COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_NO_SUCH_INDEX, command->drop(args.get()));
}

TEST_F(Admin_command_index_test, drop_index_with_column) {
  One_row_resultset column_name{DECIMAL_COLUMN};

  EXPECT_CALL(data_context, is_sql_mode_set(_)).WillOnce(Return(false));

  EXPECT_CALL(data_context, execute(Eq(Sql(GET_INDEX_COLUMNS)), _, _))
      .WillOnce(DoAll(SetUpResultset(column_name), Return(ngs::Success())));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE DROP_INDEX DROP_COLUMN(DECIMAL_COLUMN))), _,
              _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA, COLLECTION, INDEX_NAME});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->drop(args.get()));
}

TEST_F(Admin_command_index_test,
       is_table_support_virtual_columns_no_collection) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(Return(ngs::Success()));

  ngs::Error_code error;
  ASSERT_FALSE(command->is_table_support_virtual_columns(ALPHA, BETA, &error));
  ASSERT_ERROR_CODE(ER_INTERNAL_ERROR, error);
}

TEST_F(Admin_command_index_test,
       is_table_support_virtual_columns_error_on_query) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(Return(ngs::Error(ER_X_ARTIFICIAL1, "artificial 1")));

  ngs::Error_code error;
  ASSERT_FALSE(command->is_table_support_virtual_columns(ALPHA, BETA, &error));
  ASSERT_ERROR_CODE(ER_X_ARTIFICIAL1, error);
}

TEST_F(Admin_command_index_test,
       is_table_support_virtual_columns_wrong_responce) {
  One_row_resultset data{BETA, "wrong responce"};
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  ngs::Error_code error;
  ASSERT_FALSE(command->is_table_support_virtual_columns(ALPHA, BETA, &error));
  ASSERT_ERROR_CODE(ER_INTERNAL_ERROR, error);
}

TEST_F(Admin_command_index_test,
       is_table_support_virtual_columns_engine_present) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  ngs::Error_code error;
  ASSERT_FALSE(command->is_table_support_virtual_columns(ALPHA, BETA, &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Admin_command_index_test, is_table_support_virtual_columns_success) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  ngs::Error_code error;
  ASSERT_TRUE(command->is_table_support_virtual_columns(ALPHA, BETA, &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Admin_command_index_test, create_invalid_schema) {
  set_arguments(Any::Object{{"schema", ""},
                            COLLECTION,
                            INDEX_NAME,
                            UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_BAD_SCHEMA, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_invalid_collection) {
  set_arguments(Any::Object{SCHEMA,
                            {"collection", ""},
                            INDEX_NAME,
                            UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_invalid_index_name) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION, {"name", ""}, UNIQUE, {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_missing_unique) {
  set_arguments(Any::Object{
      SCHEMA, COLLECTION, {"name", ""}, {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_check_virtual_support_no_collection) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(Return(ngs::Error(ER_X_ARTIFICIAL1, "artificial 1")));

  set_arguments(Any::Object{
      SCHEMA, COLLECTION, INDEX_NAME, UNIQUE, {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_BAD_TABLE, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_check_virtual_support_goes_wrong) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(Return(ngs::Error(ER_INTERNAL_ERROR, "internal error")));

  set_arguments(Any::Object{
      SCHEMA, COLLECTION, INDEX_NAME, UNIQUE, {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_INTERNAL_ERROR, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_bad_constraint) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  set_arguments(Any::Object{
      SCHEMA, COLLECTION, INDEX_NAME, UNIQUE, {"constraint", Any::Object{}}});
  ASSERT_ERROR_CODE(ER_X_CMD_NUM_ARGUMENTS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_regular_index_with_virtual_column) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(DECIMAL_COLUMN, "DECIMAL",
                                            EXTRACT_PATH, "VIRTUAL NOT NULL")
                         ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN)))),
              _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_regular_index_with_stored_column) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(DECIMAL_COLUMN, "DECIMAL",
                                            EXTRACT_PATH, "STORED NOT NULL")
                         ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN)))),
              _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_regular_index_without_column) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  One_row_resultset data{"column is present"};
  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN)))), _,
              _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_regular_index_with_two_virtual_column) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_t_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(DECIMAL_COLUMN, "DECIMAL",
                                            EXTRACT_PATH, "VIRTUAL NOT NULL")
                         ADD_COLUMN("$ix_t_" PATH_HASH, "TEXT",
                                    "JSON_UNQUOTE(" EXTRACT_PATH ")", "VIRTUAL")
                             ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN) "," IDENT(
                                                    "$ix_t_" PATH_HASH)))),
              _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(
      Any::Object{SCHEMA,
                  COLLECTION,
                  INDEX_NAME,
                  NOT_UNIQUE,
                  INDEX_TYPE_PLAIN,
                  {"constraint", Any::Array{DECIMAL_FIELD, TEXT_FIELD}}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_spatial_index) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_gj_r_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(
          Eq(Sql(ALTER_TABLE ADD_COLUMN("$ix_gj_r_" PATH_HASH, "GEOMETRY",
                                        "ST_GEOMFROMGEOJSON(" EXTRACT_PATH
                                        ",1,4326)",
                                        "STORED NOT NULL SRID 4326")
                     ADD_INDEX("SPATIAL INDEX", IDENT("$ix_gj_r_" PATH_HASH)))),
          _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            INDEX_TYPE_SPATIAL,
                            {"constraint", GEOJSON_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_unique_spatial_index) {
  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            UNIQUE,
                            INDEX_TYPE_SPATIAL,
                            {"constraint", GEOJSON_FIELD}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_unable_to_create) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(DECIMAL_COLUMN, "DECIMAL",
                                            EXTRACT_PATH, "VIRTUAL NOT NULL")
                         ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN)))),
              _, _))
      .WillOnce(Return(ngs::Error(ER_X_ARTIFICIAL1, "artificial error")));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_ARTIFICIAL1, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_bd_null_error_required_field_missing) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS(DECIMAL_COLUMN))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(DECIMAL_COLUMN, "DECIMAL",
                                            EXTRACT_PATH, "VIRTUAL NOT NULL")
                         ADD_INDEX("INDEX", IDENT(DECIMAL_COLUMN)))),
              _, _))
      .WillOnce(Return(ngs::Error(ER_BAD_NULL_ERROR, "bad null error")));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            {"constraint", DECIMAL_FIELD}});
  ASSERT_ERROR_CODE(ER_X_DOC_REQUIRED_FIELD_MISSING,
                    command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_bd_null_error) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_INNODB_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_t_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(
                  "$ix_t_" PATH_HASH, "TEXT", "JSON_UNQUOTE(" EXTRACT_PATH ")",
                  "VIRTUAL") ADD_INDEX("INDEX", IDENT("$ix_t_" PATH_HASH)))),
              _, _))
      .WillOnce(Return(ngs::Error(ER_BAD_NULL_ERROR, "bad null error")));

  set_arguments(Any::Object{
      SCHEMA, COLLECTION, INDEX_NAME, NOT_UNIQUE, {"constraint", TEXT_FIELD}});
  ASSERT_ERROR_CODE(ER_BAD_NULL_ERROR, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_unable_to_craete_spatial_index) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_gj_r_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(
          Eq(Sql(ALTER_TABLE ADD_COLUMN("$ix_gj_r_" PATH_HASH, "GEOMETRY",
                                        "ST_GEOMFROMGEOJSON(" EXTRACT_PATH
                                        ",1,4326)",
                                        "STORED NOT NULL SRID 4326")
                     ADD_INDEX("SPATIAL INDEX", IDENT("$ix_gj_r_" PATH_HASH)))),
          _, _))
      .WillOnce(Return(
          ngs::Error(ER_SPATIAL_CANT_HAVE_NULL, "spatial cant have null")));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            INDEX_TYPE_SPATIAL,
                            {"constraint", GEOJSON_FIELD}});
  ASSERT_ERROR_CODE(ER_X_DOC_REQUIRED_FIELD_MISSING,
                    command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_fulltext_index) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_ft_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(Eq(Sql(ALTER_TABLE ADD_COLUMN(
                  "$ix_ft_" PATH_HASH, "TEXT", "JSON_UNQUOTE(" EXTRACT_PATH ")",
                  "STORED") ADD_INDEX("FULLTEXT INDEX",
                                      IDENT("$ix_ft_" PATH_HASH)))),
              _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            INDEX_TYPE_FULLTEXT,
                            {"constraint", FULLTEXT_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_fulltext_index_with_parser) {
  EXPECT_CALL(data_context, execute(Eq(Sql(SHOW_CREATE_TABLE)), _, _))
      .WillOnce(DoAll(SetUpResultset(TABLE_WITH_MYISAM_ENGINE),
                      Return(ngs::Success())));

  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_ft_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));

  EXPECT_CALL(
      data_context,
      execute(
          Eq(Sql(
              ALTER_TABLE ADD_COLUMN("$ix_ft_" PATH_HASH, "TEXT",
                                     "JSON_UNQUOTE(" EXTRACT_PATH ")", "STORED")
                  ADD_INDEX("FULLTEXT INDEX",
                            IDENT("$ix_ft_" PATH_HASH)) " WITH PARSER ngram")),
          _, _))
      .WillOnce(Return(ngs::Success()));

  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            NOT_UNIQUE,
                            INDEX_TYPE_FULLTEXT,
                            {"with_parser", "ngram"},
                            {"constraint", FULLTEXT_FIELD}});
  ASSERT_ERROR_CODE(ER_X_SUCCESS, command->create(args.get()));
}

TEST_F(Admin_command_index_test, create_unique_fulltext_index) {
  set_arguments(Any::Object{SCHEMA,
                            COLLECTION,
                            INDEX_NAME,
                            UNIQUE,
                            INDEX_TYPE_FULLTEXT,
                            {"constraint", FULLTEXT_FIELD}});
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE, command->create(args.get()));
}

}  // namespace test
}  // namespace xpl
