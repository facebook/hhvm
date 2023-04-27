/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <ctime>

#include "gcs_base_test.h"

#include "gcs_xcom_statistics_interface.h"

namespace gcs_xcom_statistics_unittest {

class XcomStatisticsTest : public GcsBaseTest {
 protected:
  XcomStatisticsTest() {}

  virtual void SetUp() { xcom_stats_if = new Gcs_xcom_statistics(); }

  virtual void TearDown() { delete xcom_stats_if; }

  Gcs_xcom_statistics *xcom_stats_if;
};

TEST_F(XcomStatisticsTest, UpdateMessageSentTest) {
  long message_length = 1000;

  xcom_stats_if->update_message_sent(message_length);

  ASSERT_EQ(message_length, xcom_stats_if->get_total_bytes_sent());
  ASSERT_EQ(1, xcom_stats_if->get_total_messages_sent());
}

TEST_F(XcomStatisticsTest, UpdateMessagesSentTest) {
  long message_length = 1000;

  xcom_stats_if->update_message_sent(message_length);
  xcom_stats_if->update_message_sent(message_length);

  EXPECT_EQ(message_length * 2, xcom_stats_if->get_total_bytes_sent());
  EXPECT_EQ(2, xcom_stats_if->get_total_messages_sent());
}

TEST_F(XcomStatisticsTest, UpdateMessageReceivedTest) {
  long message_length = 1000;

  xcom_stats_if->update_message_received(message_length);

  EXPECT_EQ(message_length, xcom_stats_if->get_total_bytes_received());
  EXPECT_EQ(1, xcom_stats_if->get_total_messages_received());
  EXPECT_GE(time(nullptr), xcom_stats_if->get_last_message_timestamp());
  EXPECT_EQ(message_length, xcom_stats_if->get_max_message_length());
  EXPECT_EQ(message_length, xcom_stats_if->get_min_message_length());
}

TEST_F(XcomStatisticsTest, UpdateMessagesReceivedTest) {
  long message_length_big = 1000;
  long message_length_small = 1000;

  xcom_stats_if->update_message_received(message_length_big);
  xcom_stats_if->update_message_received(message_length_small);

  EXPECT_EQ(message_length_big + message_length_small,
            xcom_stats_if->get_total_bytes_received());

  EXPECT_EQ(2, xcom_stats_if->get_total_messages_received());
  EXPECT_GE(time(nullptr), xcom_stats_if->get_last_message_timestamp());
  EXPECT_EQ(message_length_big, xcom_stats_if->get_max_message_length());
  EXPECT_EQ(message_length_small, xcom_stats_if->get_min_message_length());
}

}  // namespace gcs_xcom_statistics_unittest
