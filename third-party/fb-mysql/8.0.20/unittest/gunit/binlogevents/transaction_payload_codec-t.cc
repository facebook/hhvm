/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <gtest/gtest.h>
#include <vector>
#include "libbinlogevents/include/binary_log.h"
#include "libbinlogevents/include/codecs/binary.h"

namespace binary_log {
namespace codecs {
namespace unittests {

class TransactionPayloadCodecTest : public ::testing::Test {
 protected:
  std::vector<std::size_t> m_payloads;
  TransactionPayloadCodecTest() {}

  virtual void SetUp() { m_payloads.push_back(128); }

  virtual void TearDown() { m_payloads.clear(); }

  void run_codec_idempotency_test(binary_log::codecs::Codec &codec) {
    for (auto payload_size : m_payloads) {
      unsigned char *payload = new unsigned char[payload_size];

      memset(payload, 'a', payload_size);
      auto ctype = binary_log::transaction::compression::type::ZSTD;
      TransactionPayloadCodecTest::codec_idempotency_test(
          codec, payload, payload_size, ctype, payload_size * 2);

      delete[] payload;
    }
  }

  static void codec_idempotency_test(
      binary_log::codecs::Codec &codec, unsigned char *payload,
      size_t payload_size,
      binary_log::transaction::compression::type compression_type,
      size_t uncompressed_size) {
    const Format_description_event fde(BINLOG_VERSION, "8.0.17");

    // encoded buffer
    unsigned char *enc_buffer = new unsigned char[payload_size * 3];
    memset(enc_buffer, 0, payload_size * 3);

    // decoded buffer
    Transaction_payload_event original((const char *)payload, payload_size,
                                       compression_type, uncompressed_size);

    // -------------------------------------
    // encoding
    // -------------------------------------
    auto enc_result = codec.encode(original, enc_buffer, payload_size * 3);

    // encoded internal header without errors
    ASSERT_FALSE(enc_result.second);

    // copy the payload
    memcpy(enc_buffer + enc_result.first, payload, payload_size);

    // -------------------------------------
    // decoding by hand
    // -------------------------------------

    Transaction_payload_event decoded(static_cast<const char *>(nullptr),
                                      static_cast<uint64_t>(0));

    // decode the post LOG_EVENT header
    auto buffer = enc_buffer;
    size_t buffer_size = payload_size * 3;

    auto dec_result = codec.decode(buffer, buffer_size, decoded);
    decoded.set_payload(reinterpret_cast<const char *>(buffer) +
                        dec_result.first);

    int payload_diff = memcmp(decoded.get_payload(), original.get_payload(),
                              decoded.get_payload_size());

    ASSERT_FALSE(dec_result.second);
    ASSERT_TRUE(decoded.get_payload_size() == original.get_payload_size());
    ASSERT_TRUE(decoded.get_compression_type() ==
                original.get_compression_type());
    ASSERT_TRUE(decoded.get_uncompressed_size() ==
                original.get_uncompressed_size());

    ASSERT_TRUE(payload_diff == 0);

    delete[] enc_buffer;
  }
};

TEST_F(TransactionPayloadCodecTest, EncodeDecodeIdempotencyBinaryTest) {
  binary_log::codecs::binary::Transaction_payload codec;
  TransactionPayloadCodecTest::run_codec_idempotency_test(codec);
}

}  // namespace unittests
}  // namespace codecs
}  // namespace binary_log
