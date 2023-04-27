/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "unittest/gunit/handler-t.h"

#include <stddef.h>
#include <sys/types.h>

#include "sql/sql_executor.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mock_field_datetime.h"
#include "unittest/gunit/test_utils.h"

namespace {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

using ::testing::StrictMock;

class HandlerTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/**
  Some handler error returns are passed on to report_handler_error()
  which will:
    - ignore errors like END_OF_FILE
    - print most errors to the error log
    - pass the error code back to handler::print_error()
 */
TEST_F(HandlerTest, ReportErrorHandler) {
  Mock_field_datetime field_datetime;
  Fake_TABLE *table = static_cast<Fake_TABLE *>(field_datetime.table);
  handlerton *hton = nullptr;
  StrictMock<Mock_HANDLER> mock_handler(hton, table->get_share());
  table->set_handler(&mock_handler);

  // This error should be ignored.
  EXPECT_EQ(-1, report_handler_error(table, HA_ERR_END_OF_FILE));

  // This one should not be printed to stderr, but passed on to the handler.
  EXPECT_CALL(mock_handler, print_error(HA_ERR_TABLE_DEF_CHANGED, 0)).Times(1);
  EXPECT_EQ(1, report_handler_error(table, HA_ERR_TABLE_DEF_CHANGED));
}

TEST_F(HandlerTest, TableInMemoryEstimate) {
  Mock_field_datetime field_datetime;
  Fake_TABLE *table = static_cast<Fake_TABLE *>(field_datetime.table);
  handlerton *hton = nullptr;
  StrictMock<Mock_HANDLER> mock_handler(hton, table->get_share());
  table->set_handler(&mock_handler);

  // Verify that the handler does not know the buffer size
  EXPECT_EQ(mock_handler.get_memory_buffer_size(), -1);
  /*
    The implementation of table_in_memory_estimate() assumes that the
    memory buffer is 100 MB if the storage engine does not report the
    size of its memory buffer.
  */
  const uint mem_buf_size = 100 * 1024 * 1024;

  /*
    Define representative table sizes to use in tests.
  */
  // Table that is less than 20% of memory buffer
  const uint table_size_small = static_cast<uint>(mem_buf_size * 0.19);
  // Table that is larger than 20% but less than 100% of memory buffer
  const uint table_size_medium = mem_buf_size / 2;
  // Table that is larger than memory buffer
  const uint table_size_large = mem_buf_size * 2;

  /*
    Verify that the default table in memory estimate for a handler has been
    correctly initialized.
  */
  EXPECT_EQ(mock_handler.stats.table_in_mem_estimate,
            IN_MEMORY_ESTIMATE_UNKNOWN);

  /*
    Test with a table that is less than 20% of memory buffer. This should
    be entirely in the memory buffer.
  */
  mock_handler.stats.data_file_length = table_size_small;
  EXPECT_EQ(mock_handler.table_in_memory_estimate(), 1.0);

  /*
    Test with a medium sized table that is more than 20% but less than
    100% of the memory buffer size.
  */
  mock_handler.stats.data_file_length = table_size_medium;
  EXPECT_GT(mock_handler.table_in_memory_estimate(), 0.0);
  EXPECT_LT(mock_handler.table_in_memory_estimate(), 1.0);

  /*
    Test with a huge table. This should not be in memory at all.
  */
  mock_handler.stats.data_file_length = table_size_large;
  EXPECT_EQ(mock_handler.table_in_memory_estimate(), 0.0);

  /*
    Simulate that the storage engine has reported that 50 percent of the
    table is in a memory buffer.
  */
  mock_handler.stats.table_in_mem_estimate = 0.5;

  /*
    Set the table size to be less than 20 percent but larger than 10K.
  */
  mock_handler.stats.data_file_length = table_size_small;
  EXPECT_DOUBLE_EQ(mock_handler.table_in_memory_estimate(), 0.5);

  /*
    Set the table size to be larger than 20 percent but less than 100 percent.
  */
  mock_handler.stats.data_file_length = table_size_medium;
  EXPECT_DOUBLE_EQ(mock_handler.table_in_memory_estimate(), 0.5);

  /*
    Set the table size to be larger than the memory buffer.
  */
  mock_handler.stats.data_file_length = table_size_large;
  EXPECT_DOUBLE_EQ(mock_handler.table_in_memory_estimate(), 0.5);
}

TEST_F(HandlerTest, IndexInMemoryEstimate) {
  Mock_field_datetime field_datetime;
  Fake_TABLE *table = static_cast<Fake_TABLE *>(field_datetime.table);
  handlerton *hton = nullptr;
  StrictMock<Mock_HANDLER> mock_handler(hton, table->get_share());
  table->set_handler(&mock_handler);
  mock_handler.change_table_ptr(table, table->get_share());
  const uint key_no = 0;

  // Verify that the handler does not know the buffer size
  EXPECT_EQ(mock_handler.get_memory_buffer_size(), -1);
  /*
    The implementation of index_in_memory_estimate() assumes that the
    memory buffer is 100 MB if the storage engine does not report the
    size of its memory buffer.
  */
  const uint mem_buf_size = 100 * 1024 * 1024;

  /*
    Define representative table and index sizes to use in tests.
  */
  // Index that is less than 20% of memory buffer
  const uint index_size_small = static_cast<uint>(mem_buf_size * 0.19);
  // Index that is larger than 20% but less than 100% of memory buffer
  const uint index_size_medium = mem_buf_size / 2;
  // Index that is larger than memory buffer
  const uint index_size_large = mem_buf_size * 2;

  // Initialize the estimate for how much of the index that is in memory
  table->key_info[key_no].set_in_memory_estimate(IN_MEMORY_ESTIMATE_UNKNOWN);

  /*
    Test with an index that is less than 20% of memory buffer. This should
    be entirely in the memory buffer.
  */
  mock_handler.stats.index_file_length = index_size_small;
  EXPECT_EQ(mock_handler.index_in_memory_estimate(key_no), 1.0);

  /*
    Test with a medium sized index that is more than 20% but less than
    100% of the memory buffer size.
  */
  mock_handler.stats.index_file_length = index_size_medium;
  EXPECT_GT(mock_handler.index_in_memory_estimate(key_no), 0.0);
  EXPECT_LT(mock_handler.index_in_memory_estimate(key_no), 1.0);

  /*
    Test with a huge index. This should not be in memory at all.
  */
  mock_handler.stats.index_file_length = index_size_large;
  EXPECT_EQ(mock_handler.index_in_memory_estimate(key_no), 0.0);

  /*
    Simulate that the storage engine has reported that 50 percent of the
    index is in a memory buffer.
  */
  table->key_info[key_no].set_in_memory_estimate(0.5);

  /*
    Set the index size to be less than 20 percent but larger than 10K.
  */
  mock_handler.stats.index_file_length = index_size_small;
  EXPECT_DOUBLE_EQ(mock_handler.index_in_memory_estimate(key_no), 0.5);

  /*
    Set the index size to be larger than 20 percent but less than 100 percent.
  */
  mock_handler.stats.index_file_length = index_size_medium;
  EXPECT_DOUBLE_EQ(mock_handler.index_in_memory_estimate(key_no), 0.5);

  /*
    Set the index size to be larger than the memory buffer.
  */
  mock_handler.stats.index_file_length = index_size_large;
  EXPECT_DOUBLE_EQ(mock_handler.index_in_memory_estimate(key_no), 0.5);
}

TEST_F(HandlerTest, SamplingInterfaceAllRows) {
  Mock_field_datetime field_datetime;
  Fake_TABLE *table = static_cast<Fake_TABLE *>(field_datetime.table);
  StrictMock<Mock_SAMPLING_HANDLER> mock_handler(nullptr, table,
                                                 table->get_share());
  table->set_handler(&mock_handler);

  uchar buffer[8];
  void *scan_ctx = nullptr;

  // rnd_init should be called exactly one time by ha_sample_init.
  EXPECT_CALL(mock_handler, rnd_init(true)).Times(1);
  EXPECT_EQ(mock_handler.ha_sample_init(scan_ctx, 100.0, 0,
                                        enum_sampling_method::SYSTEM),
            0);
  EXPECT_EQ(mock_handler.inited, handler::SAMPLING);

  /*
    Since we have set the sampling rate to 100%, all rows should be returned.
    Thus, rnd_next should be called exactly as many times as ha_sample_next().
  */
  const int num_iterations = 100;
  EXPECT_CALL(mock_handler, rnd_next(buffer)).Times(num_iterations);

  for (int i = 0; i < num_iterations; ++i)
    mock_handler.ha_sample_next(scan_ctx, buffer);

  // rnd_end should be called exactly one time by ha_sample_end.
  EXPECT_CALL(mock_handler, rnd_end()).Times(1);
  EXPECT_EQ(mock_handler.ha_sample_end(scan_ctx), 0);
  EXPECT_EQ(mock_handler.inited, handler::NONE);
}

TEST_F(HandlerTest, SamplingInterfaceNoRows) {
  Mock_field_datetime field_datetime;
  Fake_TABLE *table = reinterpret_cast<Fake_TABLE *>(field_datetime.table);
  StrictMock<Mock_SAMPLING_HANDLER> mock_handler(nullptr, table,
                                                 table->get_share());
  table->set_handler(&mock_handler);

  uchar buffer[8];
  void *scan_ctx = nullptr;

  // rnd_init should be called exactly one time by ha_sample_init.
  EXPECT_CALL(mock_handler, rnd_init(true)).Times(1);
  EXPECT_EQ(mock_handler.ha_sample_init(scan_ctx, 0.0, 0,
                                        enum_sampling_method::SYSTEM),
            0);
  EXPECT_EQ(mock_handler.inited, handler::SAMPLING);

  /*
    Since we have set the sampling rate to 0%, no rows should be returned. Thus,
    rnd_next should never be called.
  */
  EXPECT_CALL(mock_handler, rnd_next(buffer)).Times(0);

  for (int i = 0; i < 100; ++i) mock_handler.ha_sample_next(scan_ctx, buffer);

  // rnd_end should be called exactly one time by ha_sample_end.
  EXPECT_CALL(mock_handler, rnd_end()).Times(1);
  EXPECT_EQ(mock_handler.ha_sample_end(scan_ctx), 0);
  EXPECT_EQ(mock_handler.inited, handler::NONE);
}

}  // namespace
