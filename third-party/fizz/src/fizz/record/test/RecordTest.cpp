/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/RecordLayer.h>
#include <fizz/record/test/Mocks.h>
#include <folly/String.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace folly;

namespace fizz {
namespace test {

class ConcreteReadRecordLayer : public PlaintextReadRecordLayer {
 public:
  MOCK_METHOD(
      ReadRecordLayer::ReadResult<TLSMessage>,
      read,
      (folly::IOBufQueue & buf, Aead::AeadOptions options));
};

class ConcreteWriteRecordLayer : public PlaintextWriteRecordLayer {
 public:
  MOCK_METHOD(
      TLSContent,
      _write,
      (TLSMessage & msg, Aead::AeadOptions options),
      (const));
  TLSContent write(TLSMessage&& msg, Aead::AeadOptions options) const override {
    return _write(msg, options);
  }
};

class RecordTest : public testing::Test {
 protected:
  StrictMock<ConcreteReadRecordLayer> read_;
  StrictMock<ConcreteWriteRecordLayer> write_;

  IOBufQueue queue_{IOBufQueue::cacheChainLength()};

  IOBufEqualTo eq_;

  static Buf getBuf(const std::string& hex) {
    auto data = unhexlify(hex);
    return IOBuf::copyBuffer(data.data(), data.size());
  }

  static void expectSame(const Buf& buf, const std::string& hex) {
    auto str = buf->moveToFbString().toStdString();
    EXPECT_EQ(hexlify(str), hex);
  }
};

TEST_F(RecordTest, TestNoData) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::none();
  }));
  EXPECT_FALSE(read_.readEvent(queue_, Aead::AeadOptions()).has_value());
  EXPECT_FALSE(read_.getRecordLayerState().key.has_value());
  EXPECT_FALSE(read_.getRecordLayerState().sequence.has_value());
}

TEST_F(RecordTest, TestReadAppData) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::application_data, IOBuf::copyBuffer("hi")});
  }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& appData = *param->asAppData();
  EXPECT_TRUE(eq_(appData.data, IOBuf::copyBuffer("hi")));
}

TEST_F(RecordTest, TestAlert) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::alert, getBuf("0202")});
  }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  EXPECT_TRUE(param->asAlert() != nullptr);
}

TEST_F(RecordTest, TestHandshake) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::handshake, getBuf("140000023232")});
  }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished = *param->asFinished();
  expectSame(finished.verify_data, "3232");
  expectSame(*finished.originalEncoding, "140000023232");
}

TEST_F(RecordTest, TestHandshakeTooLong) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::handshake, getBuf("14400000")});
  }));
  EXPECT_ANY_THROW(read_.readEvent(queue_, Aead::AeadOptions()));
}

TEST_F(RecordTest, TestHandshakeFragmentedImmediate) {
  EXPECT_CALL(read_, read(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, getBuf("14000008aabbccdd")});
      }))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, getBuf("11223344")});
      }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  EXPECT_FALSE(read_.hasUnparsedHandshakeData());
  auto& finished = *param->asFinished();
  expectSame(finished.verify_data, "aabbccdd11223344");
}

TEST_F(RecordTest, TestHandshakeFragmentedDelayed) {
  EXPECT_CALL(read_, read(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, getBuf("14000008aabbccdd")});
      }))
      .WillOnce(InvokeWithoutArgs([]() { return folly::none; }));
  EXPECT_FALSE(read_.readEvent(queue_, Aead::AeadOptions()).has_value());
  EXPECT_TRUE(read_.hasUnparsedHandshakeData());
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::handshake, getBuf("11223344")});
  }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished = *param->asFinished();
  expectSame(finished.verify_data, "aabbccdd11223344");
}

TEST_F(RecordTest, TestHandshakeCoalesced) {
  EXPECT_CALL(read_, read(_, _)).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<TLSMessage>::from(
        TLSMessage{ContentType::handshake, getBuf("14000002aabb14000002ccdd")});
  }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished = *param->asFinished();
  expectSame(finished.verify_data, "aabb");
  EXPECT_TRUE(read_.hasUnparsedHandshakeData());
  param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished2 = *param->asFinished();
  expectSame(finished2.verify_data, "ccdd");
  EXPECT_FALSE(read_.hasUnparsedHandshakeData());
}

TEST_F(RecordTest, TestHandshakeSpliced) {
  EXPECT_CALL(read_, read(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, getBuf("01000010abcd")});
      }))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::application_data, IOBuf::copyBuffer("hi")});
      }));
  EXPECT_ANY_THROW(read_.readEvent(queue_, Aead::AeadOptions()));
}

TEST_F(RecordTest, TestMultipleHandshakeMessages) {
  EXPECT_CALL(read_, read(_, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, getBuf("14000002aabb14000002")});
      }))
      .WillOnce(InvokeWithoutArgs([]() {
        // Really large message to force the record layer to
        // allocate more space as well the tail end of the previous
        // finished message
        auto message = getBuf("ccdd");
        for (size_t i = 0; i < 1000; ++i) {
          message->prependChain(getBuf("14000002aabb"));
        }
        message->coalesce();
        return ReadRecordLayer::ReadResult<TLSMessage>::from(
            TLSMessage{ContentType::handshake, std::move(message)});
      }));
  auto param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished = *param->asFinished();
  expectSame(finished.verify_data, "aabb");
  EXPECT_TRUE(read_.hasUnparsedHandshakeData());
  param = read_.readEvent(queue_, Aead::AeadOptions());
  auto& finished2 = *param->asFinished();
  expectSame(finished2.verify_data, "ccdd");
  EXPECT_TRUE(read_.hasUnparsedHandshakeData());
}

TEST_F(RecordTest, TestWriteAppData) {
  EXPECT_CALL(write_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = write_.getEncryptionLevel();
        content.data = nullptr;
        EXPECT_EQ(msg.type, ContentType::application_data);
        return content;
      }));
  write_.writeAppData(IOBuf::copyBuffer("hi"), Aead::AeadOptions());
}

TEST_F(RecordTest, TestWriteAlert) {
  EXPECT_CALL(write_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        EXPECT_EQ(msg.type, ContentType::alert);
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = write_.getEncryptionLevel();
        content.data = nullptr;
        return content;
      }));
  write_.writeAlert(Alert());
}

TEST_F(RecordTest, TestWriteHandshake) {
  EXPECT_CALL(write_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        EXPECT_EQ(msg.type, ContentType::handshake);
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = write_.getEncryptionLevel();
        content.data = nullptr;
        return content;
      }));
  write_.writeHandshake(IOBuf::copyBuffer("msg1"), IOBuf::copyBuffer("msg2"));
}

} // namespace test
} // namespace fizz
