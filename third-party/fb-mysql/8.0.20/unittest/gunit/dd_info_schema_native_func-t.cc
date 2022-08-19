/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "sql/item_func.h"
#include "sql/item_timefunc.h"
#include "sql/parse_tree_helpers.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"

namespace dd_info_schema_native_func {
using my_testing::Server_initializer;

/*
  Test fixture for testing native functions introduced for the
  INFORMATION_SCHEMA.
*/

class ISNativeFuncTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }

  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

// Test case to verify native functions with all NULL arguments.
TEST_F(ISNativeFuncTest, AllNullArguments) {
  Item *item = nullptr;
  Item_null *null = new (thd()->mem_root) Item_null();
  PT_item_list *null_list = new (thd()->mem_root) PT_item_list;
  auto prepare_null_list = [null_list, null](int cnt) {
    for (int i = 0; i < cnt; i++) null_list->push_front(null);
    return null_list;
  };

#define NULL_ARG null
#define TWO_NULL_ARGS NULL_ARG, NULL_ARG
#define THREE_NULL_ARGS TWO_NULL_ARGS, NULL_ARG
#define FOUR_NULL_ARGS THREE_NULL_ARGS, NULL_ARG
#define FIVE_NULL_ARGS FOUR_NULL_ARGS, NULL_ARG
#define CREATE_ITEM(X, ARGS) item = new (thd()->mem_root) X(POS(), ARGS)

  // INTERNAL_TABLE_ROWS(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  CREATE_ITEM(Item_func_internal_table_rows, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_AVG_ROW_LENGTH(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_avg_row_length, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_DATA_LENGTH(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_data_length, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_MAX_DATA_LENGTH(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_max_data_length, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_INDEX_LENGTH(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_index_length, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_DATA_FREE(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_data_free, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_AUTO_INCREMENT(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  // NULL)
  CREATE_ITEM(Item_func_internal_auto_increment, prepare_null_list(9));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_UPDATE_TIME(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_update_time, prepare_null_list(8));
  MYSQL_TIME ldate;
  item->get_date(&ldate, 0);
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_CHECK_TIME(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_check_time, prepare_null_list(8));
  item->get_date(&ldate, 0);
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_CHECKSUM(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_checksum, prepare_null_list(8));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_DD_CHAR_LENGTH(NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_dd_char_length, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_GET_VIEW_WARNING_OR_ERROR(NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_get_view_warning_or_error,
              prepare_null_list(4));
  // null_value is not set in this function. So verifying only val_int() return
  // value.
  EXPECT_EQ(0, item->val_int());

  // INTERNAL_GET_COMMENT_OR_ERROR(NULL, NULL, NULL, NULL, NULL)
  String str;
  CREATE_ITEM(Item_func_internal_get_comment_or_error, prepare_null_list(5));
  item->val_str(&str);
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_INDEX_COLUMN_CARDINALITY(NULL, NULL, NULL, NULL, NULL,
  //                                   NULL, NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_internal_index_column_cardinality,
              prepare_null_list(11));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // GET_DD_INDEX_SUB_PART_LENGTH(NULL, NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_get_dd_index_sub_part_length, prepare_null_list(5));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // GET_DD_COLUMN_PRIVILEGES(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_get_dd_column_privileges, THREE_NULL_ARGS);
  // Empty string value is returned in this case.
  EXPECT_EQ(static_cast<size_t>(0), (item->val_str(&str))->length());

  // INTERNAL_KEYS_DISABLED(NULL)
  CREATE_ITEM(Item_func_internal_keys_disabled, NULL_ARG);
  EXPECT_EQ(0, item->val_int());

  // CAN_ACCESS_DATABASE(NULL)
  CREATE_ITEM(Item_func_can_access_database, NULL_ARG);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_TABLE(NULL, NULL)
  CREATE_ITEM(Item_func_can_access_table, TWO_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_VIEW(NULL, NULL, NULL, NULL)
  CREATE_ITEM(Item_func_can_access_view, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_COLUMN(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_can_access_column, THREE_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_TRIGGER(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_can_access_trigger, TWO_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_ROUTINE(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_can_access_routine, prepare_null_list(5));
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // CAN_ACCESS_EVENT(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_can_access_event, NULL_ARG);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // GET_DD_CREATE_OPTIONS(NULL, NULL, NULL)
  CREATE_ITEM(Item_func_get_dd_create_options, THREE_NULL_ARGS);
  // Empty string value is returned in this case.
  EXPECT_EQ(static_cast<size_t>(0), (item->val_str(&str))->length());

  // INTERNAL_GET_PARTITION_NODEGROUP()
  CREATE_ITEM(Item_func_get_partition_nodegroup, NULL_ARG);
  EXPECT_EQ(0, strcmp((item->val_str(&str))->ptr(), "default"));

  // INTERNAL_TABLESPACE_ID()
  CREATE_ITEM(Item_func_internal_tablespace_id, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_TYPE()
  CREATE_ITEM(Item_func_internal_tablespace_type, FOUR_NULL_ARGS);
  EXPECT_EQ(nullptr, item->val_str(&str));

  // INTERNAL_TABLESPACE_FREE_EXTENTS()
  CREATE_ITEM(Item_func_internal_tablespace_free_extents, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_TOTAL_EXTENTS()
  CREATE_ITEM(Item_func_internal_tablespace_total_extents, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_EXTENT_SIZE()
  CREATE_ITEM(Item_func_internal_tablespace_extent_size, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_INITIAL_SIZE()
  CREATE_ITEM(Item_func_internal_tablespace_initial_size, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_MAXIMUM_SIZE()
  CREATE_ITEM(Item_func_internal_tablespace_maximum_size, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_AUTOEXTEND_SIZE()
  CREATE_ITEM(Item_func_internal_tablespace_autoextend_size, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_DATA_FREE()
  CREATE_ITEM(Item_func_internal_tablespace_data_free, FOUR_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_TABLESPACE_STATUS()
  CREATE_ITEM(Item_func_internal_tablespace_status, FOUR_NULL_ARGS);
  EXPECT_EQ(nullptr, item->val_str(&str));

  // GET_DD_PROPERTY_KEY_VALUE()
  CREATE_ITEM(Item_func_get_dd_property_key_value, TWO_NULL_ARGS);
  EXPECT_EQ(nullptr, item->val_str(&str));

  // REMOVE_DD_PROPERTY_KEY()
  CREATE_ITEM(Item_func_remove_dd_property_key, TWO_NULL_ARGS);
  EXPECT_EQ(nullptr, item->val_str(&str));

  // INTERNAL_GET_DD_COLUMN_EXTRA()
  CREATE_ITEM(Item_func_internal_get_dd_column_extra, prepare_null_list(6));
  item->val_str(&str);
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_GET_USERNAME()
  CREATE_ITEM(Item_func_internal_get_username, prepare_null_list(1));
  EXPECT_EQ(nullptr, item->val_str(&str));

  // INTERNAL_GET_HOSTNAME()
  CREATE_ITEM(Item_func_internal_get_hostname, prepare_null_list(1));
  EXPECT_EQ(nullptr, item->val_str(&str));

  // INTERNAL_IS_MANDATORY_ROLE()
  CREATE_ITEM(Item_func_internal_is_mandatory_role, TWO_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);

  // INTERNAL_IS_ENABLED_ROLE()
  CREATE_ITEM(Item_func_internal_is_enabled_role, TWO_NULL_ARGS);
  item->val_int();
  EXPECT_EQ(1, item->null_value);
}
}  // namespace dd_info_schema_native_func
