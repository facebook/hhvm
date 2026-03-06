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
#include <thrift/lib/cpp2/transport/rocket/server/RocketStreamClientCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/test/MockIRocketServerConnection.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DEFINE_bool(enable_rocket_connection_observers, false);

using namespace ::testing;
using namespace apache::thrift;
using namespace apache::thrift::rocket;
using namespace apache::thrift::rocket::test;

class MockStreamServerCallback : public StreamServerCallback {
 public:
  MOCK_METHOD(bool, onStreamRequestN, (int32_t), (override));
  MOCK_METHOD(void, onStreamCancel, (), (override));
  MOCK_METHOD(void, resetClientCallback, (StreamClientCallback&), (override));
  MOCK_METHOD(void, pauseStream, (), (override));
  MOCK_METHOD(void, resumeStream, (), (override));
  MOCK_METHOD(bool, onSinkHeaders, (HeadersPayload&&), (override));
};

class RocketStreamClientCallbackTest : public ::testing::Test {
 protected:
  static constexpr StreamId kStreamId{1};
  static constexpr uint32_t kInitialRequestN{10};

  void SetUp() override {
    callback_ = std::make_unique<RocketStreamClientCallback>(
        kStreamId, connection_, kInitialRequestN);
  }

  /**
   * Simulates the first response to transition the callback to "ready" state
   * with a server callback attached.
   */
  void makeReady() {
    EXPECT_CALL(serverCallback_, onStreamRequestN(_))
        .WillRepeatedly(Return(true));
    FirstResponsePayload firstResponse(
        folly::IOBuf::copyBuffer(""), ResponseRpcMetadata());
    callback_->onFirstResponse(
        std::move(firstResponse),
        &connection_.getEventBase(),
        &serverCallback_);
  }

  NiceMock<MockIRocketServerConnection> connection_;
  NiceMock<MockStreamServerCallback> serverCallback_;
  std::unique_ptr<RocketStreamClientCallback> callback_;
};

TEST_F(RocketStreamClientCallbackTest, HandleRequestNPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onStreamRequestN(5)).WillOnce(Return(true));

  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleRequestNPreReadyClosesConnection) {
  EXPECT_CALL(connection_, close(_)).Times(1);

  RequestNFrame frame(kStreamId, 5);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleCancelPreReady) {
  // Cancel before first response should mark as cancelled, not crash
  EXPECT_CALL(connection_, close(_)).Times(0);

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleCancelPostReady) {
  makeReady();

  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);
  EXPECT_CALL(connection_, freeStream(kStreamId, true)).Times(1);

  CancelFrame frame(kStreamId);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandlePayloadClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  PayloadFrame frame(kStreamId, Payload{}, Flags());
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleErrorClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  ErrorFrame frame(kStreamId, ErrorCode::APPLICATION_ERROR, Payload{});
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleExtPreReadyClosesConnection) {
  EXPECT_CALL(connection_, close(_)).Times(1);

  ExtFrame frame(kStreamId, Payload{}, Flags(), ExtFrameType::UNKNOWN);
  callback_->handleFrame(std::move(frame));
}

TEST_F(
    RocketStreamClientCallbackTest,
    HandleExtPostReadyNoIgnoreClosesConnection) {
  makeReady();

  EXPECT_CALL(connection_, close(_)).Times(1);

  ExtFrame frame(kStreamId, Payload{}, Flags(), ExtFrameType::UNKNOWN);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleExtPostReadyIgnoreFlag) {
  makeReady();

  // ExtFrame with ignore flag set should be silently dropped
  EXPECT_CALL(connection_, close(_)).Times(0);

  Flags flags;
  flags.ignore(true);
  ExtFrame frame(kStreamId, Payload{}, flags, ExtFrameType::UNKNOWN);
  callback_->handleFrame(std::move(frame));
}

TEST_F(RocketStreamClientCallbackTest, HandleConnectionClose) {
  makeReady();

  EXPECT_CALL(connection_, sendErrorAfterDrain(kStreamId, _)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(1);

  callback_->handleConnectionClose();
}

TEST_F(RocketStreamClientCallbackTest, HandleConnectionClosePreReady) {
  // Before first response, sendErrorAfterDrain should still be called
  // but server callback should not be notified
  EXPECT_CALL(connection_, sendErrorAfterDrain(kStreamId, _)).Times(1);
  EXPECT_CALL(serverCallback_, onStreamCancel()).Times(0);

  callback_->handleConnectionClose();
}
