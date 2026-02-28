/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include <fizz/record/Types.h>

using namespace folly;
using namespace folly::io;

namespace fizz {
namespace test {

TEST(TestTypes, WriteAndRead24BitsNormal) {
  uint32_t len = 0x102030;
  auto buf = IOBuf::create(3);
  Appender appender(buf.get(), 0);
  Error err;
  EXPECT_EQ(detail::writeBits24(err, len, appender), Status::Success);

  EXPECT_EQ(0x10, buf->data()[0]);
  EXPECT_EQ(0x20, buf->data()[1]);
  EXPECT_EQ(0x30, buf->data()[2]);

  Cursor cursor(buf.get());
  uint32_t actualLength = detail::readBits24(cursor);
  EXPECT_EQ(len, actualLength);
}

TEST(TestTypes, Write24BitsOverflow) {
  uint32_t len = 0x10203040;
  auto buf = IOBuf::create(3);
  Appender appender(buf.get(), 0);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(detail::writeBits24(err, len, appender), err),
      std::runtime_error);
}

TEST(TestTypes, Write24BitsBuffer) {
  auto buf = IOBuf::create(0x1020);
  buf->append(0x1020);

  auto out = IOBuf::create(10);
  Appender appender(out.get(), 10);
  Error err;
  EXPECT_EQ(
      detail::writeBuf<detail::bits24>(err, buf, appender), Status::Success);
  EXPECT_EQ(buf->length() + 3, out->computeChainDataLength());
}

TEST(TestTypes, Write24BitsBufferOverflow) {
  auto buf = IOBuf::create(0x1000000);
  buf->append(0x1000000);

  auto out = IOBuf::create(10);
  Appender appender(out.get(), 0);
  Error err;
  EXPECT_THROW(
      FIZZ_THROW_ON_ERROR(
          detail::writeBuf<detail::bits24>(err, buf, appender), err),
      std::runtime_error);
}

TEST(TestTypes, WriteBuf) {
  auto buf = IOBuf::create(20);
  buf->append(20);

  auto out1 = IOBuf::create(10);
  Appender appender1(out1.get(), 10);
  Error err;
  EXPECT_EQ(detail::writeBuf<uint16_t>(err, buf, appender1), Status::Success);
  EXPECT_EQ(2 + buf->length(), out1->computeChainDataLength());

  auto out2 = IOBuf::create(10);
  Appender appender2(out2.get(), 10);
  EXPECT_EQ(detail::writeBuf<uint64_t>(err, buf, appender2), Status::Success);
  EXPECT_EQ(8 + buf->length(), out2->computeChainDataLength());
}

TEST(TestTypes, WriteReadString) {
  std::string str{"str.buf.com"};

  auto out = IOBuf::create(10);
  Appender appender(out.get(), 10);
  detail::writeString<uint8_t>(str, appender);
  EXPECT_EQ(detail::getStringSize<uint8_t>(str), out->computeChainDataLength());

  Cursor cursor(out.get());
  std::string decodedStr;
  auto len = detail::readString<uint8_t>(decodedStr, cursor);
  ASSERT_TRUE(cursor.isAtEnd());
  EXPECT_EQ(len, out->computeChainDataLength());
  EXPECT_EQ(str, decodedStr);
}
} // namespace test
} // namespace fizz
