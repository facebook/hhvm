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
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

// Forward declarations to avoid circular dependencies
namespace folly {
class exception_wrapper;
}

namespace apache::thrift::rocket {

// Forward declarations for WriteBatcher types
using FdsAndOffsets = std::vector<std::pair<folly::SocketFds, size_t>>;
struct WriteBatchContext;

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
      std::unique_ptr<folly::IOBuf> writes, WriteBatchContext&& context) {
    connection_->flushWrites(std::move(writes), std::move(context));
  }
  void flushWritesWithFds(
      std::unique_ptr<folly::IOBuf> writes,
      WriteBatchContext&& context,
      FdsAndOffsets&& fdsAndOffsets) {
    connection_->flushWritesWithFds(
        std::move(writes), std::move(context), std::move(fdsAndOffsets));
  }

  // Method for OutgoingFrameHandler to send serialized frame data
  void handleSerializedFrame(std::unique_ptr<folly::IOBuf> serializedFrame) {
    connection_->send(std::move(serializedFrame));
  }

 private:
  AdaptedConnectionT* connection_;
};

} // namespace apache::thrift::rocket
