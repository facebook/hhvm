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
  detail::writeBits24(len, appender);

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
  EXPECT_THROW(detail::writeBits24(len, appender), std::runtime_error);
}

TEST(TestTypes, Write24BitsBuffer) {
  auto buf = IOBuf::create(0x1020);
  buf->append(0x1020);

  auto out = IOBuf::create(10);
  Appender appender(out.get(), 10);
  detail::writeBuf<detail::bits24>(buf, appender);
  EXPECT_EQ(buf->length() + 3, out->computeChainDataLength());
}

TEST(TestTypes, Write24BitsBufferOverflow) {
  auto buf = IOBuf::create(0x1000000);
  buf->append(0x1000000);

  auto out = IOBuf::create(10);
  Appender appender(out.get(), 0);
  EXPECT_THROW(
      detail::writeBuf<detail::bits24>(buf, appender), std::runtime_error);
}

TEST(TestTypes, WriteBuf) {
  auto buf = IOBuf::create(20);
  buf->append(20);

  auto out1 = IOBuf::create(10);
  Appender appender1(out1.get(), 10);
  detail::writeBuf<uint16_t>(buf, appender1);
  EXPECT_EQ(2 + buf->length(), out1->computeChainDataLength());

  auto out2 = IOBuf::create(10);
  Appender appender2(out2.get(), 10);
  detail::writeBuf<uint64_t>(buf, appender2);
  EXPECT_EQ(8 + buf->length(), out2->computeChainDataLength());
}
} // namespace test
} // namespace fizz
