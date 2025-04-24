/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <gmock/gmock.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <quic/codec/QuicInteger.h>

using namespace proxygen;
using namespace folly;
using namespace testing;

class TestCapsuleCodecCallback : public CapsuleCodec::Callback {
 public:
  MOCK_METHOD(void, onCapsule, (uint64_t, uint64_t)); // parses type + length
  MOCK_METHOD(void, onConnectionError, (CapsuleCodec::ErrorCode));
  MOCK_METHOD(void,
              onStringCapsule,
              (uint64_t, uint64_t, std::string)); // parses payload
};

class TestCapsuleCodec : public CapsuleCodec {
 public:
  explicit TestCapsuleCodec(TestCapsuleCodecCallback* callback = nullptr)
      : CapsuleCodec(callback) {
  }

  bool canParseCapsule(uint64_t capsuleType) override {
    return capsuleType == 0x01;
  }

  folly::Expected<folly::Unit, ErrorCode> parseCapsule(
      io::Cursor& cursor) override {
    if (cursor.length() > curCapsuleLength_) {
      VLOG(4) << "remainingLength > curCapsuleLength_";
      return folly::makeUnexpected(ErrorCode::PARSE_UNDERFLOW);
    }
    auto payload = cursor.readFixedString(curCapsuleLength_);
    static_cast<TestCapsuleCodecCallback*>(callback_)->onStringCapsule(
        curCapsuleType_, curCapsuleLength_, payload);
    return folly::unit;
  }
};

class CapsuleCodecTest : public Test {
 protected:
  void SetUp() override {
    callback_ = std::make_unique<TestCapsuleCodecCallback>();
    codec_ = std::make_unique<TestCapsuleCodec>(callback_.get());
  }
  std::unique_ptr<TestCapsuleCodecCallback> callback_;
  std::unique_ptr<TestCapsuleCodec> codec_;
};

quic::BufQueue generateStringCapsule(uint64_t type,
                                     uint64_t length,
                                     const std::string& value) {
  auto buf = folly::IOBuf::create(1024);
  quic::BufAppender appender(buf.get(), 1024);
  quic::encodeQuicInteger(type, [&](auto val) { appender.writeBE(val); });
  quic::encodeQuicInteger(length, [&](auto val) { appender.writeBE(val); });
  quic::BufQueue queue(std::move(buf));
  auto str = folly::IOBuf::copyBuffer(value);
  queue.append(std::move(str));
  return queue;
}

TEST_F(CapsuleCodecTest, ValidCapsuleParsing) {
  // encode type=0x01, length=5, payload="AAAAA"
  auto capsule = generateStringCapsule(0x01, 5, "AAAAA");
  EXPECT_CALL(*callback_, onCapsule(0x01, 5));
  EXPECT_CALL(*callback_, onStringCapsule(0x1, 5, "AAAAA"));
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, InvalidCapsuleType) {
  // partial QUIC integer for capsule type (1 byte of a 2-byte header)
  auto partialType = folly::IOBuf::copyBuffer(R"(@)"); // 0x40 = 2-byte integer
  quic::BufQueue capsule(std::move(partialType));
  EXPECT_CALL(*callback_,
              onConnectionError(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW));
  EXPECT_CALL(*callback_, onCapsule(_, _)).Times(0);
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, TooLargePayloadAndEom) {
  auto capsule = generateStringCapsule(0x01, 5, "Invalid Payload");
  EXPECT_CALL(*callback_, onCapsule(0x01, 5));
  EXPECT_CALL(*callback_,
              onConnectionError(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW));
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, ParseUnderflowInvalidPayload) {
  auto capsule = generateStringCapsule(0x01, 5, "BAD");
  EXPECT_CALL(*callback_, onCapsule(0x01, 5));
  EXPECT_CALL(*callback_,
              onConnectionError(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW));
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, ParseUnderflowLength) {
  // Encode type=0x01, missing length
  auto buf = folly::IOBuf::create(1024);
  quic::BufAppender appender(buf.get(), 1024);
  quic::encodeQuicInteger(0x01, [&](auto val) { appender.writeBE(val); });
  quic::BufQueue capsule(std::move(buf));
  EXPECT_CALL(*callback_, onCapsule(_, _)).Times(0);
  EXPECT_CALL(*callback_,
              onConnectionError(CapsuleCodec::ErrorCode::PARSE_UNDERFLOW));
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, SkipCapsule) {
  auto capsule = generateStringCapsule(0x02, 5, "AAAAA");
  EXPECT_CALL(*callback_, onCapsule(0x02, 5));
  EXPECT_CALL(*callback_, onStringCapsule(_, _, _)).Times(0); // skipping this
  codec_->onIngress(capsule.clone(), true);
}

TEST_F(CapsuleCodecTest, SkipCapsuleMultiple) {
  // encode type=0x02, length=5, payload="AAAAA"
  auto capsule1 = generateStringCapsule(0x02, 5, "AAAAA");
  EXPECT_CALL(*callback_, onCapsule(0x02, 5));
  EXPECT_CALL(*callback_, onStringCapsule(_, _, _)).Times(0); // skipping this
  codec_->onIngress(capsule1.clone(), true);
  // encode type=0x01, length=3, payload="BBB"
  auto capsule2 = generateStringCapsule(0x01, 3, "BBB");
  EXPECT_CALL(*callback_, onCapsule(0x01, 3));
  EXPECT_CALL(*callback_, onStringCapsule(0x01, 3, "BBB"));
  codec_->onIngress(capsule2.clone(), true);
}

TEST_F(CapsuleCodecTest, ParseUnderflowAndNotEom) {
  // encode type=0x01, length=5, payload="AAA", false EOMs before true EOM
  auto buf = folly::IOBuf::create(1024);
  quic::BufAppender appender(buf.get(), 1024);
  quic::encodeQuicInteger(0x01, [&](auto val) { appender.writeBE(val); });
  quic::encodeQuicInteger(3, [&](auto val) { appender.writeBE(val); });
  quic::BufQueue capsule1(std::move(buf));
  EXPECT_CALL(*callback_, onCapsule(0x01, 3));
  EXPECT_CALL(*callback_, onConnectionError(_)).Times(0);
  codec_->onIngress(capsule1.clone(), false);

  auto buf2 = folly::IOBuf::create(1024);
  quic::BufQueue capsule2(std::move(buf2));
  auto str = folly::IOBuf::copyBuffer("AA");
  capsule2.append(std::move(str));
  codec_->onIngress(capsule2.clone(), false);

  auto buf3 = folly::IOBuf::create(1024);
  quic::BufQueue capsule3(std::move(buf3));
  auto str3 = folly::IOBuf::copyBuffer("A");
  capsule3.append(std::move(str3));
  EXPECT_CALL(*callback_, onStringCapsule(0x01, 3, "AAA"));
  codec_->onIngress(capsule3.clone(), true);
}

TEST_F(CapsuleCodecTest, MultipleCapsules) {
  // first capsule: type=0x01, length=5, payload="AAAAA"
  auto capsule1 = generateStringCapsule(0x01, 5, "AAAAA");
  // second capsule: type=0x01, length=3, payload="BBB"
  auto capsule2 = generateStringCapsule(0x01, 3, "BBB");
  capsule1.append(capsule2.move()); // combine queues

  EXPECT_CALL(*callback_, onCapsule(0x01, 5));
  EXPECT_CALL(*callback_, onStringCapsule(0x01, 5, "AAAAA"));
  EXPECT_CALL(*callback_, onCapsule(0x01, 3));
  EXPECT_CALL(*callback_, onStringCapsule(0x01, 3, "BBB"));
  codec_->onIngress(capsule1.clone(), true);
}
