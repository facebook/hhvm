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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <glog/logging.h>

#include <folly/CPortability.h>
#include <folly/ExceptionWrapper.h>
#include <folly/coro/Task.h>
#include <folly/stop_watch.h>
#include <thrift/lib/cpp/StreamEventHandler.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorStorage.h>
#include <thrift/lib/cpp2/util/TypeErasedRef.h>

namespace apache::thrift::detail {

/**
 * Manages per-stream state for all service interceptors.
 *
 * Each stream gets its own context instance, which holds:
 * - A unique stream identifier
 * - Sequence number for payload ordering
 * - Moved request storage (owned locally, survives request destruction)
 *
 * Connection context is intentionally NOT stored here. Streams run on the
 * CPU pool thread and may outlive the connection (which is destroyed on the
 * IO thread). This avoids the TOCTOU race between checking connection
 * liveness and dereferencing the connection context pointer. Interceptors
 * that need connection-level state should capture it in their RequestState
 * during onRequest, which runs while the connection is guaranteed alive.
 *
 * LIFETIME REQUIREMENT: The metricCallback reference must remain valid for the
 * lifetime of this context. Typically, the callback is owned by ThriftServer
 * which must outlive all streams.
 */
class StreamInterceptorContext {
 public:
  StreamInterceptorContext(
      StreamId streamId,
      std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors,
      InterceptorMetricCallback& metricCallback,
      std::string serviceName,
      std::string methodName)
      : streamId_(streamId),
        interceptors_(std::move(interceptors)),
        metricCallback_(metricCallback),
        requestStorage_(interceptors_.size()),
        serviceName_(std::move(serviceName)),
        methodName_(std::move(methodName)) {}

  StreamId getStreamId() const { return streamId_; }

  /**
   * Move request storage from request context into this context.
   * Must be called before the request is destroyed.
   */
  void moveRequestStorage(Cpp2RequestContext* reqCtx) {
    for (std::size_t i = 0; i < requestStorage_.size(); ++i) {
      auto* srcStorage =
          reqCtx->getStorageForServiceInterceptorOnRequestByIndex(i);
      if (srcStorage) {
        requestStorage_[i] = std::move(*srcStorage);
      }
    }
  }

  uint64_t getTotalPayloads() const {
    return sequenceNumber_.load(std::memory_order_seq_cst);
  }

  // ============ Interceptor Invocation Methods ============

  /**
   * Invoke onStreamBegin for all registered interceptors.
   * Called when a stream is established.
   */
  folly::coro::Task<void> invokeOnStreamBegin() {
    folly::stop_watch<std::chrono::microseconds> totalTimer;
    for (std::size_t i = 0; i < interceptors_.size(); ++i) {
      auto streamInfo = ServiceInterceptorBase::StreamInfo{
          .streamId = streamId_,
          .requestStorage = &requestStorage_[i],
          .direction = ServiceInterceptorBase::StreamDirection::ServerStream,
          .serviceName = serviceName_,
          .methodName = methodName_,
      };

      co_await interceptors_[i]->internal_onStreamBegin(
          streamInfo, metricCallback_);
    }
    metricCallback_.onStreamBeginTotalComplete(totalTimer.elapsed());
  }

  /**
   * Invoke onStreamPayload for all registered interceptors.
   * Called for each typed payload BEFORE serialization.
   */
  template <typename T>
  folly::coro::Task<void> invokeOnStreamPayload(const T& payload) {
    // Get sequence number once for this payload - all interceptors see same
    // value. Use seq_cst to ensure proper ordering visibility across threads.
    const auto sequenceNumber =
        sequenceNumber_.fetch_add(1, std::memory_order_seq_cst);

    folly::stop_watch<std::chrono::microseconds> totalTimer;
    for (std::size_t i = 0; i < interceptors_.size(); ++i) {
      auto payloadInfo = ServiceInterceptorBase::StreamPayloadInfo{
          .streamId = streamId_,
          .requestStorage = &requestStorage_[i],
          .payload = util::TypeErasedRef::of<T>(payload),
          .sequenceNumber = sequenceNumber,
      };

      co_await interceptors_[i]->internal_onStreamPayload(
          payloadInfo, metricCallback_);
    }
    metricCallback_.onStreamPayloadTotalComplete(totalTimer.elapsed());
  }

  /**
   * Invoke onStreamEnd for all registered interceptors.
   * Called when stream ends (complete, error, or cancelled).
   * Interceptors are called in REVERSE order (LIFO) to match onResponse
   * pattern.
   */
  folly::coro::Task<void> invokeOnStreamEnd(
      details::STREAM_ENDING_TYPES reason,
      folly::exception_wrapper error = {}) {
    // Call in reverse order (LIFO pattern like onResponse)
    folly::stop_watch<std::chrono::microseconds> totalTimer;
    for (auto i = std::ptrdiff_t(interceptors_.size()) - 1; i >= 0; --i) {
      auto endInfo = ServiceInterceptorBase::StreamEndInfo{
          .streamId = streamId_,
          .requestStorage = &requestStorage_[i],
          .reason = reason,
          .error = error,
          .totalPayloads = sequenceNumber_.load(std::memory_order_seq_cst),
      };

      co_await interceptors_[i]->internal_onStreamEnd(endInfo, metricCallback_);
    }
    metricCallback_.onStreamEndTotalComplete(totalTimer.elapsed());
  }

 private:
  StreamId streamId_;
  std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  InterceptorMetricCallback& metricCallback_;
  std::vector<ServiceInterceptorOnRequestStorage> requestStorage_;
  std::atomic<uint64_t> sequenceNumber_{0};
  std::string serviceName_;
  std::string methodName_;
};

FOLLY_EXPORT inline StreamId generateStreamId() {
  static std::atomic<StreamId> counter{0};
  return ++counter;
}

} // namespace apache::thrift::detail
