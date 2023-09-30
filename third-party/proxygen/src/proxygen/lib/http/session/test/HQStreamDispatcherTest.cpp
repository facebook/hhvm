/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <deque>
#include <folly/io/Cursor.h>
#include <proxygen/lib/http/session/test/HQSessionMocks.h>
#include <proxygen/lib/http/session/test/HQSessionTestCommon.h>
#include <quic/api/test/MockQuicSocket.h>

using namespace proxygen;
using namespace quic;
using namespace testing;

class MockDispatcher
    : public HQUniStreamDispatcher::Callback
    , public HQBidiStreamDispatcher::Callback {
 public:
  using ReadCallbackAssignF =
      std::function<void(quic::StreamId, hq::UnidirectionalStreamType, size_t)>;
  using PrefaceParseF =
      std::function<folly::Optional<hq::UnidirectionalStreamType>(uint64_t)>;
  using PrefaceBidiParseF =
      std::function<folly::Optional<hq::BidirectionalStreamType>(uint64_t)>;
  using StreamRejectF = std::function<void(quic::StreamId)>;
  using NewPushStreamF =
      std::function<void(quic::StreamId, hq::PushId, size_t)>;
  using NewWTStreamF =
      std::function<void(quic::StreamId, quic::StreamId, size_t)>;
  using NewRequestF = std::function<void(quic::StreamId)>;

  explicit MockDispatcher(folly::EventBase* evb) : evb_(evb) {
  }

  void expectOnNewPushStream(const NewPushStreamF& impl) {
    auto& exp = EXPECT_CALL(
        *this, dispatchPushStream(::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectOnNewWTUni(const NewWTStreamF& impl) {
    auto& exp = EXPECT_CALL(
        *this, dispatchUniWTStream(::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectOnNewWTBidi(const NewWTStreamF& impl) {
    auto& exp = EXPECT_CALL(
        *this, dispatchBidiWTStream(::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectOnNewRequest(const NewRequestF& impl) {
    auto& exp = EXPECT_CALL(*this, dispatchRequestStream(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectAssignReadCallback(const ReadCallbackAssignF& impl) {
    auto& exp = EXPECT_CALL(
        *this, dispatchControlStream(::testing::_, ::testing::_, ::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  void expectParsePreface(const PrefaceParseF& impl) {
    auto& exp = EXPECT_CALL(*this, parseUniStreamPreface(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl)).RetiresOnSaturation();
    }
  }

  void expectParseBidiPreface(const PrefaceBidiParseF& impl) {
    auto& exp = EXPECT_CALL(*this, parseBidiStreamPreface(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl)).RetiresOnSaturation();
    }
  }

  void expectRejectStream(const StreamRejectF& impl) {
    auto& exp = EXPECT_CALL(*this, rejectStream(::testing::_));
    if (impl) {
      exp.WillOnce(::testing::Invoke(impl));
    }
  }

  folly::EventBase* getEventBase() const override {
    return evb_;
  }

  std::chrono::milliseconds getDispatchTimeout() const override {
    return std::chrono::seconds(1);
  }

  MOCK_METHOD(void, dispatchPushStream, (quic::StreamId, hq::PushId, size_t));
  MOCK_METHOD(void,
              dispatchUniWTStream,
              (quic::StreamId, quic::StreamId, size_t));
  MOCK_METHOD(void,
              dispatchControlStream,
              (quic::StreamId, hq::UnidirectionalStreamType, size_t));
  MOCK_METHOD(folly::Optional<hq::UnidirectionalStreamType>,
              parseUniStreamPreface,
              (uint64_t));
  MOCK_METHOD(void, rejectStream, (quic::StreamId));

  // Bidi methods
  MOCK_METHOD(folly::Optional<hq::BidirectionalStreamType>,
              parseBidiStreamPreface,
              (uint64_t));

  MOCK_METHOD(void, dispatchRequestStream, (quic::StreamId));

  MOCK_METHOD(void,
              dispatchBidiWTStream,
              (quic::StreamId, quic::StreamId, size_t));
  folly::EventBase* evb_{nullptr};
};

template <typename T>
class HQStreamDispatcherTest : public Test {
 public:
  using PeekIterator = std::deque<StreamBuffer>::const_iterator;
  using PeekData = MockDispatcher::PeekData;
  using ReadError = MockDispatcher::ReadError;

  void SetUp() override {
    incomingData_.clear();
  }

  void TearDown() override {
  }

  // Different methods to feed the dispatcher

  // Encodes and sends a single encoded integer the dispatcher
  void sendData(quic::StreamId id, uint64_t val, uint8_t atLeast) {
    folly::IOBufQueue data{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&data, atLeast);
    encodeQuicIntegerWithAtLeast(val, atLeast, appender);
    sendData(id, data.move());
  }

  // Encodes preface + push id
  void sendData(quic::StreamId id,
                uint64_t preface,
                uint64_t pushId,
                uint8_t atLeast,
                uint8_t split) {
    folly::IOBufQueue data{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&data, atLeast);
    encodeQuicIntegerWithAtLeast(preface, atLeast, appender);
    encodeQuicIntegerWithAtLeast(pushId, atLeast, appender);

    for (size_t toClone = split;; toClone += split) {
      incomingData_.clear();
      folly::io::Cursor c(data.front());
      auto buf = folly::IOBuf::create(data.chainLength());
      c.cloneAtMost(buf, toClone);
      sendData(id, std::move(buf));
      if (toClone >= data.chainLength()) {
        break;
      }
    }
  }

  void sendData(quic::StreamId id, std::unique_ptr<folly::IOBuf> data) {
    incomingData_.emplace_back(std::move(data), 0, false);
    dispatcher_.onDataAvailable(
        id,
        folly::Range<PeekIterator>(incomingData_.cbegin(),
                                   incomingData_.size()));
  }

 protected:
  folly::EventBase evb_;
  std::deque<StreamBuffer> incomingData_;
  MockDispatcher dispatcherCallback_{&evb_};
  T dispatcher_{dispatcherCallback_, proxygen::TransportDirection::UPSTREAM};
};

using UnidirectionalReadDispatcherTest =
    HQStreamDispatcherTest<HQUniStreamDispatcher>;
using BidirectionalReadDispatcherTest =
    HQStreamDispatcherTest<HQBidiStreamDispatcher>;

TEST_F(UnidirectionalReadDispatcherTest, TestDispatchControlPreface) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_.expectParsePreface([&](uint64_t /* type */) {
    return hq::UnidirectionalStreamType::CONTROL;
  });

  // Expect the assign call
  dispatcherCallback_.expectAssignReadCallback(
      [&](quic::StreamId id,
          hq::UnidirectionalStreamType /* type */,
          size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(consumed, atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> assignReadCallback
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::CONTROL),
           atLeastBytes);
}

TEST_F(UnidirectionalReadDispatcherTest,
       TestDispatchPushPrefaceNewPushStreamApi) {

  quic::StreamId expectedId = 5;
  hq::PushId expectedPushId = 151234567;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  for (auto i = 0; i < 5; i++) {
    dispatcherCallback_.expectParsePreface([&](uint64_t /* type */) {
      return hq::UnidirectionalStreamType::PUSH;
    });
  }

  // Expect the assign call
  dispatcherCallback_.expectOnNewPushStream(
      [&](quic::StreamId id, hq::PushId pushId, size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(pushId, expectedPushId);
        ASSERT_EQ(consumed, atLeastBytes + atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> onNewPushId
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::PUSH),
           expectedPushId,
           atLeastBytes,
           /*split=*/1);
}

TEST_F(UnidirectionalReadDispatcherTest, TestDispatchWTUni) {

  quic::StreamId expectedId = 5;
  quic::StreamId expectedSessionId = 151234567;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  for (auto i = 0; i < 5; i++) {
    dispatcherCallback_.expectParsePreface([&](uint64_t /* type */) {
      return hq::UnidirectionalStreamType::WEBTRANSPORT;
    });
  }

  // Expect the assign call
  dispatcherCallback_.expectOnNewWTUni(
      [&](quic::StreamId id, quic::StreamId sessionId, size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(sessionId, expectedSessionId);
        ASSERT_EQ(consumed, atLeastBytes + atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> onNewWTUni
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::WEBTRANSPORT),
           expectedSessionId,
           atLeastBytes,
           /*split=*/1);
}

TEST_F(UnidirectionalReadDispatcherTest, TestRejectUnrecognizedPreface) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_.expectParsePreface(
      [&](uint64_t /* type */) { return folly::none; });

  // Expect the reject call
  dispatcherCallback_.expectRejectStream(
      [&](quic::StreamId id) { ASSERT_EQ(id, expectedId); });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> assignReadCallback
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::CONTROL),
           atLeastBytes);
}

TEST_F(BidirectionalReadDispatcherTest, TestDispatchRequest) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_.expectParseBidiPreface([&](uint64_t /* type */) {
    return hq::BidirectionalStreamType::REQUEST;
  });
  // Expect the assign call
  dispatcherCallback_.expectOnNewRequest(
      [&](quic::StreamId id) { ASSERT_EQ(id, expectedId); });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> onNewRequest
  sendData(
      expectedId, static_cast<uint64_t>(hq::FrameType::HEADERS), atLeastBytes);
}

TEST_F(BidirectionalReadDispatcherTest, TestDispatchWTBidi) {

  quic::StreamId expectedId = 5;
  quic::StreamId expectedSessionId = 151234567;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  for (auto i = 0; i < 5; i++) {
    dispatcherCallback_.expectParseBidiPreface([&](uint64_t /* type */) {
      return hq::BidirectionalStreamType::WEBTRANSPORT;
    });
  }
  // Expect the assign call
  dispatcherCallback_.expectOnNewWTBidi(
      [&](quic::StreamId id, quic::StreamId sessionId, size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(sessionId, expectedSessionId);
        ASSERT_EQ(consumed, atLeastBytes + atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> onNewWTBidi
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::WEBTRANSPORT),
           expectedSessionId,
           atLeastBytes,
           /*split=*/1);
}

TEST_F(BidirectionalReadDispatcherTest, TestRejectUnrecognizedPreface) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_.expectParseBidiPreface(
      [&](uint64_t /* type */) { return folly::none; });

  // Expect the reject call
  dispatcherCallback_.expectRejectStream(
      [&](quic::StreamId id) { ASSERT_EQ(id, expectedId); });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_.takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> assignReadCallback
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::WEBTRANSPORT),
           atLeastBytes);
}
