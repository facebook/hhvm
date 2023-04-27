/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "sql/item.h"
#include "sql/opt_costmodel.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/test_utils.h"

namespace costconstants_unittest {

using my_testing::Server_initializer;

/*
  Default values for cost constants. These needs to be updated when
  cost constants in opt_costconstans.h are changed.
*/

// Default value for Server_cost_constants::ROW_EVALUATE_COST
const double default_row_evaluate_cost = 0.1;

// Default value for Server_cost_constants::KEY_COMPARE_COST
const double default_key_compare_cost = 0.05;

// Default value for Server_cost_constants::HEAP_TEMPTABLE_CREATE_COST
const double default_memory_temptable_create_cost = 1.0;

// Default value for Server_cost_constants::HEAP_TEMPTABLE_ROW_COST
const double default_memory_temptable_row_cost = 0.1;

// Default value for Server_cost_constants::DISK_TEMPTABLE_CREATE_COST
const double default_disk_temptable_create_cost = 20.0;

// Default value for Server_cost_constants::DISK_TEMPTABLE_ROW_COST
const double default_disk_temptable_row_cost = 0.5;

//  Default value SE_cost_constants::MEMORY_BLOCK_READ_COST
const double default_memory_block_read_cost = 0.25;

//  Default value SE_cost_constants::IO_BLOCK_READ_COST
const double default_io_block_read_cost = 1.0;

class CostConstantsTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    initializer.SetUp();

    // Initilize some storage engines
    LEX_CSTRING engine_name0 = {STRING_WITH_LEN("InnoDB0")};
    LEX_CSTRING engine_name1 = {STRING_WITH_LEN("InnoDB1")};
    LEX_CSTRING engine_name2 = {STRING_WITH_LEN("InnoDB2")};
    LEX_CSTRING engine_name4 = {STRING_WITH_LEN("InnoDB4")};
    LEX_CSTRING engine_name7 = {STRING_WITH_LEN("InnoDB7")};

    insert_hton2plugin(0, new st_plugin_int())->name = engine_name0;
    insert_hton2plugin(1, new st_plugin_int())->name = engine_name1;
    insert_hton2plugin(2, new st_plugin_int())->name = engine_name2;
    insert_hton2plugin(4, new st_plugin_int())->name = engine_name4;
    insert_hton2plugin(7, new st_plugin_int())->name = engine_name7;
  }

  virtual void TearDown() {
    initializer.TearDown();
    delete remove_hton2plugin(0);
    delete remove_hton2plugin(1);
    delete remove_hton2plugin(2);
    delete remove_hton2plugin(4);
    delete remove_hton2plugin(7);
  }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/*
  Class for making it possible to test protected member functions
  of the SE_cost_constants class.
*/
class Testable_SE_cost_constants : public SE_cost_constants {
 public:
  /*
    Wrapper function that allows testing of the protected update() function.
  */
  cost_constant_error test_update_func(const LEX_CSTRING &name,
                                       const double value) {
    return update(name, value);
  }

  /*
    Wrapper function that allows testing of the protected update() function.
  */
  cost_constant_error test_update_default_func(const LEX_CSTRING &name,
                                               const double value) {
    return update_default(name, value);
  }
};

/*
  Storage engines can extend the cost model by adding new cost
  constants.  This is done by making a subclass of the
  SE_cost_constant class.  To make a limited test for this, a
  fictional storage engine called TapeEngine is used. The following
  class adds a new cost constant called TAPE_IO_COST.  Note that this
  should inherit from SE_cost_constants, but to make it easier to write
  a unit test, we inherit from Testable_SE_cost_constants.
*/
class TapeEngine_cost_constants : public Testable_SE_cost_constants {
 public:
  TapeEngine_cost_constants()
      : m_tape_io_cost(200.0), m_tape_io_cost_default(true) {}

  double tape_io_cost() const { return m_tape_io_cost; }

 protected:
  cost_constant_error set(const LEX_CSTRING &name, const double value,
                          bool default_value) {
    // Process TAPE_IO_COST here
    if (!my_strcasecmp(&my_charset_utf8_general_ci, "TAPE_IO_COST", name.str)) {
      update_cost_value(&m_tape_io_cost, &m_tape_io_cost_default, value,
                        default_value);
      return COST_CONSTANT_OK;
    }

    // If the cost constant name was not recognized here, call the parent
    return Testable_SE_cost_constants::set(name, value, default_value);
  }

 private:
  double m_tape_io_cost;
  bool m_tape_io_cost_default;
};

/*
  Class for making it easier to test protected member function of the
  Cost_model_class. To make it possible to call some of the functions
  that relies on storage engine plugins being loaded, without having
  the plugins loaded, this class re-implements a fake version of one
  of the utility functions.
*/
class Testable_Cost_model_constants : public Cost_model_constants {
 public:
  /*
    Wrapper function that allows testing of the protected inc_ref_count()
    function.
  */
  void test_inc_ref_count() { inc_ref_count(); }

  /*
    Wrapper function that allows testing of the protected dec_ref_count()
    function.
  */
  unsigned int test_dec_ref_count() { return dec_ref_count(); }

 private:
  /**
    The implementation of Cost_model_constants relies on using the
    installed plugins for looking up the handler's slot id based on the
    name of the storage engine. For unit tests this does not work since
    no storage engines are installed. We fake this here by overriding the
    function that looks up the handler's slot id.

    This function supports two storage engines, "Karius" installed in slot
    4 and "Baktus" installed in slot 7.
  */

  uint find_handler_slot_from_name(THD *, const LEX_CSTRING &name) const {
    if (my_strcasecmp(&my_charset_utf8_general_ci, "Karius", name.str) == 0)
      return 4;

    if (my_strcasecmp(&my_charset_utf8_general_ci, "Baktus", name.str) == 0)
      return 7;

    // There is no handler for a storage engine with the given name
    return HA_SLOT_UNDEF;
  }
};

/**
  Validates that a cost constant object for server cost constants has
  the expected default values.
*/

static void validate_default_server_cost_constants(
    const Server_cost_constants *cost) {
  EXPECT_EQ(cost->row_evaluate_cost(), default_row_evaluate_cost);
  EXPECT_EQ(cost->key_compare_cost(), default_key_compare_cost);
  EXPECT_EQ(cost->memory_temptable_create_cost(),
            default_memory_temptable_create_cost);
  EXPECT_EQ(cost->memory_temptable_row_cost(),
            default_memory_temptable_row_cost);
  EXPECT_EQ(cost->disk_temptable_create_cost(),
            default_disk_temptable_create_cost);
  EXPECT_EQ(cost->disk_temptable_row_cost(), default_disk_temptable_row_cost);
}

/*
  Test the Server_cost_constants interface.
*/
TEST_F(CostConstantsTest, CostConstantsServer) {
  Server_cost_constants server_constants;

  // Validate expected default values for cost constants
  validate_default_server_cost_constants(&server_constants);

  /*
    Test updating values for cost constants
  */
  const double new_value = 3.14;

  // row_evaluate_cost
  const LEX_CSTRING row_evaluate_name = {STRING_WITH_LEN("ROW_EVALUATE_COST")};
  EXPECT_EQ(server_constants.row_evaluate_cost(), default_row_evaluate_cost);
  EXPECT_EQ(server_constants.set(row_evaluate_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.row_evaluate_cost(), new_value);

  // key_compare_cost
  const LEX_CSTRING key_compare_name = {STRING_WITH_LEN("KEY_COMPARE_COST")};
  EXPECT_EQ(server_constants.key_compare_cost(), default_key_compare_cost);
  EXPECT_EQ(server_constants.set(key_compare_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.key_compare_cost(), new_value);

  // memory_temptable_create_cost
  const LEX_CSTRING memory_temptable_create_name = {
      STRING_WITH_LEN("MEMORY_TEMPTABLE_CREATE_COST")};
  EXPECT_EQ(server_constants.memory_temptable_create_cost(),
            default_memory_temptable_create_cost);
  EXPECT_EQ(server_constants.set(memory_temptable_create_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.memory_temptable_create_cost(), new_value);

  // memory_temptable_row_cost
  const LEX_CSTRING memory_temptable_row_name = {
      STRING_WITH_LEN("MEMORY_TEMPTABLE_ROW_COST")};
  EXPECT_EQ(server_constants.memory_temptable_row_cost(),
            default_memory_temptable_row_cost);
  EXPECT_EQ(server_constants.set(memory_temptable_row_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.memory_temptable_row_cost(), new_value);

  // disk_temptable_create_cost
  const LEX_CSTRING disk_temptable_create_name = {
      STRING_WITH_LEN("DISK_TEMPTABLE_CREATE_COST")};
  EXPECT_EQ(server_constants.disk_temptable_create_cost(),
            default_disk_temptable_create_cost);
  EXPECT_EQ(server_constants.set(disk_temptable_create_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.disk_temptable_create_cost(), new_value);

  // disk_temptable_row_cost
  const LEX_CSTRING disk_temptable_row_name = {
      STRING_WITH_LEN("DISK_TEMPTABLE_ROW_COST")};
  EXPECT_EQ(server_constants.disk_temptable_row_cost(),
            default_disk_temptable_row_cost);
  EXPECT_EQ(server_constants.set(disk_temptable_row_name, new_value),
            COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.disk_temptable_row_cost(), new_value);

  /*
    Test with upper, lower and mixed case for cost constant name.
  */
  const LEX_CSTRING name_upper = {STRING_WITH_LEN("ROW_EVALUATE_COST")};
  const LEX_CSTRING name_lower = {STRING_WITH_LEN("row_evaluate_cost")};
  const LEX_CSTRING name_mixed = {STRING_WITH_LEN("rOw_EvAlUaTe_CoSt")};
  const LEX_CSTRING name_space = {STRING_WITH_LEN("ROW_EVALUATE_COST ")};

  const double value1 = 2.74;
  const double value2 = 3.14;

  EXPECT_EQ(server_constants.set(name_upper, value1), COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);
  EXPECT_EQ(server_constants.set(name_lower, value2), COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value2);
  EXPECT_EQ(server_constants.set(name_mixed, value1), COST_CONSTANT_OK);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);

  EXPECT_EQ(server_constants.set(name_space, value2), UNKNOWN_COST_NAME);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);

  /*
    Test with an unknown cost constant name.
  */
  const LEX_CSTRING unknown_name = {STRING_WITH_LEN("UNKNOWN_COST")};
  EXPECT_EQ(server_constants.set(unknown_name, value1), UNKNOWN_COST_NAME);

  /*
    Test with illegal cost values: negative or zero should not be allowed.
  */
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);
  EXPECT_EQ(server_constants.set(row_evaluate_name, -1.0), INVALID_COST_VALUE);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);
  EXPECT_EQ(server_constants.set(row_evaluate_name, 0.0), INVALID_COST_VALUE);
  EXPECT_EQ(server_constants.row_evaluate_cost(), value1);
}

/*
  Test the SE_cost_constants interface.
*/
TEST_F(CostConstantsTest, CostConstantsStorageEngine) {
  SE_cost_constants se_constants;

  // Validate expected default values for cost constants
  EXPECT_EQ(se_constants.memory_block_read_cost(),
            default_memory_block_read_cost);
  EXPECT_EQ(se_constants.io_block_read_cost(), default_io_block_read_cost);

  /*
    Test updating values for cost constants
  */
  Testable_SE_cost_constants se_constants2;

  const double new_value1 = 2.74;
  const double new_value2 = 3.14;

  /*
    Test memory_block_read_cost
  */
  const LEX_CSTRING memory_block_read_name = {
      STRING_WITH_LEN("MEMORY_BLOCK_READ_COST")};

  // Update the default value, first time
  EXPECT_EQ(se_constants2.test_update_default_func(memory_block_read_name,
                                                   new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value1);

  // Update the default value, second time
  EXPECT_EQ(se_constants2.test_update_default_func(memory_block_read_name,
                                                   new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value2);

  // Update with a engine specific value, first time
  EXPECT_EQ(se_constants2.test_update_func(memory_block_read_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value1);

  // Update with a engine specific value, second time
  EXPECT_EQ(se_constants2.test_update_func(memory_block_read_name, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value2);

  // Update the default value, this should not change the value
  EXPECT_EQ(se_constants2.test_update_default_func(memory_block_read_name,
                                                   new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value2);

  /*
    Test with upper, lower and mixed case for cost constant name.
  */
  const LEX_CSTRING mem_name_upper = {
      STRING_WITH_LEN("MEMORY_BLOCK_READ_COST")};
  const LEX_CSTRING mem_name_lower = {
      STRING_WITH_LEN("memory_block_read_cost")};
  const LEX_CSTRING mem_name_mixed = {
      STRING_WITH_LEN("mEmOrY_bLoCk_ReAd_CoSt")};

  EXPECT_EQ(se_constants2.test_update_func(mem_name_upper, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value1);
  EXPECT_EQ(se_constants2.test_update_func(mem_name_lower, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value2);
  EXPECT_EQ(se_constants2.test_update_func(mem_name_mixed, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.memory_block_read_cost(), new_value1);

  /*
    Test io_block_read_cost
  */
  const LEX_CSTRING io_block_read_name = {
      STRING_WITH_LEN("IO_BLOCK_READ_COST")};

  // Update the default value, first time
  EXPECT_EQ(
      se_constants2.test_update_default_func(io_block_read_name, new_value1),
      COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);

  // Update the default value, second time
  EXPECT_EQ(
      se_constants2.test_update_default_func(io_block_read_name, new_value2),
      COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value2);

  // Update with a engine specific value, first time
  EXPECT_EQ(se_constants2.test_update_func(io_block_read_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);

  // Update with a engine specific value, second time
  EXPECT_EQ(se_constants2.test_update_func(io_block_read_name, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value2);

  // Update the default value, this should not change the value
  EXPECT_EQ(
      se_constants2.test_update_default_func(io_block_read_name, new_value1),
      COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value2);

  /*
    Test with upper, lower and mixed case for cost constant name.
  */
  const LEX_CSTRING io_name_upper = {STRING_WITH_LEN("IO_BLOCK_READ_COST")};
  const LEX_CSTRING io_name_lower = {STRING_WITH_LEN("io_block_read_cost")};
  const LEX_CSTRING io_name_mixed = {STRING_WITH_LEN("iO_bLoCk_ReAd_CoSt")};

  EXPECT_EQ(se_constants2.test_update_func(io_name_upper, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);
  EXPECT_EQ(se_constants2.test_update_func(io_name_lower, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value2);
  EXPECT_EQ(se_constants2.test_update_func(io_name_mixed, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);

  /*
    Test with an unknown cost constant name.
  */
  const LEX_CSTRING unknown_name = {STRING_WITH_LEN("UNKNOWN_COST")};
  EXPECT_EQ(se_constants2.test_update_func(unknown_name, new_value2),
            UNKNOWN_COST_NAME);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);

  /*
    Test with illegal cost values: negative and zero is illegal.
  */
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);
  EXPECT_EQ(se_constants2.test_update_func(io_block_read_name, -1.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);
  EXPECT_EQ(se_constants2.test_update_func(io_block_read_name, 0.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(se_constants2.io_block_read_cost(), new_value1);

  /*
    Test with cost constants for a storage engine that has added a new
    storage engine specific cost constant named TAPE_IO_COST.
  */
  TapeEngine_cost_constants tape_constants;

  // Validate expected default values for cost constants
  EXPECT_EQ(tape_constants.io_block_read_cost(), default_io_block_read_cost);
  EXPECT_EQ(tape_constants.tape_io_cost(), 200.0);

  // change io_block_read_cost
  EXPECT_EQ(tape_constants.io_block_read_cost(), default_io_block_read_cost);
  EXPECT_EQ(tape_constants.test_update_func(io_block_read_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(tape_constants.io_block_read_cost(), new_value1);

  // change tape_io_cost
  const LEX_CSTRING tape_io_name = {STRING_WITH_LEN("TAPE_IO_COST")};
  EXPECT_EQ(tape_constants.tape_io_cost(), 200.0);

  // Change default value for tape_io_cost
  EXPECT_EQ(tape_constants.test_update_default_func(tape_io_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(tape_constants.tape_io_cost(), new_value1);

  // Change engine specific value for tape_io_cost
  EXPECT_EQ(tape_constants.test_update_func(tape_io_name, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(tape_constants.tape_io_cost(), new_value2);

  // Test with an unknown cost constant name.
  EXPECT_EQ(tape_constants.test_update_func(unknown_name, new_value1),
            UNKNOWN_COST_NAME);
}

/*
  Test the Cost model constants interface.
*/
TEST_F(CostConstantsTest, CostConstants) {
  /*
    Test ref counter.
  */
  {
    Testable_Cost_model_constants cost_constants_ref_test;

    cost_constants_ref_test.test_inc_ref_count();
    cost_constants_ref_test.test_inc_ref_count();
    EXPECT_EQ(cost_constants_ref_test.test_dec_ref_count(), 1U);
    EXPECT_EQ(cost_constants_ref_test.test_dec_ref_count(), 0U);
  }

  /*
    Test default server cost constants.
  */
  Cost_model_constants cost_constants;

  const Server_cost_constants *server_const =
      cost_constants.get_server_cost_constants();

  validate_default_server_cost_constants(server_const);

  /*
    Test default table cost constants.
  */
  Fake_TABLE table(1, false);

  const SE_cost_constants *se_const =
      cost_constants.get_se_cost_constants(&table);

  // Validate expected default values for cost constants
  EXPECT_EQ(se_const->memory_block_read_cost(), default_memory_block_read_cost);
  EXPECT_EQ(se_const->io_block_read_cost(), default_io_block_read_cost);

  /*
    Test updating server constants.
  */
  const double new_value1 = 2.74;
  const double new_value2 = 3.14;
  const double new_value3 = 5.00;

  const LEX_CSTRING row_evaluate_name = {STRING_WITH_LEN("ROW_EVALUATE_COST")};
  EXPECT_EQ(
      cost_constants.update_server_cost_constant(row_evaluate_name, new_value1),
      COST_CONSTANT_OK);
  server_const = cost_constants.get_server_cost_constants();
  EXPECT_EQ(server_const->row_evaluate_cost(), new_value1);

  const LEX_CSTRING unknown_name = {STRING_WITH_LEN("UNKNOWN_COST")};
  EXPECT_EQ(
      cost_constants.update_server_cost_constant(unknown_name, new_value2),
      UNKNOWN_COST_NAME);

  // Test with illegal cost constant values
  EXPECT_EQ(server_const->row_evaluate_cost(), new_value1);
  EXPECT_EQ(cost_constants.update_server_cost_constant(row_evaluate_name, -1.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(server_const->row_evaluate_cost(), new_value1);
  EXPECT_EQ(cost_constants.update_server_cost_constant(row_evaluate_name, 0.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(server_const->row_evaluate_cost(), new_value1);

  /*
    Test updating table cost constants.
  */
  const LEX_CSTRING default_name = {STRING_WITH_LEN("default")};
  const LEX_CSTRING io_block_read_name = {
      STRING_WITH_LEN("IO_BLOCK_READ_COST")};
  EXPECT_EQ(cost_constants.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, new_value1),
            COST_CONSTANT_OK);

  // Verify that the cost constant is updated
  se_const = cost_constants.get_se_cost_constants(&table);
  EXPECT_EQ(se_const->io_block_read_cost(), new_value1);

  /*
    Do a second update of the same default value.
  */
  EXPECT_EQ(cost_constants.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, new_value2),
            COST_CONSTANT_OK);

  // Verify that the cost constant is updated
  se_const = cost_constants.get_se_cost_constants(&table);
  EXPECT_EQ(se_const->io_block_read_cost(), new_value2);

  /*
    Test with illegal cost constant values.
  */
  EXPECT_EQ(cost_constants.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, -1.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(se_const->io_block_read_cost(), new_value2);
  EXPECT_EQ(cost_constants.update_engine_cost_constant(nullptr, default_name, 0,
                                                       io_block_read_name, 0.0),
            INVALID_COST_VALUE);
  EXPECT_EQ(se_const->io_block_read_cost(), new_value2);

  /*
    Test updating with a none existing cost constant.
  */
  const LEX_CSTRING lunch_cost_name = {STRING_WITH_LEN("LUNCH_COST")};
  EXPECT_EQ(cost_constants.update_engine_cost_constant(
                nullptr, default_name, 0, lunch_cost_name, new_value1),
            UNKNOWN_COST_NAME);

  /*
    Test handling of illegal storage category number.
  */
  EXPECT_EQ(cost_constants.update_engine_cost_constant(
                nullptr, default_name, 100, io_block_read_name, new_value1),
            INVALID_DEVICE_TYPE);

  // Verify that the cost constant is not updated
  se_const = cost_constants.get_se_cost_constants(&table);
  EXPECT_EQ(se_const->io_block_read_cost(), new_value2);

  /*
    Create two table objects that are stored in different storage engines.
  */
  Fake_TABLE table_se1(1, false);
  table_se1.file->ht->slot = 1;
  Fake_TABLE table_se2(1, false);
  table_se2.file->ht->slot = 2;

  Cost_model_constants cost_constants2;

  const SE_cost_constants *se_cost1 =
      cost_constants2.get_se_cost_constants(&table_se1);
  const SE_cost_constants *se_cost2 =
      cost_constants2.get_se_cost_constants(&table_se2);

  // Verify that both tables have the same default value
  EXPECT_EQ(se_cost1->io_block_read_cost(), default_io_block_read_cost);
  EXPECT_EQ(se_cost2->io_block_read_cost(), default_io_block_read_cost);

  // Update the default value and verify that the cost constant for both
  // tables are changed.
  EXPECT_EQ(cost_constants2.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_cost1->io_block_read_cost(), new_value1);
  EXPECT_EQ(se_cost2->io_block_read_cost(), new_value1);

  /*
    Create a cost model constants set that has cost constants for the
    two storage engines named "Karius" and "Baktus". We use this instead
    of the real Cost_model_constant class since that relies on that the
    plugins for storage engines have been initialized.
  */
  Testable_Cost_model_constants cost_constants3;

  const LEX_CSTRING karius_name = {STRING_WITH_LEN("Karius")};

  // Create two tables that use these storage engines
  Fake_TABLE table_karius(1, false);
  table_karius.file->ht->slot = 4;  // Karius is in ht->slot 4
  Fake_TABLE table_baktus(1, false);
  table_baktus.file->ht->slot = 7;  // Baktus is in ht->slot 7

  // Get cost constants to use for these tables
  const SE_cost_constants *se_cost_karius =
      cost_constants3.get_se_cost_constants(&table_karius);
  const SE_cost_constants *se_cost_baktus =
      cost_constants3.get_se_cost_constants(&table_baktus);

  // Verify that both tables have the same default value
  EXPECT_EQ(se_cost_karius->io_block_read_cost(), default_io_block_read_cost);
  EXPECT_EQ(se_cost_baktus->io_block_read_cost(), default_io_block_read_cost);

  // Update the default value and verify that the cost constant for both
  // tables are changed.
  EXPECT_EQ(cost_constants3.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, new_value1),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_cost_karius->io_block_read_cost(), new_value1);
  EXPECT_EQ(se_cost_baktus->io_block_read_cost(), new_value1);

  // Update one of the storage engines with a new cost value and verify that
  // only this engine got the new cost value
  EXPECT_EQ(cost_constants3.update_engine_cost_constant(
                nullptr, karius_name, 0, io_block_read_name, new_value2),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_cost_karius->io_block_read_cost(), new_value2);
  EXPECT_EQ(se_cost_baktus->io_block_read_cost(), new_value1);

  // Do a new update of the default value and verify that only the storage
  // engine that still is using the default value is changed.
  EXPECT_EQ(cost_constants3.update_engine_cost_constant(
                nullptr, default_name, 0, io_block_read_name, new_value3),
            COST_CONSTANT_OK);
  EXPECT_EQ(se_cost_karius->io_block_read_cost(), new_value2);
  EXPECT_EQ(se_cost_baktus->io_block_read_cost(), new_value3);
}

}  // namespace costconstants_unittest
