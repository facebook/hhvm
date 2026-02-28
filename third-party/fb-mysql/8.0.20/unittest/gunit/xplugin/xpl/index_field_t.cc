/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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
#include "plugin/x/src/index_field.h"
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

namespace {
#define PATH "$.path"
#define PATH_HASH "6EA549FAA434CCD150A7DB5FF9C0AEC77C4F5D25"
#define NOT_REQUIRED false
#define REQUIRED true
#define OPTIONS 42u
#define SRID 666u
#define DEFAULT_VALUE std::numeric_limits<uint64_t>::max()

#define SHOW_COLUMNS(field)                 \
  "SHOW COLUMNS FROM `schema`.`collection`" \
  " WHERE Field = '" field "'"

struct Field_info : public Admin_command_index::Index_field_info {
  Field_info(const std::string &path, const std::string &type,
             const bool is_required = NOT_REQUIRED,
             const uint64_t options = DEFAULT_VALUE,
             const uint64_t srid = DEFAULT_VALUE) {
    m_path = path;
    m_type = type;
    m_is_required = is_required;
    m_options = options;
    m_srid = srid;
  }
};

}  // namespace

class Index_field_create_test : public ::testing::TestWithParam<Field_info> {};

TEST_P(Index_field_create_test, fail_on_create) {
  const auto &param = GetParam();
  ngs::Error_code error;
  std::unique_ptr<const Index_field> field(
      Index_field::create(true, param, &error));
  ASSERT_ERROR_CODE(ER_X_CMD_ARGUMENT_VALUE, error);
  ASSERT_EQ(nullptr, field.get());
}

Field_info fail_on_create_param[] = {
    {"", "DECIMAL"},
    {PATH, ""},
    {PATH, "DECIMAL SIGNED"},
    {PATH, "tinyint(10,2)"},
    {PATH, "tinyint", NOT_REQUIRED, OPTIONS},
    {PATH, "tinyint", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "smallint(10,2)"},
    {PATH, "smallint", NOT_REQUIRED, OPTIONS},
    {PATH, "smallint", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "mediumint(10,2)"},
    {PATH, "mediumint", NOT_REQUIRED, OPTIONS},
    {PATH, "mediumint", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "int(10,2)"},
    {PATH, "int", NOT_REQUIRED, OPTIONS},
    {PATH, "int", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "integer(10,2)"},
    {PATH, "integer", NOT_REQUIRED, OPTIONS},
    {PATH, "integer", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "bigint(10,2)"},
    {PATH, "bigint", NOT_REQUIRED, OPTIONS},
    {PATH, "bigint", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "real", NOT_REQUIRED, OPTIONS},
    {PATH, "real", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "float", NOT_REQUIRED, OPTIONS},
    {PATH, "float", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "double", NOT_REQUIRED, OPTIONS},
    {PATH, "double", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "numeric", NOT_REQUIRED, OPTIONS},
    {PATH, "numeric", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "date(10)"},
    {PATH, "date(10,2)"},
    {PATH, "date unsigned"},
    {PATH, "date", NOT_REQUIRED, OPTIONS},
    {PATH, "date", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "time(10,2)"},
    {PATH, "time unsigned"},
    {PATH, "time", NOT_REQUIRED, OPTIONS},
    {PATH, "time", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "timestamp(10,2)"},
    {PATH, "timestamp unsigned"},
    {PATH, "timestamp", NOT_REQUIRED, OPTIONS},
    {PATH, "timestamp", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "datetime(10,2)"},
    {PATH, "datetime (10)"},
    {PATH, "datetime unsigned"},
    {PATH, "datetime", NOT_REQUIRED, OPTIONS},
    {PATH, "datetime", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "year(10,2)"},
    {PATH, "year unsigned"},
    {PATH, "year", NOT_REQUIRED, OPTIONS},
    {PATH, "year", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "bit(10,2)"},
    {PATH, "bit (10)"},
    {PATH, "bit unsigned "},
    {PATH, "bit", NOT_REQUIRED, OPTIONS},
    {PATH, "bit", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "blob(10,2)"},
    {PATH, "blob unsigned"},
    {PATH, "blob", NOT_REQUIRED, OPTIONS},
    {PATH, "blob", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "text(10,2)"},
    {PATH, "text unsigned"},
    {PATH, "text", NOT_REQUIRED, OPTIONS},
    {PATH, "text", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "geojson(10)"},
    {PATH, "geojson(10,2)"},
    {PATH, "geojson unsigned"},
    {PATH, "fulltext(10)"},
    {PATH, "fulltext unsigned"},
    {PATH, "fulltext", NOT_REQUIRED, OPTIONS},
    {PATH, "fulltext", NOT_REQUIRED, DEFAULT_VALUE, SRID},
    {PATH, "char(10,2)"},
};

INSTANTIATE_TEST_CASE_P(fail_on_create_field, Index_field_create_test,
                        ::testing::ValuesIn(fail_on_create_param));

struct Param_index_field_add_field {
  std::string expect;
  Field_info info;
};

class Index_field_add_field_test
    : public ::testing::TestWithParam<Param_index_field_add_field> {};

TEST_P(Index_field_add_field_test, add_field) {
  const auto &param = GetParam();
  Query_string_builder qb;
  ngs::Error_code error;
  std::unique_ptr<const Index_field> field(
      Index_field::create(true, param.info, &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
  field->add_field(&qb);
  ASSERT_STREQ(std::string(param.expect).c_str(), qb.get().c_str());
}

Param_index_field_add_field add_field_param[] = {
    {"`$ix_xd_" PATH_HASH "`", {PATH, "DECIMAL", NOT_REQUIRED}},
    {"`$ix_xd_" PATH_HASH "`", {PATH, "decimal", NOT_REQUIRED}},
    {"`$ix_xd_" PATH_HASH "`", {PATH, "DEcimAL", NOT_REQUIRED}},
    {"`$ix_xd32_" PATH_HASH "`", {PATH, "DECIMAL(32)", NOT_REQUIRED}},
    {"`$ix_xd32_16_" PATH_HASH "`", {PATH, "DECIMAL(32,16)", NOT_REQUIRED}},
    {"`$ix_xd_16_" PATH_HASH "`", {PATH, "DECIMAL(0,16)", NOT_REQUIRED}},
    {"`$ix_xd32_16_u_" PATH_HASH "`",
     {PATH, "DECIMAL(32,16) UNSIGNED", NOT_REQUIRED}},
    {"`$ix_xd32_16_ur_" PATH_HASH "`",
     {PATH, "DECIMAL(32,16) UNSIGNED", REQUIRED}},
    {"`$ix_xd32_16_ur_" PATH_HASH "`",
     {PATH, "DECIMAL(32,16)    UNSIGNED", REQUIRED}},
    {"`$ix_xd32_16_r_" PATH_HASH "`", {PATH, "DECIMAL(32,16)", REQUIRED}},
    {"`$ix_xd_ur_" PATH_HASH "`", {PATH, "DECIMAL UNSIGNED", REQUIRED}},
    {"`$ix_xd_ur_" PATH_HASH "`", {PATH, "DECIMAL unsigned", REQUIRED}},
    {"`$ix_xd_ur_" PATH_HASH "`", {PATH, "DECIMAL UNsignED", REQUIRED}},
    {"`$ix_xd_ur_" PATH_HASH "`", {PATH, "DECIMAL    UNSIGNED", REQUIRED}},
    {"`$ix_it_" PATH_HASH "`", {PATH, "tinyint", NOT_REQUIRED}},
    {"`$ix_is_" PATH_HASH "`", {PATH, "smallint", NOT_REQUIRED}},
    {"`$ix_im_" PATH_HASH "`", {PATH, "mediumint", NOT_REQUIRED}},
    {"`$ix_i_" PATH_HASH "`", {PATH, "int", NOT_REQUIRED}},
    {"`$ix_i_" PATH_HASH "`", {PATH, "integer", NOT_REQUIRED}},
    {"`$ix_ib_" PATH_HASH "`", {PATH, "bigint", NOT_REQUIRED}},
    {"`$ix_fr_" PATH_HASH "`", {PATH, "real", NOT_REQUIRED}},
    {"`$ix_f_" PATH_HASH "`", {PATH, "float", NOT_REQUIRED}},
    {"`$ix_fd_" PATH_HASH "`", {PATH, "double", NOT_REQUIRED}},
    {"`$ix_fd32_16_" PATH_HASH "`", {PATH, "double(32,  16)", NOT_REQUIRED}},
    {"`$ix_fd32_16_" PATH_HASH "`", {PATH, "double(32  ,16)", NOT_REQUIRED}},
    {"`$ix_fd32_16_" PATH_HASH "`", {PATH, "double(32  ,  16)", NOT_REQUIRED}},
    {"`$ix_xn_" PATH_HASH "`", {PATH, "numeric", NOT_REQUIRED}},
    {"`$ix_d_" PATH_HASH "`", {PATH, "date", NOT_REQUIRED}},
    {"`$ix_dt_" PATH_HASH "`", {PATH, "time", NOT_REQUIRED}},
    {"`$ix_ds_" PATH_HASH "`", {PATH, "timestamp", NOT_REQUIRED}},
    {"`$ix_dd_" PATH_HASH "`", {PATH, "datetime", NOT_REQUIRED}},
    {"`$ix_dy_" PATH_HASH "`", {PATH, "year", NOT_REQUIRED}},
    {"`$ix_t_" PATH_HASH "`", {PATH, "bit", NOT_REQUIRED}},
    {"`$ix_bt_" PATH_HASH "`", {PATH, "blob", NOT_REQUIRED}},
    {"`$ix_t_" PATH_HASH "`", {PATH, "text", NOT_REQUIRED}},
    {"`$ix_gj_" PATH_HASH "`", {PATH, "geojson", NOT_REQUIRED}},
    {"`$ix_ft_" PATH_HASH "`", {PATH, "fulltext", NOT_REQUIRED}},
    {"`$ix_t32_" PATH_HASH "`(32)", {PATH, "text(32)", NOT_REQUIRED}},
    {"`$ix_c_" PATH_HASH "`", {PATH, "char", NOT_REQUIRED}},
    {"`$ix_c10_" PATH_HASH "`", {PATH, "char(10)", NOT_REQUIRED}},
    {"`$ix_c20_" PATH_HASH "`",
     {PATH, "char(20) charset latin1", NOT_REQUIRED}},
    {"`$ix_c30_" PATH_HASH "`",
     {PATH, "char(30) collate latin1_bin", NOT_REQUIRED}},
    {"`$ix_c40_" PATH_HASH "`",
     {PATH, "char(40) character set latin1 collate latin1_bin", NOT_REQUIRED}},
    {"`$ix_t128_" PATH_HASH "`(128)",
     {PATH, "text(128) character set latin1 collate latin1_bin", NOT_REQUIRED}},
    {"`$ix_c_" PATH_HASH "`",
     {PATH, "char character set latin1", NOT_REQUIRED}},
};

INSTANTIATE_TEST_CASE_P(get_index_field_name, Index_field_add_field_test,
                        ::testing::ValuesIn(add_field_param));

struct Param_index_field_add_column {
  std::string expect;
  bool virtual_supported;
  Field_info info;
};

class Index_field_add_column_test
    : public ::testing::TestWithParam<Param_index_field_add_column> {};

TEST_P(Index_field_add_column_test, add_column) {
  const Param_index_field_add_column &param = GetParam();
  Query_string_builder qb;
  ngs::Error_code error;
  std::unique_ptr<const Index_field> field(
      Index_field::create(param.virtual_supported, param.info, &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
  field->add_column(&qb);
  ASSERT_STREQ(param.expect.c_str(), qb.get().c_str());
}

Param_index_field_add_column add_column_param[] = {
    {" ADD COLUMN `$ix_xd_" PATH_HASH
     "` DECIMAL GENERATED ALWAYS AS (JSON_EXTRACT(doc, '$.path')) VIRTUAL",
     true,
     {PATH, "DECIMAL", NOT_REQUIRED}},
    {" ADD COLUMN `$ix_xd_" PATH_HASH
     "` DECIMAL GENERATED ALWAYS AS (JSON_EXTRACT(doc, '$.path')) STORED",
     false,
     {PATH, "DECIMAL", NOT_REQUIRED}},
    {" ADD COLUMN `$ix_t32_" PATH_HASH
     "` TEXT GENERATED ALWAYS AS (JSON_UNQUOTE(JSON_EXTRACT(doc, "
     "'$.path'))) VIRTUAL",
     true,
     {PATH, "TEXT(32)", NOT_REQUIRED}},
    {" ADD COLUMN `$ix_t32_r_" PATH_HASH
     "` TEXT GENERATED ALWAYS AS (JSON_UNQUOTE(JSON_EXTRACT(doc, "
     "'$.path'))) VIRTUAL NOT NULL",
     true,
     {PATH, "TEXT(32)", REQUIRED}},
    {" ADD COLUMN `$ix_gj_r_" PATH_HASH
     "` GEOMETRY GENERATED ALWAYS AS (ST_GEOMFROMGEOJSON"
     "(JSON_EXTRACT(doc, '$.path'),1,4326)) STORED NOT NULL SRID 4326",
     true,
     {PATH, "GEOJSON", REQUIRED}},
    {" ADD COLUMN `$ix_gj_" PATH_HASH
     "` GEOMETRY GENERATED ALWAYS AS (ST_GEOMFROMGEOJSON("
     "JSON_EXTRACT(doc, '$.path'),42,4326)) STORED SRID 4326",
     true,
     {PATH, "GEOJSON", NOT_REQUIRED, OPTIONS}},
    {" ADD COLUMN `$ix_gj_" PATH_HASH
     "` GEOMETRY GENERATED ALWAYS AS (ST_GEOMFROMGEOJSON("
     "JSON_EXTRACT(doc, '$.path'),1,666)) STORED SRID 666",
     false,
     {PATH, "GEOJSON", NOT_REQUIRED, DEFAULT_VALUE, SRID}},
    {" ADD COLUMN `$ix_ft_" PATH_HASH
     "` TEXT GENERATED ALWAYS AS (JSON_UNQUOTE("
     "JSON_EXTRACT(doc, '$.path'))) STORED",
     false,
     {PATH, "FULLTEXT", NOT_REQUIRED}},
    {" ADD COLUMN `$ix_c20_" PATH_HASH
     "` CHAR(20) GENERATED ALWAYS AS (JSON_UNQUOTE("
     "JSON_EXTRACT(doc, '$.path'))) STORED",
     false,
     {PATH, "CHAR(20)", NOT_REQUIRED}},
    {" ADD COLUMN `$ix_t64_" PATH_HASH
     "` TEXT CHARACTER SET latin1 COLLATE latin1_bin"
     " GENERATED ALWAYS AS (JSON_UNQUOTE(JSON_EXTRACT(doc, '$.path'))) STORED",
     false,
     {PATH, "TEXT(64) CHARACTER SET latin1 COLLATE latin1_bin", NOT_REQUIRED}},
};

INSTANTIATE_TEST_CASE_P(add_column, Index_field_add_column_test,
                        ::testing::ValuesIn(add_column_param));

class Index_field_is_column_exists_test : public ::testing::Test {
 public:
  void SetUp() {
    ngs::Error_code error;
    field.reset(
        Index_field::create(true, Field_info(PATH, "int", REQUIRED), &error));
    ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
  }

  using Sql = ngs::PFS_string;
  ::testing::StrictMock<Mock_sql_data_context> data_context;
  std::unique_ptr<const Index_field> field;
};

TEST_F(Index_field_is_column_exists_test, column_is_not_exist) {
  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_i_r_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Success()));
  ngs::Error_code error;
  ASSERT_FALSE(
      field->is_column_exists(&data_context, "schema", "collection", &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}

TEST_F(Index_field_is_column_exists_test, column_is_not_exist_error) {
  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_i_r_" PATH_HASH))), _, _))
      .WillOnce(Return(ngs::Error(ER_X_ARTIFICIAL1, "internal error")));
  ngs::Error_code error;
  ASSERT_FALSE(
      field->is_column_exists(&data_context, "schema", "collection", &error));
  ASSERT_ERROR_CODE(ER_X_ARTIFICIAL1, error);
}

TEST_F(Index_field_is_column_exists_test, column_is_exist) {
  One_row_resultset data{"anything"};
  EXPECT_CALL(data_context,
              execute(Eq(Sql(SHOW_COLUMNS("$ix_i_r_" PATH_HASH))), _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));
  ngs::Error_code error;
  ASSERT_TRUE(
      field->is_column_exists(&data_context, "schema", "collection", &error));
  ASSERT_ERROR_CODE(ER_X_SUCCESS, error);
}
}  // namespace test
}  // namespace xpl
