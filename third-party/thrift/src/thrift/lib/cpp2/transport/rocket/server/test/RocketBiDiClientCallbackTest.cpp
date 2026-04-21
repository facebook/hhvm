/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketBiDiClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/test/MockIRocketServerConnection.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace ::testing;
using namespace apache::thrift;
using namespace apache::thrift::rocket;
using namespace apache::thrift::rocket::test;

class MockBiDiServerCallback : public BiDiServerCallback {
 public:
  MOCK_METHOD(bool, onSinkNext, (StreamPayload&&), (override));
  MOCK_METHOD(bool, onSinkError, (folly::exception_wrapper), (override));
  MOCK_METHOD(bool, onSinkComplete, (), (override));
  MOCK_METHOD(bool, onStreamRequestN, (int32_t), (override));
  MOCK_METHOD(bool, onStreamCancel, (), (override));
  MOCK_METHOD(void, resetClientCallback, (BiDiClientCallback&), (override));
  MOCK_METHOD(void, pauseStream, (), (override));
  MOCK_METHOD(void, resumeStream, (), (override));
};

class RocketBiDiClientCallbackTest : public ::testing::Test {
 protected:
  static constexpr StreamId kStreamId{1};

  void SetUp() override {
    callback_ = std::make_unique<RocketBiDiClientCallback>(
        kStreamId, connection_, /*initialTokens=*/0);
  }

  void makeReady() {
    FirstResponsePayload firstResponse(
        folly::IOBuf::copyBuffer(""), ResponseRpcMetadata());
    callback_->onFirstResponse(
        std::move(firstResponse),
        &connection_.getEventBase(),
        &serverCallback_);
  }

  void makeReadyWithTokens(int32_t tokens) {
    callback_ = std::make_unique<RocketBiDiClientCallback>(
        kStreamId, connection_, tokens);
    EXPECT_CALL(serverCallback_, onStreamRequestN(tokens))
        .WillOnce(Return(true));
    makeReady();
  }

  NiceMock<MockIRocketServerConnection> connection_;
  NiceMock<MockBiDiServerCallback> serverCallback_;
  std::unique_ptr<RocketBiDiClientCallback> callback_;
};

TEST_F(RocketBiDiClientCallbackTest, HandlePayloadOnSinkNext) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));

  auto data = folly::IOBuf::copyBuffer("test");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);
  PayloadFrame frame(kStreamId, std::move(packed), Flags().next(true));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandlePayloadOnSinkComplete) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));

  PayloadFrame frame(kStreamId, Payload{}, Flags().complete(true));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleErrorCanceledPreReady) {
  // Cancel error before first response should mark as early cancelled
  EXPECT_CALL(connection_, close(_)).Times(0);

  ErrorFrame frame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, OnFirstResponseAfterEarlyCancelTeardown) {
  // Simulate early cancellation via a cancel error frame before first response.
  EXPECT_CALL(connection_, close(_)).Times(0);
  ErrorFrame cancelFrame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(cancelFrame));

  // When onFirstResponse is called after early cancel, it should use the
  // serverCallback parameter (not the null member) to tear down both halves.
  // This is a regression test for a null pointer dereference where
  // serverCallback_ (member, still nullptr) was used instead of serverCallback
  // (parameter).
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));
  EXPECT_CALL(connection_, freeStream(kStreamId, _)).Times(1);

  FirstResponsePayload firstResponse(
      folly::IOBuf::copyBuffer(""), ResponseRpcMetadata());
  bool result = callback_->onFirstResponse(
      std::move(firstResponse), &connection_.getEventBase(), &serverCallback_);

  EXPECT_FALSE(result);
}

TEST_F(RocketBiDiClientCallbackTest, HandleErrorCanceledPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));

  ErrorFrame frame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleErrorNonCanceledPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_))
      .WillOnce([](folly::exception_wrapper ew) {
        EXPECT_TRUE(ew.is_compatible_with<RocketException>());
        return true;
      });

  ErrorFrame frame(
      kStreamId,
      ErrorCode::APPLICATION_ERROR,
      Payload::makeFromData(folly::IOBuf::copyBuffer("error")));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleRequestNPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onStreamRequestN(5)).WillOnce(Return(true));

  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleRequestNPreReadyClosesConnection) {
  EXPECT_CALL(connection_, close(_)).Times(1);

  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleCancelPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleCancelPreReadyClosesConnection) {
  EXPECT_CALL(connection_, close(_)).Times(1);

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleCancelAfterStreamClosedIsNoop) {
  makeReady();

  // Close the stream half by completing it from the server side.
  callback_->onStreamComplete();

  // A cancel frame arriving after the stream is already closed should be
  // silently ignored (not forwarded to the server callback).
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleExtNoIgnoreClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  ExtFrame frame(kStreamId, Payload{}, Flags(), ExtFrameType::UNKNOWN);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, HandleConnectionClose) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);

  callback_->handleConnectionClose();
}

TEST_F(RocketBiDiClientCallbackTest, HandlePayloadFragmentReassembly) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));

  // Pack a real payload, then send it as a fragment (follows=true) followed by
  // an empty final fragment. This exercises the bufferOrGetFullPayload path.
  auto data = folly::IOBuf::copyBuffer("test");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);

  // Fragment 1: full packed payload with follows flag — gets buffered.
  PayloadFrame fragment1(
      kStreamId, std::move(packed), Flags().next(true).follows(true));
  callback_->handleFrame(std::move(fragment1));

  // Fragment 2: empty data-only payload, final — triggers reassembly and
  // onSinkNext.
  PayloadFrame fragment2(
      kStreamId,
      Payload::makeFromData(folly::IOBuf::create(0)),
      Flags().next(true));
  callback_->handleFrame(std::move(fragment2));
}

TEST_F(RocketBiDiClientCallbackTest, HandlePausedByConnectionForwardsPause) {
  makeReady();
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(true));
  EXPECT_CALL(serverCallback_, pauseStream()).Times(1);
  callback_->handlePausedByConnection();
}

TEST_F(RocketBiDiClientCallbackTest, HandleResumedByConnectionForwardsResume) {
  makeReady();
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(false));
  EXPECT_CALL(serverCallback_, resumeStream()).Times(1);
  callback_->handleResumedByConnection();
}

TEST_F(RocketBiDiClientCallbackTest, HandlePausedByConnectionPreReady) {
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(true));
  EXPECT_CALL(serverCallback_, pauseStream()).Times(0);
  callback_->handlePausedByConnection();
}

TEST_F(RocketBiDiClientCallbackTest, HandleResumedByConnectionPreReady) {
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(false));
  EXPECT_CALL(serverCallback_, resumeStream()).Times(0);
  callback_->handleResumedByConnection();
}

TEST_F(
    RocketBiDiClientCallbackTest,
    OnFirstResponsePausesIfConnectionAlreadyPaused) {
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(true));
  EXPECT_CALL(serverCallback_, pauseStream()).Times(1);
  makeReady();
}

TEST_F(
    RocketBiDiClientCallbackTest,
    ResetServerCallbackPausesIfConnectionAlreadyPaused) {
  makeReady();
  NiceMock<MockBiDiServerCallback> newServerCallback;
  EXPECT_CALL(connection_, areStreamsPaused()).WillRepeatedly(Return(true));
  EXPECT_CALL(newServerCallback, pauseStream()).Times(1);
  callback_->resetServerCallback(newServerCallback);
}

TEST_F(RocketBiDiClientCallbackTest, HandleStreamHeadersPushIsNoop) {
  makeReady();
  EXPECT_CALL(connection_, close(_)).Times(0);
  callback_->handleStreamHeadersPush(HeadersPayload(HeadersPayloadContent{}));
}

TEST_F(RocketBiDiClientCallbackTest, HandlePayloadNextAndComplete) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));

  auto data = folly::IOBuf::copyBuffer("final");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);
  PayloadFrame frame(
      kStreamId, std::move(packed), Flags().next(true).complete(true));
  callback_->handleFrame(std::move(frame));
}

// --- Stream credit starvation timeout tests ---

TEST_F(RocketBiDiClientCallbackTest, TimeoutScheduledWhenTokensExhausted) {
  makeReadyWithTokens(2);

  // First onStreamNext: tokens go from 2 to 1, no timeout yet.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(0);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // Second onStreamNext: tokens go from 1 to 0, timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("b"), StreamPayloadMetadata()));
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutNotScheduledWhenTokensRemain) {
  makeReadyWithTokens(2);

  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(0);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutCancelledOnRequestN) {
  makeReadyWithTokens(1);

  // Exhaust tokens — timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // Client sends REQUEST_N — timeout cancelled, tokens replenished.
  EXPECT_CALL(serverCallback_, onStreamRequestN(5)).WillOnce(Return(true));
  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));

  // Verify timeout is cancelled by sending more payloads without timeout
  // being scheduled until tokens are exhausted again.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(0);
  for (int i = 0; i < 4; ++i) {
    callback_->onStreamNext(
        StreamPayload(folly::IOBuf::copyBuffer("x"), StreamPayloadMetadata()));
  }

  // 5th payload exhausts tokens again.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("x"), StreamPayloadMetadata()));
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutExpiredCancelsStreamAndSink) {
  makeReadyWithTokens(1);

  // Exhaust tokens.
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // Expect: sink error notified, stream cancelled, cancel + error sent, freed.
  // Sink is notified through the Stapler before stream cancel to prevent
  // a use-after-free from the deferred canceled() callback.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(connection_, sendCancel(kStreamId)).Times(1);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->timeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutExpiredWhenSinkAlreadyClosed) {
  makeReadyWithTokens(1);

  // Close sink first via onSinkComplete.
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));
  callback_->onSinkComplete();

  // Exhaust tokens.
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // Sink is already closed, so no sendCancel should be called.
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(connection_, sendCancel(_)).Times(0);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->timeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutExpiredWhenStreamAlreadyClosed) {
  makeReadyWithTokens(1);

  // Close the stream half first via onStreamComplete.
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));
  callback_->onStreamComplete();

  // Stream is already closed, so no onStreamCancel should be called.
  // Only the sink half should be notified.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);
  EXPECT_CALL(connection_, sendCancel(kStreamId)).Times(1);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->timeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutCancelledOnStreamError) {
  makeReadyWithTokens(1);

  // Exhaust tokens — timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // onStreamError cancels the timeout.
  callback_->onStreamError(
      folly::make_exception_wrapper<RocketException>(
          ErrorCode::APPLICATION_ERROR, "err"));

  // Verify no timeout fires.
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutCancelledOnStreamCancel) {
  makeReadyWithTokens(1);

  // Exhaust tokens — timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // onStreamCancel cancels the timeout.
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  callback_->onStreamCancel();
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutCancelledOnStreamComplete) {
  makeReadyWithTokens(1);

  // Exhaust tokens — timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // onStreamComplete cancels the timeout.
  callback_->onStreamComplete();

  // Verify no timeout fires (if it did, it would call onStreamCancel which
  // we don't expect).
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);
}

TEST_F(RocketBiDiClientCallbackTest, TimeoutCancelledOnConnectionClose) {
  makeReadyWithTokens(1);

  // Exhaust tokens — timeout scheduled.
  EXPECT_CALL(connection_, scheduleStreamTimeout(_)).Times(1);
  callback_->onStreamNext(
      StreamPayload(folly::IOBuf::copyBuffer("a"), StreamPayloadMetadata()));

  // handleConnectionClose cancels the timeout.
  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);
  callback_->handleConnectionClose();
}

// --- Sink chunk timeout tests ---

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutScheduledWhenCreditsGranted) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  EXPECT_CALL(
      connection_, scheduleSinkTimeout(_, std::chrono::milliseconds{100}))
      .Times(1);
  callback_->onSinkRequestN(5);
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutRescheduledOnPayload) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits — timeout scheduled.
  EXPECT_CALL(connection_, scheduleSinkTimeout(_, _)).Times(1);
  callback_->onSinkRequestN(5);

  // Receive payload — timeout rescheduled (credits go from 5 to 4).
  EXPECT_CALL(connection_, scheduleSinkTimeout(_, _)).Times(1);
  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));
  auto data = folly::IOBuf::copyBuffer("test");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);
  PayloadFrame frame(kStreamId, std::move(packed), Flags().next(true));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutCancelledWhenCreditsExhausted) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant 1 credit.
  callback_->onSinkRequestN(1);

  // Receive 1 payload — credits exhausted, timeout cancelled.
  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));
  auto data = folly::IOBuf::copyBuffer("test");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);
  PayloadFrame frame(kStreamId, std::move(packed), Flags().next(true));
  callback_->handleFrame(std::move(frame));

  // No more scheduleSinkTimeout calls after credits are exhausted.
  // (The cancelSinkTimeout was called internally.)
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutExpiredCancelsAll) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits to arm timeout.
  callback_->onSinkRequestN(5);

  // Expect: sink error, stream cancel, cancel sent, error sent, stream freed.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(connection_, sendCancel(kStreamId)).Times(1);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->sinkChunkTimeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutCancelledOnSinkComplete) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits.
  callback_->onSinkRequestN(5);

  // Sink complete cancels the timeout.
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));
  callback_->onSinkComplete();
}

TEST_F(
    RocketBiDiClientCallbackTest, SinkTimeoutExpiredWhenStreamAlreadyClosed) {
  makeReadyWithTokens(1);
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant sink credits to arm timeout.
  callback_->onSinkRequestN(5);

  // Close the stream half first via onStreamComplete.
  callback_->onStreamComplete();

  // Now fire the sink chunk timeout. The stream is already closed, so only the
  // sink half should be cancelled. Previously this was a use-after-free because
  // onSinkError() would call freeStreamAndReturn() (destroying `this`) before
  // the rest of sinkChunkTimeoutExpired() could run.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);
  EXPECT_CALL(connection_, sendCancel(kStreamId)).Times(1);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->sinkChunkTimeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutExpiredWhenSinkAlreadyClosed) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant sink credits to arm timeout.
  callback_->onSinkRequestN(5);

  // Close the sink half first via onSinkComplete.
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));
  callback_->onSinkComplete();

  // Now fire the sink chunk timeout. The sink is already closed, so only the
  // stream half should be cancelled.
  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(0);
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(connection_, sendCancel(_)).Times(0);
  EXPECT_CALL(connection_, sendError(kStreamId, _, _)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  callback_->sinkChunkTimeoutExpired();
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutCancelledOnSinkError) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits to arm timeout.
  callback_->onSinkRequestN(5);

  // Sink error cancels the timeout.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  callback_->onSinkError(
      folly::make_exception_wrapper<std::runtime_error>("test error"));
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutCancelledOnSinkCancel) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits to arm timeout.
  callback_->onSinkRequestN(5);

  // onSinkCancel cancels the timeout.
  callback_->onSinkCancel();
}

TEST_F(RocketBiDiClientCallbackTest, SinkTimeoutCancelledOnConnectionClose) {
  makeReady();
  callback_->setChunkTimeout(std::chrono::milliseconds{100});

  // Grant credits to arm timeout.
  callback_->onSinkRequestN(5);

  // handleConnectionClose cancels the sink timeout.
  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);
  callback_->handleConnectionClose();
}

TEST_F(RocketBiDiClientCallbackTest, NoSinkTimeoutWithoutSetChunkTimeout) {
  makeReady();

  // Without calling setChunkTimeout, no timeout is scheduled.
  EXPECT_CALL(connection_, scheduleSinkTimeout(_, _)).Times(0);
  callback_->onSinkRequestN(5);
}

// --- serverCallback_ lifetime safety tests ---

TEST_F(RocketBiDiClientCallbackTest, HandleConnectionCloseNullsServerCallback) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);

  callback_->handleConnectionClose();

  // After connection close, serverCallback_ should be cleared to prevent
  // use-after-free if the server callback object is destroyed.
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(
    RocketBiDiClientCallbackTest,
    HandleConnectionCloseNullsServerCallbackBeforeCalls) {
  makeReady();

  // Verify that serverCallback_ is already nulled DURING the onSinkError call.
  // This prevents use-after-free if the bridge's onSinkError destroys the
  // server callback, which would make the subsequent onStreamCancel call crash.
  EXPECT_CALL(serverCallback_, onSinkError(_))
      .WillOnce([this](folly::exception_wrapper) {
        EXPECT_FALSE(callback_->serverCallbackReady());
        return true;
      });
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));

  callback_->handleConnectionClose();
}

TEST_F(RocketBiDiClientCallbackTest, HandleConnectionCloseWithOnlySinkOpen) {
  makeReady();

  // Close the stream direction first.
  callback_->onStreamComplete();

  // Only onSinkError should be called (stream is already closed).
  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);

  callback_->handleConnectionClose();
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(RocketBiDiClientCallbackTest, HandleConnectionCloseWithOnlyStreamOpen) {
  makeReady();

  // Close the sink direction first.
  callback_->onSinkCancel();

  // Only onStreamCancel should be called (sink is already closed).
  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(0);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);

  callback_->handleConnectionClose();
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(RocketBiDiClientCallbackTest, SinkErrorAfterStreamCloseFreesStream) {
  makeReady();

  // Close the stream direction from the server side.
  callback_->onStreamComplete();

  // Now close sink via error — state becomes terminal. The stream should be
  // freed and the callback should report not-alive.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onSinkError(
      folly::make_exception_wrapper<std::runtime_error>("test error"));
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(RocketBiDiClientCallbackTest, SinkCompleteAfterStreamCloseFreesStream) {
  makeReady();

  // Close the stream direction from the server side.
  callback_->onStreamComplete();

  // Now complete sink — state becomes terminal.
  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onSinkComplete();
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(RocketBiDiClientCallbackTest, StreamCancelAfterSinkCloseFreesStream) {
  makeReady();

  // Close the sink direction from the server side.
  callback_->onSinkCancel();

  // Now cancel stream — state becomes terminal.
  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(true));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onStreamCancel();
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

// --- Terminal state + server callback returns false (contract violation) ---

TEST_F(
    RocketBiDiClientCallbackTest,
    SinkErrorContractViolationAfterStreamCloseFreesStream) {
  makeReady();

  callback_->onStreamComplete();

  // Server callback returns false (contract violation) but state is terminal.
  // The stream should still be freed and serverCallback_ nulled.
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(false));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onSinkError(
      folly::make_exception_wrapper<std::runtime_error>("test error"));
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(
    RocketBiDiClientCallbackTest,
    SinkCompleteContractViolationAfterStreamCloseFreesStream) {
  makeReady();

  callback_->onStreamComplete();

  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(false));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onSinkComplete();
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

TEST_F(
    RocketBiDiClientCallbackTest,
    StreamCancelContractViolationAfterSinkCloseFreesStream) {
  makeReady();

  callback_->onSinkCancel();

  EXPECT_CALL(serverCallback_, onStreamCancel()).WillOnce(Return(false));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  bool alive = callback_->onStreamCancel();
  EXPECT_FALSE(alive);
  EXPECT_FALSE(callback_->serverCallbackReady());
}

// --- Regression test for P2251980525 crash ---
// Simulates the production crash path: stream completes from the server side,
// then an ErrorFrame arrives from the client. The ErrorFrame's onSinkError
// closes the last open direction (terminal state). Without the fix, the stream
// would not be freed and serverCallback_ would be left dangling for subsequent
// frames.

TEST_F(RocketBiDiClientCallbackTest, ErrorFrameAfterStreamCompleteFreesStream) {
  makeReady();

  // Step 1: Server completes the stream direction.
  callback_->onStreamComplete();

  // Step 2: ErrorFrame arrives from client — handleFrame dispatches to
  // onSinkError, which closes the last direction (terminal state).
  EXPECT_CALL(serverCallback_, onSinkError(_)).WillOnce(Return(true));
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  ErrorFrame frame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(frame));

  // The stream should be freed and serverCallback_ nulled, preventing any
  // subsequent frame from reaching the freed server callback object.
  EXPECT_FALSE(callback_->serverCallbackReady());
  EXPECT_FALSE(callback_->isSinkOpen());
  EXPECT_FALSE(callback_->isStreamOpen());
}
