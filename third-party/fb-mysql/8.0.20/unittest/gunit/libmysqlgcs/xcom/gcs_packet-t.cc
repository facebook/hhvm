/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "gcs_internal_message.h"
#include "gcs_message_stage_lz4.h"
#include "gcs_message_stages.h"
#include "gcs_xcom_statistics_interface.h"
#include "mysql/gcs/gcs_message.h"

namespace gcs_xcom_packet_unittest {

class GcsPacketTest : public GcsBaseTest {
 protected:
  GcsPacketTest() {}

  virtual void SetUp() { lz4_stage = new Gcs_message_stage_lz4(true, 1024); }

  virtual void TearDown() { delete lz4_stage; }

  Gcs_message_stage_lz4 *lz4_stage;

 public:
  static const unsigned long long LARGE_PAYLOAD_LEN;
  static const unsigned long long SMALL_PAYLOAD_LEN;
};

const unsigned long long GcsPacketTest::LARGE_PAYLOAD_LEN =
    1024 + Gcs_internal_message_header::WIRE_TOTAL_FIXED_HEADER_SIZE;
const unsigned long long GcsPacketTest::SMALL_PAYLOAD_LEN =
    1024 - Gcs_internal_message_header::WIRE_TOTAL_FIXED_HEADER_SIZE;

TEST_F(GcsPacketTest, PacketInit) {
  const char content[] = "OLA123";
  unsigned int content_len = sizeof(content);

  /*
   Simulate a message that was prepared by an upper layer such as group
   replication.
   */
  Gcs_member_identifier origin(std::string("luis"));
  Gcs_message msg(origin, new Gcs_message_data(0, content_len));
  Cargo_type cargo = Cargo_type::CT_INTERNAL_STATE_EXCHANGE;
  msg.get_message_data().append_to_payload((const unsigned char *)content,
                                           content_len);

  /*
   Create an internal gcs message that will be eventually delivered to
   the group communication layer.
   */
  Gcs_message_data &msg_data = msg.get_message_data();
  unsigned long long payload_length = msg_data.get_encode_size();

  bool packet_ok;
  Gcs_packet p;
  std::tie(packet_ok, p) = Gcs_packet::make_outgoing_packet(
      cargo, Gcs_protocol_version::V1, {}, {}, payload_length);
  ASSERT_TRUE(packet_ok);

  /*
   Encode the payload encapsulated in the group replication message into
   the gcs message.
   */
  uint64_t buffer_size = p.get_payload_length();
  ASSERT_FALSE(msg_data.encode(p.get_payload_pointer(), &buffer_size));

  ASSERT_EQ(p.get_payload_length(), payload_length);
  ASSERT_EQ(p.get_total_length(),
            payload_length +
                Gcs_internal_message_header::WIRE_TOTAL_FIXED_HEADER_SIZE);

  /*
   Decode the payload from the gcs message into the group replication
   message.
   */
  Gcs_message_data msg_decoded(p.get_payload_length());
  msg_decoded.decode(p.get_payload_pointer(), p.get_payload_length());

  ASSERT_TRUE(strncmp((const char *)msg_decoded.get_payload(),
                      (const char *)content, content_len) == 0);
}

}  // namespace gcs_xcom_packet_unittest
