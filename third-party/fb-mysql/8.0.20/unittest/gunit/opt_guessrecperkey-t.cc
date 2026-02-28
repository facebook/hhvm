/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>

#include "sql/key.h"                    // KEY
#include "sql/opt_statistics.h"         // guess_rec_per_key
#include "unittest/gunit/fake_key.h"    // Fake_KEY
#include "unittest/gunit/fake_table.h"  // Fake_TABLE
#include "unittest/gunit/test_utils.h"

namespace guessrecperkey_unittest {

using my_testing::Server_initializer;

class GuessRecPerKeyTest : public ::testing::Test {
  virtual void SetUp() { initializer.SetUp(); }

  virtual void TearDown() { initializer.TearDown(); }

 private:
  Server_initializer initializer;
};

TEST_F(GuessRecPerKeyTest, GuessRecPerKeyMultiColumn) {
  const uint key_parts = 3;

  Fake_TABLE table(key_parts, false);
  Fake_KEY key(key_parts, false);
  Fake_KEY unique_key(key_parts, true);

  // Table is empty, records per key estimate should be 1
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &key, 2), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &key, 3), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 2), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 3), 1.0f);

  /*
    Test with a large table and a non-unique key.
  */

  // Increase the size of the table
  const ha_rows large_table_size = 10000;
  table.file->stats.records = large_table_size;

  // Rec per key for first key part should be 1 percent of table size
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 1),
                  rec_per_key_t(large_table_size / 100));

  // Rec per key for last key part should be 10 for a non-unique key
  EXPECT_EQ(guess_rec_per_key(&table, &key, 3), 10.0f);

  // Rec per key for the second key part should be somewhere between first
  // and last key part
  rec_per_key_t rec_per_key = guess_rec_per_key(&table, &key, 2);
  EXPECT_LT(rec_per_key, rec_per_key_t(large_table_size / 100));
  EXPECT_GT(rec_per_key, 10.0f);

  /*
    Test with a large table and a unique key.
  */

  // Rec per key for first key part should be 1 percent of table size
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &unique_key, 1),
                  rec_per_key_t(large_table_size / 100));

  // Rec per key for last key part should be 1 for a unique key
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 3), 1.0f);

  // Rec per key for the second key part should be somewhere between first
  // and last key part
  rec_per_key = guess_rec_per_key(&table, &unique_key, 2);
  EXPECT_LT(rec_per_key, rec_per_key_t(large_table_size / 100));
  EXPECT_GT(rec_per_key, 1.0f);

  /*
    Test with a small table and a non-unique key.
  */

  // Set the size of the table
  const ha_rows small_table_size = 150;
  table.file->stats.records = small_table_size;

  // Rec per key for first key part should be 1 percent of table size, but
  // not smaller than rec per key for the last key
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 10.0f);

  // Rec per key for last key part should be 10 for a non-unique key
  EXPECT_EQ(guess_rec_per_key(&table, &key, 3), 10.0f);

  // Rec per key for the second key part should be somewhere between first
  // and last key part, but in this case they are identical
  EXPECT_EQ(guess_rec_per_key(&table, &key, 2), 10.0f);

  /*
    Test with a small table and a unique key.
  */

  // Rec per key for first key part should be 1 percent of table size
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &unique_key, 1),
                  rec_per_key_t(small_table_size) / 100);

  // Rec per key for last key part should be 1 for a unique key
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 3), 1.0f);

  // Rec per key for the second key part should be somewhere between first
  // and last key part
  rec_per_key = guess_rec_per_key(&table, &unique_key, 2);
  EXPECT_LT(rec_per_key, rec_per_key_t(small_table_size) / 100);
  EXPECT_GT(rec_per_key, 1.0f);

  /*
    Test with a tiny table and a non-unique key.
  */

  // Set the size of the table
  const ha_rows tiny_table_size = 30;
  table.file->stats.records = tiny_table_size;

  // Rec per key for first key part should be 1 percent of table size, but
  // not smaller than rec per key for the last key
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 1),
                  rec_per_key_t(tiny_table_size) / 10);

  // Rec per key for last key part should be 10 for a non-unique key, but
  // there should be at least then unique values in the table
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 3),
                  rec_per_key_t(tiny_table_size) / 10);

  // Rec per key for the second key part should be somewhere between first
  // and last key part, but in this case they are identical
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 2),
                  rec_per_key_t(tiny_table_size) / 10);

  /*
    Test with a tiny table and a unique key.
  */

  // Rec per key for first key part should be 1 percent of table size, but
  // not smaller than rec per key for the last key
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);

  // Rec per key for last key part should be 1 for a unique key
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 3), 1.0f);

  // Rec per key for the second key part should be somewhere between first
  // and last key part, but in this case they are indentical
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 2), 1.0f);

  /*
    Test that setting rec per key value for the last key part will be
    taken into account.
  */
  key.rec_per_key[key_parts - 1] = 2;

  // This is a tiny table so all rec per keys estimates should be
  // identical to the estimate for the last key
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 2.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &key, 2), 2.0f);

  // This even is the case for a unique key
  unique_key.rec_per_key[key_parts - 1] = 2;
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 2.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 2), 2.0f);
}

TEST_F(GuessRecPerKeyTest, GuessRecPerKeySingleColumn) {
  const uint key_parts = 1;

  Fake_TABLE table(key_parts, false);
  Fake_KEY key(key_parts, false);
  Fake_KEY unique_key(key_parts, true);

  // Table is empty, records per key estimate should be 1
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);

  /*
    Test with a large table and a non-unique key.
  */

  // Increase the size of the table
  const ha_rows large_table_size = 10000;
  table.file->stats.records = large_table_size;

  // Rec per key for first key part should be 1 percent of table size
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 1),
                  rec_per_key_t(large_table_size / 100));
  /*
    Test with a large table and a unique key.
  */

  // Rec per key for a unique index should always be 1
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);

  /*
    Test with a small table and a non-unique key.
  */

  // Set the size of the table
  const ha_rows small_table_size = 150;
  table.file->stats.records = small_table_size;

  // Rec per key for a non-unique table should be 1 percent of table size
  // but not lower than 10.
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 10.0f);

  /*
    Test with a small table and a unique key.
  */

  // Rec per key for a unique index should always be 1
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);

  /*
    Test with a tiny table and a non-unique key.
  */

  // Set the size of the table
  const ha_rows tiny_table_size = 30;
  table.file->stats.records = tiny_table_size;

  // Rec per key for first key part should be 1 percent of table size, but
  // not smaller than 10% of the table
  EXPECT_FLOAT_EQ(guess_rec_per_key(&table, &key, 1),
                  rec_per_key_t(tiny_table_size) / 10);

  /*
    Test with a tiny table and a unique key.
  */

  // Rec per key for a unique index should always be 1
  EXPECT_EQ(guess_rec_per_key(&table, &unique_key, 1), 1.0f);
}

TEST_F(GuessRecPerKeyTest, GuessRecPerKeyIndexExtension) {
  const uint columns = 5;
  const uint pk_parts = 2;
  const uint key_parts = 2;

  Fake_TABLE table(columns, false);
  Fake_KEY key(key_parts, key_parts + pk_parts, false);

  // Table is empty, records per key estimate should be 1
  EXPECT_EQ(guess_rec_per_key(&table, &key, 1), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &key, 2), 1.0f);
  EXPECT_EQ(guess_rec_per_key(&table, &key, 3), 1.0f);
}

}  // namespace guessrecperkey_unittest
