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

#pragma once

#include <folly/io/async/EventBase.h>
#include <folly/portability/GMock.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>

namespace apache::thrift::rocket::test {

/**
 * GMock mock of IRocketServerConnection for unit testing callback handlers.
 *
 * Only the methods called by RocketStreamClientCallback,
 * RocketSinkClientCallback, and RocketBiDiClientCallback are mocked.
 * Remaining pure virtuals have minimal stubs to satisfy the interface.
 */
class MockIRocketServerConnection : public IRocketServerConnection {
 public:
  MockIRocketServerConnection() : evb_(std::make_unique<folly::EventBase>()) {
    using ::testing::Return;
    using ::testing::ReturnRef;

    ON_CALL(*this, getEventBase()).WillByDefault(ReturnRef(*evb_));
    ON_CALL(*this, getRawSocket()).WillByDefault(Return(nullptr));
    ON_CALL(*this, areStreamsPaused()).WillByDefault(Return(false));
    ON_CALL(*this, isDecodingMetadataUsingBinaryProtocol())
        .WillByDefault(Return(false));
    ON_CALL(*this, getPayloadSerializer()).WillByDefault([]() {
      return PayloadSerializer::getInstance();
    });
  }

  // Methods used by callback handlers
  MOCK_METHOD(void, close, (folly::exception_wrapper), (override));
  MOCK_METHOD(void, freeStream, (StreamId, bool), (override));
  MOCK_METHOD(
      void, sendErrorAfterDrain, (StreamId, RocketException&&), (override));
  MOCK_METHOD(
      void,
      sendPayload,
      (StreamId, Payload&&, Flags, MessageChannel::SendCallbackPtr),
      (override));
  MOCK_METHOD(
      void,
      sendError,
      (StreamId, RocketException&&, MessageChannel::SendCallbackPtr),
      (override));
  MOCK_METHOD(void, sendRequestN, (StreamId, int32_t), (override));
  MOCK_METHOD(void, sendCancel, (StreamId), (override));
  MOCK_METHOD(
      void, sendMetadataPush, (std::unique_ptr<folly::IOBuf>), (override));
  MOCK_METHOD(PayloadSerializer::Ptr, getPayloadSerializer, (), (override));
  MOCK_METHOD(bool, isDecodingMetadataUsingBinaryProtocol, (), (override));
  MOCK_METHOD(folly::EventBase&, getEventBase, (), (const, override));
  MOCK_METHOD(folly::AsyncSocket*, getRawSocket, (), (const, override));
  MOCK_METHOD(
      void,
      scheduleStreamTimeout,
      (folly::HHWheelTimer::Callback*),
      (override));
  MOCK_METHOD(
      void,
      scheduleSinkTimeout,
      (folly::HHWheelTimer::Callback*, std::chrono::milliseconds),
      (override));
  MOCK_METHOD(bool, areStreamsPaused, (), (const, noexcept, override));
  MOCK_METHOD(void, incInflightFinalResponse, (), (override));
  MOCK_METHOD(void, decInflightFinalResponse, (), (override));
  MOCK_METHOD(bool, incMemoryUsage, (uint32_t), (override));
  MOCK_METHOD(void, decMemoryUsage, (uint32_t), (override));

  // Stubs for remaining pure virtuals (not used by callback handlers)
  MOCK_METHOD(
      void,
      send,
      (std::unique_ptr<folly::IOBuf>,
       MessageChannel::SendCallbackPtr,
       StreamId,
       folly::SocketFds),
      (override));
  MOCK_METHOD(
      RocketStreamClientCallback*,
      createStreamClientCallback,
      (StreamId, IRocketServerConnection&, uint32_t),
      (override));
  MOCK_METHOD(
      RocketSinkClientCallback*,
      createSinkClientCallback,
      (StreamId, IRocketServerConnection&),
      (override));
  MOCK_METHOD(
      std::optional<ChannelRequestCallbackFactory>,
      createChannelClientCallback,
      (StreamId, IRocketServerConnection&, uint32_t),
      (override));
  MOCK_METHOD(void, handleFrame, (std::unique_ptr<folly::IOBuf>), (override));

  void writeStarting() noexcept override {}
  void writeSuccess() noexcept override {}
  void writeErr(size_t, const folly::AsyncSocketException&) noexcept override {}
  void onEgressBuffered() override {}
  void onEgressBufferCleared() override {}
  size_t getNumStreams() const override { return 0; }
  MOCK_METHOD(void, applyQosMarking, (const RequestSetupMetadata&), (override));
  Cpp2ConnContext* getCpp2ConnContext() override { return nullptr; }
  void setOnWriteQuiescenceCallback(
      folly::Function<void(ReadPausableHandle)>) override {}
  int32_t getVersion() const override { return 0; }
  size_t getNumActiveRequests() const override { return 0; }
  size_t getNumPendingWrites() const override { return 0; }
  const folly::SocketAddress& getPeerAddress() const noexcept override {
    static folly::SocketAddress addr;
    return addr;
  }
  void applyCustomCompression(std::shared_ptr<CustomCompressor>) override {}
  std::vector<InteractionInfo> getInteractionSnapshots() const override {
    return {};
  }

  // ManagedConnection pure virtuals
  void timeoutExpired() noexcept override {}
  void describe(std::ostream&) const override {}
  bool isBusy() const override { return false; }
  void notifyPendingShutdown() override {}
  void closeWhenIdle() override {}
  void dropConnection(const std::string& = "") override {}
  void dumpConnectionState(uint8_t) override {}

 private:
  void closeIfNeeded() override {}
  void incrementActivePauseHandlers() override {}
  void decrementActivePauseHandlers() override {}
  void tryResumeSocketReading() override {}
  void pauseSocketReading() override {}
  void incInflightRequests() override {}
  void decInflightRequests() override {}
  void requestComplete() override {}
  RocketServerHandler& getFrameHandler() override {
    LOG(FATAL) << "getFrameHandler() not implemented in mock";
    __builtin_unreachable();
  }

  std::unique_ptr<folly::EventBase> evb_;
};

} // namespace apache::thrift::rocket::test
