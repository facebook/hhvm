/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/HQFramerTest.h>

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <quic/codec/QuicInteger.h>

using namespace folly;
using namespace folly::io;
using namespace proxygen::hq;
using namespace proxygen;
using namespace std;
using namespace testing;

namespace {
// Return true if (lhs < rhs), false otherwise
bool comparePushId(PushId lhs, PushId rhs) {
  return (lhs < rhs) ? false : true;
}

// Validate the given push ID.
bool isValidPushId(folly::Optional<PushId> maxAllowedPushId, PushId pushId) {
  if (!maxAllowedPushId.hasValue()) {
    VLOG(3) << __func__ << "maximum push ID value has not been set";
    return false;
  } else if (!comparePushId(maxAllowedPushId.value(), pushId)) {
    VLOG(3) << __func__ << "given pushid=" << pushId
            << "exceeds possible push ID value "
            << "maxAllowedPushId_=" << maxAllowedPushId.value();
    return false;
  }
  return true;
}
} // namespace

template <class T>
class HQFramerTestFixture : public T {
 public:
  HQFramerTestFixture() {
  }

  template <typename Func, typename... Args>
  void parse(ParseResult parseError,
             Func&& parseFn,
             FrameHeader& outHeader,
             Args&&... outArgs) {
    parse(parseError,
          queue_.front(),
          parseFn,
          outHeader,
          std::forward<Args>(outArgs)...);
  }

  template <typename Func, typename... Args>
  void parse(ParseResult parseError,
             const IOBuf* data,
             Func&& parseFn,
             FrameHeader& outHeader,
             Args&&... outArgs) {
    Cursor cursor(data);
    size_t prevLen = cursor.totalLength();

    auto type = quic::decodeQuicInteger(cursor);
    ASSERT_TRUE(type.hasValue());
    outHeader.type = FrameType(type->first);
    auto length = quic::decodeQuicInteger(cursor);
    ASSERT_TRUE(length.hasValue());
    outHeader.length = length->first;

    auto readBytes = prevLen - cursor.totalLength();
    ASSERT_LE(type->second + length->second, kMaxFrameHeaderSize);
    ASSERT_EQ(type->second + length->second, readBytes);
    auto ret2 = (*parseFn)(cursor, outHeader, std::forward<Args>(outArgs)...);
    ASSERT_EQ(ret2, parseError);
  }

  IOBufQueue queue_{IOBufQueue::cacheChainLength()};
};

class HQFramerTest : public HQFramerTestFixture<testing::Test> {};

TEST_F(HQFramerTest, TestValidPushId) {
  PushId maxValidPushId = 10;
  PushId validPushId = 9;
  PushId exceedingPushId = 11;

  auto expectValid = isValidPushId(maxValidPushId, validPushId);
  EXPECT_TRUE(expectValid);

  auto expectTooLarge = isValidPushId(maxValidPushId, exceedingPushId);
  EXPECT_FALSE(expectTooLarge);

  auto expectMatching = isValidPushId(maxValidPushId, maxValidPushId);
  EXPECT_TRUE(expectMatching);

  auto expectEmpty = isValidPushId(folly::none, validPushId);
  EXPECT_FALSE(expectEmpty);
}

TEST_F(HQFramerTest, TestWriteFrameHeaderManual) {
  auto res = writeFrameHeaderManual(
      queue_, 0, static_cast<uint8_t>(proxygen::hq::FrameType::DATA));
  EXPECT_EQ(res, 2);
}

TEST_F(HQFramerTest, TestWriteUnframedBytes) {
  auto data = IOBuf::copyBuffer("I just met you and this is crazy.");
  auto dataLen = data->length();
  auto res = data->computeChainDataLength();
  EXPECT_EQ(res, dataLen);
  queue_.append(std::move(data));
  EXPECT_EQ("I just met you and this is crazy.",
            queue_.front()->clone()->moveToFbString().toStdString());
}

TEST_F(HQFramerTest, DataFrameZeroLength) {
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(proxygen::hq::FrameType::DATA), 0);
  FrameHeader outHeader;
  std::unique_ptr<IOBuf> outBuf;
  Cursor cursor(queue_.front());
  parse(folly::none, parseData, outHeader, outBuf);
}

TEST_F(HQFramerTest, TestWriteGreaseFrame) {
  auto res = writeGreaseFrame(queue_);
  EXPECT_FALSE(res.hasError());

  Cursor cursor(queue_.front());
  auto type = quic::decodeQuicInteger(cursor);
  EXPECT_TRUE(type.hasValue());
  EXPECT_TRUE(hq::isGreaseId(type->first));

  auto length = quic::decodeQuicInteger(cursor);
  EXPECT_TRUE(length.hasValue());
  EXPECT_EQ(length->first, 0);
}

TEST_F(HQFramerTest, TestWriteWebTransportStreamPreface) {
  auto streamType = hq::WebTransportStreamType::UNI;
  auto wtSessionId = 1977;
  auto res = writeWTStreamPreface(queue_, streamType, wtSessionId);
  EXPECT_FALSE(res.hasError());

  Cursor cursor(queue_.front());
  auto type = quic::decodeQuicInteger(cursor);
  EXPECT_TRUE(type.hasValue());
  EXPECT_EQ(type->first,
            folly::to_underlying(hq::UnidirectionalStreamType::WEBTRANSPORT));

  auto sessionId = quic::decodeQuicInteger(cursor);
  EXPECT_TRUE(sessionId.hasValue());
  EXPECT_EQ(sessionId->first, wtSessionId);
}

struct FrameHeaderLengthParams {
  uint8_t headerLength;
  ParseResult error;
};

struct DataOnlyFrameParams {
  proxygen::hq::FrameType type;
  WriteResult (*writeFn)(folly::IOBufQueue&, std::unique_ptr<folly::IOBuf>);
  ParseResult (*parseFn)(Cursor& cursor,
                         const FrameHeader&,
                         std::unique_ptr<folly::IOBuf>&);
  HTTP3::ErrorCode error;
};

class HQFramerTestDataOnlyFrames
    : public HQFramerTestFixture<TestWithParam<DataOnlyFrameParams>> {};

TEST_P(HQFramerTestDataOnlyFrames, TestDataOnlyFrame) {
  // Test writing and parsing a valid frame
  auto data = makeBuf(500);
  auto res = GetParam().writeFn(queue_, data->clone());
  EXPECT_FALSE(res.hasError());
  FrameHeader header;
  std::unique_ptr<IOBuf> outBuf;
  parse(folly::none, GetParam().parseFn, header, outBuf);
  EXPECT_EQ(outBuf->moveToFbString(), data->moveToFbString());
}

INSTANTIATE_TEST_SUITE_P(
    DataOnlyFrameWriteParseTests,
    HQFramerTestDataOnlyFrames,
    Values((DataOnlyFrameParams){proxygen::hq::FrameType::DATA,
                                 writeData,
                                 parseData,
                                 HTTP3::ErrorCode::HTTP_FRAME_ERROR},
           (DataOnlyFrameParams){proxygen::hq::FrameType::HEADERS,
                                 writeHeaders,
                                 parseHeaders,
                                 HTTP3::ErrorCode::HTTP_FRAME_ERROR}));

TEST_F(HQFramerTest, ParsePushPromiseFrameOK) {
  auto data = makeBuf(1000);
  PushId inPushId = 4563;
  auto result = writePushPromise(queue_, inPushId, data->clone());
  EXPECT_FALSE(result.hasError());

  FrameHeader outHeader;
  PushId outPushId;
  std::unique_ptr<IOBuf> outBuf;
  parse(folly::none, parsePushPromise, outHeader, outPushId, outBuf);
  EXPECT_EQ(outPushId, inPushId);
  EXPECT_EQ(outBuf->moveToFbString(), data->moveToFbString());
}

struct IdOnlyFrameParams {
  proxygen::hq::FrameType type;
  WriteResult (*writeFn)(folly::IOBufQueue&, uint64_t);
  ParseResult (*parseFn)(Cursor& cursor, const FrameHeader&, uint64_t&);
  HTTP3::ErrorCode error;
};

class HQFramerTestIdOnlyFrames
    : public HQFramerTestFixture<TestWithParam<IdOnlyFrameParams>> {};

TEST_P(HQFramerTestIdOnlyFrames, TestIdOnlyFrame) {
  // test writing a valid ID
  {
    queue_.move();
    uint64_t validVarLenInt = 123456;
    auto result = GetParam().writeFn(queue_, validVarLenInt);
    EXPECT_FALSE(result.hasError());

    FrameHeader header;
    uint64_t outId;
    parse(folly::none, GetParam().parseFn, header, outId);
    EXPECT_EQ(GetParam().type, header.type);
    EXPECT_EQ(validVarLenInt, outId);
  }

  // test writing an invalid ID
  {
    queue_.move();
    uint64_t invalidVarLenInt = std::numeric_limits<uint64_t>::max();
    auto result = GetParam().writeFn(queue_, invalidVarLenInt);
    EXPECT_TRUE(result.hasError());
  }
  // test writing a valid ID, then modifying to make the parser try to read
  // too much data
  {
    queue_.move();
    uint64_t validVarLenInt = 63; // requires just 1 byte
    auto result = GetParam().writeFn(queue_, validVarLenInt);
    EXPECT_FALSE(result.hasError());

    // modify one byte in the buf
    auto buf = queue_.move();
    buf->coalesce();
    RWPrivateCursor wcursor(buf.get());
    // 2 bytes frame header (payload length is just 1)
    wcursor.skip(2);
    wcursor.writeBE<uint8_t>(0x42); // this varint requires two bytes
    queue_.append(std::move(buf));

    FrameHeader header;
    uint64_t outId;
    parse(GetParam().error, GetParam().parseFn, header, outId);
  }

  {
    queue_.move();
    uint64_t id = 3; // requires just 1 byte
    auto result = GetParam().writeFn(queue_, id);
    EXPECT_FALSE(result.hasError());

    // Trim the frame header off
    queue_.trimStart(2);
    auto buf = queue_.move();
    // Put in a new frame header (too long)
    auto badLength = buf->computeChainDataLength() - 1 + 4;
    writeFrameHeaderManual(
        queue_, static_cast<uint64_t>(GetParam().type), badLength);
    // clip the bad frame length
    queue_.trimEnd(1);
    queue_.append(std::move(buf));
    queue_.append(IOBuf::copyBuffer("junk"));

    FrameHeader header;
    uint64_t outId;
    parse(GetParam().error, GetParam().parseFn, header, outId);
  }
}

INSTANTIATE_TEST_SUITE_P(
    IdOnlyFrameWriteParseTests,
    HQFramerTestIdOnlyFrames,
    Values((IdOnlyFrameParams){proxygen::hq::FrameType::CANCEL_PUSH,
                               writeCancelPush,
                               parseCancelPush,
                               HTTP3::ErrorCode::HTTP_FRAME_ERROR},
           (IdOnlyFrameParams){proxygen::hq::FrameType::GOAWAY,
                               writeGoaway,
                               parseGoaway,
                               HTTP3::ErrorCode::HTTP_FRAME_ERROR},
           (IdOnlyFrameParams){proxygen::hq::FrameType::MAX_PUSH_ID,
                               writeMaxPushId,
                               parseMaxPushId,
                               HTTP3::ErrorCode::HTTP_FRAME_ERROR}));

TEST_F(HQFramerTest, SettingsFrameOK) {
  deque<hq::SettingPair> settings = {
      {hq::SettingId::MAX_HEADER_LIST_SIZE, (SettingValue)4},
      // Unknown IDs get ignored, and identifiers of the format
      // "0x1f * N + 0x21" are reserved exactly for this
      {(hq::SettingId)*getGreaseId(kMaxGreaseIdIndex), (SettingValue)5}};
  writeSettings(queue_, settings);

  FrameHeader header;
  std::deque<hq::SettingPair> outSettings;
  parse(folly::none, &parseSettings, header, outSettings);

  ASSERT_EQ(proxygen::hq::FrameType::SETTINGS, header.type);
  // Remove the last setting before comparison,
  // it must be ignored since it's a GREASE setting
  settings.pop_back();
  ASSERT_EQ(settings, outSettings);
}

TEST_F(HQFramerTest, MaxPushIdFrameOK) {
  PushId maxPushId = 10;
  writeMaxPushId(queue_, maxPushId);

  FrameHeader header;
  PushId resultingPushId;
  parse(folly::none, &parseMaxPushId, header, resultingPushId);

  // Ensure header of frame and Push ID value are equivalent
  // when writing and parsing
  ASSERT_EQ(proxygen::hq::FrameType::MAX_PUSH_ID, header.type);
  ASSERT_EQ(maxPushId, resultingPushId);
}

TEST_F(HQFramerTest, MaxPushIdFrameLargePushId) {
  // Test with largest possible number
  PushId maxPushId = quic::kEightByteLimit;
  writeMaxPushId(queue_, maxPushId);

  FrameHeader header;
  PushId resultingPushId;
  parse(folly::none, &parseMaxPushId, header, resultingPushId);

  // Ensure header of frame and Push ID value are equivalent
  // when writing and parsing
  ASSERT_EQ(proxygen::hq::FrameType::MAX_PUSH_ID, header.type);
  ASSERT_EQ(maxPushId, resultingPushId);
}

TEST_F(HQFramerTest, MaxPushIdTooLarge) {
  // Test kEightByteLimit + 1 as over the limit
  PushId maxPushId = (quic::kEightByteLimit + 1);
  auto res = writeMaxPushId(queue_, maxPushId);

  ASSERT_TRUE(res.hasError());
}

struct SettingsValuesParams {
  hq::SettingId id;
  hq::SettingValue value;
  bool allowed;
};

class HQFramerTestSettingsValues
    : public HQFramerTestFixture<TestWithParam<SettingsValuesParams>> {};

TEST_P(HQFramerTestSettingsValues, ValueAllowed) {
  deque<hq::SettingPair> settings = {{GetParam().id, GetParam().value}};

  writeSettings(queue_, settings);

  FrameHeader header;
  std::deque<hq::SettingPair> outSettings;
  ParseResult expectedParseResult = folly::none;
  if (!GetParam().allowed) {
    expectedParseResult = HTTP3::ErrorCode::HTTP_FRAME_ERROR;
  }
  parse(expectedParseResult, &parseSettings, header, outSettings);

  ASSERT_EQ(proxygen::hq::FrameType::SETTINGS, header.type);
  if (GetParam().allowed) {
    ASSERT_EQ(settings, outSettings);
  } else {
    ASSERT_EQ(outSettings.size(), 0);
  }
}

INSTANTIATE_TEST_SUITE_P(
    SettingsValuesAllowedTests,
    HQFramerTestSettingsValues,
    Values((SettingsValuesParams){hq::SettingId::MAX_HEADER_LIST_SIZE, 0, true},
           (SettingsValuesParams){hq::SettingId::MAX_HEADER_LIST_SIZE,
                                  std::numeric_limits<uint32_t>::max(),
                                  true},
           (SettingsValuesParams){
               hq::SettingId::QPACK_BLOCKED_STREAMS, 0, true},
           (SettingsValuesParams){hq::SettingId::QPACK_BLOCKED_STREAMS,
                                  std::numeric_limits<uint32_t>::max(),
                                  true},
           (SettingsValuesParams){hq::SettingId::HEADER_TABLE_SIZE, 0, true},
           (SettingsValuesParams){hq::SettingId::HEADER_TABLE_SIZE,
                                  std::numeric_limits<uint32_t>::max(),
                                  true}));

TEST_F(HQFramerTest, SettingsFrameEmpty) {
  const deque<hq::SettingPair> settings = {};
  writeSettings(queue_, settings);

  FrameHeader header;
  std::deque<hq::SettingPair> outSettings;
  parse(folly::none, &parseSettings, header, outSettings);

  ASSERT_EQ(proxygen::hq::FrameType::SETTINGS, header.type);
  ASSERT_EQ(settings, outSettings);
}

TEST_F(HQFramerTest, SettingsFrameTrailingJunk) {
  deque<hq::SettingPair> settings = {
      {hq::SettingId::MAX_HEADER_LIST_SIZE, (SettingValue)4},
      // Unknown IDs get ignored, and identifiers of the format
      // "0x1f * N + 0x21" are reserved exactly for this
      {(hq::SettingId)*getGreaseId(1234), (SettingValue)5}};
  writeSettings(queue_, settings);
  // Trim the frame header off
  queue_.trimStart(2);
  auto buf = queue_.move();
  // Put in a new frame header (too long)
  auto badLength = buf->computeChainDataLength() + 1;
  writeFrameHeaderManual(
      queue_, static_cast<uint64_t>(FrameType::SETTINGS), badLength);
  queue_.append(std::move(buf));
  queue_.append(IOBuf::copyBuffer("j"));

  FrameHeader header;
  std::deque<hq::SettingPair> outSettings;
  parse(
      HTTP3::ErrorCode::HTTP_FRAME_ERROR, &parseSettings, header, outSettings);
}

TEST_F(HQFramerTest, SettingsFrameWriteError) {
  deque<hq::SettingPair> settings = {
      {(hq::SettingId)*getGreaseId(54321),
       SettingValue(std::numeric_limits<uint64_t>::max())}};
  auto res = writeSettings(queue_, settings);
  ASSERT_TRUE(res.hasError());
}

TEST_F(HQFramerTest, SettingsFrameUnknownId) {
  deque<hq::SettingPair> settings = {
      {(hq::SettingId)0x1234, SettingValue(100000)}};
  writeSettings(queue_, settings);

  FrameHeader header;
  std::deque<hq::SettingPair> outSettings;
  parse(folly::none, &parseSettings, header, outSettings);

  ASSERT_EQ(proxygen::hq::FrameType::SETTINGS, header.type);
  ASSERT_TRUE(outSettings.empty());
}

TEST_F(HQFramerTest, PriorityUpdate) {
  writePriorityUpdate(queue_, 126, "u=3, i");
  FrameHeader header;
  quic::StreamId outId;
  HTTPPriority outPriority;
  parse(folly::none, &parsePriorityUpdate, header, outId, outPriority);

  EXPECT_EQ(126, outId);
  EXPECT_EQ(proxygen::hq::FrameType::FB_PRIORITY_UPDATE, header.type);
  EXPECT_EQ(3, outPriority.urgency);
  EXPECT_TRUE(outPriority.incremental);
}

TEST_F(HQFramerTest, PriorityUpdateWithoutIncremental) {
  writePriorityUpdate(queue_, 357, "u=7");
  FrameHeader header;
  quic::StreamId outId;
  HTTPPriority outPriority;
  parse(folly::none, &parsePriorityUpdate, header, outId, outPriority);

  EXPECT_EQ(357, outId);
  EXPECT_EQ(proxygen::hq::FrameType::FB_PRIORITY_UPDATE, header.type);
  EXPECT_EQ(7, outPriority.urgency);
  EXPECT_FALSE(outPriority.incremental);
}

TEST_F(HQFramerTest, BadPriorityUpdate) {
  writePriorityUpdate(queue_, 357, "b=ad");
  FrameHeader header;
  quic::StreamId outId;
  HTTPPriority outPriority;
  parse(HTTP3::ErrorCode::HTTP_FRAME_ERROR,
        &parsePriorityUpdate,
        header,
        outId,
        outPriority);
}
