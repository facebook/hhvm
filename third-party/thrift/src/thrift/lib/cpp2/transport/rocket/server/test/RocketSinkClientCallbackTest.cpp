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
#include <thrift/lib/cpp2/transport/rocket/server/RocketSinkClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/test/MockIRocketServerConnection.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace ::testing;
using namespace apache::thrift;
using namespace apache::thrift::rocket;
using namespace apache::thrift::rocket::test;

class MockSinkServerCallback : public SinkServerCallback {
 public:
  MOCK_METHOD(bool, onSinkNext, (StreamPayload&&), (override));
  MOCK_METHOD(void, onSinkError, (folly::exception_wrapper), (override));
  MOCK_METHOD(bool, onSinkComplete, (), (override));
  MOCK_METHOD(void, resetClientCallback, (SinkClientCallback&), (override));
};

class RocketSinkClientCallbackTest : public ::testing::Test {
 protected:
  static constexpr StreamId kStreamId{1};

  void SetUp() override {
    callback_ =
        std::make_unique<RocketSinkClientCallback>(kStreamId, connection_);
  }

  void makeReady() {
    FirstResponsePayload firstResponse(
        folly::IOBuf::copyBuffer(""), ResponseRpcMetadata());
    callback_->onFirstResponse(
        std::move(firstResponse),
        &connection_.getEventBase(),
        &serverCallback_);
  }

  NiceMock<MockIRocketServerConnection> connection_;
  NiceMock<MockSinkServerCallback> serverCallback_;
  std::unique_ptr<RocketSinkClientCallback> callback_;
};

TEST_F(RocketSinkClientCallbackTest, HandlePayloadOnSinkNext) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkNext(_)).WillOnce(Return(true));

  auto data = folly::IOBuf::copyBuffer("test");
  StreamPayload sp(std::move(data), StreamPayloadMetadata());
  auto packed =
      connection_.getPayloadSerializer()->pack(std::move(sp), false, nullptr);
  PayloadFrame frame(kStreamId, std::move(packed), Flags().next(true));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandlePayloadOnSinkComplete) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkComplete()).WillOnce(Return(true));

  PayloadFrame frame(kStreamId, Payload{}, Flags().complete(true));
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandleErrorCanceledPreReady) {
  // Cancel error before first response should mark as early cancelled
  EXPECT_CALL(connection_, close(_)).Times(0);

  ErrorFrame frame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandleErrorPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  ErrorFrame frame(kStreamId, ErrorCode::CANCELED, Payload{});
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandleRequestNClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandleCancelClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandleExtClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  ExtFrame frame(kStreamId, Payload{}, Flags(), ExtFrameType::UNKNOWN);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketSinkClientCallbackTest, HandlePayloadFragmentReassembly) {
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

TEST_F(RocketSinkClientCallbackTest, HandlePayloadNextAndComplete) {
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

TEST_F(RocketSinkClientCallbackTest, HandleConnectionClose) {
  makeReady();

  EXPECT_CALL(serverCallback_, onSinkError(_)).Times(1);

  callback_->handleConnectionClose();
}
