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

#include <chrono>
#include <deque>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <utility>
#include <variant>

#include <folly/ExceptionWrapper.h>
#include <folly/Executor.h>
#include <folly/ObserverContainer.h>
#include <folly/Portability.h>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/net/NetOps.h>
#include <folly/observer/Observer.h>

#include <wangle/acceptor/ManagedConnection.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/server/MemoryTracker.h>
#include <thrift/lib/cpp2/server/metrics/StreamMetricCallback.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressor.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>
#include <thrift/lib/cpp2/transport/rocket/server/IRocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionAdapter.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionBufferCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/ConnectionWriterCallback.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/IncomingFrameBatcher.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/IncomingFrameHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/WriteBatcher.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(enable_rocket_connection_observers);

namespace apache::thrift {

class Cpp2ConnContext;
class RocketSinkClientCallback;
class RocketStreamClientCallback;

namespace rocket {

class RefactoredRocketServerConnection final : public IRocketServerConnection {
 public:
  using DestructorGuard = folly::DelayedDestruction::DestructorGuard;
  using UniquePtr = std::unique_ptr<
      RefactoredRocketServerConnection,
      folly::DelayedDestruction::Destructor>;

  // (configuration parameters mostly inherited from the server)
  struct Config {
    Config() {}

    std::chrono::milliseconds socketWriteTimeout{
        std::chrono::milliseconds::zero()};
    std::chrono::milliseconds streamStarvationTimeout{std::chrono::seconds{60}};
    std::chrono::milliseconds writeBatchingInterval{
        std::chrono::milliseconds::zero()};
    size_t writeBatchingSize{0};
    size_t writeBatchingByteSize{0};
    size_t egressBufferBackpressureThreshold{0};
    double egressBufferBackpressureRecoveryFactor{0.0};
    const folly::SocketOptionMap* socketOptions{nullptr};
    std::shared_ptr<rocket::ParserAllocatorType> parserAllocator{nullptr};
  };

  RefactoredRocketServerConnection(
      folly::AsyncTransport::UniquePtr socket,
      std::unique_ptr<RocketServerHandler> frameHandler,
      MemoryTracker& ingressMemoryTracker,
      MemoryTracker& egressMemoryTracker,
      StreamMetricCallback& streamMetricCallback,
      const IRocketServerConnection::Config& cfg = {});

  void send(
      std::unique_ptr<folly::IOBuf> data,
      MessageChannel::SendCallbackPtr cb = nullptr,
      StreamId streamId = StreamId(),
      folly::SocketFds fds = folly::SocketFds{}) override;

  void sendErrorAfterDrain(StreamId streamId, RocketException&& rex) override;

  // does not create callback and returns nullptr if streamId is already in use
  RocketStreamClientCallback* FOLLY_NULLABLE createStreamClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t initialRequestN) override;

  // does not create callback and returns nullptr if streamId is already in use
  RocketSinkClientCallback* FOLLY_NULLABLE createSinkClientCallback(
      StreamId streamId, IRocketServerConnection& connection) override;

  // does not create callback and returns nullptr if streamId is already in use
  std::optional<ChannelRequestCallbackFactory> createChannelClientCallback(
      StreamId streamId,
      IRocketServerConnection& connection,
      uint32_t initialRequestN) override;

  // Parser callbacks - must be public for Parser access
  void handleFrame(std::unique_ptr<folly::IOBuf> frame) override;
  void close(folly::exception_wrapper ew) override;

  // AsyncTransport::WriteCallback implementation
  void writeStarting() noexcept override {}
  void writeSuccess() noexcept override {}
  void writeErr(
      size_t /*bytesWritten*/,
      const folly::AsyncSocketException& /*ex*/) noexcept override {}

  // AsyncTransport::BufferCallback implementation
  void onEgressBuffered() override {}
  void onEgressBufferCleared() override {}

  folly::EventBase& getEventBase() const override { return evb_; }
  size_t getNumStreams() const override { return streams_.size(); }

  void sendPayload(
      StreamId streamId,
      Payload&& payload,
      Flags flags,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr) override;
  void sendError(
      StreamId streamId,
      RocketException&& rex,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr) override;
  void sendRequestN(StreamId streamId, int32_t n) override;
  void sendCancel(StreamId streamId) override;
  void sendMetadataPush(std::unique_ptr<folly::IOBuf> metadata) override;

  void freeStream(StreamId streamId, bool markRequestComplete) override;

  void scheduleStreamTimeout(folly::HHWheelTimer::Callback*) override;
  void scheduleSinkTimeout(
      folly::HHWheelTimer::Callback*,
      std::chrono::milliseconds timeout) override;

  void incInflightFinalResponse() override { inflightSinkFinalResponses_++; }
  void decInflightFinalResponse() override {
    DCHECK(inflightSinkFinalResponses_ != 0);
    inflightSinkFinalResponses_--;
    closeIfNeeded();
  }

  void applyQosMarking(const RequestSetupMetadata& setupMetadata) override;

  void setOnWriteQuiescenceCallback(
      folly::Function<void(ReadPausableHandle)> cb) override {
    onWriteQuiescence_ = std::move(cb);
  }

  bool isDecodingMetadataUsingBinaryProtocol() override {
    // default to compact protocol
    if (!decodeMetadataUsingBinary_.has_value()) {
      return false;
    }
    return decodeMetadataUsingBinary_.value();
  }

  bool incMemoryUsage(uint32_t memSize) override;

  void decMemoryUsage(uint32_t memSize) override {
    ingressMemoryTracker_.decrement(memSize);
  }

  int32_t getVersion() const override { return frameHandler_->getVersion(); }

  DestructorGuard getDestructorGuard() { return DestructorGuard(this); }

  bool areStreamsPaused() const noexcept override { return streamsPaused_; }

  size_t getNumActiveRequests() const final { return inflightRequests_; }

  size_t getNumPendingWrites() const final {
    size_t result = 0;
    for (const WriteBatchContext& batch : inflightWritesQueue_) {
      result += batch.requestCompleteCount;
    }
    return result;
  }

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return peerAddress_;
  }

  folly::AsyncSocket* getRawSocket() const override { return rawSocket_; }

 private:
  void startDrain(std::optional<DrainCompleteCode> drainCompleteCode);

  // Note that attachEventBase()/detachEventBase() are not supported in server
  // code
  folly::EventBase& evb_;
  folly::AsyncTransport::UniquePtr const socket_;
  folly::AsyncSocket* const rawSocket_;
  folly::SocketAddress peerAddress_;

  Parser<RefactoredRocketServerConnection> parser_;
  std::unique_ptr<RocketServerHandler> frameHandler_;
  bool setupFrameReceived_{false};
  std::optional<bool> decodeMetadataUsingBinary_;

  folly::F14NodeMap<
      StreamId,
      std::variant<
          RequestResponseFrame,
          RequestFnfFrame,
          RequestStreamFrame,
          RequestChannelFrame>>
      partialRequestFrames_;
  folly::F14FastMap<StreamId, Payload> bufferedFragments_;

  // Total number of active Request* frames ("streams" in protocol parlance)
  size_t inflightRequests_{0};

  // Context for each inflight write to the underlying transport.
  // The size of the queue is equal to the total number of inflight writes to
  // the underlying transport, i.e., writes for which the
  // writeSuccess()/writeErr() has not yet been called.
  std::deque<WriteBatchContext> inflightWritesQueue_;
  // Totoal number of inflight final response for sink semantic, the counter
  // only bumps when sink is in waiting for final response state,
  // (onSinkComplete get called)
  size_t inflightSinkFinalResponses_{0};
  // Write buffer size (aka egress size). Represents the total allocated size of
  // buffers that have not yet been written to the underlying socket.
  size_t egressBufferSize_{0};

  enum class ConnectionState : uint8_t {
    ALIVE,
    DRAINING, // Rejecting all new requests, waiting for inflight requests to
              // complete.
    CLOSING, // No longer reading form the socket, waiting for pending writes to
             // complete.
    CLOSED,
  };
  ConnectionState state_{ConnectionState::ALIVE};
  std::optional<DrainCompleteCode> drainCompleteCode_;

  using ClientCallbackUniquePtr = std::variant<
      std::unique_ptr<RocketStreamClientCallback>,
      std::unique_ptr<RocketSinkClientCallback>,
      std::unique_ptr<RocketBiDiClientCallback>>;
  using ClientCallbackPtr = std::variant<
      RocketStreamClientCallback*,
      RocketSinkClientCallback*,
      RocketBiDiClientCallback*>;
  folly::F14FastMap<StreamId, ClientCallbackUniquePtr> streams_;
  const std::chrono::milliseconds streamStarvationTimeout_;
  const size_t egressBufferBackpressureThreshold_;
  const size_t egressBufferRecoverySize_;
  bool streamsPaused_{false};

  class SocketDrainer : private folly::HHWheelTimer::Callback {
   public:
    explicit SocketDrainer(RefactoredRocketServerConnection& connection)
        : connection_(connection) {}

    void activate() {
      if (!drainComplete_) {
        // Make sure the EventBase doesn't get destroyed until the timer
        // expires.
        evbKA_ = folly::getKeepAliveToken(connection_.evb_);
        connection_.evb_.timer().scheduleTimeout(this, kTimeout);
      }
    }

    void drainComplete() {
      if (!drainComplete_) {
        cancelTimeout();
        evbKA_.reset();
        connection_.socket_->setReadCB(nullptr);
        drainComplete_ = true;
      }
    }

    bool isDrainComplete() const { return drainComplete_; }

   private:
    void timeoutExpired() noexcept final {
      drainComplete();
      connection_.closeIfNeeded();
    }

    RefactoredRocketServerConnection& connection_;
    bool drainComplete_{false};
    folly::Executor::KeepAlive<> evbKA_;
    static constexpr std::chrono::seconds kTimeout{1};
  };
  SocketDrainer socketDrainer_;
  size_t activePausedHandlers_{0};
  folly::Function<void(ReadPausableHandle)> onWriteQuiescence_;

  MemoryTracker& ingressMemoryTracker_;
  MemoryTracker& egressMemoryTracker_;
  StreamMetricCallback& streamMetricCallback_;

  ~RefactoredRocketServerConnection() override;

  // Override methods from IRocketServerConnection
  Cpp2ConnContext* getCpp2ConnContext() override {
    return frameHandler_->getCpp2ConnContext();
  }

 private:
  void closeIfNeeded() override;
  void incrementActivePauseHandlers() override { ++activePausedHandlers_; }
  void decrementActivePauseHandlers() override { --activePausedHandlers_; }
  void tryResumeSocketReading() override;
  void pauseSocketReading() override;
  void incInflightRequests() override { ++inflightRequests_; }
  void decInflightRequests() override {
    --inflightRequests_;
    closeIfNeeded();
  }
  void requestComplete() override;
  RocketServerHandler& getFrameHandler() override { return *frameHandler_; }

  void flushWrites(
      std::unique_ptr<folly::IOBuf> writes, WriteBatchContext&& context);
  void flushWritesWithFds(
      std::unique_ptr<folly::IOBuf> writes,
      WriteBatchContext&& context,
      FdsAndOffsets&&);

  void timeoutExpired() noexcept final;
  void describe(std::ostream&) const final {}
  bool isBusy() const final;
  void notifyPendingShutdown() final;
  void closeWhenIdle() final;
  void dropConnection(const std::string& errorMsg = "") final;
  void dumpConnectionState(uint8_t) final {}
  folly::Optional<Payload> bufferOrGetFullPayload(PayloadFrame&& payloadFrame);

  void handleStreamFrame(
      std::unique_ptr<folly::IOBuf> frame,
      StreamId streamId,
      FrameType frameType,
      Flags flags,
      folly::io::Cursor cursor,
      RocketStreamClientCallback& clientCallback);
  void handleSinkFrame(
      std::unique_ptr<folly::IOBuf> frame,
      StreamId streamId,
      FrameType frameType,
      Flags flags,
      folly::io::Cursor cursor,
      RocketSinkClientCallback& clientCallback);
  void handleBiDiFrame(
      std::unique_ptr<folly::IOBuf> frame,
      StreamId streamId,
      FrameType frameType,
      Flags flags,
      folly::io::Cursor cursor,
      RocketBiDiClientCallback& clientCallback);
  void handleUntrackedFrame(
      std::unique_ptr<folly::IOBuf> frame,
      StreamId streamId,
      FrameType frameType,
      Flags flags,
      folly::io::Cursor cursor);

  template <typename RequestFrame>
  void handleRequestFrame(RequestFrame&& frame) {
    auto streamId = frame.streamId();
    if (UNLIKELY(frame.hasFollows())) {
      partialRequestFrames_.emplace(
          streamId, std::forward<RequestFrame>(frame));
    } else {
      RocketServerFrameContext(*this, streamId)
          .onFullFrame(std::forward<RequestFrame>(frame));
    }
  }

  void pauseStreams();
  void resumeStreams();

  friend class RocketServerFrameContext;

 public:
  PayloadSerializer::Ptr getPayloadSerializer() override {
    if (payloadSerializerHolder_) {
      return payloadSerializerHolder_->get();
    }

    return PayloadSerializer::getInstance();
  }

  void applyCustomCompression(
      std::shared_ptr<CustomCompressor> compressor) override {
    CustomCompressionPayloadSerializerStrategyOptions options;
    options.compressor = compressor;

    if (!payloadSerializerHolder_) {
      payloadSerializerHolder_.emplace();
    }

    CustomCompressionPayloadSerializerStrategy<DefaultPayloadSerializerStrategy>
        strategy{options};
    payloadSerializerHolder_->initialize(std::move(strategy));
    customCompressor_ = compressor;
  }

 private:
  std::optional<PayloadSerializer::PayloadSerializerHolder>
      payloadSerializerHolder_;
  std::shared_ptr<CustomCompressor> customCompressor_;

  friend class ConnectionAdapter<RefactoredRocketServerConnection>;
  using ConnectionAdapter = apache::thrift::rocket::ConnectionAdapter<
      RefactoredRocketServerConnection>;

  ConnectionAdapter connectionAdapter_;

  WriteBatcher<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter>
      writeBatcher_;

  // Callback classes for handling socket events
  ConnectionWriterCallback<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter>
      writerCallback_;
  ConnectionBufferCallback<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter>
      bufferCallback_;

  SetupFrameAcceptor<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter,
      RocketServerHandler>
      setupFrameAcceptor_;
  RequestResponseHandler<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter>
      requestResponseHandler_;

  using IncomingFrameHandler = apache::thrift::rocket::IncomingFrameHandler<
      RefactoredRocketServerConnection,
      apache::thrift::rocket::ConnectionAdapter,
      RocketServerHandler>;
  IncomingFrameHandler incomingFrameHandler_;

  using IncomingFrameBatcher = apache::thrift::rocket::IncomingFrameBatcher<
      IncomingFrameHandler,
      apache::thrift::rocket::ConnectionAdapter<
          RefactoredRocketServerConnection>>;
  folly::EventBaseLocal<IncomingFrameBatcher> batcher_;
};

} // namespace rocket
} // namespace apache::thrift
