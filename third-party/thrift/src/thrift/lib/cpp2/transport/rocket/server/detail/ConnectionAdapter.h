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

#include <utility>
#include <vector>
#include <folly/IntrusiveList.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnectionObserver.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/WriteBatchTypes.h>

// Forward declarations to avoid circular dependencies
namespace folly {
class exception_wrapper;
}

namespace apache::thrift::rocket {

/**
 * Adaptor between RocketServerConnection
 * and the refactored code so they are decoupled. This classes uses
 * a template to prevent cyclic dependencies.
 */
template <typename AdaptedConnectionT>
class ConnectionAdapter {
 public:
  using AdaptedConnection = AdaptedConnectionT;

  explicit ConnectionAdapter(AdaptedConnectionT& connection)
      : connection_(&connection) {}
  ConnectionAdapter(const ConnectionAdapter&) = delete;
  ConnectionAdapter& operator=(const ConnectionAdapter&) = delete;
  ConnectionAdapter(ConnectionAdapter&&) noexcept = default;
  ConnectionAdapter& operator=(ConnectionAdapter&&) noexcept = default;

  template <typename RequestFrame>
  void emplacePartialFrame(StreamId streamId, RequestFrame&& frame) {
    connection_->partialRequestFrames_.emplace(
        streamId, std::forward<RequestFrame>(frame));
  }

  auto getDestructorGuard() { return connection_->getDestructorGuard(); }

  void close(folly::exception_wrapper ew) { connection_->close(std::move(ew)); }

  AdaptedConnectionT* getWrappedConnection() { return connection_; }

  // Accessor methods for WriteCallback functionality
  auto& getInflightWritesQueue() { return connection_->inflightWritesQueue_; }
  auto* getSocket() { return connection_->socket_.get(); }
  auto* getFrameHandler() { return connection_->frameHandler_.get(); }
  auto* getObserverContainer() { return connection_->getObserverContainer(); }
  bool hasWriteQuiescenceCallback() {
    return static_cast<bool>(connection_->onWriteQuiescence_);
  }
  bool isWriteBatcherEmpty() { return connection_->writeBatcher_.empty(); }
  void invokeWriteQuiescenceCallback() {
    if (connection_->onWriteQuiescence_) {
      connection_->onWriteQuiescence_(
          typename AdaptedConnectionT::ReadPausableHandle(connection_));
    }
  }
  void closeIfNeeded() { connection_->closeIfNeeded(); }

  // Accessor methods for BufferCallback functionality
  auto* getRawSocket() { return connection_->rawSocket_; }
  size_t getEgressBufferSize() { return connection_->egressBufferSize_; }
  void setEgressBufferSize(size_t size) {
    connection_->egressBufferSize_ = size;
  }
  auto& getEgressMemoryTracker() { return connection_->egressMemoryTracker_; }
  const auto& getPeerAddress() { return connection_->peerAddress_; }
  size_t getEgressBufferBackpressureThreshold() {
    return connection_->egressBufferBackpressureThreshold_;
  }
  size_t getEgressBufferRecoverySize() {
    return connection_->egressBufferRecoverySize_;
  }
  bool areStreamsPaused() { return connection_->streamsPaused_; }
  void pauseStreams() { connection_->pauseStreams(); }
  void resumeStreams() { connection_->resumeStreams(); }

  // Accessor methods for WriteBatcher functionality
  size_t numObservers() { return connection_->numObservers(); }
  folly::EventBase& getEventBase() { return connection_->getEventBase(); }
  void flushWrites(
      std::unique_ptr<folly::IOBuf> writes,
      apache::thrift::rocket::WriteBatchContext&& context) {
    connection_->flushWrites(std::move(writes), std::move(context));
  }
  void flushWritesWithFds(
      std::unique_ptr<folly::IOBuf> writes,
      apache::thrift::rocket::WriteBatchContext&& context,
      apache::thrift::rocket::FdsAndOffsets&& fdsAndOffsets) {
    connection_->flushWritesWithFds(
        std::move(writes), std::move(context), std::move(fdsAndOffsets));
  }

  // Method for OutgoingFrameHandler to send serialized frame data
  void handleSerializedFrame(std::unique_ptr<folly::IOBuf> serializedFrame) {
    connection_->send(std::move(serializedFrame));
  }

  /**
   * Add a pending write to this connection's buffer list.
   */
  void addPendingWrite(
      std::unique_ptr<folly::IOBuf> data,
      apache::thrift::MessageChannel::SendCallbackPtr sendCallback = nullptr,
      StreamId streamId = StreamId{0});

  /**
   * Flush all pending writes for this connection.
   * Called by OutgoingFrameHandler during drain().
   */
  void flushPendingWrites();

  /**
   * Check if this connection has pending writes buffered.
   */
  bool hasPendingWrites() const noexcept { return hasPendingWrites_; }

  /**
   * Reset the pending writes state (used internally after flush).
   */
  void resetPendingWritesState() noexcept {
    hasPendingWrites_ = false;
    totalBytesBuffered_ = 0;
    guard_.reset();
  }

  // Safe intrusive list hook for pending connections list - automatically
  // removes itself from lists when the ConnectionAdapter is destroyed
  folly::SafeIntrusiveListHook listHook_;

 private:
  AdaptedConnectionT* connection_;
  bool hasPendingWrites_{false};
  folly::IOBufQueue pendingWrites_;
  std::vector<apache::thrift::MessageChannel::SendCallbackPtr>
      pendingSendCallbacks_;
  size_t totalBytesBuffered_{0};
  std::vector<
      apache::thrift::rocket::RocketServerConnectionObserver::WriteEvent>
      writeEvents_;

  // Prevent event loop destruction while writes are in flight
  std::optional<folly::DelayedDestruction::DestructorGuard> guard_;
};

template <typename AdaptedConnectionT>
void ConnectionAdapter<AdaptedConnectionT>::addPendingWrite(
    std::unique_ptr<folly::IOBuf> data,
    apache::thrift::MessageChannel::SendCallbackPtr sendCallback,
    StreamId streamId) {
  DCHECK(connection_);
  DCHECK(data);

  guard_.emplace(connection_->getDestructorGuard());

  size_t bytesInWrite = 0;
  bool needsObserverTracking = connection_->numObservers() > 0;

  if (data && needsObserverTracking) {
    bytesInWrite = data->computeChainDataLength();
  }

  if (data) {
    pendingWrites_.append(std::move(data));
  }

  // Add callback to batch
  if (sendCallback) {
    pendingSendCallbacks_.push_back(std::move(sendCallback));
  }

  // Track write event for observers only if needed
  if (needsObserverTracking && bytesInWrite > 0) {
    writeEvents_.emplace_back(streamId, totalBytesBuffered_, bytesInWrite);
    totalBytesBuffered_ += bytesInWrite;
  }

  // Mark as having pending writes
  hasPendingWrites_ = true;
}

template <typename AdaptedConnectionT>
void ConnectionAdapter<AdaptedConnectionT>::flushPendingWrites() {
  if (!hasPendingWrites_) {
    return;
  }

  // Call sendQueued() on all callbacks before sending
  for (auto& cb : pendingSendCallbacks_) {
    if (cb) {
      cb->sendQueued();
    }
  }

  // Build WriteBatchContext for the flushWrites call
  apache::thrift::rocket::WriteBatchContext context;
  context.sendCallbacks = std::move(pendingSendCallbacks_);
  context.writeEvents = std::move(writeEvents_);

  // Build writeEventsContext for connection observers
  if (!context.writeEvents.empty()) {
    context.writeEventsContext.startRawByteOffset = 0;
    context.writeEventsContext.endRawByteOffset = totalBytesBuffered_;
  }

  // Get the batched IOBuf chain and flush to underlying connection
  auto batchedWrites = pendingWrites_.move();
  if (batchedWrites) {
    flushWrites(std::move(batchedWrites), std::move(context));
  }

  // Reset state for next batch
  resetPendingWritesState();
}

} // namespace apache::thrift::rocket
