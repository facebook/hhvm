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
#include <folly/experimental/observer/Observer.h>
#include <folly/io/IOBuf.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/net/NetOps.h>

#include <wangle/acceptor/ManagedConnection.h>

#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/server/MemoryTracker.h>
#include <thrift/lib/cpp2/transport/core/ManagedConnectionIf.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerFrameContext.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(enable_rocket_connection_observers);

namespace apache {
namespace thrift {

class Cpp2ConnContext;
class RocketSinkClientCallback;
class RocketStreamClientCallback;

namespace rocket {

class RocketServerConnection final
    : public ManagedConnectionIf,
      private folly::AsyncTransport::WriteCallback,
      private folly::AsyncTransport::BufferCallback {
 public:
  using UniquePtr = std::
      unique_ptr<RocketServerConnection, folly::DelayedDestruction::Destructor>;

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
  };

  RocketServerConnection(
      folly::AsyncTransport::UniquePtr socket,
      std::unique_ptr<RocketServerHandler> frameHandler,
      MemoryTracker& ingressMemoryTracker,
      MemoryTracker& egressMemoryTracker,
      const Config& cfg = {});

  void send(
      std::unique_ptr<folly::IOBuf> data,
      MessageChannel::SendCallbackPtr cb = nullptr,
      StreamId streamId = StreamId(),
      folly::SocketFds fds = folly::SocketFds{});

  // does not create callback and returns nullptr if streamId is already in use
  RocketStreamClientCallback* FOLLY_NULLABLE createStreamClientCallback(
      StreamId streamId,
      RocketServerConnection& connection,
      uint32_t initialRequestN);

  // does not create callback and returns nullptr if streamId is already in use
  RocketSinkClientCallback* FOLLY_NULLABLE createSinkClientCallback(
      StreamId streamId, RocketServerConnection& connection);

  // Parser callbacks
  void handleFrame(std::unique_ptr<folly::IOBuf> frame);
  void close(folly::exception_wrapper ew);

  // AsyncTransport::WriteCallback implementation
  void writeStarting() noexcept final;
  void writeSuccess() noexcept final;
  void writeErr(
      size_t bytesWritten,
      const folly::AsyncSocketException& ex) noexcept final;

  // AsyncTransport::BufferCallback implementation
  void onEgressBuffered() final;
  void onEgressBufferCleared() final;

  folly::EventBase& getEventBase() const { return evb_; }

  size_t getNumStreams() const { return streams_.size(); }

  void sendPayload(
      StreamId streamId,
      Payload&& payload,
      Flags flags,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr);
  void sendError(
      StreamId streamId,
      RocketException&& rex,
      apache::thrift::MessageChannel::SendCallbackPtr cb = nullptr);
  void sendRequestN(StreamId streamId, int32_t n);
  void sendCancel(StreamId streamId);
  void sendMetadataPush(std::unique_ptr<folly::IOBuf> metadata);

  void freeStream(StreamId streamId, bool markRequestComplete);

  void scheduleStreamTimeout(folly::HHWheelTimer::Callback*);
  void scheduleSinkTimeout(
      folly::HHWheelTimer::Callback*, std::chrono::milliseconds timeout);

  void incInflightFinalResponse() { inflightSinkFinalResponses_++; }
  void decInflightFinalResponse() {
    DCHECK(inflightSinkFinalResponses_ != 0);
    inflightSinkFinalResponses_--;
    closeIfNeeded();
  }

  void applyQosMarking(const RequestSetupMetadata& setupMetadata);

  class ReadResumableHandle {
   public:
    explicit ReadResumableHandle(RocketServerConnection* connection);
    ~ReadResumableHandle();

    ReadResumableHandle(ReadResumableHandle&&) noexcept;
    ReadResumableHandle(const ReadResumableHandle&) = delete;
    ReadResumableHandle& operator=(const ReadResumableHandle&) = delete;
    ReadResumableHandle& operator=(ReadResumableHandle&&) = delete;

    void resume() &&;
    folly::EventBase& getEventBase() { return connection_->evb_; }

    Cpp2ConnContext* getCpp2ConnContext() {
      return connection_->frameHandler_->getCpp2ConnContext();
    }

   private:
    RocketServerConnection* connection_;
  };

  class ReadPausableHandle {
   public:
    explicit ReadPausableHandle(RocketServerConnection* connection);
    ~ReadPausableHandle();

    ReadPausableHandle(ReadPausableHandle&&) noexcept;
    ReadPausableHandle(const ReadPausableHandle&) = delete;
    ReadPausableHandle& operator=(const ReadPausableHandle&) = delete;
    ReadPausableHandle& operator=(ReadPausableHandle&&) = delete;

    ReadResumableHandle pause() &&;
    folly::EventBase& getEventBase() { return connection_->evb_; }

    Cpp2ConnContext* getCpp2ConnContext() {
      return connection_->frameHandler_->getCpp2ConnContext();
    }

   private:
    RocketServerConnection* connection_;
  };

  void setOnWriteQuiescenceCallback(
      folly::Function<void(ReadPausableHandle)> cb) {
    onWriteQuiescence_ = std::move(cb);
  }

  bool incMemoryUsage(uint32_t memSize) {
    if (!ingressMemoryTracker_.increment(memSize)) {
      ingressMemoryTracker_.decrement(memSize);
      socket_->setReadCB(nullptr);
      startDrain(DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT);
      return false;
    } else {
      return true;
    }
  }

  void decMemoryUsage(uint32_t memSize) {
    ingressMemoryTracker_.decrement(memSize);
  }

  int32_t getVersion() const { return frameHandler_->getVersion(); }

  bool areStreamsPaused() const noexcept { return streamsPaused_; }

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

  folly::AsyncSocket* getRawSocket() const { return rawSocket_; }

  using RocketServerConnectionObserverContainer = folly::ObserverContainer<
      RocketServerConnectionObserver,
      RocketServerConnection,
      folly::ObserverContainerBasePolicyDefault<
          RocketServerConnectionObserver::Events /* EventEnum */,
          32 /* BitsetSize (max number of interface events) */>>;
  using Observer = RocketServerConnectionObserverContainer::Observer;
  using ManagedObserver =
      RocketServerConnectionObserverContainer::ManagedObserver;

  /**
   * Adds an observer.
   *
   * If the observer is already added, this is a no-op.
   *
   * @param observer     Observer to add.
   * @return             Whether the observer was added (fails if no list).
   */
  bool addObserver(Observer* observer) {
    if (auto list = getObserverContainer()) {
      list->addObserver(observer);
      return true;
    }
    return false;
  }

  /**
   * Removes an observer.
   *
   * @param observer     Observer to remove.
   * @return             Whether the observer was found and removed.
   */
  bool removeObserver(Observer* observer) {
    if (auto list = getObserverContainer()) {
      return list->removeObserver(observer);
    }
    return false;
  }

  /**
   * Get the number of observers.
   *
   * @return             Number of observers.
   */
  size_t numObservers() const {
    if (auto list = getObserverContainer()) {
      return list->numObservers();
    }
    return 0;
  }

  /**
   * Returns list of attached observers that are of type T.
   *
   * @return             Attached observers of type T.
   */
  template <typename T = Observer>
  std::vector<T*> findObservers() const {
    if (auto list = getObserverContainer()) {
      return list->findObservers<T>();
    }
    return {};
  }

 private:
  /**
   * Returns the RocketServerConnectionObserverContainer or nullptr if not
   * available.
   */
  RocketServerConnectionObserverContainer* getObserverContainer() const {
    if (THRIFT_FLAG(enable_rocket_connection_observers)) {
      return const_cast<RocketServerConnectionObserverContainer*>(
          &observerContainer_);
    }
    return nullptr;
  }

 private:
  void startDrain(std::optional<DrainCompleteCode> drainCompleteCode);

  // Note that attachEventBase()/detachEventBase() are not supported in server
  // code
  folly::EventBase& evb_;
  folly::AsyncTransport::UniquePtr const socket_;
  folly::AsyncSocket* const rawSocket_;
  folly::SocketAddress peerAddress_;

  Parser<RocketServerConnection> parser_{*this};
  std::unique_ptr<RocketServerHandler> frameHandler_;
  bool setupFrameReceived_{false};
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
  struct WriteBatchContext {
    // the counts of completed requests in each inflight write
    size_t requestCompleteCount{0};
    // the counts of valid sendCallbacks in each inflight write
    std::vector<apache::thrift::MessageChannel::SendCallbackPtr> sendCallbacks;
    // the WriteEvent objects associated with each write in the batch
    std::vector<RocketServerConnectionObserver::WriteEvent> writeEvents;
    // the raw byte offset at the beginning and end of the inflight write
    RocketServerConnectionObserver::WriteEventBatchContext writeEventsContext;
  };
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
      std::unique_ptr<RocketSinkClientCallback>>;
  using ClientCallbackPtr =
      std::variant<RocketStreamClientCallback*, RocketSinkClientCallback*>;
  folly::F14FastMap<StreamId, ClientCallbackUniquePtr> streams_;
  const std::chrono::milliseconds streamStarvationTimeout_;
  const size_t egressBufferBackpressureThreshold_;
  const size_t egressBufferRecoverySize_;
  bool streamsPaused_{false};

  using FdsAndOffsets = std::vector<std::pair<folly::SocketFds, size_t>>;

  class WriteBatcher : private folly::EventBase::LoopCallback,
                       private folly::HHWheelTimer::Callback {
   public:
    WriteBatcher(
        RocketServerConnection& connection,
        std::chrono::milliseconds batchingInterval,
        size_t batchingSize,
        size_t batchingByteSize)
        : connection_(connection),
          batchingInterval_(batchingInterval),
          batchingSize_(batchingSize),
          batchingByteSize_(batchingByteSize) {}

    void enqueueWrite(
        std::unique_ptr<folly::IOBuf> data,
        MessageChannel::SendCallbackPtr cb,
        StreamId streamId,
        folly::SocketFds fds) {
      if (cb) {
        cb->sendQueued();
        bufferedWritesContext_.sendCallbacks.push_back(std::move(cb));
      }

      if (auto hasObservers = connection_.numObservers() != 0;
          batchingByteSize_ || hasObservers) {
        auto totalBytesInWrite = data->computeChainDataLength();

        if (hasObservers) {
          bufferedWritesContext_.writeEvents.emplace_back(
              streamId, totalBytesBuffered_, totalBytesInWrite);
        }
        totalBytesBuffered_ += totalBytesInWrite;
      }

      if (!bufferedWrites_) {
        bufferedWrites_ = std::move(data);
        if (batchingInterval_ != std::chrono::milliseconds::zero()) {
          connection_.getEventBase().timer().scheduleTimeout(
              this, batchingInterval_);
        } else {
          connection_.getEventBase().runInLoop(this, true /* thisIteration */);
        }
      } else {
        bufferedWrites_->prependChain(std::move(data));
      }

      // We want the FDs to arrive no later than the last byte of `data`.
      // By attaching the FDs after growing `bufferedWrites_`, it
      // means that `fds` are associated with `[prev offset, offset)`.
      if (!fds.empty()) {
        fdsAndOffsets_.emplace_back(
            std::move(fds),
            // This is costly, but the alternatives are all bad:
            //  - Too fragile: capturing the IOBuf* before `appendToChain`
            //    above would access invalid memory if the chain were coalesced.
            //  - Too messy: `totalBytesBuffered_` as currently implemented
            //    isn't trustworthy -- it's not always set, and even if it
            //    were, it could be wrong because `hasObservers` could've
            //    changed midway through the batch.
            bufferedWrites_->computeChainDataLength());
      }

      ++bufferedWritesCount_;
      if (batchingInterval_ != std::chrono::milliseconds::zero() &&
          (bufferedWritesCount_ == batchingSize_ ||
           (batchingByteSize_ != 0 &&
            totalBytesBuffered_ >= batchingByteSize_ &&
            !earlyFlushRequested_))) {
        earlyFlushRequested_ = true;
        cancelTimeout();
        connection_.getEventBase().runInLoop(this, true /* thisIteration */);
      }
    }

    void enqueueRequestComplete() {
      DCHECK(!empty());
      bufferedWritesContext_.requestCompleteCount++;
    }

    void drain() noexcept {
      if (!bufferedWrites_) {
        return;
      }
      cancelLoopCallback();
      cancelTimeout();
      flushPendingWrites();
    }

    bool empty() const { return !bufferedWrites_; }

   private:
    void runLoopCallback() noexcept final { flushPendingWrites(); }

    void timeoutExpired() noexcept final { flushPendingWrites(); }

    void flushPendingWrites() noexcept {
      bufferedWritesCount_ = 0;
      totalBytesBuffered_ = 0;
      earlyFlushRequested_ = false;
      if (fdsAndOffsets_.empty()) {
        // Fast path: no FDs, write as one batch.
        connection_.flushWrites(
            std::move(bufferedWrites_),
            std::exchange(bufferedWritesContext_, WriteBatchContext{}));
      } else {
        // Slow path: each set of FDs is split into its own batch.
        connection_.flushWritesWithFds(
            std::move(bufferedWrites_),
            std::exchange(bufferedWritesContext_, WriteBatchContext{}),
            std::exchange(fdsAndOffsets_, FdsAndOffsets{}));
      }
    }

    RocketServerConnection& connection_;
    std::chrono::milliseconds batchingInterval_;
    size_t batchingSize_;
    size_t batchingByteSize_;
    // Callback is scheduled iff bufferedWrites_ is not empty.
    std::unique_ptr<folly::IOBuf> bufferedWrites_;
    size_t bufferedWritesCount_{0};
    size_t totalBytesBuffered_{0};
    bool earlyFlushRequested_{false};
    WriteBatchContext bufferedWritesContext_;
    // Offset in `bufferedWrites_` before which these FDs must be sent.
    FdsAndOffsets fdsAndOffsets_;
  };
  WriteBatcher writeBatcher_;
  class SocketDrainer : private folly::HHWheelTimer::Callback {
   public:
    explicit SocketDrainer(RocketServerConnection& connection)
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

    RocketServerConnection& connection_;
    bool drainComplete_{false};
    folly::Executor::KeepAlive<> evbKA_;
    static constexpr std::chrono::seconds kTimeout{1};
  };
  SocketDrainer socketDrainer_;
  size_t activePausedHandlers_{0};
  folly::Function<void(ReadPausableHandle)> onWriteQuiescence_;

  MemoryTracker& ingressMemoryTracker_;
  MemoryTracker& egressMemoryTracker_;

  ~RocketServerConnection();

  void closeIfNeeded();
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

  void handleUntrackedFrame(
      std::unique_ptr<folly::IOBuf> frame,
      StreamId streamId,
      FrameType frameType,
      Flags flags,
      folly::io::Cursor cursor);
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

  void incInflightRequests() { ++inflightRequests_; }

  void decInflightRequests() {
    --inflightRequests_;
    closeIfNeeded();
  }

  void requestComplete() {
    if (!writeBatcher_.empty()) {
      writeBatcher_.enqueueRequestComplete();
      return;
    }
    if (!inflightWritesQueue_.empty()) {
      inflightWritesQueue_.back().requestCompleteCount++;
      return;
    }
    frameHandler_->requestComplete();
  }

  void pauseStreams();
  void resumeStreams();

  friend class RocketServerFrameContext;

  // Container of observers for the RocketServerConnection.
  //
  // This member MUST be last in the list of members to ensure it is destroyed
  // first, before any other members are destroyed. This ensures that observers
  // can inspect any state available through public methods
  // when destruction of the Rocket connection begins.
  RocketServerConnectionObserverContainer observerContainer_;
};

} // namespace rocket
} // namespace thrift
} // namespace apache
