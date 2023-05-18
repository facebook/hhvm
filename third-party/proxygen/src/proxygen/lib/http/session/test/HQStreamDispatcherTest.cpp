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

class MockDispatcher : public HQUniStreamDispatcher::Callback {
 public:
  using ReadCallbackAssignF =
      std::function<void(quic::StreamId, hq::UnidirectionalStreamType, size_t)>;
  using PrefaceParseF =
      std::function<folly::Optional<hq::UnidirectionalStreamType>(uint64_t)>;
  using StreamRejectF = std::function<void(quic::StreamId)>;
  using NewPushStreamF =
      std::function<void(quic::StreamId, hq::PushId, size_t)>;

  explicit MockDispatcher(folly::EventBase* evb) : evb_(evb) {
  }

  void expectOnNewPushStream(const NewPushStreamF& impl) {
    auto& exp = EXPECT_CALL(
        *this, dispatchPushStream(::testing::_, ::testing::_, ::testing::_));
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
      exp.WillOnce(::testing::Invoke(impl));
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

  folly::EventBase* evb_{nullptr};
};

class UnidirectionalReadDispatcherTest : public Test {
 public:
  using PeekIterator = std::deque<StreamBuffer>::const_iterator;
  using PeekData = MockDispatcher::PeekData;
  using ReadError = MockDispatcher::ReadError;

  void SetUp() override {
    incomingData_.clear();
    dispatcherCallback_ = std::make_unique<MockDispatcher>(&evb_);
    dispatcher_ = std::make_unique<HQUniStreamDispatcher>(
        *dispatcherCallback_, proxygen::TransportDirection::UPSTREAM);
  }

  void TearDown() override {
  }

  // Different methods to feed the dispatcher

  // Encodes and sends a single encoded integer the dispatcher
  void sendData(quic::StreamId id, uint64_t val, uint8_t atLeast) {
    std::unique_ptr<folly::IOBufQueue> data =
        std::make_unique<folly::IOBufQueue>();
    folly::io::QueueAppender appender(data.get(), atLeast);
    encodeQuicIntegerWithAtLeast(val, atLeast, appender);
    sendData(id, std::move(data));
  }

  // Encodes preface + push id in a single event
  void sendData(quic::StreamId id,
                uint64_t preface,
                uint64_t pushId,
                uint8_t atLeast) {
    std::unique_ptr<folly::IOBufQueue> data =
        std::make_unique<folly::IOBufQueue>();
    folly::io::QueueAppender appender(data.get(), atLeast);
    encodeQuicIntegerWithAtLeast(preface, atLeast, appender);
    encodeQuicIntegerWithAtLeast(pushId, atLeast, appender);
    sendData(id, std::move(data));
  }

  void sendData(quic::StreamId id, std::unique_ptr<folly::IOBufQueue> data) {
    auto dbuf = data->move();
    incomingData_.emplace_back(std::move(dbuf), 0, false);
    dispatcher_->onDataAvailable(
        id,
        folly::Range<PeekIterator>(incomingData_.cbegin(),
                                   incomingData_.size()));
  }

 protected:
  folly::EventBase evb_;
  std::deque<StreamBuffer> incomingData_;
  std::unique_ptr<HQUniStreamDispatcher> dispatcher_;
  std::unique_ptr<MockDispatcher> dispatcherCallback_;
};

TEST_F(UnidirectionalReadDispatcherTest, TestDispatchControlPreface) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_->expectParsePreface([&](uint64_t /* type */) {
    return hq::UnidirectionalStreamType::CONTROL;
  });

  // Expect the assign call
  dispatcherCallback_->expectAssignReadCallback(
      [&](quic::StreamId id,
          hq::UnidirectionalStreamType /* type */,
          size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(consumed, atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_->takeTemporaryOwnership(expectedId);

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
  dispatcherCallback_->expectParsePreface(
      [&](uint64_t /* type */) { return hq::UnidirectionalStreamType::PUSH; });

  // Expect the assign call
  dispatcherCallback_->expectOnNewPushStream(
      [&](quic::StreamId id, hq::PushId pushId, size_t consumed) {
        ASSERT_EQ(id, expectedId);
        ASSERT_EQ(pushId, expectedPushId);
        ASSERT_EQ(consumed, atLeastBytes + atLeastBytes);
      });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_->takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> onNewPushId
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::PUSH),
           expectedPushId,
           atLeastBytes);
}

TEST_F(UnidirectionalReadDispatcherTest, TestRejectUnrecognizedPreface) {

  quic::StreamId expectedId = 5;
  uint8_t atLeastBytes = 4;
  // Mock the preface parsing
  dispatcherCallback_->expectParsePreface(
      [&](uint64_t /* type */) { return folly::none; });

  // Expect the reject call
  dispatcherCallback_->expectRejectStream(
      [&](quic::StreamId id) { ASSERT_EQ(id, expectedId); });

  // Prior to sending data, give the dispatcher ownership
  // on the stream id (like "onNewUnidirectionalStream" does)
  dispatcher_->takeTemporaryOwnership(expectedId);

  // Attempt to write the preface
  // Expected invocation chain:
  //   sendData -> dispatcher->onData -> (...) -> assignReadCallback
  sendData(expectedId,
           static_cast<uint64_t>(hq::UnidirectionalStreamType::CONTROL),
           atLeastBytes);
}
