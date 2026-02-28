/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <array>

#include <gtest/gtest.h>
#include "libbinlogevents/include/binary_log.h"
#include "libbinlogevents/include/compression/none.h"
#include "libbinlogevents/include/compression/zstd.h"

namespace binary_log {
namespace transaction {
namespace unittests {

static std::size_t MAX_BUFFER_SIZE = 1024 * 1024 * 512;

static std::array<std::size_t, 4> buffer_sizes{128, 256, 512,
                                               MAX_BUFFER_SIZE / 2};

class TransactionPayloadCompressionTest : public ::testing::Test {
 public:
  std::vector<std::size_t> m_payloads;

 protected:
  TransactionPayloadCompressionTest() {}

  virtual void SetUp() {
    for (auto size : buffer_sizes) m_payloads.push_back(size);
  }

  virtual void TearDown() { m_payloads.clear(); }

  static void compression_idempotency_test(
      binary_log::transaction::compression::Compressor &c,
      binary_log::transaction::compression::Decompressor &d,
      size_t buffer_size) {
    std::size_t size = 0, old_size = 0;
    std::size_t left{0};
    bool fail{false};

    // prepare a buffer for the compressed event
    auto ibuf = (unsigned char *)malloc(buffer_size);
    auto obuf = (unsigned char *)malloc(buffer_size);
    auto cbuf = (unsigned char *)malloc(buffer_size * 2);

    int buffer_diff = 0;
    std::size_t capacity = 0;
    unsigned char *ptr = nullptr;
    ASSERT_FALSE(ibuf == nullptr);
    ASSERT_FALSE(obuf == nullptr);
    ASSERT_FALSE(cbuf == nullptr);

    memset(obuf, 0, buffer_size);
    memset(cbuf, 0, buffer_size);
    memset(ibuf, 'a', buffer_size);

    c.set_buffer(cbuf, buffer_size * 2);
    ASSERT_FALSE(c.open());
    std::tie(left, fail) = c.compress(ibuf, buffer_size);
    ASSERT_FALSE(fail);
    ASSERT_FALSE(c.close());

    // get the compressed size
    std::tie(ptr, size, capacity) = c.get_buffer();

    // the compressor may have realloc'ed the cbuf
    cbuf = ptr;
    // assert that it is smaller than the uncompressed size
    if (c.compression_type_code() != binary_log::transaction::compression::NONE)
      ASSERT_TRUE(size < buffer_size);
    else
      ASSERT_TRUE(size == buffer_size);
    old_size = size;

    d.set_buffer(obuf, buffer_size);
    ASSERT_FALSE(d.open());
    std::tie(left, fail) = d.decompress(cbuf, size);
    ASSERT_FALSE(fail);
    ASSERT_FALSE(d.close());

    std::tie(ptr, size, capacity) = d.get_buffer();

    // the compressor may have realloc'ed the obuf
    obuf = ptr;

    // assert that it is smaller than the uncompressed size
    if (d.compression_type_code() != binary_log::transaction::compression::NONE)
      ASSERT_TRUE(old_size < size);
    else
      ASSERT_TRUE(old_size == size);

    buffer_diff = memcmp(ibuf, obuf, buffer_size);
    ASSERT_TRUE(buffer_diff == 0);

    free(ibuf);
    free(obuf);
    free(cbuf);
  }

  static void compress_decompress_in_stages(
      binary_log::transaction::compression::Compressor &c,
      binary_log::transaction::compression::Decompressor &d) {
    std::size_t size = 0, old_size = 0;
    std::size_t buffer_size = MAX_BUFFER_SIZE;
    std::size_t chunk_size = buffer_size / 8;
    std::size_t next_pos = 0;

    // prepare a buffer for the compressed event
    auto ibuf = (unsigned char *)malloc(buffer_size);
    auto obuf = (unsigned char *)malloc(buffer_size);
    auto cbuf = (unsigned char *)malloc(buffer_size * 2);

    int buffer_diff = 0;
    std::size_t capacity = 0;
    unsigned char *ptr = nullptr;
    ASSERT_FALSE(ibuf == nullptr);
    ASSERT_FALSE(obuf == nullptr);
    ASSERT_FALSE(cbuf == nullptr);

    memset(obuf, 0, buffer_size);
    memset(cbuf, 0, buffer_size);
    memset(ibuf, 'a', buffer_size);
    c.set_buffer(cbuf, buffer_size * 2);
    ASSERT_FALSE(c.open());
    for (next_pos = 0; next_pos != buffer_size; next_pos += chunk_size) {
      auto left{0};
      auto fail{false};
      std::tie(left, fail) = c.compress(ibuf + next_pos, chunk_size);
      ASSERT_FALSE(fail);
      ASSERT_TRUE(left == 0);
    }
    ASSERT_FALSE(c.close());

    // get the compressed size
    std::tie(ptr, size, capacity) = c.get_buffer();

    // the compressor may have realloc'ed the cbuf
    cbuf = ptr;
    // assert that it is smaller than the uncompressed size
    if (c.compression_type_code() != binary_log::transaction::compression::NONE)
      ASSERT_TRUE(size < buffer_size);
    else
      ASSERT_TRUE(size == buffer_size);
    old_size = size;

    d.set_buffer(obuf, buffer_size);
    ASSERT_FALSE(d.open());
    auto left{0};
    auto fail{false};
    std::tie(left, fail) = d.decompress(ptr, size);
    ASSERT_FALSE(fail);
    ASSERT_FALSE(d.close());

    std::tie(ptr, size, capacity) = d.get_buffer();

    // the compressor may have realloc'ed the obuf
    obuf = ptr;

    // assert that it is smaller than the uncompressed size
    if (d.compression_type_code() != binary_log::transaction::compression::NONE)
      ASSERT_TRUE(old_size < size);
    else
      ASSERT_TRUE(old_size == size);
    buffer_diff = memcmp(ibuf, obuf, buffer_size);
    ASSERT_TRUE(buffer_diff == 0);

    free(ibuf);
    free(obuf);
    free(cbuf);
  }
};

TEST_F(TransactionPayloadCompressionTest, CompressDecompressZstdTest) {
  for (auto size : m_payloads) {
    binary_log::transaction::compression::Zstd_dec d;
    binary_log::transaction::compression::Zstd_comp c;
    TransactionPayloadCompressionTest::compression_idempotency_test(c, d, size);
    c.set_compression_level(22);
    TransactionPayloadCompressionTest::compression_idempotency_test(c, d, size);
  }
}

TEST_F(TransactionPayloadCompressionTest, CompressDecompressNoneTest) {
  for (auto size : m_payloads) {
    binary_log::transaction::compression::None_dec d;
    binary_log::transaction::compression::None_comp c;
    TransactionPayloadCompressionTest::compression_idempotency_test(c, d, size);
  }
}

TEST_F(TransactionPayloadCompressionTest, CompressDecompressInStagesZstdTest) {
  binary_log::transaction::compression::Zstd_dec d;
  binary_log::transaction::compression::Zstd_comp c;
  TransactionPayloadCompressionTest::compress_decompress_in_stages(c, d);
}

TEST_F(TransactionPayloadCompressionTest, CompressDecompressInStagesNoneTest) {
  binary_log::transaction::compression::None_dec d;
  binary_log::transaction::compression::None_comp c;
  TransactionPayloadCompressionTest::compress_decompress_in_stages(c, d);
}

}  // namespace unittests
}  // namespace transaction
}  // namespace binary_log
