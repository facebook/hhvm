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

#include "gcs_base_test.h"

#include "mysql/gcs/gcs_group_identifier.h"
#include "mysql/gcs/gcs_member_identifier.h"
#include "mysql/gcs/gcs_message.h"
#include "template_utils.h"

#include <string>
#include <vector>

using std::vector;

namespace gcs_message_unittest {

class MessageEncodingDecodingTest : public GcsBaseTest {};

TEST_F(MessageEncodingDecodingTest, EncodeDecodeTest) {
  const unsigned int n16 = 256;
  const unsigned int n32 = 65536;
  Gcs_message_data *message_data16 = new Gcs_message_data(n16, n16);
  Gcs_message_data *message_data32 = new Gcs_message_data(n32, n32);

  uchar uint32_buf[n32];
  uchar uint16_buf[n16];

  memset(uint32_buf, 0, n32);
  memset(uint16_buf, 0, n16);

  std::string uint16_buf_str("buffer16");
  std::string uint32_buf_str("buffer32");

  uint16_buf_str.copy((char *)uint16_buf, uint16_buf_str.size(), 0);
  uint32_buf_str.copy((char *)uint32_buf, uint32_buf_str.size(), 0);

  message_data16->append_to_header(uint16_buf, n16);
  message_data16->append_to_payload(uint16_buf, n16);

  message_data32->append_to_header(uint32_buf, n32);
  message_data32->append_to_payload(uint32_buf, n32);

  uchar *buffer16 = nullptr;
  uint64_t buffer16_len = 0;
  message_data16->encode(&buffer16, &buffer16_len);
  message_data16->release_ownership();

  uchar *buffer32 = nullptr;
  uint64_t buffer32_len = 0;
  message_data32->encode(&buffer32, &buffer32_len);
  message_data32->release_ownership();

  EXPECT_TRUE(buffer16 != nullptr);
  EXPECT_EQ(WIRE_HEADER_LEN_SIZE + WIRE_PAYLOAD_LEN_SIZE + n16 * 2,
            buffer16_len);

  EXPECT_TRUE(buffer32 != nullptr);
  EXPECT_EQ(WIRE_HEADER_LEN_SIZE + WIRE_PAYLOAD_LEN_SIZE + n32 * 2,
            buffer32_len);

  Gcs_message_data to_decode16(buffer16_len);
  Gcs_message_data to_decode32(buffer32_len);

  to_decode16.decode(buffer16, buffer16_len);
  to_decode32.decode(buffer32, buffer32_len);

  EXPECT_EQ(n16, to_decode16.get_header_length());
  EXPECT_EQ(n16, to_decode16.get_payload_length());

  std::string returned_header16((const char *)to_decode16.get_header());
  EXPECT_EQ("buffer16", returned_header16);

  std::string returned_payload16((const char *)to_decode16.get_payload());
  EXPECT_EQ("buffer16", returned_payload16);

  std::string returned_header32((const char *)to_decode32.get_header());
  EXPECT_EQ("buffer32", returned_header32);

  std::string returned_payload32((const char *)to_decode32.get_payload());
  EXPECT_EQ("buffer32", returned_payload32);

  free(buffer16);
  free(buffer32);

  delete message_data16;
  delete message_data32;
}

class MessageDataTest : public GcsBaseTest {};

TEST_F(MessageDataTest, AppendtoHeaderTest) {
  std::string test_data("to_append");
  Gcs_message_data *message_data = new Gcs_message_data(test_data.length(), 0);

  message_data->append_to_header(pointer_cast<const uchar *>(test_data.c_str()),
                                 test_data.length());

  EXPECT_EQ(test_data.length(), message_data->get_header_length());
  EXPECT_EQ((size_t)0, message_data->get_payload_length());
  delete message_data;
}

TEST_F(MessageDataTest, AppendtoPayloadTest) {
  std::string test_data("to_append");
  Gcs_message_data *message_data = new Gcs_message_data(0, test_data.length());

  message_data->append_to_payload(
      pointer_cast<const uchar *>(test_data.c_str()), test_data.length());

  EXPECT_EQ(test_data.length(), message_data->get_payload_length());
  EXPECT_EQ((size_t)0, message_data->get_header_length());
  delete message_data;
}

TEST_F(MessageDataTest, EncodeTest) {
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_message_data *message_data =
      new Gcs_message_data(test_header.length(), test_payload.length());

  message_data->append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());

  message_data->append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  uchar *buffer = nullptr;
  uint64_t buffer_len = 0;
  message_data->encode(&buffer, &buffer_len);
  message_data->release_ownership();

  EXPECT_TRUE(buffer != nullptr);

  EXPECT_EQ(WIRE_HEADER_LEN_SIZE + WIRE_PAYLOAD_LEN_SIZE +
                test_header.length() + test_payload.length(),
            buffer_len);

  free(buffer);
  delete message_data;
}

TEST_F(MessageDataTest, EncodeNullTest) {
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_message_data *message_data =
      new Gcs_message_data(test_header.length(), test_payload.length());

  message_data->append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());

  message_data->append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  uchar *buffer = nullptr;
  uint64_t buffer_len = 0;

  EXPECT_TRUE(message_data->encode(static_cast<uchar **>(nullptr),
                                   static_cast<uint64_t *>(nullptr)));

  EXPECT_TRUE(message_data->encode(&buffer, static_cast<uint64_t *>(nullptr)));

  EXPECT_TRUE(
      message_data->encode(static_cast<uchar **>(nullptr), &buffer_len));

  EXPECT_TRUE(message_data->encode(static_cast<uchar *>(nullptr),
                                   static_cast<uint64_t *>(nullptr)));

  buffer = static_cast<uchar *>(malloc(1));
  EXPECT_TRUE(message_data->encode(buffer, static_cast<uint64_t *>(nullptr)));

  EXPECT_TRUE(message_data->encode(buffer, &buffer_len));

  free(buffer);
  delete message_data;
}

TEST_F(MessageDataTest, DecodeTest) {
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_message_data *message_data =
      new Gcs_message_data(test_header.length() + 1, test_payload.length() + 1);

  message_data->append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()),
      test_header.length() + 1);

  message_data->append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()),
      test_payload.length() + 1);

  uchar *buffer = nullptr;
  uint64_t buffer_len = 0;
  message_data->encode(&buffer, &buffer_len);
  message_data->release_ownership();

  EXPECT_TRUE(buffer != nullptr);
  EXPECT_EQ(WIRE_HEADER_LEN_SIZE + WIRE_PAYLOAD_LEN_SIZE +
                test_header.length() + test_payload.length() + 2,
            buffer_len);

  Gcs_message_data to_decode(buffer_len);

  to_decode.decode(buffer, buffer_len);

  EXPECT_EQ(test_header.length() + 1, to_decode.get_header_length());
  EXPECT_EQ(test_payload.length() + 1, to_decode.get_payload_length());

  std::string returned_header((const char *)to_decode.get_header());
  EXPECT_EQ(test_header, returned_header);

  std::string returned_payload((const char *)to_decode.get_payload());
  EXPECT_EQ(test_payload, returned_payload);

  free(buffer);
  delete message_data;
}

TEST_F(MessageDataTest, DecodeNullTest) {
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_message_data *message_data =
      new Gcs_message_data(test_header.length(), test_payload.length());

  message_data->append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());
  message_data->append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  uchar *buffer = nullptr;
  uint64_t buffer_len = 0;

  EXPECT_TRUE(message_data->decode(buffer, buffer_len));

  buffer = static_cast<uchar *>(malloc(1));
  EXPECT_TRUE(message_data->decode(buffer, buffer_len));

  buffer_len = 1024;
  EXPECT_TRUE(message_data->decode(buffer, buffer_len));

  free(buffer);
  delete message_data;
}

}  // namespace gcs_message_unittest
