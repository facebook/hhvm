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

#include <folly/Format.h>
#include <folly/GLog.h>
#include <folly/io/async/AsyncTransport.h>

namespace apache::thrift::rocket {

template <typename ConnectionT, template <typename> class ConnectionAdapter>
class ConnectionBufferCallback : public folly::AsyncTransport::BufferCallback {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit ConnectionBufferCallback(Connection& connection) noexcept
      : connection_(&connection) {}

  // AsyncTransport::BufferCallback implementation
  void onEgressBuffered() final {
    auto* rawSocket = connection_->getRawSocket();

    const auto buffered = rawSocket->getAllocatedBytesBuffered();
    const auto oldBuffered = connection_->getEgressBufferSize();
    connection_->setEgressBufferSize(buffered);

    // Track egress memory consumption, drop connection if necessary
    if (buffered < oldBuffered) {
      const auto delta = oldBuffered - buffered;
      connection_->getEgressMemoryTracker().decrement(delta);
      DVLOG(10) << fmt::format("buffered: {} (-{}) B", buffered, delta);
    } else {
      const auto delta = buffered - oldBuffered;
      const auto exceeds =
          !connection_->getEgressMemoryTracker().increment(delta);
      DVLOG(10) << fmt::format("buffered: {} (+{}) B", buffered, delta);

      if (exceeds && rawSocket->good()) {
        auto dg = connection_->getDestructorGuard();
        FB_LOG_EVERY_MS(ERROR, 1000) << fmt::format(
            "Dropping connection for ({}): exceeded egress memory limit ({}). The config value of min increment size: ({})",
            connection_->getPeerAddress().describe(),
            connection_->getEgressMemoryTracker().getMemLimit(),
            connection_->getEgressMemoryTracker().getMinIncrementSize());

        // TODO: Add connection event logging if needed

        rawSocket->closeNow(); // triggers writeErr() events now
        return;
      }
    }

    // Pause streams if buffer size reached backpressure threshold
    const auto egressBufferBackpressureThreshold =
        connection_->getEgressBufferBackpressureThreshold();
    if (!egressBufferBackpressureThreshold) {
      return;
    } else if (
        buffered > egressBufferBackpressureThreshold &&
        !connection_->areStreamsPaused()) {
      connection_->pauseStreams();
    } else if (
        connection_->areStreamsPaused() &&
        buffered < connection_->getEgressBufferRecoverySize()) {
      connection_->resumeStreams();
    }
  }
  void onEgressBufferCleared() final {
    const auto egressBufferSize = connection_->getEgressBufferSize();

    if (egressBufferSize) {
      connection_->getEgressMemoryTracker().decrement(egressBufferSize);
      DVLOG(10) << "buffered: 0 (-" << egressBufferSize << ") B";
      connection_->setEgressBufferSize(0);
    }

    if (UNLIKELY(connection_->areStreamsPaused())) {
      connection_->resumeStreams();
    }
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
