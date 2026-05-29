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
  reply.message() = message;

  carbon::CarbonProtocolWriter writer(storage);
  writer.writeRaw(reply);

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
  reader.readRawInto(inputReply);

  EXPECT_EQ(carbon::Result::REMOTE_ERROR, *inputReply.result());
  EXPECT_EQ(message, *inputReply.message());
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
  writeToBuf(manyFields.buf1(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf2(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf3(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf4(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf5(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf6(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf7(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf8(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf9(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf10(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf11(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf12(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf13(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf14(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf15(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf16(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf17(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf18(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf19(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf20(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf21(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf22(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf23(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf24(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf25(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf26(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf27(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf28(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf29(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf30(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf31(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf32(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf33(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf34(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf35(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf36(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf37(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf38(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf39(), str1, std::strlen(str1) + 1);
  writeToBuf(manyFields.buf40(), str2, std::strlen(str2) + 1);

  carbon::CarbonProtocolWriter writer(storage);

  // This will trigger CarbonQueueAppenderStorage::coalesce() logic
  writer.writeRaw(manyFields);

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
  reader.readRawInto(manyFields2);

  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf1()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf2()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf3()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf4()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf5()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf6()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf7()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf8()->data()));
  EXPECT_STREQ(str1, reinterpret_cast<const char*>(manyFields2.buf9()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf10()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf11()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf12()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf13()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf14()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf15()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf16()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf17()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf18()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf19()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf20()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf21()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf22()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf23()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf24()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf25()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf26()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf27()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf28()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf29()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf30()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf31()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf32()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf33()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf34()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf35()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf36()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf37()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf38()->data()));
  EXPECT_STREQ(
      str1, reinterpret_cast<const char*>(manyFields2.buf39()->data()));
  EXPECT_STREQ(
      str2, reinterpret_cast<const char*>(manyFields2.buf40()->data()));
}
