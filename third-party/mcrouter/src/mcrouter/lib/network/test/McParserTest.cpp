/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "mcrouter/lib/network/CaretProtocol.h"
#include "mcrouter/lib/network/McParser.h"

using namespace facebook::memcache;

namespace {

class NoOpCallback : public McParser::ParserCallback {
 public:
  bool caretMessageReady(const CaretMessageInfo&, const folly::IOBuf&)
      override {
    return true;
  }

  void handleAscii(folly::IOBuf&) override {}

  void parseError(carbon::Result, folly::StringPiece) override {}
};

} // namespace

TEST(McParserTest, ReadZeroLengthMessage) {
  NoOpCallback cb;
  McParser parser{cb, 1024, 1024};

  void* buf;
  size_t bufLen;
  std::tie(buf, bufLen) = parser.getReadBuffer();
  EXPECT_GE(bufLen, 1024);

  // Cause an overflow when calculating messageSize in McParser. This happens
  // when McParser sees a headerSize of 9 after parsing the message. Why? The
  // maximum value of a 32-bit integer is 2^32 - 1 = 4,294,967,295 and
  // 4,294,967,287 + 9 = 4,294,967,296.
  CaretMessageInfo msgInfo;
  msgInfo.bodySize = 4'294'967'287;

  auto bytesWritten = caretPrepareHeader(msgInfo, reinterpret_cast<char*>(buf));
  EXPECT_GT(bytesWritten, 0);

  EXPECT_FALSE(parser.readDataAvailable(bytesWritten));
}
