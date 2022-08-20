/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/uio.h>

#include <cstring>
#include <string>

#include <gtest/gtest.h>

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>

#include "mcrouter/lib/carbon/CarbonQueueAppender.h"
#include "mcrouter/lib/network/CaretProtocol.h"
#include "mcrouter/lib/network/gen/MemcacheMessages.h"
#include "mcrouter/lib/network/test/gen/CarbonTestMessages.h"

using namespace facebook::memcache;

TEST(CarbonQueueAppenderTest, longString) {
  carbon::CarbonQueueAppenderStorage storage;
  McGetReply reply(carbon::Result::REMOTE_ERROR);

  // Require more space than CarbonQueueAppenderStorage's internal 512B buffer.
  // This will append() a copy of the string allocated on the heap.
  const std::string message(1024, 'a');
  reply.message_ref() = message;

  carbon::CarbonProtocolWriter writer(storage);
  reply.serialize(writer);

  CaretMessageInfo info;
  info.bodySize = storage.computeBodySize();
  info.typeId = 123;
  info.reqId = 456;
  info.traceId = {17, 18};

  size_t headerSize =
      caretPrepareHeader(info, reinterpret_cast<char*>(storage.getHeaderBuf()));
  storage.reportHeaderSize(headerSize);

  folly::IOBuf input(folly::IOBuf::CREATE, 2048);
  const auto iovs = storage.getIovecs();
  for (size_t i = 0; i < iovs.second; ++i) {
    const struct iovec* iov = iovs.first + i;
    std::memcpy(input.writableTail(), iov->iov_base, iov->iov_len);
    input.append(iov->iov_len);
  }

  // Read the serialized data back in and check that it's what we wrote.
  CaretMessageInfo inputHeader;
  caretParseHeader((uint8_t*)input.data(), input.length(), inputHeader);
  EXPECT_EQ(123, inputHeader.typeId);
  EXPECT_EQ(456, inputHeader.reqId);
  EXPECT_EQ(17, inputHeader.traceId.first);
  EXPECT_EQ(18, inputHeader.traceId.second);

  McGetReply inputReply;
  auto inputBody = folly::IOBuf::wrapBuffer(
      input.data() + inputHeader.headerSize, inputHeader.bodySize);
  carbon::CarbonProtocolReader reader(folly::io::Cursor(inputBody.get()));
  inputReply.deserialize(reader);

  EXPECT_EQ(carbon::Result::REMOTE_ERROR, *inputReply.result_ref());
  EXPECT_EQ(message, *inputReply.message_ref());
}

namespace {
void writeToBuf(
    apache::thrift::field_ref<folly::IOBuf&> dest,
    const char* src,
    size_t len) {
  dest = folly::IOBuf(folly::IOBuf::CREATE, len);
  std::memcpy(dest->writableTail(), src, len);
  dest->append(len);
}
} // namespace

TEST(CarbonQueueAppender, manyFields) {
  carbon::CarbonQueueAppenderStorage storage;
  test::ManyFields manyFields;

  // Each IOBuf must have length() > 0 in order to be serialized
  const char str1[] = "abcde";
  const char str2[] = "xyzzyx";
  // Write null-terminating character too so we can use EXPECT_STREQ
  writeToBuf(manyFields.buf1_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf2_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf3_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf4_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf5_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf6_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf7_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf8_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf9_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf10_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf11_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf12_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf13_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf14_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf15_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf16_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf17_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf18_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf19_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf20_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf21_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf22_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf23_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf24_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf25_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf26_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf27_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf28_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf29_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf30_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf31_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf32_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf33_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf34_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf35_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf36_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf37_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf38_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf39_ref(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf40_ref(), str2, std::strlen(str2) + 1);

  carbon::CarbonProtocolWriter writer(storage);

  // This will trigger CarbonQueueAppenderStorage::coalesce() logic
  manyFields.serialize(writer);

  CaretMessageInfo info;
  info.bodySize = storage.computeBodySize();
  info.typeId = 123;
  info.reqId = 456;
  info.traceId = {17, 18};

  size_t headerSize =
      caretPrepareHeader(info, reinterpret_cast<char*>(storage.getHeaderBuf()));
  storage.reportHeaderSize(headerSize);

  folly::IOBuf input(folly::IOBuf::CREATE, 1024);
  const auto iovs = storage.getIovecs();
  for (size_t i = 0; i < iovs.second; ++i) {
    const struct iovec* iov = iovs.first + i;
    std::memcpy(input.writableTail(), iov->iov_base, iov->iov_len);
    input.append(iov->iov_len);
  }

  // Read the serialized data back in and check that it's what we wrote.
  CaretMessageInfo inputHeader;
  caretParseHeader((uint8_t*)input.data(), input.length(), inputHeader);
  EXPECT_EQ(123, inputHeader.typeId);
  EXPECT_EQ(456, inputHeader.reqId);
  EXPECT_EQ(17, inputHeader.traceId.first);
  EXPECT_EQ(18, inputHeader.traceId.second);

  test::ManyFields manyFields2;
  auto inputBody = folly::IOBuf::wrapBuffer(
      input.data() + inputHeader.headerSize, inputHeader.bodySize);
  carbon::CarbonProtocolReader reader(folly::io::Cursor(inputBody.get()));
  manyFields2.deserialize(reader);

  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf1_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf2_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf3_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf4_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf5_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf6_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf7_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf8_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf9_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf10_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf11_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf12_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf13_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf14_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf15_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf16_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf17_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf18_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf19_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf20_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf21_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf22_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf23_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf24_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf25_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf26_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf27_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf28_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf29_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf30_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf31_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf32_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf33_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf34_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf35_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf36_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf37_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf38_ref()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf39_ref()->data()));
  EXPECT_STREQ(
      str2, reinterpret_cast<const char*>(manyFields2.buf40_ref()->data()));
}
