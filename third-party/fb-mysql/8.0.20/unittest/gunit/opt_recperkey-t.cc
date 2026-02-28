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

#include "sql/key.h"                  // KEY
#include "unittest/gunit/fake_key.h"  // Fake_KEY

namespace recperkey_unittest {

/**
  Test the API for setting and getting records per key values.
*/

TEST(RecPerKeyTest, RecPerKeyAPI) {
  const uint key_parts = 3;

  Fake_KEY key(key_parts, false);

  /*
    Test that the rec_per_key values are default/unknown.
  */
  for (uint kp = 0; kp < key_parts; kp++) {
    EXPECT_FALSE(key.has_records_per_key(kp));
    EXPECT_EQ(key.records_per_key(kp), REC_PER_KEY_UNKNOWN);
  }

  /*
    Test setting the values using the API
  */

  // Give one of the key parts a value
  key.set_records_per_key(1, 2.0);

  // Validate that only one of the key parts has gotten a value
  EXPECT_FALSE(key.has_records_per_key(0));
  EXPECT_TRUE(key.has_records_per_key(1));
  EXPECT_FALSE(key.has_records_per_key(2));

  EXPECT_EQ(key.records_per_key(0), REC_PER_KEY_UNKNOWN);
  EXPECT_EQ(key.records_per_key(1), 2.0);
  EXPECT_EQ(key.records_per_key(2), REC_PER_KEY_UNKNOWN);

  // Reset the rec_per_key value to default/unkown
  key.set_records_per_key(1, REC_PER_KEY_UNKNOWN);
  EXPECT_FALSE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), REC_PER_KEY_UNKNOWN);
}

/**
  Test the old integer based rec_per_key implementation used
  together with the records per key API.
*/

TEST(RecPerKeyTest, RecPerKey) {
  const uint key_parts = 3;

  Fake_KEY key(key_parts, false);

  // Set values directly in the integer based rec_per_key array
  key.rec_per_key[0] = 0;  // default/unknown value
  key.rec_per_key[1] = 1;
  key.rec_per_key[2] = 2;

  // Validate that this produces correct values using the API functions
  EXPECT_FALSE(key.has_records_per_key(0));
  EXPECT_EQ(key.records_per_key(0), REC_PER_KEY_UNKNOWN);
  EXPECT_TRUE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), 1.0);
  EXPECT_TRUE(key.has_records_per_key(2));
  EXPECT_EQ(key.records_per_key(2), 2.0);

  // Reset the values
  key.rec_per_key[1] = 0;
  key.rec_per_key[2] = 0;
  EXPECT_FALSE(key.has_records_per_key(0));
  EXPECT_EQ(key.records_per_key(0), REC_PER_KEY_UNKNOWN);
  EXPECT_FALSE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), REC_PER_KEY_UNKNOWN);
  EXPECT_FALSE(key.has_records_per_key(2));
  EXPECT_EQ(key.records_per_key(2), REC_PER_KEY_UNKNOWN);

  /*
    Test that if both integer and float rec_per_key values are set,
    the float values will be used.
  */

  // Give a value using the integer based rec_per_key
  key.rec_per_key[1] = 1;
  EXPECT_TRUE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), 1.0);

  // Update the value using the API
  key.set_records_per_key(1, 2.0);
  EXPECT_TRUE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), 2.0);

  // Set the value back to default/unknown
  key.set_records_per_key(1, REC_PER_KEY_UNKNOWN);

  /*
    This result might not be intuitive but when we set the value using
    the API, only the float value will be updated. When the float value
    is set to default/unknown, the integer value will be used instead.
    In pratice this is not an issue, but can be solved by setting
    the integer value to 0, when setting the float value.
  */
  EXPECT_TRUE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), 1.0);

  /*
    Verify that we get default/unknown when also setting the integer
    value to the default/unknown value.
  */
  key.rec_per_key[1] = 0;
  EXPECT_FALSE(key.has_records_per_key(1));
  EXPECT_EQ(key.records_per_key(1), REC_PER_KEY_UNKNOWN);
}

}  // namespace recperkey_unittest
