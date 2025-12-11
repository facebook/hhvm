/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fizz/experimental/psp/PSP.h>
#include <folly/test/TestUtils.h>

#include <array>

TEST(PSPTest, KeyLength) {
  EXPECT_EQ(fizz::psp::detail::keyLength(0xff), std::nullopt);
  EXPECT_EQ(fizz::psp::detail::keyLength(0), 16);
  EXPECT_EQ(fizz::psp::detail::keyLength(1), 32);
  EXPECT_EQ(fizz::psp::detail::keyLength(2), 16);
  EXPECT_EQ(fizz::psp::detail::keyLength(3), 32);
}

TEST(PSPTest, EncodeSA) {
  {
    fizz::psp::SA sa;
    sa.psp_version = 0;
    sa.spi = 0xaabbccdd;
    sa.key = {
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x07,
        0x08,
        0x09,
        0x0a,
        0x0b,
        0x0c,
        0x0d,
        0x0e,
        0x0f,
        0x10};

    auto encoded = fizz::psp::detail::encodeTLV(sa);
    ASSERT_NE(encoded, nullptr);

    auto buf = encoded->coalesce();
    EXPECT_EQ(buf.size(), 42);

    auto expected = std::to_array<uint8_t>({
        0x01, 0x28, 0xaa, 0xbb, 0xcc, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    });
    EXPECT_EQ(memcmp(buf.data(), expected.data(), expected.size()), 0);

    auto saPayload = folly::IOBuf::copyBuffer(buf.data() + 2, buf.size() - 2);
    auto decoded = fizz::psp::detail::tryDecodeSA(std::move(saPayload));
    ASSERT_TRUE(decoded.hasValue());
    EXPECT_EQ(decoded->psp_version, sa.psp_version);
    EXPECT_EQ(decoded->spi, sa.spi);
    EXPECT_EQ(decoded->key, sa.key);
  }

  {
    fizz::psp::SA sa;
    sa.psp_version = 1;
    sa.spi = 0xdeadbeef;
    sa.key.resize(32);
    for (size_t i = 0; i < 32; i++) {
      sa.key[i] = static_cast<uint8_t>(i * 2);
    }

    auto encoded = fizz::psp::detail::encodeTLV(sa);
    ASSERT_NE(encoded, nullptr);

    auto buf = encoded->coalesce();
    EXPECT_EQ(buf.size(), 42);

    auto expected = std::to_array<uint8_t>({
        0x01, 0x28, 0xde, 0xad, 0xbe, 0xef, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16,
        0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c,
        0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
    });
    EXPECT_EQ(memcmp(buf.data(), expected.data(), expected.size()), 0);

    auto saPayload = folly::IOBuf::copyBuffer(buf.data() + 2, buf.size() - 2);
    auto decoded = fizz::psp::detail::tryDecodeSA(std::move(saPayload));
    ASSERT_TRUE(decoded.hasValue());
    EXPECT_EQ(decoded->psp_version, sa.psp_version);
    EXPECT_EQ(decoded->spi, sa.spi);
    EXPECT_EQ(decoded->key, sa.key);
  }
}

TEST(PSPTest, TestReadTLV) {
  // Empty buffer
  {
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    auto result = fizz::psp::detail::readTLV(readBuf);
    EXPECT_FALSE(result.has_value());
  }

  // Incomplete TLV (only tag, no length)
  {
    auto data = std::to_array<uint8_t>({0x01});
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    readBuf.append(folly::IOBuf::wrapBuffer(data.data(), data.size()));

    auto result = fizz::psp::detail::readTLV(readBuf);
    EXPECT_FALSE(result.has_value());
  }

  // Tag and length but incomplete value
  {
    auto data = std::to_array<uint8_t>({0x01, 0x05, 0xaa});
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    readBuf.append(folly::IOBuf::wrapBuffer(data.data(), data.size()));

    auto result = fizz::psp::detail::readTLV(readBuf);
    EXPECT_FALSE(result.has_value());
  }

  // Complete TLV message
  {
    auto data = std::to_array<uint8_t>(
        {0x01, // tag
         0x04, // length = 4
         0xaa,
         0xbb,
         0xcc,
         0xdd});
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    readBuf.append(folly::IOBuf::wrapBuffer(data.data(), data.size()));

    auto result = fizz::psp::detail::readTLV(readBuf);
    ASSERT_TRUE(result.has_value());

    auto [tag, value] = std::move(result).value();
    EXPECT_EQ(tag, 0x01);

    auto valueBuf = value->coalesce();
    EXPECT_EQ(valueBuf.size(), 4);
    auto expected = std::to_array<uint8_t>({0xaa, 0xbb, 0xcc, 0xdd});
    EXPECT_EQ(memcmp(valueBuf.data(), expected.data(), 4), 0);

    // Buffer should be empty after reading the TLV
    EXPECT_EQ(readBuf.chainLength(), 0);
  }

  // Zero-length value
  {
    auto data = std::to_array<uint8_t>({0x00, 0x00});
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    readBuf.append(folly::IOBuf::wrapBuffer(data.data(), data.size()));

    auto result = fizz::psp::detail::readTLV(readBuf);
    ASSERT_TRUE(result.has_value());

    auto [tag, value] = std::move(result).value();
    EXPECT_EQ(tag, 0x00);
    EXPECT_EQ(value->computeChainDataLength(), 0);
  }

  // Multiple TLV messages (should only read the first one)
  {
    auto data = std::to_array<uint8_t>(
        {// First TLV
         0x01,
         0x03,
         0x11,
         0x22,
         0x33,
         // Second TLV
         0x02,
         0x02,
         0x44,
         0x55});
    folly::IOBufQueue readBuf{folly::IOBufQueue::cacheChainLength()};
    readBuf.append(folly::IOBuf::wrapBuffer(data.data(), data.size()));

    // Read first TLV
    auto result1 = fizz::psp::detail::readTLV(readBuf);
    ASSERT_TRUE(result1.has_value());

    auto [tag1, value1] = std::move(result1).value();
    EXPECT_EQ(tag1, 0x01);

    auto valueBuf1 = value1->coalesce();
    EXPECT_EQ(valueBuf1.size(), 3);
    auto expected1 = std::to_array<uint8_t>({0x11, 0x22, 0x33});
    EXPECT_EQ(memcmp(valueBuf1.data(), expected1.data(), 3), 0);

    // Second TLV should still be in the buffer
    EXPECT_EQ(readBuf.chainLength(), 4);

    // Read second TLV
    auto result2 = fizz::psp::detail::readTLV(readBuf);
    ASSERT_TRUE(result2.has_value());

    auto [tag2, value2] = std::move(result2).value();
    EXPECT_EQ(tag2, 0x02);

    auto valueBuf2 = value2->coalesce();
    EXPECT_EQ(valueBuf2.size(), 2);
    auto expected2 = std::to_array<uint8_t>({0x44, 0x55});
    EXPECT_EQ(memcmp(valueBuf2.data(), expected2.data(), 2), 0);

    // Buffer should be empty now
    EXPECT_EQ(readBuf.chainLength(), 0);
  }
}
