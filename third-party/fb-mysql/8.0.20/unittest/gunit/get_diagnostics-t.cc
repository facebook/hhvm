/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/sql_class.h"
#include "sql/sql_get_diagnostics.h"
#include "unittest/gunit/test_utils.h"

namespace get_diagnostics_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class GetDiagnosticsTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

class FailHelper {
 public:
  void fail(const char *message) { FAIL() << message; }
};

LEX_CSTRING var_name1 = {STRING_WITH_LEN("var1")};
LEX_CSTRING var_name2 = {STRING_WITH_LEN("var2")};

class MockDiagInfoItem : public Diagnostics_information_item {
 public:
  MockDiagInfoItem(Item *target, int value)
      : Diagnostics_information_item(target), m_value(value) {}

  Item *get_value(THD *thd, const Diagnostics_area *) {
    return new (thd->mem_root) Item_int(m_value);
  }

 private:
  int m_value;
};

class MockDiagInfo : public Diagnostics_information, private FailHelper {
 public:
  MockDiagInfo(List<MockDiagInfoItem> *items) : m_items(items) {}

 protected:
  bool aggregate(THD *thd, const Diagnostics_area *da) {
    bool rv = false;
    MockDiagInfoItem *diag_info_item;
    List_iterator<MockDiagInfoItem> it(*m_items);

    while ((diag_info_item = it++)) {
      if ((rv = evaluate(thd, diag_info_item, da))) break;
    }

    return rv;
  }

  ~MockDiagInfo() { fail("MockDiagInfo destructor invoked."); }

 private:
  List<MockDiagInfoItem> *m_items;
};

// GET [CURRENT] DIAGNOSTICS @var1 = 1, @var2 = 2;
TEST_F(GetDiagnosticsTest, Cmd) {
  Item *var;
  Sql_cmd *cmd;
  MockDiagInfo *info;
  MockDiagInfoItem *diag_info_item;
  List<MockDiagInfoItem> items;
  MEM_ROOT *mem_root = thd()->mem_root;

  // set var1 item
  var = new (mem_root) Item_func_get_user_var(var_name1);
  diag_info_item = new (mem_root) MockDiagInfoItem(var, 1);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // set var2 item
  var = new (mem_root) Item_func_get_user_var(var_name2);
  diag_info_item = new (mem_root) MockDiagInfoItem(var, 2);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // Information list and command
  info = new (mem_root) MockDiagInfo(&items);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  EXPECT_FALSE(cmd->execute(thd()));
  EXPECT_TRUE(thd()->get_stmt_da()->is_ok());

  // check var1 value
  var = new (mem_root) Item_func_get_user_var(var_name1);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(1, var->val_int());

  // check var2 value
  var = new (mem_root) Item_func_get_user_var(var_name2);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(2, var->val_int());
}

// Verifies death with a DBUG_ASSERT if target item is not settable.
// Google Test recommends DeathTest suffix for classes used in death tests.
typedef GetDiagnosticsTest GetDiagnosticsTestDeathTest;

#if GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)
TEST_F(GetDiagnosticsTestDeathTest, DieWhenUnsettableItem) {
  Item *var;
  Sql_cmd *cmd;
  MockDiagInfo *info;
  MockDiagInfoItem *diag_info_item;
  List<MockDiagInfoItem> items;
  MEM_ROOT *mem_root = thd()->mem_root;

  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // Unsettable item
  var = new (mem_root) Item_int(1);
  diag_info_item = new (mem_root) MockDiagInfoItem(var, 1);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // Information list and command
  info = new (mem_root) MockDiagInfo(&items);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  EXPECT_DEATH(cmd->execute(thd()), ".*Assertion.*srp.*");
}
#endif  // GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)

class MockDiagInfoError : public Diagnostics_information {
 public:
  MockDiagInfoError(bool fatal_error) : m_fatal_error(fatal_error) {}

 protected:
  bool aggregate(THD *thd, const Diagnostics_area *) {
    myf flag = m_fatal_error ? MYF(ME_FATALERROR) : MYF(0);
    my_message_sql(ER_UNKNOWN_ERROR, "Unknown error", flag);
    return thd->is_error();
  }

 private:
  bool m_fatal_error;
};

// GET DIAGNOSTICS itself causes an error.
TEST_F(GetDiagnosticsTest, Error) {
  Sql_cmd *cmd;
  MockDiagInfoError *info;
  MEM_ROOT *mem_root = thd()->mem_root;

  // Pre-existing warning
  push_warning_printf(thd(), Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED,
                      "Data truncated");

  // Simulate GET DIAGNOSTICS as a new command separated
  // from the one that generated the warning
  thd()->reset_for_next_command();

  // Error bound "information" and command
  info = new (mem_root) MockDiagInfoError(false);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  initializer.set_expected_error(ER_UNKNOWN_ERROR);

  // Should succeed, not a fatal error
  EXPECT_FALSE(cmd->execute(thd()));
  EXPECT_TRUE(thd()->get_stmt_da()->is_ok());

  // New condition for the error
  EXPECT_EQ(1U, thd()->get_stmt_da()->last_statement_cond_count());

  // Counted as a error
  EXPECT_EQ(1U, thd()->get_stmt_da()->error_count(thd()));

  // Error is appended
  EXPECT_EQ(2U, thd()->get_stmt_da()->warn_count(thd()));
}

// GET DIAGNOSTICS itself causes a fatal error.
TEST_F(GetDiagnosticsTest, FatalError) {
  Sql_cmd *cmd;
  MockDiagInfoError *info;
  MEM_ROOT *mem_root = thd()->mem_root;

  // Pre-existing warning
  push_warning_printf(thd(), Sql_condition::SL_WARNING, WARN_DATA_TRUNCATED,
                      "Data truncated");

  // Simulate GET DIAGNOSTICS as a new command separated
  // from the one that generated the warning
  thd()->reset_for_next_command();

  // Error bound "information" and command
  info = new (mem_root) MockDiagInfoError(true);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  initializer.set_expected_error(ER_UNKNOWN_ERROR);

  // Should not succeed due to a fatal error
  EXPECT_TRUE(cmd->execute(thd()));
  EXPECT_TRUE(thd()->get_stmt_da()->is_error());

  // No new condition for the error
  EXPECT_EQ(0U, thd()->get_stmt_da()->error_count(thd()));

  // Fatal error is set, not appended
  EXPECT_EQ(1U, thd()->get_stmt_da()->warn_count(thd()));
}

// GET [CURRENT] DIAGNOSTICS @var1 = NUMBER, @var2 = ROW_COUNT;
TEST_F(GetDiagnosticsTest, StatementInformation) {
  Item *var;
  Sql_cmd *cmd;
  Statement_information *info;
  Statement_information_item *diag_info_item;
  List<Statement_information_item> items;
  MEM_ROOT *mem_root = thd()->mem_root;

  // NUMBER = 1 warning
  thd()->raise_warning(ER_UNKNOWN_ERROR);
  // ROW_COUNT = 5
  thd()->set_row_count_func(5U);

  // var1 will receive the value of NUMBER
  var = new (mem_root) Item_func_get_user_var(var_name1);
  diag_info_item = new (mem_root)
      Statement_information_item(Statement_information_item::NUMBER, var);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // var2 will receive the value of ROW_COUNT
  var = new (mem_root) Item_func_get_user_var(var_name2);
  diag_info_item = new (mem_root)
      Statement_information_item(Statement_information_item::ROW_COUNT, var);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // Information list and command
  info = new (mem_root) Statement_information(&items);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  EXPECT_FALSE(cmd->execute(thd()));
  EXPECT_TRUE(thd()->get_stmt_da()->is_ok());

  // check var1 value
  var = new (mem_root) Item_func_get_user_var(var_name1);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(1U, var->val_uint());

  // check var2 value
  var = new (mem_root) Item_func_get_user_var(var_name2);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(5U, var->val_int());
}

// GET DIAGNOSTICS CONDITION 1 @var1 = MYSQL_ERRNO, @var2 = MESSAGE_TEXT;
TEST_F(GetDiagnosticsTest, ConditionInformation) {
  Item *var;
  String str;
  Sql_cmd *cmd;
  Condition_information *info;
  Condition_information_item *diag_info_item;
  List<Condition_information_item> items;
  MEM_ROOT *mem_root = thd()->mem_root;

  // Pre-existing error
  my_message_sql(ER_UNKNOWN_ERROR, "Unknown error", MYF(0));

  // Simulate GET DIAGNOSTICS as a new command separated
  // from the one that generated the error
  thd()->reset_for_next_command();

  // var1 will receive the value of MYSQL_ERRNO
  var = new (mem_root) Item_func_get_user_var(var_name1);
  diag_info_item = new (mem_root)
      Condition_information_item(Condition_information_item::MYSQL_ERRNO, var);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // var2 will receive the value of MESSAGE_TEXT
  var = new (mem_root) Item_func_get_user_var(var_name2);
  diag_info_item = new (mem_root)
      Condition_information_item(Condition_information_item::MESSAGE_TEXT, var);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // condition number (1)
  var = new (mem_root) Item_uint(1);

  // Information list and command
  info = new (mem_root) Condition_information(var, &items);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  EXPECT_FALSE(cmd->execute(thd()));
  EXPECT_TRUE(thd()->get_stmt_da()->is_ok());

  // check var1 value
  var = new (mem_root) Item_func_get_user_var(var_name1);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(ulonglong(ER_UNKNOWN_ERROR), var->val_uint());

  // check var2 value
  var = new (mem_root) Item_func_get_user_var(var_name2);
  EXPECT_FALSE(var->fix_fields(thd(), &var));
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("Unknown error", str.c_ptr_safe());
}

static Item *get_cond_info_item(THD *thd, uint number,
                                Condition_information_item::Name name) {
  Item *var;
  Sql_cmd *cmd;
  Condition_information *info;
  Condition_information_item *diag_info_item;
  List<Condition_information_item> items;
  MEM_ROOT *mem_root = thd->mem_root;
  LEX_CSTRING var_name = {STRING_WITH_LEN("get_cond_info_item")};

  // Simulate GET DIAGNOSTICS as a new command
  thd->reset_for_next_command();

  // var1 will receive the value of MYSQL_ERRNO
  var = new (mem_root) Item_func_get_user_var(var_name);
  diag_info_item = new (mem_root) Condition_information_item(name, var);
  EXPECT_FALSE(items.push_back(diag_info_item));

  // condition number
  var = new (mem_root) Item_uint(number);

  // Information list and command
  info = new (mem_root) Condition_information(var, &items);
  info->set_which_da(Diagnostics_information::CURRENT_AREA);
  cmd = new (mem_root) Sql_cmd_get_diagnostics(info);

  EXPECT_FALSE(cmd->execute(thd));
  EXPECT_TRUE(thd->get_stmt_da()->is_ok());

  // make a user var item
  var = new (mem_root) Item_func_get_user_var(var_name);
  EXPECT_FALSE(var->fix_fields(thd, &var));

  return var;
}

// GET DIAGNOSTICS CONDITION 1 @var = CLASS_ORIGIN;
// GET DIAGNOSTICS CONDITION 1 @var = SUBCLASS_ORIGIN;
TEST_F(GetDiagnosticsTest, ConditionInformationClassOrigin) {
  Item *var;
  String str;

  // "MySQL" origin
  push_warning_printf(thd(), Sql_condition::SL_WARNING, ER_XAER_NOTA,
                      "Unknown XID");

  // "ISO 9075" origin
  push_warning_printf(thd(), Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                      "Unknown error");

  // Condition 1 CLASS_ORIGIN
  var = get_cond_info_item(thd(), 1, Condition_information_item::CLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("MySQL", str.c_ptr_safe());

  // Condition 1 SUBCLASS_ORIGIN
  var =
      get_cond_info_item(thd(), 1, Condition_information_item::SUBCLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("MySQL", str.c_ptr_safe());

  // Condition 2 CLASS_ORIGIN
  var = get_cond_info_item(thd(), 2, Condition_information_item::CLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("ISO 9075", str.c_ptr_safe());

  // Condition 2 CLASS_ORIGIN
  var =
      get_cond_info_item(thd(), 2, Condition_information_item::SUBCLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("ISO 9075", str.c_ptr_safe());
}

// Push + pop diagnostics area
TEST_F(GetDiagnosticsTest, PushPopDiagnosticsArea) {
  Diagnostics_area da1(false);
  Diagnostics_area da2(false);
  Diagnostics_area *org_da = thd()->get_stmt_da();

  thd()->push_diagnostics_area(&da1);
  EXPECT_EQ(&da1, thd()->get_stmt_da());
  EXPECT_EQ(org_da, thd()->get_stacked_da());

  thd()->push_diagnostics_area(&da2);
  EXPECT_EQ(&da2, thd()->get_stmt_da());
  EXPECT_EQ(&da1, thd()->get_stacked_da());

  thd()->pop_diagnostics_area();
  EXPECT_EQ(&da1, thd()->get_stmt_da());
  EXPECT_EQ(org_da, thd()->get_stacked_da());

  thd()->pop_diagnostics_area();
  EXPECT_EQ(org_da, thd()->get_stmt_da());
  EXPECT_TRUE(thd()->get_stacked_da() == nullptr);
}

// Pop when there is just one diagnostics area = assert
#if GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)
TEST_F(GetDiagnosticsTestDeathTest, DiePopDiagnosticsArea) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(thd()->pop_diagnostics_area(), ".*Assertion.*m_stacked_da*");
}
#endif  // GTEST_HAS_DEATH_TEST && !defined(DBUG_OFF)

// Pushed diagnostics area should initially contain copy of conditions
TEST_F(GetDiagnosticsTest, PushDiagnosticsArea) {
  Diagnostics_area da(false);
  Diagnostics_area *org_da = thd()->get_stmt_da();

  Item *var;
  String str;

  // "MySQL" origin
  push_warning_printf(thd(), Sql_condition::SL_WARNING, ER_XAER_NOTA,
                      "Unknown XID");

  // "ISO 9075" origin
  push_warning_printf(thd(), Sql_condition::SL_WARNING, ER_UNKNOWN_ERROR,
                      "Unknown error");

  // Push new diagnostics area, clear old
  thd()->push_diagnostics_area(&da);
  org_da->reset_condition_info(thd());
  EXPECT_TRUE(org_da->cond_count() == 0);
  EXPECT_FALSE(da.cond_count() == 0);
  EXPECT_FALSE(thd()->get_stmt_da()->cond_count() == 0);

  // Condition 1 CLASS_ORIGIN
  var = get_cond_info_item(thd(), 1, Condition_information_item::CLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("MySQL", str.c_ptr_safe());

  // Condition 1 SUBCLASS_ORIGIN
  var =
      get_cond_info_item(thd(), 1, Condition_information_item::SUBCLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("MySQL", str.c_ptr_safe());

  // Condition 2 CLASS_ORIGIN
  var = get_cond_info_item(thd(), 2, Condition_information_item::CLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("ISO 9075", str.c_ptr_safe());

  // Condition 2 CLASS_ORIGIN
  var =
      get_cond_info_item(thd(), 2, Condition_information_item::SUBCLASS_ORIGIN);
  EXPECT_EQ(&str, var->val_str(&str));
  EXPECT_STREQ("ISO 9075", str.c_ptr_safe());

  thd()->pop_diagnostics_area();
  EXPECT_TRUE(thd()->get_stacked_da() == nullptr);
  EXPECT_TRUE(thd()->get_stmt_da()->cond_count() == 0);
}

}  // namespace get_diagnostics_unittest
