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
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/core/FrameUtil.h>
#include <thrift/lib/cpp2/transport/rocket/core/RingBuffer.h>
#include <thrift/lib/cpp2/transport/rocket/core/StreamUtil.h>
#include <thrift/lib/cpp2/transport/rocket/framing/FrameType.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/WriteBatchTypes.h>

namespace apache::thrift::rocket {

namespace detail::out_going_frame_handler {

using namespace apache::thrift::rocket::stream_util;
using namespace apache::thrift::rocket::frame_util;

template <typename ConnectionT, template <typename> class ConnectionAdapter>
struct SerializationEvent {
  using Connection = ConnectionAdapter<ConnectionT>;
  SerializationEvent() = default;

  template <typename Frame>
  SerializationEvent(
      Frame&& frame,
      Connection* connection,
      folly::DelayedDestruction::DestructorGuard&& guard,
      apache::thrift::MessageChannel::SendCallbackPtr sendCallback = nullptr)
      : container(std::forward<Frame>(frame)),
        connection(connection),
        guard(std::move(guard)),
        sendCallback(std::move(sendCallback)),
        streamId(frame.streamId()) {}

  SerializationEvent(SerializationEvent&& other) = delete;
  SerializationEvent& operator=(SerializationEvent&& other) = delete;
  SerializationEvent(const SerializationEvent& other) = delete;
  SerializationEvent& operator=(const SerializationEvent& other) = delete;
  ~SerializationEvent() noexcept = default;

  FrameContainer container;
  Connection* connection;
  std::optional<folly::DelayedDestruction::DestructorGuard> guard;
  apache::thrift::MessageChannel::SendCallbackPtr sendCallback;
  StreamId streamId;
};

struct WriteBatch {
  WriteBatch() = default;
  WriteBatch(
      std::unique_ptr<folly::IOBuf>&& chain,
      folly::DelayedDestruction::DestructorGuard&& guard)
      : chain(std::move(chain)), guard(std::move(guard)) {}

  WriteBatch(WriteBatch&& other) noexcept = default;
  WriteBatch& operator=(WriteBatch&& other) noexcept = default;
  WriteBatch(const WriteBatch& other) = delete;
  WriteBatch& operator=(const WriteBatch& other) = delete;
  ~WriteBatch() noexcept = default;

  std::unique_ptr<folly::IOBuf> chain;
  std::optional<folly::DelayedDestruction::DestructorGuard> guard;
  // SendCallbacks from batched frames for WriteBatchContext
  std::vector<apache::thrift::MessageChannel::SendCallbackPtr> sendCallbacks;
  // WriteEvents for connection observers
  std::vector<
      apache::thrift::rocket::RocketServerConnectionObserver::WriteEvent>
      writeEvents;
  // Track total bytes for WriteEvent construction
  size_t totalBytesBuffered{0};
};

} // namespace detail::out_going_frame_handler

/**
 * OutgoingFrameHandler receives Frames in batches in the handle method. It then
 * schedules a a job on the event loop to process the frames up to the batch
 * size. If it hits the current batch size it'll process them immediately rather
 * than waiting, and rescheduling if there's frames left. This class is meant to
 * only run on the event base threads and is *NOT* thread-safe. It's shared
 * by all connections on an event loop to encourage batching.
 *
 * Default batch size is 64, but specified in log size - so 1 << 6.
 */
template <typename ConnectionT, template <typename> class ConnectionAdapter>
class OutgoingFrameHandler : public folly::EventBase::LoopCallback {
  using Connection = ConnectionAdapter<ConnectionT>;
  using SerializationEvent = detail::out_going_frame_handler::
      SerializationEvent<ConnectionT, ConnectionAdapter>;
  using WriteBatch = detail::out_going_frame_handler::WriteBatch;

 public:
  OutgoingFrameHandler(
      folly::EventBase& evb, size_t batchLogSize = 4 /* 1<<4 == 16 */)
      : batchSize_(1 << batchLogSize),
        evb_(evb),
        queue_(batchLogSize),
        batches_(batchLogSize) {}

  template <typename T>
  FOLLY_ALWAYS_INLINE void handle(T&& t, Connection& connection) {
    evb_.dcheckIsInEventBaseThread();
    bool ret = queue_.emplace_back(
        std::move(t),
        &connection,
        folly::DelayedDestruction::DestructorGuard(
            connection.getWrappedConnection()));
    FOLLY_SAFE_DCHECK(ret, "queue is full");
    tryForceDrainOrSchedule();
  }

  template <typename T>
  FOLLY_ALWAYS_INLINE void handle(
      T&& t,
      Connection& connection,
      apache::thrift::MessageChannel::SendCallbackPtr sendCallback) {
    evb_.dcheckIsInEventBaseThread();
    // Note: ConnectionAdapter doesn't have getConnectionMetrics(), skip for now
    // connection.getConnectionMetrics().incrNumOutgoingFrames();
    bool ret = queue_.emplace_back(
        std::move(t),
        &connection,
        folly::DelayedDestruction::DestructorGuard(
            connection.getWrappedConnection()),
        std::move(sendCallback));
    FOLLY_SAFE_DCHECK(ret, "queue is full");
    tryForceDrainOrSchedule();
  }

  void runLoopCallback() noexcept { drain<false>(); }

  FOLLY_ALWAYS_INLINE size_t pending() const noexcept { return queue_.size(); }

 private:
  const size_t batchSize_;
  folly::EventBase& evb_;
  RingBuffer<SerializationEvent> queue_;
  folly::F14NodeMap<Connection*, WriteBatch> batches_;

  FOLLY_ERASE void dcheckSerializedEvent(SerializationEvent& event) {
    if constexpr (folly::kIsDebug) {
      FOLLY_SAFE_DCHECK(
          event.guard.has_value(),
          "missing connection state destruction guard");
      FOLLY_SAFE_DCHECK(
          event.connection != nullptr, "connection state is null");
      FOLLY_SAFE_DCHECK(!event.container.empty(), "missing frame");
    }
  }

  void processWriteBatches() {
    for (auto it = batches_.begin(); it != batches_.end(); ++it) {
      Connection* connection = it->first;
      WriteBatch& batch = it->second;

      // Call sendQueued() on all sendCallbacks before sending
      for (auto& cb : batch.sendCallbacks) {
        if (cb) {
          cb->sendQueued();
        }
      }

      // Build WriteBatchContext for the direct flushWrites call
      apache::thrift::rocket::WriteBatchContext context;
      context.sendCallbacks = std::move(batch.sendCallbacks);
      context.writeEvents = std::move(batch.writeEvents);

      // Build writeEventsContext for connection observers
      if (!context.writeEvents.empty()) {
        context.writeEventsContext.startRawByteOffset = 0;
        context.writeEventsContext.endRawByteOffset = batch.totalBytesBuffered;
      }

      // Call flushWrites directly instead of handleSerializedFrame
      connection->flushWrites(std::move(batch.chain), std::move(context));
    }
    batches_.clear();
  }

  template <bool Forced>
  void drain() {
    size_t processed = queue_.consume(
        [this](SerializationEvent& event) {
          dcheckSerializedEvent(event);
          Connection* connection = event.connection;
          auto serialized = event.container.serialize();
          auto it = batches_.find(connection);
          if (it == batches_.end()) {
            WriteBatch batch(
                std::move(serialized),
                folly::DelayedDestructionBase::DestructorGuard(
                    connection->getWrappedConnection()));

            // Add sendCallback if present
            if (event.sendCallback) {
              batch.sendCallbacks.push_back(std::move(event.sendCallback));
            }

            // Build WriteEvent for connection observers if needed
            if (connection->numObservers() > 0) {
              auto totalBytesInWrite = batch.chain->computeChainDataLength();
              batch.writeEvents.emplace_back(
                  event.streamId, // Use the streamId from the event
                  batch.totalBytesBuffered,
                  totalBytesInWrite);
              batch.totalBytesBuffered += totalBytesInWrite;
            }

            batches_.try_emplace(connection, std::move(batch));
          } else {
            // Add sendCallback if present
            if (event.sendCallback) {
              it->second.sendCallbacks.push_back(std::move(event.sendCallback));
            }

            // Build WriteEvent for connection observers if needed
            if (connection->numObservers() > 0) {
              auto totalBytesInWrite = serialized->computeChainDataLength();
              it->second.writeEvents.emplace_back(
                  event.streamId, // Use the streamId from the event
                  it->second.totalBytesBuffered,
                  totalBytesInWrite);
              it->second.totalBytesBuffered += totalBytesInWrite;
            }

            it->second.chain->appendToChain(std::move(serialized));
          }
        },
        batchSize_);

    if (processed > 0) {
      processWriteBatches();
    }

    auto p = pending();
    if (Forced && p == 0 && isLoopCallbackScheduled()) {
      cancelLoopCallback();
    } else if (p > 0) {
      tryScheduleDrain();
    }
  }

  FOLLY_ALWAYS_INLINE void tryScheduleDrain() {
    if (!isLoopCallbackScheduled()) {
      evb_.runInLoop(this, true);
    }
  }

  FOLLY_ALWAYS_INLINE void tryForceDrainOrSchedule() {
    evb_.dcheckIsInEventBaseThread();
    if (pending() >= batchSize_) {
      drain<true>();
    } else {
      tryScheduleDrain();
    }
  }
};

} // namespace apache::thrift::rocket
