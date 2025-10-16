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

#include <folly/io/async/AsyncTransport.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

namespace apache::thrift::rocket {

template <typename ConnectionT, template <typename> class ConnectionAdapter>
class ConnectionWriterCallback : public folly::AsyncTransport::WriteCallback {
  using Connection = ConnectionAdapter<ConnectionT>;

 public:
  explicit ConnectionWriterCallback(Connection& connection) noexcept
      : connection_(&connection) {}

  // AsyncTransport::WriteCallback implementation
  void writeStarting() noexcept final {
    auto dg = connection_->getDestructorGuard();

    auto& inflightWritesQueue = connection_->getInflightWritesQueue();
    DCHECK(!inflightWritesQueue.empty());
    auto& context = inflightWritesQueue.front();
    DCHECK(!context.writeEventsContext.startRawByteOffset.has_value());

    auto* socket = connection_->getSocket();
    context.writeEventsContext.startRawByteOffset =
        socket->getRawBytesWritten();

    if (auto observerContainer = connection_->getObserverContainer();
        observerContainer && observerContainer->numObservers()) {
      for (const auto& writeEvent : context.writeEvents) {
        observerContainer->invokeInterfaceMethodAllObservers(
            [&](auto observer, auto observed) {
              observer->writeStarting(observed, writeEvent);
            });
      }
    }
  }

  void writeSuccess() noexcept final {
    auto dg = connection_->getDestructorGuard();

    auto& inflightWritesQueue = connection_->getInflightWritesQueue();
    DCHECK(!inflightWritesQueue.empty());
    auto& context = inflightWritesQueue.front();

    for (auto processingCompleteCount = context.requestCompleteCount;
         processingCompleteCount > 0;
         --processingCompleteCount) {
      connection_->getFrameHandler()->requestComplete();
    }

    DCHECK(!context.writeEventsContext.endRawByteOffset.has_value());
    if (context.writeEventsContext.startRawByteOffset.has_value()) {
      auto* socket = connection_->getSocket();
      context.writeEventsContext.endRawByteOffset = std::max(
          context.writeEventsContext.startRawByteOffset.value(),
          socket->getRawBytesWritten() - 1);
    }

    if (auto observerContainer = connection_->getObserverContainer();
        observerContainer && observerContainer->numObservers()) {
      for (const auto& writeEvent : context.writeEvents) {
        observerContainer->invokeInterfaceMethodAllObservers(
            [&](auto observer, auto observed) {
              observer->writeSuccess(
                  observed, writeEvent, context.writeEventsContext);
            });
      }
    }

    for (auto& cb : context.sendCallbacks) {
      cb.release()->messageSent();
    }

    inflightWritesQueue.pop_front();

    if (connection_->hasWriteQuiescenceCallback() &&
        connection_->isWriteBatcherEmpty() && inflightWritesQueue.empty()) {
      connection_->invokeWriteQuiescenceCallback();
      return;
    }

    connection_->closeIfNeeded();
  }

  void writeErr(
      size_t /* bytesWritten */,
      const folly::AsyncSocketException& ex) noexcept final {
    auto dg = connection_->getDestructorGuard();

    auto& inflightWritesQueue = connection_->getInflightWritesQueue();
    DCHECK(!inflightWritesQueue.empty());
    auto& context = inflightWritesQueue.front();

    for (auto processingCompleteCount = context.requestCompleteCount;
         processingCompleteCount > 0;
         --processingCompleteCount) {
      connection_->getFrameHandler()->requestComplete();
    }

    auto ew = folly::make_exception_wrapper<transport::TTransportException>(ex);
    for (auto& cb : context.sendCallbacks) {
      cb.release()->messageSendError(folly::copy(ew));
    }

    inflightWritesQueue.pop_front();
    connection_->close(std::move(ew));
  }

 private:
  Connection* connection_;
};

} // namespace apache::thrift::rocket
