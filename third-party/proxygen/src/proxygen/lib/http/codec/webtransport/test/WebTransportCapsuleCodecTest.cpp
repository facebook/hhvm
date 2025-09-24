/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <gmock/gmock.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>

using namespace proxygen;
using namespace testing;

class WebTransportCapsuleCodecTest : public Test {
 protected:
  void SetUp() override {
    h2_codec_ = std::make_unique<WebTransportCapsuleCodec>(&callback_,
                                                           CodecVersion::H2);
    h3_codec_ = std::make_unique<WebTransportCapsuleCodec>(&callback_,
                                                           CodecVersion::H3);
  }

  class TestWebTransportCapsuleCodecCallback
      : public WebTransportCapsuleCodec::Callback {
   public:
    MOCK_METHOD(void, onPaddingCapsule, (PaddingCapsule), (override));
    MOCK_METHOD(void,
                onWTResetStreamCapsule,
                (WTResetStreamCapsule),
                (override));
    MOCK_METHOD(void,
                onWTStopSendingCapsule,
                (WTStopSendingCapsule),
                (override));
    MOCK_METHOD(void, onWTStreamCapsule, (WTStreamCapsule), (override));
    MOCK_METHOD(void, onWTMaxDataCapsule, (WTMaxDataCapsule), (override));
    MOCK_METHOD(void,
                onWTMaxStreamDataCapsule,
                (WTMaxStreamDataCapsule),
                (override));
    MOCK_METHOD(void,
                onWTMaxStreamsBidiCapsule,
                (WTMaxStreamsCapsule),
                (override));
    MOCK_METHOD(void,
                onWTMaxStreamsUniCapsule,
                (WTMaxStreamsCapsule),
                (override));
    MOCK_METHOD(void,
                onWTDataBlockedCapsule,
                (WTDataBlockedCapsule),
                (override));
    MOCK_METHOD(void,
                onWTStreamDataBlockedCapsule,
                (WTStreamDataBlockedCapsule),
                (override));
    MOCK_METHOD(void,
                onWTStreamsBlockedBidiCapsule,
                (WTStreamsBlockedCapsule),
                (override));
    MOCK_METHOD(void,
                onWTStreamsBlockedUniCapsule,
                (WTStreamsBlockedCapsule),
                (override));
    MOCK_METHOD(void, onDatagramCapsule, (DatagramCapsule), (override));
    MOCK_METHOD(void,
                onCloseWebTransportSessionCapsule,
                (CloseWebTransportSessionCapsule),
                (override));
    MOCK_METHOD(void,
                onDrainWebTransportSessionCapsule,
                (DrainWebTransportSessionCapsule),
                (override));
    MOCK_METHOD(void, onConnectionError, (CapsuleCodec::ErrorCode), (override));
  };

  // Simulate PARSE_UNDERFLOW by modifying buf to signify 8-byte varint. For all
  // tests, the first payload byte will be the fifth byte due to all types being
  // 4 bytes and the length being 1 byte.
  void testParseUnderflow(CapsuleCodec::ErrorCode err, CodecVersion version) {
    auto buf = queue_.move();
    buf->writableData()[5] = 0xFF;
    EXPECT_CALL(callback_, onConnectionError(_))
        .WillOnce(Invoke([&](auto errorCode) { EXPECT_EQ(errorCode, err); }));
    auto codec = (version == CodecVersion::H2) ? std::move(h2_codec_)
                                               : std::move(h3_codec_);
    codec->onIngress(std::move(buf), true);
  }

  std::unique_ptr<WebTransportCapsuleCodec> h2_codec_;
  std::unique_ptr<WebTransportCapsuleCodec> h3_codec_;
  TestWebTransportCapsuleCodecCallback callback_;
  folly::IOBufQueue queue_;
};

TEST_F(WebTransportCapsuleCodecTest, OnPaddingCapsule) {
  PaddingCapsule capsule{100};
  writePadding(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onPaddingCapsule(_))
      .WillOnce(Invoke([&](PaddingCapsule capsule) {
        EXPECT_EQ(capsule.paddingLength, 100);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTResetStreamCapsule) {
  WTResetStreamCapsule capsule{
      .streamId = 1, .appProtocolErrorCode = 2, .reliableSize = 3};
  writeWTResetStream(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTResetStreamCapsule(_))
      .WillOnce(Invoke([&](WTResetStreamCapsule capsule) {
        EXPECT_EQ(capsule.streamId, 1);
        EXPECT_EQ(capsule.appProtocolErrorCode, 2);
        EXPECT_EQ(capsule.reliableSize, 3);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTResetStreamCapsuleError) {
  WTResetStreamCapsule capsule{
      .streamId = 1,
      .appProtocolErrorCode = std::numeric_limits<uint32_t>::max(),
      .reliableSize = std::numeric_limits<uint32_t>::max()};
  writeWTResetStream(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStopSendingCapsule) {
  WTStopSendingCapsule capsule{.streamId = 1, .appProtocolErrorCode = 2};
  writeWTStopSending(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStopSendingCapsule(_))
      .WillOnce(Invoke([&](WTStopSendingCapsule capsule) {
        EXPECT_EQ(capsule.streamId, 1);
        EXPECT_EQ(capsule.appProtocolErrorCode, 2);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStopSendingCapsuleError) {
  WTStopSendingCapsule capsule{.streamId = 1,
                               .appProtocolErrorCode =
                                   std::numeric_limits<uint32_t>::max()};
  writeWTStopSending(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStreamCapsule) {
  WTStreamCapsule capsule{.streamId = 1,
                          .streamData = folly::IOBuf::copyBuffer("roast beef"),
                          .fin = true};
  writeWTStream(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamCapsule(_))
      .WillOnce(Invoke([&](WTStreamCapsule capsule) {
        EXPECT_EQ(capsule.streamId, 1);
        EXPECT_EQ(capsule.streamData->moveToFbString().toStdString(),
                  "roast beef");
        EXPECT_TRUE(capsule.fin);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStreamCapsuleError) {
  WTStreamCapsule capsule{.streamId = 1,
                          .streamData = folly::IOBuf::copyBuffer("roast"),
                          .fin = true};
  writeWTStream(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxDataCapsule) {
  WTMaxDataCapsule capsule{100};
  writeWTMaxData(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxDataCapsule(_))
      .WillOnce(Invoke([&](WTMaxDataCapsule capsule) {
        EXPECT_EQ(capsule.maximumData, 100);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxDataCapsule) {
  WTMaxDataCapsule capsule{100};
  writeWTMaxData(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxDataCapsule(_))
      .WillOnce(Invoke([&](WTMaxDataCapsule capsule) {
        EXPECT_EQ(capsule.maximumData, 100);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxDataCapsuleError) {
  WTMaxDataCapsule capsule{100};
  writeWTMaxData(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxDataCapsuleError) {
  WTMaxDataCapsule capsule{100};
  writeWTMaxData(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTMaxStreamDataCapsule) {
  WTMaxStreamDataCapsule capsule{.streamId = 1, .maximumStreamData = 100};
  writeWTMaxStreamData(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxStreamDataCapsule(_))
      .WillOnce(Invoke([&](WTMaxStreamDataCapsule capsule) {
        EXPECT_EQ(capsule.streamId, 1);
        EXPECT_EQ(capsule.maximumStreamData, 100);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTMaxStreamDataCapsuleError) {
  WTMaxStreamDataCapsule capsule{
      .streamId = 1, .maximumStreamData = std::numeric_limits<uint32_t>::max()};
  writeWTMaxStreamData(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxStreamsCapsule) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, true);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxStreamsBidiCapsule(_))
      .WillOnce(Invoke([&](WTMaxStreamsCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 200);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxStreamsCapsule) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, true);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxStreamsBidiCapsule(_))
      .WillOnce(Invoke([&](WTMaxStreamsCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 200);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxStreamsCapsuleError) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, true);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxStreamsCapsuleError) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, true);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxStreamsUniCapsule) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, false);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxStreamsUniCapsule(_))
      .WillOnce(Invoke([&](WTMaxStreamsCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 200);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxStreamsUniCapsule) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, false);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTMaxStreamsUniCapsule(_))
      .WillOnce(Invoke([&](WTMaxStreamsCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 200);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTMaxStreamsUniCapsuleError) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, false);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTMaxStreamsUniCapsuleError) {
  WTMaxStreamsCapsule capsule{200};
  writeWTMaxStreams(queue_, capsule, false);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnDataBlockedCapsule) {
  WTDataBlockedCapsule capsule{300};
  writeWTDataBlocked(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTDataBlockedCapsule(_))
      .WillOnce(Invoke([&](WTDataBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumData, 300);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnDataBlockedCapsule) {
  WTDataBlockedCapsule capsule{300};
  writeWTDataBlocked(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTDataBlockedCapsule(_))
      .WillOnce(Invoke([&](WTDataBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumData, 300);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnDataBlockedCapsuleError) {
  WTDataBlockedCapsule capsule{300};
  writeWTDataBlocked(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnDataBlockedCapsuleError) {
  WTDataBlockedCapsule capsule{300};
  writeWTDataBlocked(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTStreamsBlockedCapsule) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, true);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamsBlockedBidiCapsule(_))
      .WillOnce(Invoke([&](WTStreamsBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 400);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTStreamsBlockedCapsule) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, true);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamsBlockedBidiCapsule(_))
      .WillOnce(Invoke([&](WTStreamsBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 400);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTStreamsBlockedCapsuleError) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, true);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTStreamsBlockedCapsuleError) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, true);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTStreamsBlockedUniCapsule) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, false);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamsBlockedUniCapsule(_))
      .WillOnce(Invoke([&](WTStreamsBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 400);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTStreamsBlockedUniCapsule) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, false);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamsBlockedUniCapsule(_))
      .WillOnce(Invoke([&](WTStreamsBlockedCapsule capsule) {
        EXPECT_EQ(capsule.maximumStreams, 400);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnWTStreamsBlockedUniCapsuleError) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, false);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnWTStreamsBlockedUniCapsuleError) {
  WTStreamsBlockedCapsule capsule{400};
  writeWTStreamsBlocked(queue_, capsule, false);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStreamDataBlockedCapsule) {
  WTStreamDataBlockedCapsule capsule{.streamId = 1, .maximumStreamData = 100};
  writeWTStreamDataBlocked(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onWTStreamDataBlockedCapsule(_))
      .WillOnce(Invoke([&](WTStreamDataBlockedCapsule capsule) {
        EXPECT_EQ(capsule.streamId, 1);
        EXPECT_EQ(capsule.maximumStreamData, 100);
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, OnWTStreamDataBlockedCapsuleError) {
  WTStreamDataBlockedCapsule capsule{
      .streamId = 1, .maximumStreamData = std::numeric_limits<uint32_t>::max()};
  writeWTStreamDataBlocked(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, OnDatagramCapsule) {
  DatagramCapsule capsule{folly::IOBuf::copyBuffer("breakfast special")};
  writeDatagram(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onDatagramCapsule(_))
      .WillOnce(Invoke([&](DatagramCapsule capsule) {
        EXPECT_EQ(capsule.httpDatagramPayload->moveToFbString().toStdString(),
                  "breakfast special");
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnCloseWebTransportSessionCapsule) {
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 500,
                                          .applicationErrorMessage = "BAD!"};
  writeCloseWebTransportSession(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onCloseWebTransportSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        EXPECT_EQ(capsule.applicationErrorCode, 500);
        EXPECT_EQ(capsule.applicationErrorMessage, "BAD!");
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnCloseWebTransportSessionCapsule) {
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 500,
                                          .applicationErrorMessage = "BAD!"};
  writeCloseWebTransportSession(queue_, capsule);

  auto buf = queue_.move();
  EXPECT_CALL(callback_, onCloseWebTransportSessionCapsule(_))
      .WillOnce(Invoke([&](const CloseWebTransportSessionCapsule& capsule) {
        EXPECT_EQ(capsule.applicationErrorCode, 500);
        EXPECT_EQ(capsule.applicationErrorMessage, "BAD!");
      }));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnCloseWebTransportSessionCapsuleError) {
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 500,
                                          .applicationErrorMessage = "BAD!"};
  writeCloseWebTransportSession(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H2);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnCloseWebTransportSessionCapsuleError) {
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 500,
                                          .applicationErrorMessage = "BAD!"};
  writeCloseWebTransportSession(queue_, capsule);
  testParseUnderflow(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW,
                     CodecVersion::H3);
}

TEST_F(WebTransportCapsuleCodecTest, H2OnDrainWebTransportSessionCapsule) {
  writeDrainWebTransportSession(queue_);
  auto buf = queue_.move();
  EXPECT_CALL(callback_, onDrainWebTransportSessionCapsule(_));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h2_codec_->onIngress(std::move(buf), true);
}

TEST_F(WebTransportCapsuleCodecTest, H3OnDrainWebTransportSessionCapsule) {
  writeDrainWebTransportSession(queue_);
  auto buf = queue_.move();
  EXPECT_CALL(callback_, onDrainWebTransportSessionCapsule(_));
  EXPECT_CALL(callback_, onConnectionError(_)).Times(0);
  h3_codec_->onIngress(std::move(buf), true);
}
